#include "ast_printer.hpp"

namespace lumen
{

void ASTPrinter::printIndent(int indent)
{
    for (int i = 0; i < indent; i++)
        std::cout << "    ";
}

const char* ASTPrinter::tokenKindToString(TokenKind kind)
{
    switch (kind)
    {
        case TokenKind::Plus:
            return "+";

        case TokenKind::Minus:
            return "-";

        case TokenKind::Star:
            return "*";

        case TokenKind::Slash:
            return "/";

        case TokenKind::Equals:
            return "=";

        case TokenKind::EqualsEquals:
            return "==";

        case TokenKind::Less:
            return "<";

        case TokenKind::LessEqual:
            return "<=";

        case TokenKind::Greater:
            return ">";

        case TokenKind::GreaterEqual:
            return ">=";

        default:
            return "?";
    }
}


void ASTPrinter::print(const Program& program)
{
    std::cout << "Program\n";

    for (const auto& function : program.functions)
    {
        printFunction(function, 1);
    }
}


void ASTPrinter::printFunction(
    const FunctionDecl& function,
    int indent)
{
    printIndent(indent);
    std::cout << "Function: " << function.name << "\n";

    printStmt(function.body.get(), indent + 1);
}

void ASTPrinter::printExpr(const Expr* expr, int indent)
{
    if (!expr)
        return;


    if (auto* number = dynamic_cast<const NumberExpr*>(expr))
    {
        printIndent(indent);
        std::cout << "Number: " << number->value << "\n";
    }


    else if (auto* variable = dynamic_cast<const VarExpr*>(expr))
    {
        printIndent(indent);
        std::cout << "Variable: "
                  << variable->name
                  << "\n";
    }


    else if (auto* assignment =
                 dynamic_cast<const AssignExpr*>(expr))
    {
        printIndent(indent);
        std::cout << "Assign: "
                  << assignment->name
                  << "\n";

        printExpr(
            assignment->value.get(),
            indent + 1
        );
    }


    else if (auto* binary =
                 dynamic_cast<const BinaryExpr*>(expr))
    {
        printIndent(indent);
        std::cout << "Binary: "
                  << tokenKindToString(binary->op)
                  << "\n";

        printExpr(
            binary->lhs.get(),
            indent + 1
        );

        printExpr(
            binary->rhs.get(),
            indent + 1
        );
    }

    else if (auto* unary =
                 dynamic_cast<const UnaryExpr*>(expr))
    {
        printIndent(indent);
        std::cout << "Unary: "
                  << tokenKindToString(unary->op)
                  << "\n";

        printExpr(
            unary->operand.get(),
            indent + 1
        );
    }
}

void ASTPrinter::printStmt(
    const Stmt* stmt,
    int indent)
{
    if (!stmt)
        return;


    if (auto* block =
            dynamic_cast<const BlockStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "Block\n";

        for (const auto& statement :
             block->statements)
        {
            printStmt(
                statement.get(),
                indent + 1
            );
        }
    }

    else if (auto* declaration =
                 dynamic_cast<const VarDeclStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "VarDecl: "
                  << declaration->name
                  << "\n";

        if (declaration->init)
        {
            printExpr(
                declaration->init.get(),
                indent + 1
            );
        }
    }

    else if (auto* expression =
                 dynamic_cast<const ExprStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "ExprStmt\n";

        printExpr(
            expression->expr.get(),
            indent + 1
        );
    }

    else if (auto* ifStmt =
                 dynamic_cast<const IfStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "If\n";

        printIndent(indent + 1);
        std::cout << "Condition\n";

        printExpr(
            ifStmt->cond.get(),
            indent + 2
        );

        printIndent(indent + 1);
        std::cout << "Then\n";

        printStmt(
            ifStmt->thenBranch.get(),
            indent + 2
        );

        if (ifStmt->elseBranch)
        {
            printIndent(indent + 1);
            std::cout << "Else\n";

            printStmt(
                ifStmt->elseBranch.get(),
                indent + 2
            );
        }
    }

    else if (auto* whileStmt =
                 dynamic_cast<const WhileStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "While\n";

        printIndent(indent + 1);
        std::cout << "Condition\n";

        printExpr(
            whileStmt->cond.get(),
            indent + 2
        );

        printIndent(indent + 1);
        std::cout << "Body\n";

        printStmt(
            whileStmt->body.get(),
            indent + 2
        );
    }

    else if (auto* forStmt =
                 dynamic_cast<const ForStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "For\n";

        printIndent(indent + 1);
        std::cout << "Init\n";

        if (forStmt->init)
        {
            printStmt(
                forStmt->init.get(),
                indent + 2
            );
        }

        if (forStmt->cond)
        {
            printIndent(indent + 1);
            std::cout << "Condition\n";

            printExpr(
                forStmt->cond.get(),
                indent + 2
            );
        }

        if (forStmt->increment)
        {
            printIndent(indent + 1);
            std::cout << "Increment\n";

            printExpr(
                forStmt->increment.get(),
                indent + 2
            );
        }

        printIndent(indent + 1);
        std::cout << "Body\n";

        printStmt(
            forStmt->body.get(),
            indent + 2
        );
    }

    else if (auto* returnStmt =
                 dynamic_cast<const ReturnStmt*>(stmt))
    {
        printIndent(indent);
        std::cout << "Return\n";

        if (returnStmt->value)
        {
            printExpr(
                returnStmt->value.get(),
                indent + 1
            );
        }
    }
}
}