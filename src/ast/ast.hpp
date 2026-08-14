#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../lexer/token.hpp"
#include "../type/type.hpp"

namespace lumen
{

    struct Expr
    {
        virtual ~Expr() = default;

        int line = 0;
        TypeKind type = TypeKind::Unknown;
    };

    using ExprPtr = std::unique_ptr<Expr>;

    struct NumberExpr : Expr
    {
        double value = 0.0;
        bool isInteger = true;
    };

    struct VarExpr : Expr
    {
        std::string name;
    };

    struct AssignExpr : Expr
    {
        std::string name;
        ExprPtr value;
    };

    struct BinaryExpr : Expr
    {
        TokenKind op;
        ExprPtr lhs;
        ExprPtr rhs;
    };

    struct UnaryExpr : Expr
    {
        TokenKind op;
        ExprPtr operand;
    };

    struct Stmt
    {
        virtual ~Stmt() = default;

        int line = 0;
    };

    using StmtPtr = std::unique_ptr<Stmt>;

    struct VarDeclStmt : Stmt
    {
        TypeKind type;
        std::string name;
        ExprPtr init; 
    };

    struct ExprStmt : Stmt
    {
        ExprPtr expr;
    };

    struct BlockStmt : Stmt
    {
        std::vector<StmtPtr> statements;
    };

    struct IfStmt : Stmt
    {
        ExprPtr cond;
        StmtPtr thenBranch;
        StmtPtr elseBranch;
    };

    struct WhileStmt : Stmt
    {
        ExprPtr cond;
        StmtPtr body;
    };

    struct ForStmt : Stmt
    {
        StmtPtr init;      
        ExprPtr cond;       
        ExprPtr increment;  
        StmtPtr body;
    };

    struct ReturnStmt : Stmt
    {
        ExprPtr value;      
    };

    struct FunctionDecl
    {
        TypeKind returnType;
        std::string name;
        std::unique_ptr<BlockStmt> body;

        FunctionDecl(
            TypeKind returnType,
            std::string name,
            std::unique_ptr<BlockStmt> body
        )
            : returnType(returnType),
            name(std::move(name)),
            body(std::move(body))
        {
        }
    };

    struct Program
    {
        std::vector<FunctionDecl> functions;
    };

}