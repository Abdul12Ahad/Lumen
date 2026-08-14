#pragma once
#include "../ast/ast.hpp"
#include "symbol_table.hpp"

namespace lumen
{
    class Sema
    {
        public:
            bool check(Program& program);

        private:
            SymbolTable symbols_;
            bool ok_ = true;

            void checkFunction(FunctionDecl& fn);
            void checkStmt(Stmt* stmt,
                TypeKind returnType);
            TypeKind checkExpr(Expr* expr);
            TypeKind checkBinary(
            BinaryExpr* expr);
            TypeKind checkUnary(
                UnaryExpr* expr);
            bool isAssignable(
                TypeKind target,
                TypeKind source);

            void error(int line,const std::string& msg);
    };
}