#include "sema.hpp"

#include <cstdio>
#include "../type/type.hpp"

namespace lumen
{

void Sema::error(
    int line,
    const std::string& msg)
{
    std::fprintf(
        stderr,
        "semantic error at line %d: %s\n",
        line,
        msg.c_str()
    );

    ok_ = false;
}


bool Sema::check(Program& program)
{
    ok_ = true;
    symbols_.clear();

    for (auto& fn : program.functions)
    {
        checkFunction(fn);
    }

    return ok_;
}


void Sema::checkFunction(FunctionDecl& fn)
{
    symbols_.pushScope();
    currentReturnType_ = fn.returnType;

    for (auto& stmt : fn.body->statements)
    {
        if (stmt)
        {
            stmt->accept(*this);
        }
    }

    symbols_.popScope();
}

void Sema::visit(VarDeclStmt& stmt)
{
    if (stmt.init)
    {
        TypeKind initType =
            checkExpr(stmt.init.get());

        if (!isAssignable(
                stmt.type,
                initType))
        {
            error(
                stmt.line,
                "cannot initialize variable '" +
                stmt.name +
                "' of type " +
                typeKindToString(stmt.type) +
                " with expression of type " +
                typeKindToString(initType)
            );
        }
    }

    try
    {
        symbols_.declare(
            Symbol{
                stmt.name,
                stmt.type
            }
        );
    }
    catch (const std::exception& e)
    {
        error(
            stmt.line,
            e.what()
        );
    }
}


void Sema::visit(ExprStmt& stmt)
{
    checkExpr(stmt.expr.get());
}


void Sema::visit(BlockStmt& stmt)
{
    symbols_.pushScope();

    for (auto& statement : stmt.statements)
    {
        if (statement)
        {
            statement->accept(*this);
        }
    }

    symbols_.popScope();
}


void Sema::visit(IfStmt& stmt)
{
    TypeKind condType =
        checkExpr(stmt.cond.get());

    if (condType != TypeKind::Bool)
    {
        error(
            stmt.line,
            "if condition must be bool"
        );
    }

    if (stmt.thenBranch)
    {
        stmt.thenBranch->accept(*this);
    }

    if (stmt.elseBranch)
    {
        stmt.elseBranch->accept(*this);
    }
}


void Sema::visit(WhileStmt& stmt)
{
    TypeKind condType =
        checkExpr(stmt.cond.get());

    if (condType != TypeKind::Bool)
    {
        error(
            stmt.line,
            "while condition must be bool"
        );
    }

    if (stmt.body)
    {
        stmt.body->accept(*this);
    }
}


void Sema::visit(ForStmt& stmt)
{
    symbols_.pushScope();

    if (stmt.init)
    {
        stmt.init->accept(*this);
    }

    if (stmt.cond)
    {
        TypeKind condType =
            checkExpr(stmt.cond.get());

        if (condType != TypeKind::Bool)
        {
            error(
                stmt.line,
                "for condition must be bool"
            );
        }
    }

    if (stmt.increment)
    {
        checkExpr(stmt.increment.get());
    }

    if (stmt.body)
    {
        stmt.body->accept(*this);
    }

    symbols_.popScope();
}


void Sema::visit(ReturnStmt& stmt)
{
    if (!stmt.value)
    {
        error(
            stmt.line,
            "return statement requires a value"
        );

        return;
    }

    TypeKind actualType =
        checkExpr(stmt.value.get());

    if (!isAssignable(
            currentReturnType_,
            actualType))
    {
        error(
            stmt.line,
            "cannot return expression of type " +
            std::string(typeKindToString(actualType)) +
            " from function returning " +
            typeKindToString(currentReturnType_)
        );
    }
}

void Sema::visit(NumberExpr& expr)
{
    if (expr.isInteger)
    {
        expr.type = TypeKind::Int;
    }
    else
    {
        expr.type = TypeKind::Double;
    }
}


void Sema::visit(VarExpr& expr)
{
    const Symbol* symbol =
        symbols_.resolve(expr.name);

    if (!symbol)
    {
        error(
            expr.line,
            "use of undeclared identifier '" +
            expr.name +
            "'"
        );

        expr.type = TypeKind::Unknown;

        return;
    }

    expr.type = symbol->type;
}


void Sema::visit(AssignExpr& expr)
{
    const Symbol* symbol =
        symbols_.resolve(expr.name);

    TypeKind valueType =
        checkExpr(expr.value.get());

    if (!symbol)
    {
        error(
            expr.line,
            "assignment to undeclared identifier '" +
            expr.name +
            "'"
        );

        expr.type = TypeKind::Unknown;

        return;
    }

    if (!isAssignable(
            symbol->type,
            valueType))
    {
        error(
            expr.line,
            std::string("cannot assign expression of type ") +
            typeKindToString(valueType) +
            " to variable '" +
            expr.name +
            "' of type " +
            typeKindToString(symbol->type)
        );
    }

    expr.type = symbol->type;
}


void Sema::visit(BinaryExpr& expr)
{
    checkBinary(&expr);
}


void Sema::visit(UnaryExpr& expr)
{
    checkUnary(&expr);
}

TypeKind Sema::checkExpr(Expr* expr)
{
    if (!expr)
    {
        return TypeKind::Unknown;
    }

    expr->accept(*this);

    return expr->type;
}

TypeKind Sema::checkBinary(
    BinaryExpr* expr)
{
    TypeKind lhsType =
        checkExpr(expr->lhs.get());

    TypeKind rhsType =
        checkExpr(expr->rhs.get());

    if (expr->op == TokenKind::Plus ||
        expr->op == TokenKind::Minus ||
        expr->op == TokenKind::Star ||
        expr->op == TokenKind::Slash ||
        expr->op == TokenKind::Mod)
    {
        if (!isNumeric(lhsType) ||
            !isNumeric(rhsType))
        {
            error(
                expr->line,
                "arithmetic operator requires numeric operands"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        if (expr->op == TokenKind::Mod &&
            (lhsType != TypeKind::Int ||
             rhsType != TypeKind::Int))
        {
            error(
                expr->line,
                "modulo operator requires int operands"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        expr->type =
            commonNumericType(
                lhsType,
                rhsType
            );

        return expr->type;
    }

    if (expr->op == TokenKind::Less ||
        expr->op == TokenKind::LessEqual ||
        expr->op == TokenKind::Greater ||
        expr->op == TokenKind::GreaterEqual)
    {
        if (!isNumeric(lhsType) ||
            !isNumeric(rhsType))
        {
            error(
                expr->line,
                "comparison requires numeric operands"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        expr->type = TypeKind::Bool;

        return TypeKind::Bool;
    }

    if (expr->op == TokenKind::EqualsEquals ||
        expr->op == TokenKind::BangEquals)
    {
        if (lhsType == TypeKind::Unknown ||
            rhsType == TypeKind::Unknown)
        {
            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        if (isNumeric(lhsType) &&
            isNumeric(rhsType))
        {
            expr->type =
                TypeKind::Bool;

            return TypeKind::Bool;
        }

        if (lhsType != rhsType)
        {
            error(
                expr->line,
                "equality operands must have compatible types"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        expr->type = TypeKind::Bool;

        return TypeKind::Bool;
    }

    if (expr->op == TokenKind::AndAnd ||
        expr->op == TokenKind::OrOr)
    {
        if (lhsType != TypeKind::Bool ||
            rhsType != TypeKind::Bool)
        {
            error(
                expr->line,
                "logical operator requires bool operands"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        expr->type = TypeKind::Bool;

        return TypeKind::Bool;
    }

    expr->type = TypeKind::Unknown;

    return TypeKind::Unknown;
}

TypeKind Sema::checkUnary(
    UnaryExpr* expr)
{
    TypeKind operandType =
        checkExpr(expr->operand.get());

    if (expr->op == TokenKind::Minus)
    {
        if (!isNumeric(operandType))
        {
            error(
                expr->line,
                "unary '-' requires a numeric operand"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        expr->type = operandType;

        return operandType;
    }

    if (expr->op == TokenKind::Bang)
    {
        if (operandType != TypeKind::Bool)
        {
            error(
                expr->line,
                "logical '!' requires a bool operand"
            );

            expr->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        expr->type = TypeKind::Bool;

        return TypeKind::Bool;
    }

    expr->type = TypeKind::Unknown;

    return TypeKind::Unknown;
}

bool Sema::isAssignable(
    TypeKind target,
    TypeKind source)
{
    if (target == TypeKind::Unknown ||
        source == TypeKind::Unknown)
    {
        return false;
    }

    if (target == source)
        return true;

    if (isNumeric(target) &&
        isNumeric(source))
    {
        return typeRank(source) <=
               typeRank(target);
    }

    return false;
}

}