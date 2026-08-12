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
            void checkStmt(Stmt* stmt);
            void checkExpr(Expr* expr);

            void error(int line,const std::string& msg);
    };
}