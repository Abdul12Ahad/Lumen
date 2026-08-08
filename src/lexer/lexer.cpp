#include "lexer.hpp"
#include <cctype>
#include <unordered_map>

namespace lumen
{
    char Lexer::advance()
    {
        char c = src_[pos_++];
        if(c=='\n')
        {
            line_++;
            column_=1;
        }
        else
            column_++;

        return c;
    }

    bool Lexer::match(char expected)
    {
        if(atEnd() || src_[pos_]!=expected)
            return false;
        advance();
        return true;
    }

    void Lexer::skipWhitespaceAndComments()
    {
        while(!atEnd())
        {
            char c = peek();
            if(c==' '|| c=='\t' || c=='\r' || c=='\n')
            {
                advance();
            }
            else if(c=='/' && peekNext() == '/')
                while(!atEnd() && peek()!='\n')
                {
                    advance();
                }
            else
                return;
        }
    }

    Token Lexer::makeToken(TokenKind kind, size_t start, int startLine, int startColumn)
    {
        std::string_view lexeme(src_.data() + start, pos_-start);
        return Token{kind,lexeme, startLine, startColumn};
    }

    Token Lexer::errorToken(const std::string& msg, int startLine, int startColumn)
    {
        return Token{TokenKind::Error, std::string_view(msg), startLine, startColumn};
    }
    
    TokenKind Lexer::keywordOrIdentifier(std::string_view text)
    {
        static const std::unordered_map<std::string_view, TokenKind> keywords = {
            {"int", TokenKind::KwInt},
            {"float", TokenKind::KwFloat},
            {"bool", TokenKind::KwBool},
            {"double", TokenKind::KwDouble},
            {"if", TokenKind::KwIf},
            {"else", TokenKind::KwElse},
            {"for", TokenKind::KwFor},
            {"while", TokenKind::KwWhile},
            {"return", TokenKind::KwReturn},
        };
        auto it = keywords.find(text);
        return it != keywords.end() ? it->second : TokenKind::Identifier;
    }
    
    Token Lexer::scanIdentifierorKeyword()
    {
        size_t start = pos_;
        int line = line_, col = column_;
        while(!atEnd() && (std::isalnum((unsigned char)peek()) || peek() =='_')) advance();
        std::string_view text(src_.data() + start, pos_ - start);
        return Token{keywordOrIdentifier(text),text, line, col};
    }

    Token Lexer::scanNumber()
    {
        size_t start = pos_;
        int line = line_, col = column_;
        while(!atEnd() && std::isdigit((unsigned char)peek())) advance();
        return makeToken(TokenKind::Number, start, line, col);
    }

    Token Lexer::scanOperatororPunctuation()
    {
        size_t start = pos_;
        int line = line_, col = column_;
        char c = advance();
        switch (c)
        {
        case '+':
            return makeToken(TokenKind::Plus, start, line,col);
            break;
        case '-':
            return makeToken(TokenKind::Minus, start, line,col);
            break;
        case '*':
            return makeToken(TokenKind::Star, start, line,col);
            break;
        case '/':
            return makeToken(TokenKind::Slash, start, line,col);
            break;
        case '%':
            return makeToken(TokenKind::Mod, start, line,col);
            break;
        case '(':
            return makeToken(TokenKind::LParen, start, line,col);
            break;
        case ')':
            return makeToken(TokenKind::RParen, start, line,col);
            break;
        case '{':
            return makeToken(TokenKind::LBrace, start, line,col);
            break;
        case '}':
            return makeToken(TokenKind::RBrace, start, line,col);
            break;
        case ';':
            return makeToken(TokenKind::Semicolon, start, line,col);
            break;
        case '=':
            if(match('=')) 
                return makeToken(TokenKind::EqualsEquals, start, line, col);
            return makeToken(TokenKind::Equals, start, line,col);
            break;
        case '<':
            if(match('=')) 
                return makeToken(TokenKind::LessEqual, start, line, col);
            return makeToken(TokenKind::Less, start, line,col);
            break;
        case '>':
            if(match('=')) 
                return makeToken(TokenKind::GreaterEqual, start, line, col);
            return makeToken(TokenKind::Greater, start, line,col);
            break;
        case '!':
            if(match('=')) 
                return makeToken(TokenKind::BangEquals, start, line, col);
            return makeToken(TokenKind::Bang, start, line,col);
            break;
        case '&':
            if(match('&')) 
                return makeToken(TokenKind::AndAnd, start, line, col);
            return makeToken(TokenKind::And, start, line,col);
            break;
        case '|':
            if(match('|')) 
                return makeToken(TokenKind::OrOr, start, line, col);
            return makeToken(TokenKind::Or, start, line,col);
            break;
        default:
            return errorToken(std::string("unexpected character '") + c + "'", line, col);
        }
    }

    std::vector<Token> Lexer::tokenize()
    {
        std::vector<Token> tokens;

        while(true)
        {
            skipWhitespaceAndComments();

            if(atEnd())
            {
                tokens.push_back(Token{TokenKind::EndOfFile, {}, line_, column_});
                break;
            }

            char c = peek();
            if(std::isalpha((unsigned char)c) || c=='_')
                tokens.push_back(scanIdentifierorKeyword());
            else if(std::isdigit((unsigned char)c))
                tokens.push_back(scanNumber());
            else
                tokens.push_back(scanOperatororPunctuation());
        }
        return tokens;
    }
}