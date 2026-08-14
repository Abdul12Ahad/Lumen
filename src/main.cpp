#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/parser.hpp"
#include "ast/ast_printer.hpp"
#include "sema/sema.hpp"

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
        lumen::DiagnosticEngine diagnostics;
        lumen::Lexer lexer(source, diagnostics);
        // so above we are actaully giving the lexer a reference to the diagnostEngine like as it belongs to the main function so lexer is sharing it with the main function so now the flow is like:Lexer detects an error → reports it to DiagnosticEngine → main.cpp reads the diagnostics and prints them. so now the tokens are printed out first and then later the errors as this provides a great arch for handling things

        auto tokens = lexer.tokenize();

        std::cout << " TOKENS \n";
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

        
        lumen::Parser parser(std::move(tokens));

        auto program = parser.parseProgram();

        std::cout <<"\n  PARSING  \n";

        std::cout << "\nParsing successful\n\n";

        lumen::Sema sema;

        bool semanticSuccess = sema.check(program);

        if (!semanticSuccess)
        {
            std::cerr << "\nSemantic analysis failed\n";
            return 1;
        }

        std::cout << "\nSemantic analysis successful\n";

        std::cout << "\n AST \n";

        lumen::ASTPrinter printer;
        printer.print(program);

        if (!diagnostics.diagnostics().empty())
        {
            std::cerr << "\n DIAGNOSTICS \n";

            for (const auto& diagnostic :
                 diagnostics.diagnostics())
            {
                std::cerr
                    << "error: Line "
                    << diagnostic.line
                    << ", Column "
                    << diagnostic.column
                    << ": "
                    << diagnostic.message
                    << '\n';
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "error: "
            << e.what()
            << '\n';

        return 1;
    }
    return 0;
}