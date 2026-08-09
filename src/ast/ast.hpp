#pragma once
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "../lexer/token.hpp"

namespace lumen
{
    struct Expr
    {
        virtual ~Expr() = default;
        int line = 0;
    };
    using ExprPtr = std::unique_ptr<Expr>;

    struct NumberExpr : Expr { int value; };
    struct varExpr : Expr { std::string name; };
    struct AssignExpr : Expr { std::string name; ExprPtr value; };
    struct BinaryExpr : Expr {TokenKind op; ExprPtr lhs; ExprPtr rhs;};
    struct UnaryExpr : Expr { TokenKind op; ExprPtr operand;};

    struct Stmt
    {
        virtual ~Stmt() = default;
        int line = 0;
    };
    using StmtPtr = std::unique_ptr<Stmt>;

    struct VarDeclStmt : Stmt { std::string name; ExprPtr init;};
    struct ExprStmt : Stmt { ExprPtr expr;};
    struct BlockStmt : Stmt {std::vector<StmtPtr> statements;};
    struct IfStmt : Stmt {ExprPtr cond; StmtPtr thenBranch; StmtPtr elseBranch;};
    struct WhileStmt : Stmt{ExprPtr cond; StmtPtr body;};
    struct ForStmt : Stmt{StmtPtr init; ExprPtr cond; ExprPtr increment; StmtPtr body;};
    struct ReturnStmt : Stmt {ExprPtr value;};

    struct FunctionDecl
    {
        std::string name;
        std::unique_ptr<BlockStmt> body;

        FunctionDecl(std::string name, std::unique_ptr<BlockStmt> body)
        : name(std::move(name)), body(std::move(body)) {}
    };

    struct Program
    {
        std::vector<FunctionDecl> functions;
    };

}