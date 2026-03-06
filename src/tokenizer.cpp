#include "tokenizer.h"
#include <vector>
#include <cctype>

static const std::vector<Keyword> KeywordTable = {
    {"CREATE",   TokenType::TOK_CREATE},
    {"DROP",     TokenType::TOK_DROP},
    {"SHOW",     TokenType::TOK_SHOW},
    {"SAVE",     TokenType::TOK_SAVE},
    {"LOAD",     TokenType::TOK_LOAD},
    {"TABLE",    TokenType::TOK_TABLE},
    {"TABLES",   TokenType::TOK_TABLES},
    {"COLUMNS",  TokenType::TOK_COLUMN},
    {"DATABASE", TokenType::TOK_DATABASE},
    {"USE",      TokenType::TOK_USE},
    {"INSERT",   TokenType::TOK_INSERT},
    {"INTO",     TokenType::TOK_INTO},
    {"VALUES",   TokenType::TOK_VALUES},
    {"SELECT",   TokenType::TOK_SELECT},
    {"FROM",     TokenType::TOK_FROM},
    {"DELETE",   TokenType::TOK_DELETE},
    {"UPDATE",   TokenType::TOK_UPDATE},
    {"SET",      TokenType::TOK_SET},
    {"WHERE",    TokenType::TOK_WHERE},
    {"INT",      TokenType::TOK_INT},
    {"DOUBLE",   TokenType::TOK_DOUBLE},
    {"STRING",   TokenType::TOK_STRING_TYPE},
    {"BOOL",     TokenType::TOK_BOOL},
    {"AND",      TokenType::TOK_AND},
    {"OR",       TokenType::TOK_OR},
    {"NOT",      TokenType::TOK_NOT},
    {"TRUE",     TokenType::TOK_TRUE},
    {"FALSE",    TokenType::TOK_FALSE},
};

static TokenType checkKeyword(const std::string& word) {
    for(const auto& wordToken : KeywordTable) {
        if(word == wordToken.word) return wordToken.type;
    }
    return TokenType::TOK_IDENTIFIER;
}

//====================================================================== TOKEN ======================================================================

Token::Token() : type(TokenType::UNKOWN), lexeme(""), line(0), column(0) {}

Token::Token(const TokenType type, const std::string& lexeme, const int line, const int column) : type(type), lexeme(lexeme), line(line), column(column) {}

TokenType Token::getType() const {
    return type;
}

std::string Token::getLexeme() const {
    return lexeme;
}

int Token::getLine() const {
    return line;
}

int Token::getColumn() const {
    return column;
}

//====================================================================== TOKENIZER ======================================================================

Tokenizer::Tokenizer(const std::string& text, int startLine) : input(text), position(0), line(startLine), column(1) {}

char Tokenizer::current() {
    if(position >= input.length()) return '\0';

    return input[position];
}

void Tokenizer::advance() {
    if(current() == '\n') {
        line++;
        column = 1;
    }
    else {
        column++;
    }

    position++;
}

void Tokenizer::skipWhiteSpace() {
    while(std::isspace(current())) advance();
}

Token Tokenizer::nextToken() {
    skipWhiteSpace();

    int tokenLine = line;
    int tokenColumn = column;

    char ch = current();

    if(ch == '\0') {
        return Token(TokenType::TOK_END_OF_FILE, "", tokenLine, tokenColumn);
    }

    if(std::isalpha(ch) || ch == '_') {
        std::string word;

        while(std::isalnum(current()) || current() == '_') {
            word += std::toupper(current());
            advance();
        }

        TokenType type = checkKeyword(word);
        return Token(type, word, tokenLine, tokenColumn);
    }

    if(std::isdigit(ch) || (ch == '-' && std::isdigit(input[position + 1]))) {
        std::string number;

        if(ch == '-') {
            number += ch;
            advance();
        }

        while(std::isdigit(current())) {
            number += current();
            advance();
        }

        if(current() == '.') {
            number += '.';
            advance();

            while(std::isdigit(current())) {
                number += current();
                advance();
            }
        }

        return Token(TokenType::TOK_NUMBER, number, tokenLine, tokenColumn);
    }

    if(ch == '"') {
        advance();

        std::string str;
        while(current() != '"' && current() != '\0') {
            str += current();
            advance();
        }

        if(current() == '"') advance();

        return Token(TokenType::TOK_STRING, str, tokenLine, tokenColumn);
    }

    if(ch == '=' && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_EQUAL_EQUAL, "==", tokenLine, tokenColumn);
    }

    if(ch == '!' && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_NOT_EQUAL, "!=", tokenLine, tokenColumn);
    }

    if(ch == '>' && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_GREATER_EQUAL, ">=", tokenLine, tokenColumn);
    }

    if(ch == '<' && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_LESS_EQUAL, "<=", tokenLine, tokenColumn);
    }

    advance();

    switch(ch) {
        case '=' : return Token(TokenType::TOK_EQUAL, "=", tokenLine, tokenColumn);
        case '>' : return Token(TokenType::TOK_GREATER, ">", tokenLine, tokenColumn);
        case '<' : return Token(TokenType::TOK_LESS, "<", tokenLine, tokenColumn);
        case '(' : return Token(TokenType::TOK_LPAREN, "(", tokenLine, tokenColumn);
        case ')' : return Token(TokenType::TOK_RPAREN, ")", tokenLine, tokenColumn);
        case ',' : return Token(TokenType::TOK_COMMA, ",", tokenLine, tokenColumn);
        case '*': return Token(TokenType::TOK_STAR, "*", tokenLine, tokenColumn);
        case ';': return Token(TokenType::TOK_SEMICOLON, ";", tokenLine, tokenColumn);
    }

    return Token(TokenType::UNKOWN, std::string(1, ch), tokenLine, tokenColumn);
}