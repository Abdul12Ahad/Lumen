#include "parser.hpp"
#include <stdexcept>
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
            "parser error at line " + std::to_string(peek().line) + ": " + msg
        );
    }

    void Parser::synchronize()
    {
        advance();
        while(!atEnd())
        {
            if(previous().kind == TokenKind::Semicolon) return;
            if(peek().kind == TokenKind::RBrace) return;
            advance();
        }
    }

    Program Parser::parseProgram()
    {
        Program program;
        while(!atEnd())
        {
            program.functions.push_back(parseFunction());
        }
        return program;
    }

    FunctionDecl Parser::parseFunction()
    {
        expect(TokenKind::KwInt, "expected return type 'int'");
        Token name = expect(TokenKind::Identifier, "expected function name");
        expect(TokenKind::LParen, "expected '('");
        expect(TokenKind::RParen, "expected ')'");
        auto body = parseBlock();
        return FunctionDecl(std::string(name.lexeme), std::move(body));
    }

    std::unique_ptr<BlockStmt> Parser::parseBlock()
    {
        expect(TokenKind::LBrace, "expected '{'");
        auto block = std::make_unique<BlockStmt>();
        while(!check(TokenKind::RBrace) && !atEnd())
        {
            try
            {
                {
                    block->statements.push_back(parseStatment());
                }
            }
            catch(const std::runtime_error& e)
            {
                fprintf(stderr, "%s\n", e.what());
                synchronize();
            }
        }
        expect(TokenKind::RBrace, "expected '}'");
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
        advance();
        Token name = expect(TokenKind::Identifier,"expected variable name");
        auto decl = std::make_unique<VarDeclStmt>();
        decl->name = std::string(name.lexeme);
        if(match(TokenKind::Equals))
            decl->init = parseExpr();
        expect (TokenKind::Semicolon, "expected ';' after variable declaration");
        return decl;
    }

    StmtPtr Parser::parseIf()
    {
        advance();
        expect (TokenKind::LParen, "expected '(' after 'if'");
        auto stmt = std::make_unique<IfStmt>();
        stmt->cond = parseExpr();
        expect(TokenKind::RParen, "expected ')'");
        stmt->thenBranch = parseStatment();
        if(match(TokenKind::KwElse))
            stmt->elseBranch = parseStatment();
            return stmt;
    }

    StmtPtr Parser::parseWhile()
    {
        advance();
        expect(TokenKind::LParen, "expected '(' after 'while'");
        auto stmt = std::make_unique<WhileStmt>();
        stmt->cond = parseExpr();
        expect(TokenKind::RParen, "expected ')'");
        stmt->body = parseStatment();
        return stmt;
    }

    StmtPtr Parser::parseFor()
    {
        advance();
        expect(TokenKind::LParen, "expected '(' after 'for'");
        auto stmt = std::make_unique<ForStmt>();
        if (!check(TokenKind::Semicolon)) {
            stmt->init = parseStatment();
        } else {
            advance();
        }
        if (!check(TokenKind::Semicolon)) {
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
        advance();
        auto stmt = std::make_unique<ReturnStmt>();
        if (!check(TokenKind::Semicolon)) stmt->value = parseExpr();
        expect(TokenKind::Semicolon, "expected ';' after return");
        return stmt;
    }

    StmtPtr Parser::parseExprStmt()
    {
        auto expr = parseExpr();
        expect(TokenKind::Semicolon, "expected ';' after expression");
        auto stmt = std::make_unique<ExprStmt>();
        stmt->expr = std::move(expr);
        return stmt;
    }

    ExprPtr Parser::parseExpr()
    {
        return parseAssignment();
    }

    ExprPtr Parser::parseAssignment()
    {
        ExprPtr expr = parseEquality();
        if (match(TokenKind::Equals)) 
        {
            if (auto* var = dynamic_cast<varExpr*>(expr.get())) {
                auto assign = std::make_unique<AssignExpr>();
                assign->name = var->name;
                assign->value = parseAssignment();
                return assign;
            }
            throw std::runtime_error("invalid assignment target");
        }
        return expr;
    }

    ExprPtr Parser::parseEquality()
    {
        ExprPtr expr = parseComparison();
        while (check(TokenKind::EqualsEquals)) {
        TokenKind op = advance().kind;
        auto bin = std::make_unique<BinaryExpr>();
        bin->op = op; 
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
            TokenKind op = advance().kind;
            auto bin = std::make_unique<BinaryExpr>();
            bin->op = op; 
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
            TokenKind op = advance().kind;
            auto bin = std::make_unique<BinaryExpr>();
            bin->op = op; 
            bin->lhs = std::move(expr); bin->rhs = parseFactor();
            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseFactor()
    {
        ExprPtr expr = parseUnary();
        while (check(TokenKind::Star) || check(TokenKind::Slash)) 
        {
            TokenKind op = advance().kind;
            auto bin = std::make_unique<BinaryExpr>();
            bin->op = op; 
            bin->lhs = std::move(expr); bin->rhs = parseUnary();
            expr = std::move(bin);
        }
        return expr;
    }

    ExprPtr Parser::parseUnary()
    {
        if (check(TokenKind::Minus)) 
        {
            TokenKind op = advance().kind;
            auto un = std::make_unique<UnaryExpr>();
            un->op = op; 
            un->operand = parseUnary();
            return un;
        }
        return parsePrimary();
    }
    ExprPtr Parser::parsePrimary()
    {
        if (check(TokenKind::Number)) 
        {
            auto tok = advance();
            auto n = std::make_unique<NumberExpr>();
            n->value = std::stoi(std::string(tok.lexeme));
            return n;
        }
        if (check(TokenKind::Identifier)) 
        {
            auto tok = advance();
            auto v = std::make_unique<varExpr>();
            v->name = std::string(tok.lexeme);
            return v;
        }
        if (match(TokenKind::LParen)) 
        {
            auto expr = parseExpr();
            expect(TokenKind::RParen, "expected ')'");
            return expr;
        }
        throw std::runtime_error(
        "parse error at line " + std::to_string(peek().line) + ": expected expression");
    }
}