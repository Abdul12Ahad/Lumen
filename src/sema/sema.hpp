#pragma once
#include "../ast/ast.hpp"
#include "../ast/ast_visitor.hpp"
#include "symbol_table.hpp"

namespace lumen
{
    class Sema : public ASTVisitor
    {
        public:
            bool check(Program& program);

        private:
            SymbolTable symbols_;
            bool ok_ = true;

            TypeKind currentReturnType_ = TypeKind::Unknown;

            void checkFunction(FunctionDecl& fn);

            void visit(VarDeclStmt& stmt) override;
            void visit(ExprStmt& stmt) override;
            void visit(BlockStmt& stmt) override;
            void visit(IfStmt& stmt) override;
            void visit(WhileStmt& stmt) override;
            void visit(ForStmt& stmt) override;
            void visit(ReturnStmt& stmt) override;

            void visit(NumberExpr& expr) override;
            void visit(VarExpr& expr) override;
            void visit(AssignExpr& expr) override;
            void visit(BinaryExpr& expr) override;
            void visit(UnaryExpr& expr) override;

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