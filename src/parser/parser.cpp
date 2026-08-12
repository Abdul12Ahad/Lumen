#include "parser.hpp"
#include <stdexcept>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

namespace lumen 
{

    const Token& Parser::expect(TokenKind k, const std::string& msg)
    {
        if(check(k))
            return advance();

        throw std::runtime_error
        (
            "parser error at line "
            + std::to_string(peek().line)
            + ", column "
            + std::to_string(peek().column)
            + ": "
            + msg
        );
    }

    void Parser::synchronize()
    {
        if(atEnd())
            return;

        advance();
        while(!atEnd())
        {
            if(previous().kind == TokenKind::Semicolon) return;
            
            if(peek().kind == TokenKind::RBrace) return;

            if(check(TokenKind::KwInt) ||
            check(TokenKind::KwIf) ||
            check(TokenKind::KwWhile) ||
            check(TokenKind::KwFor) ||
            check(TokenKind::KwReturn) ||
            check(TokenKind::LBrace))
                return;

            advance();
        }
    }

    Program Parser::parseProgram()
    {
        Program program;
        while(!atEnd())
        {
            try
            {
                program.functions.push_back(parseFunction());
            }
            catch (const std::runtime_error& e)
            {
                std::fprintf(stderr, "%s\n", e.what());
                synchronize();
            }
        }
        return program;
    }

    FunctionDecl Parser::parseFunction()
    {
        Token returnType =
        expect
        (
            TokenKind::KwInt,
            "expected return type 'int'"
        );

        Token name =
            expect
            (
                TokenKind::Identifier,
                "expected function name"
            );

        expect
        (
            TokenKind::LParen,
            "expected '('"
        );

        expect
        (
            TokenKind::RParen,
            "expected ')'"
        );

        auto body = parseBlock();

        return FunctionDecl
        (
            std::string(name.lexeme),
            std::move(body)
        );
    }

    std::unique_ptr<BlockStmt> Parser::parseBlock()
    {
        Token openBrace =
        expect
        (
            TokenKind::LBrace,
            "expected '{'"
        );
            auto block = std::make_unique<BlockStmt>();

        block->line = openBrace.line;

        while (
            !check(TokenKind::RBrace)
            && !atEnd()
        )
        {
            try
            {
                block->statements.push_back(
                    parseStatment()
                );
            }
            catch (const std::runtime_error& e)
            {
                std::fprintf(stderr, "%s\n", e.what());

                synchronize();
            }
        }

        expect(
            TokenKind::RBrace,
            "expected '}'"
        );

        return block;
    }

    StmtPtr Parser::parseStatment()
    {
        if(check(TokenKind::KwInt))
        return parseVarDecl();

        if(check(TokenKind::KwIf))
            return parseIf();

        if(check(TokenKind::KwWhile))
            return parseWhile();

        if(check(TokenKind::KwFor))
            return parseFor();

        if(check(TokenKind::KwReturn))
            return parseReturn();

        if(check(TokenKind::LBrace))
            return parseBlock();

        return parseExprStmt();
    }

    StmtPtr Parser::parseVarDecl()
    {
        Token intToken = advance();
        Token name = expect(TokenKind::Identifier,"expected variable name");
        auto decl = std::make_unique<VarDeclStmt>();
        decl->line = intToken.line;
        decl->name = std::string(name.lexeme);
        if(match(TokenKind::Equals))
            decl->init = parseExpr();
        expect 
        (
            TokenKind::Semicolon, "expected ';' after variable declaration"
        );
        return decl;
    }

    StmtPtr Parser::parseIf()
    {
        Token ifToken = advance();
        expect (TokenKind::LParen, "expected '(' after 'if'");
        auto stmt = std::make_unique<IfStmt>();
        stmt->line = ifToken.line;
        stmt->cond = parseExpr();
        expect(TokenKind::RParen, "expected ')'");
        stmt->thenBranch = parseStatment();
        if(match(TokenKind::KwElse))
            stmt->elseBranch = parseStatment();
            return stmt;
    }

    StmtPtr Parser::parseWhile()
    {
        Token whileToken = advance();
        expect(TokenKind::LParen, "expected '(' after 'while'");
        auto stmt = std::make_unique<WhileStmt>();
        stmt->line = whileToken.line;
        stmt->cond = parseExpr();
        expect(TokenKind::RParen, "expected ')'");
        stmt->body = parseStatment();
        return stmt;
    }

    StmtPtr Parser::parseFor()
    {
        Token forToken = advance();
        expect(TokenKind::LParen, "expected '(' after 'for'");
        auto stmt = std::make_unique<ForStmt>();
        stmt->line = forToken.line;
        if (check(TokenKind::KwInt))
        {
            stmt->init = parseVarDecl();
        }
        else if (!check(TokenKind::Semicolon)) {
            stmt->init = parseStatment();
        } else 
        {
            advance();
        }
        if (!check(TokenKind::Semicolon)) 
        {
            stmt->cond = parseExpr();
        }
        expect(TokenKind::Semicolon, "expected ';' after for condition");
        if (!check(TokenKind::RParen)) {
            stmt->increment = parseExpr();
        }
        expect(TokenKind::RParen, "expected ')' after for clauses");
        stmt->body = parseStatment();
        return stmt;
    }

