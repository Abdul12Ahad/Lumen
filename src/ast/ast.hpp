#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../lexer/token.hpp"
#include "../type/type.hpp"
#include "ast_visitor.hpp"

namespace lumen
{

    struct Expr
    {
        virtual ~Expr() = default;

        int line = 0;
        TypeKind type = TypeKind::Unknown;

        virtual void accept(ASTVisitor& visitor) = 0;
    };

    using ExprPtr = std::unique_ptr<Expr>;

    struct NumberExpr : Expr
    {
        double value = 0.0;
        bool isInteger = true;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct VarExpr : Expr
    {
        std::string name;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct AssignExpr : Expr
    {
        std::string name;
        ExprPtr value;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct BinaryExpr : Expr
    {
        TokenKind op;
        ExprPtr lhs;
        ExprPtr rhs;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct UnaryExpr : Expr
    {
        TokenKind op;
        ExprPtr operand;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct Stmt
    {
        virtual ~Stmt() = default;

        int line = 0;

        virtual void accept(ASTVisitor& visitor) = 0;
    };

    using StmtPtr = std::unique_ptr<Stmt>;

    struct VarDeclStmt : Stmt
    {
        TypeKind type;
        std::string name;
        ExprPtr init; 

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct ExprStmt : Stmt
    {
        ExprPtr expr;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct BlockStmt : Stmt
    {
        std::vector<StmtPtr> statements;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct IfStmt : Stmt
    {
        ExprPtr cond;
        StmtPtr thenBranch;
        StmtPtr elseBranch;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct WhileStmt : Stmt
    {
        ExprPtr cond;
        StmtPtr body;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct ForStmt : Stmt
    {
        StmtPtr init;      
        ExprPtr cond;       
        ExprPtr increment;  
        StmtPtr body;

        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
    };

    struct ReturnStmt : Stmt
    {
        ExprPtr value;
        
        void accept(ASTVisitor& visitor) override
        {
            visitor.visit(*this);
        }
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