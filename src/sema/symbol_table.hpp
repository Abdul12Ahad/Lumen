#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace lumen
{
    struct Symbol
    {
        std::string name;
        std::string type = "int";
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
                scopes_.pop_back();
            }

            void declare(const Symbol& sym)
            {
                auto &innermost = scopes_.back();
                if(innermost.count(sym.name))
                {
                    throw std::runtime_error("redeclaration of '" + sym.name + "' in same scope");
                }
                innermost[sym.name] = sym;
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
