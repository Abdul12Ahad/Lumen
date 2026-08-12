#include "sema.hpp"
#include <cstdio>

namespace lumen
{
    void Sema::error(int line, const std::string& msg)
    {
        fprintf(stderr,"semantic error at line %d: %s\n", line, msg.c_str());
        ok_ = false;
    }

    bool Sema::check(Program& program)
    {
        for(auto& fn : program.functions)
            checkFunction(fn);
        return ok_;
    }

    void Sema::checkFunction(FunctionDecl& fn)
    {
        symbols_.pushScope();
        for(auto& stmt : fn.body->statements)
            checkStmt(stmt.get());
        symbols_.popScope();
    }

    void Sema::checkStmt(Stmt* stmt)
    {
        if(auto* decl = dynamic_cast<VarDeclStmt*>(stmt))
        {
            if(decl->init)
                checkExpr(decl->init.get());
            try
            {
                symbols_.declare(Symbol{decl->name});
            }
            catch(const std::exception& e)
            {
                error(decl->line,e.what());
            }
        }
        else if(auto* block = dynamic_cast<BlockStmt*>(stmt))
        {
            symbols_.pushScope();
            for(auto& s : block->statements)
                checkStmt(s.get());
            symbols_.popScope();
        }
        else if(auto* ifs = dynamic_cast<IfStmt*>(stmt))
        {
            checkExpr(ifs->cond.get());
            checkStmt(ifs->thenBranch.get());
        }
        else if(auto* whiles = dynamic_cast<WhileStmt*>(stmt))
        {
            checkExpr(whiles->cond.get());
            checkStmt(whiles->body.get());
        }
        else if(auto* ret = dynamic_cast<ReturnStmt*>(stmt))
        {
            if(ret->value)
                checkExpr(ret->value.get());
        }
        else if(auto* es = dynamic_cast<ExprStmt*>(stmt))
        {
            checkExpr(es->expr.get());
        }
    }

    void Sema::checkExpr(Expr* expr) 
    {
        if (auto* var = dynamic_cast<VarExpr*>(expr)) {
            if (!symbols_.resolve(var->name)) {
                error(var->line, "use of undeclared identifier '" + var->name + "'");
            }
        } else if (auto* assign = dynamic_cast<AssignExpr*>(expr)) {
            if (!symbols_.resolve(assign->name)) {
                error(assign->line, "assignment to undeclared identifier '" + assign->name + "'");
            }
            checkExpr(assign->value.get());
        } else if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
            checkExpr(bin->lhs.get());
            checkExpr(bin->rhs.get());
        } else if (auto* un = dynamic_cast<UnaryExpr*>(expr)) {
            checkExpr(un->operand.get());
        }
    }
}