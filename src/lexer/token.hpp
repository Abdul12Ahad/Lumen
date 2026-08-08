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
        EndOfFile, Error
    
    };
    
    struct Token
    {
        TokenKind kind;
        std::string_view lexeme;
        int line;
        int column;
    };
}