#pragma once
#include <string_view>

namespace lumen
{
    enum class TokenKind
    {
        //Literals & Numbers
        Identifier, Number,
        //KeyWords
        KwIf, KwElse, KwWhile, KwFor, KwReturn, KwInt, KwFloat, KwBool, KwDouble,
        //punctuation and Opt
        Plus, Minus, Star, Slash, Mod,
        Equals, EqualsEquals, Less, LessEqual, GreaterEqual, Greater, LParen, RParen, LBrace, RBrace, Semicolon, BangEquals, Bang, AndAnd, OrOr, And, Or,
        //control
        EndOfFile
    
    };

    enum class DiagnosticSeverity
    {
        Warning,
        Error
    };
    
    struct Token
    {
        TokenKind kind;
        std::string_view lexeme;
        int line;
        int column;
    };

    struct Diagnostic
    {
        DiagnosticSeverity severity;
        int line;
        int column;
        std::string message;
    };

    class DiagnosticEngine
    {
        public:
            void reportError(
                int line,
                int column,
                const std::string& message
            );

            const std::vector<Diagnostic>& diagnostics() const;

        private:
            std::vector<Diagnostic> diagnostics_;
    };
}