#pragma once
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

namespace lumen 
{
    class Parser
    {
        public:
            explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {} Program parseProgram();

        private:
            std::vector<Token> tokens_;
            size_t pos_ = 0;
            bool hadError_ = false;

            const Token& peek() const 
            {
                return tokens_[pos_];
            }
            const Token& previous() const 
            {
                return tokens_[pos_-1];
            }
            bool check(TokenKind k) const
            {
                return peek().kind == k;
            }
            bool atEnd() const
            {
                return check(TokenKind::EndOfFile);
            }
            bool hadError() const
            {
                return hadError_;
            }
            const Token& advance()
            {
                if(!atEnd())
                    pos_++;
                return previous();
            }
            bool match(TokenKind k)
            {
                if (check(k))
                {
                    advance();
                    return true;
                }
                return false;
            }
            const Token& expect
            (
                TokenKind k, const std::string& msg
            );
            void synchronize ();

            TypeKind parseType();
            FunctionDecl parseFunction();
            StmtPtr parseStatment();
            StmtPtr parseVarDecl();
            std::unique_ptr<BlockStmt> parseBlock();
            StmtPtr parseIf();
            StmtPtr parseWhile();
            StmtPtr parseFor();
            StmtPtr parseReturn();
            StmtPtr parseExprStmt();

            ExprPtr parseExpr();
            ExprPtr parseAssignment();
            ExprPtr parseOr();
            ExprPtr parseAnd();
            ExprPtr parseEquality();
            ExprPtr parseComparison();
            ExprPtr parseTerm();
            ExprPtr parseFactor();
            ExprPtr parseUnary();
            ExprPtr parsePrimary();
    };
}