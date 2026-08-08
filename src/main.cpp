#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "lexer/lexer.hpp"
#include "lexer/token.hpp"

static std::string readFile(const std::string& path)
{
    std::ifstream file(path);
    if(!file)
    {
        throw std::runtime_error("Could not open file : " + path);
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        std::cerr << "usage : lumen <source-file>\n";
        return 1;
    }

    try
    {
        std::string source = readFile(argv[1]);
        lumen::Lexer lexer(source);

        auto tokens = lexer.tokenize();

        for(const auto& token : tokens)
        {
            std::cout
                << static_cast<int>(token.kind)
                << " | "
                << token.lexeme
                << " | Line"
                << token.line
                << ", Column "
                << token.column
                << '\n';
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}