#pragma once

#include "ast.hpp"
#include <iostream>

namespace lumen
{

    class ASTPrinter
    {
        public:
            void print(const Program& program);

        private:
            void printFunction(const FunctionDecl& function, int indent);
            void printStmt(const Stmt* stmt, int indent);
            void printExpr(const Expr* expr, int indent);

            void printIndent(int indent);
            const char* tokenKindToString(TokenKind kind);
    };
}