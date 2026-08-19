#pragma once

namespace lumen
{
    struct NumberExpr;
    struct VarExpr;
    struct AssignExpr;
    struct BinaryExpr;
    struct UnaryExpr;

    struct VarDeclStmt;
    struct ExprStmt;
    struct BlockStmt;
    struct IfStmt;
    struct WhileStmt;
    struct ForStmt;
    struct ReturnStmt;

    class ASTVisitor
    {
        public:
            virtual ~ASTVisitor() = default;

            // creating the visitor functions for expression and statements
            /*
                flow of program becomes
                Expr -> accept(sema) -> [Actual object] -> (returns the Expr type) -> sema.visit(ExprType&)
            
            */ 
            
            virtual void visit(NumberExpr& expr) = 0;
            virtual void visit(VarExpr& expr) = 0;
            virtual void visit(AssignExpr& expr) = 0;
            virtual void visit(BinaryExpr& expr) = 0;
            virtual void visit(UnaryExpr& expr) = 0;

            virtual void visit(VarDeclStmt& stmt) = 0;
            virtual void visit(ExprStmt& stmt) = 0;
            virtual void visit(BlockStmt& stmt) = 0;
            virtual void visit(IfStmt& stmt) = 0;
            virtual void visit(WhileStmt& stmt) = 0;
            virtual void visit(ForStmt& stmt) = 0;
            virtual void visit(ReturnStmt& stmt) = 0;
    };
}