    StmtPtr Parser::parseReturn()
    {
        Token returnToken = advance();
        auto stmt = std::make_unique<ReturnStmt>();
        stmt->line = returnToken.line;
        if (!check(TokenKind::Semicolon)) stmt->value = parseExpr();
        expect(TokenKind::Semicolon, "expected ';' after return");
        return stmt;
    }

    StmtPtr Parser::parseExprStmt()
    {
        auto expr = parseExpr();
        expect(TokenKind::Semicolon, "expected ';' after expression");
        auto stmt = std::make_unique<ExprStmt>();
        stmt->line = expr->line;
        stmt->expr = std::move(expr);
        return stmt;
    }

    ExprPtr Parser::parseExpr()
    {
        return parseAssignment();
    }

    ExprPtr Parser::parseAssignment()
    {
        ExprPtr expr = parseOr();
        if (match(TokenKind::Equals))
        {
            auto* var =
                dynamic_cast<VarExpr*>(expr.get());

            if (var == nullptr)
            {
                throw std::runtime_error(
                    "parser error at line "
                    + std::to_string(peek().line)
                    + ", column "
                    + std::to_string(peek().column)
                    + ": invalid assignment target"
                );
            }
            auto assign =
                std::make_unique<AssignExpr>();
            assign->line = var->line;
            assign->name = var->name;
            assign->value = parseAssignment();

            return assign;
        }
        return expr;
    }

    ExprPtr Parser::parseOr()
    {
        ExprPtr expr = parseAnd();

        while(check(TokenKind::OrOr))
        {
            Token opToken = advance();

            auto bin = std::make_unique<BinaryExpr>();

            bin->line = opToken.line;
            bin->op = opToken.kind;
            bin->lhs = std::move(expr);
            bin->rhs = parseAnd();

            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseAnd()
    {
        ExprPtr expr = parseEquality();

        while(check(TokenKind::AndAnd))
        {
            Token opToken = advance();

            auto bin = std::make_unique<BinaryExpr>();

            bin->line = opToken.line;
            bin->op = opToken.kind;
            bin->lhs = std::move(expr);
            bin->rhs = parseEquality();

            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseEquality()
    {
        ExprPtr expr = parseComparison();
        while (check(TokenKind::EqualsEquals) ||
    check(TokenKind::BangEquals)) 
    {
        Token opToken = advance();
        auto bin = std::make_unique<BinaryExpr>();
        bin->line = opToken.line;
        bin->op = opToken.kind; 
        bin->lhs = std::move(expr); 
        bin->rhs = parseComparison();
        expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseComparison()
    {
        ExprPtr expr = parseTerm();
        while (check(TokenKind::Less) || check(TokenKind::LessEqual) ||
        check(TokenKind::Greater) || check(TokenKind::GreaterEqual)) 
        {
            Token opToken  = advance();
            auto bin = std::make_unique<BinaryExpr>();
            bin->line = opToken.line;
            bin->op = opToken.kind; 
            bin->lhs = std::move(expr); bin->rhs = parseTerm();
            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseTerm()
    {
        ExprPtr expr = parseFactor();
        while (check(TokenKind::Plus) || check(TokenKind::Minus)) 
        {
            Token opToken = advance();
            auto bin = std::make_unique<BinaryExpr>();
            bin->line = opToken.line;
            bin->op = opToken.kind; 
            bin->lhs = std::move(expr); bin->rhs = parseFactor();
            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseFactor()
    {
        ExprPtr expr = parseUnary();
        while (check(TokenKind::Star) || check(TokenKind::Slash) || check(TokenKind::Mod)) 
        {
            Token opToken = advance();
            auto bin = std::make_unique<BinaryExpr>();
            bin->line = opToken.line;
            bin->op = opToken.kind; 
            bin->lhs = std::move(expr); bin->rhs = parseUnary();
            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseUnary()
    {
        if (check(TokenKind::Minus)) 
        {
            Token opToken = advance();
            auto un = std::make_unique<UnaryExpr>();
            un->line = opToken.line;
            un->op = opToken.kind; 
            un->operand = parseUnary();
            return un;
        }
        return parsePrimary();
    }
    
    ExprPtr Parser::parsePrimary()
    {
        if (check(TokenKind::Number)) 
        {
            Token tok = advance();
            auto n = std::make_unique<NumberExpr>();
            n->line = tok.line;
            n->value = std::stoi(std::string(tok.lexeme));
            return n;
        }
        if (check(TokenKind::Identifier)) 
        {
            Token tok = advance();
            auto v = std::make_unique<VarExpr>();
            v->line = tok.line;
            v->name = std::string(tok.lexeme);
            return v;
        }
        if (match(TokenKind::LParen)) 
        {
            auto expr = parseExpr();
            expect(TokenKind::RParen, "expected ')'");
            return expr;
        }
        throw std::runtime_error
        (
            "parse error at line " + std::to_string(peek().line) + ", column "
            + std::to_string(peek().column)
            + ": expected expression"
        );
    }
}