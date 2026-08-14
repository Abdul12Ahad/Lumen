#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include "../type/type.hpp"

namespace lumen
{
    struct Symbol
    {
        std::string name;
        TypeKind type;
    };

    class SymbolTable
    {
        public:
            void pushScope()
            {
                scopes_.emplace_back();
            }
            void popScope()
            {
                if (!scopes_.empty())
                    scopes_.pop_back();
            }
            void clear()
            {
                scopes_.clear();
            }
            void declare(const Symbol& sym)
            {
                if (scopes_.empty())
                    throw std::runtime_error("no active scope");

                auto& currentScope = scopes_.back();

                if (currentScope.count(sym.name))
                {
                    throw std::runtime_error(
                        "redeclaration of '" + sym.name +
                        "' in same scope"
                    );
                }

                currentScope[sym.name] = sym;
            }

            const Symbol* resolve(const std::string& name) const
            {
                for(auto it = scopes_.rbegin();it!=scopes_.rend();++it)
                {
                    auto found = it->find(name);
                    if(found != it->end())
                        return &found->second;
                }
                return nullptr;
            }

        private:
            std::vector<std::unordered_map<std::string, Symbol>> scopes_;
    };
}
