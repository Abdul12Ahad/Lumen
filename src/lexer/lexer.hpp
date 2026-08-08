#pragma once
#include <string>
#include <vector>
#include "token.hpp"

namespace lumen
{
    class Lexer
    {
        public:
            explicit Lexer(const std::string& source) : src_(source) {}

            std::vector<Token> tokenize();

        private:
            const std::string& src_;
            size_t pos_ = 0;
            int line_ = 1;
            int column_ = 1;

            bool atEnd() const 
            {
                return pos_>=src_.size();
            }
            char peek() const 
            {
                return atEnd() ? '\0' : src_[pos_];
            }
            char peekNext() const
            {
                return pos_+1 < src_.size() ? src_[pos_+1] : '\0';
            }
            char advance();
            bool match(char expected);
            void skipWhitespaceAndComments();
            Token makeToken(TokenKind kind, size_t start, int startLine, int startColumn);
            Token errorToken(const std::string& msg, int startLine, int startColumn);
            Token scanIdentifierorKeyword();
            Token scanNumber();
            Token scanOperatororPunctuation();

            static TokenKind keywordOrIdentifier(std::string_view text);
    };
}