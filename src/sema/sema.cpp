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

    for (auto& stmt : fn.body->statements)
    {
        checkStmt(stmt.get(),fn.returnType);
    }

    symbols_.popScope();
}


void Sema::checkStmt(Stmt* stmt,TypeKind returnType)
{
    if (!stmt)
        return;

    if (auto* decl =
            dynamic_cast<VarDeclStmt*>(stmt))
    {
        if (decl->init)
        {
            TypeKind initType =
                checkExpr(decl->init.get());

            if (!isAssignable(
                    decl->type,
                    initType))
            {
                error(
                    decl->line,
                    "cannot initialize variable '" +
                    decl->name +
                    "' of type " +
                    typeKindToString(decl->type) +
                    " with expression of type " +
                    typeKindToString(initType)
                );
            }
        }

        try
        {
            symbols_.declare(
                Symbol{
                    decl->name,
                    decl->type
                }
            );
        }
        catch (const std::exception& e)
        {
            error(
                decl->line,
                e.what()
            );
        }

        return;
    }

    if (auto* block =
            dynamic_cast<BlockStmt*>(stmt))
    {
        symbols_.pushScope();

        for (auto& s :
             block->statements)
        {
            checkStmt(s.get(), returnType);
        }

        symbols_.popScope();

        return;
    }

    if (auto* ifs =
            dynamic_cast<IfStmt*>(stmt))
    {
        TypeKind condType =
            checkExpr(ifs->cond.get());

        if (condType != TypeKind::Bool)
        {
            error(
                ifs->line,
                "if condition must be bool"
            );
        }

        checkStmt(
            ifs->thenBranch.get(),
            returnType
        );

        if (ifs->elseBranch)
        {
            checkStmt(
                ifs->elseBranch.get(),
                returnType
            );
        }

        return;
    }

    if (auto* whiles =
            dynamic_cast<WhileStmt*>(stmt))
    {
        TypeKind condType =
            checkExpr(whiles->cond.get());

        if (condType != TypeKind::Bool)
        {
            error(
                whiles->line,
                "while condition must be bool"
            );
        }

        checkStmt(
            whiles->body.get(),
            returnType
        );

        return;
    }

    if (auto* forStmt =
            dynamic_cast<ForStmt*>(stmt))
    {
        symbols_.pushScope();

        if (forStmt->init)
        {
            checkStmt(
                forStmt->init.get(),
                returnType
            );
        }

        if (forStmt->cond)
        {
            TypeKind condType =
                checkExpr(
                    forStmt->cond.get()
                );

            if (condType != TypeKind::Bool)
            {
                error(
                    forStmt->line,
                    "for condition must be bool"
                );
            }
        }

        if (forStmt->increment)
        {
            checkExpr(
                forStmt->increment.get()
            );
        }

        checkStmt(
            forStmt->body.get(),
            returnType
        );

        symbols_.popScope();

        return;
    }

    if (auto* ret =
            dynamic_cast<ReturnStmt*>(stmt))
    {
        if (!ret->value)
        {
            error(
                ret->line,
                "return statement requires a value"
            );

            return;
        }

        TypeKind actualType =
            checkExpr(
                ret->value.get()
            );

        if (!isAssignable(
                returnType,
                actualType))
        {
            error(
                ret->line,
                "cannot return expression of type " +
                std::string(typeKindToString(actualType)) +
                " from function returning " +
                typeKindToString(returnType)
            );
        }

        return;
    }

    if (auto* es =
            dynamic_cast<ExprStmt*>(stmt))
    {
        checkExpr(
            es->expr.get()
        );

        return;
    }
}


TypeKind Sema::checkExpr(Expr* expr)
{
    if (!expr)
        return TypeKind::Unknown;

    if (auto* number =
            dynamic_cast<NumberExpr*>(expr))
    {
        if (number->isInteger)
        {
            number->type = TypeKind::Int;
        }
        else
        {
            number->type = TypeKind::Double;
        }

        return number->type;
    }

    if (auto* var =
            dynamic_cast<VarExpr*>(expr))
    {
        const Symbol* symbol =
            symbols_.resolve(var->name);

        if (!symbol)
        {
            error(
                var->line,
                "use of undeclared identifier '" +
                var->name +
                "'"
            );

            var->type = TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        var->type = symbol->type;

        return var->type;
    }

    if (auto* assign =
            dynamic_cast<AssignExpr*>(expr))
    {
        const Symbol* symbol =
            symbols_.resolve(
                assign->name
            );

        TypeKind valueType =
            checkExpr(
                assign->value.get()
            );

        if (!symbol)
        {
            error(
                assign->line,
                "assignment to undeclared identifier '" +
                assign->name +
                "'"
            );

            assign->type =
                TypeKind::Unknown;

            return TypeKind::Unknown;
        }

        if (!isAssignable(
                symbol->type,
                valueType))
        {
            error(
                assign->line,
                std::string("cannot assign expression of type ") +
                typeKindToString(valueType) +
                " to variable '" +
                assign->name +
                "' of type " +
                typeKindToString(symbol->type)
            );
        }

        assign->type = symbol->type;

        return assign->type;
    }

    if (auto* binary =
            dynamic_cast<BinaryExpr*>(expr))
    {
        return checkBinary(binary);
    }

    if (auto* unary =
            dynamic_cast<UnaryExpr*>(expr))
    {
        return checkUnary(unary);
    }
    return TypeKind::Unknown;
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
        checkExpr(
            expr->operand.get()
        );

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