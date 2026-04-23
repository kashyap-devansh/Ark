#include "tokenizer.h"
#include <unordered_map>
#include <cctype>

static const std::unordered_map<std::string, TokenType> KeywordTable = {
    {"CREATE",    TokenType::TOK_CREATE},
    {"DROP",      TokenType::TOK_DROP},
    {"SHOW",      TokenType::TOK_SHOW},
    {"SAVE",      TokenType::TOK_SAVE},
    {"LOAD",      TokenType::TOK_LOAD},
    {"TABLE",     TokenType::TOK_TABLE},
    {"TABLES",    TokenType::TOK_TABLES},
    {"RENAME",    TokenType::TOK_RENAME},
    {"COLUMN",    TokenType::TOK_COLUMN},
    {"COLUMNS",   TokenType::TOK_COLUMNS},
    {"DATABASE",  TokenType::TOK_DATABASE},
    {"DATABASES", TokenType::TOK_DATABASES},
    {"USE",       TokenType::TOK_USE},
    {"INSERT",    TokenType::TOK_INSERT},
    {"INTO",      TokenType::TOK_INTO},
    {"VALUES",    TokenType::TOK_VALUES},
    {"SELECT",    TokenType::TOK_SELECT},
    {"FROM",      TokenType::TOK_FROM},
    {"DELETE",    TokenType::TOK_DELETE},
    {"TRUNCATE",  TokenType::TOK_TRUNCATE},
    {"UPDATE",    TokenType::TOK_UPDATE},
    {"AS",        TokenType::TOK_AS},
    {"DISTINCT",  TokenType::TOK_DISTINCT},
    {"SET",       TokenType::TOK_SET},
    {"WHERE",     TokenType::TOK_WHERE},
    {"LIMIT",     TokenType::TOK_LIMIT},
    {"LIKE",      TokenType::TOK_LIKE},
    {"ORDER",     TokenType::TOK_ORDER},
    {"BY",        TokenType::TOK_BY},
    {"TO",        TokenType::TOK_TO},
    {"JOIN",      TokenType::TOK_JOIN},
    {"INNER",     TokenType::TOK_INNER},
    {"LEFT",      TokenType::TOK_LEFT},
    {"RIGHT",     TokenType::TOK_RIGHT},
    {"FULL",      TokenType::TOK_FULL},
    {"OUTER",     TokenType::TOK_OUTER},
    {"ALTER",     TokenType::TOK_ALTER},
    {"ADD",       TokenType::TOK_ADD},
    {"ON",        TokenType::TOK_ON},
    {"ASC",       TokenType::TOK_ASC},
    {"DESC",      TokenType::TOK_DESC},
    {"COUNT",     TokenType::TOK_COUNT},
    {"SUM",       TokenType::TOK_SUM},
    {"AVG",       TokenType::TOK_AVG},
    {"MIN",       TokenType::TOK_MIN},
    {"MAX",       TokenType::TOK_MAX},
    {"INT",       TokenType::TOK_INT},
    {"DOUBLE",    TokenType::TOK_DOUBLE},
    {"STRING",    TokenType::TOK_STRING_TYPE},
    {"BOOL",      TokenType::TOK_BOOL},
    {"AND",       TokenType::TOK_AND},
    {"OR",        TokenType::TOK_OR},
    {"NOT",       TokenType::TOK_NOT},
    {"TRUE",      TokenType::TOK_TRUE},
    {"FALSE",     TokenType::TOK_FALSE},
    {"NULL",      TokenType::TOK_NULL},
};

static TokenType checkKeyword(const std::string& word) {
    auto it = KeywordTable.find(word);
    return (it != KeywordTable.end()) ? it->second : TokenType::TOK_IDENTIFIER;
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

    if(std::isdigit(ch) || (ch == '-' && ( position + 1 < input.size() ) && std::isdigit(input[position + 1]))) {
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

    if(ch == '=' && ( position + 1 < input.size() ) && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_EQUAL_EQUAL, "==", tokenLine, tokenColumn);
    }

    if(ch == '!' && ( position + 1 < input.size() ) && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_NOT_EQUAL, "!=", tokenLine, tokenColumn);
    }

    if(ch == '>' && ( position + 1 < input.size() ) && input[position + 1] == '=') {
        advance(); advance();
        return Token(TokenType::TOK_GREATER_EQUAL, ">=", tokenLine, tokenColumn);
    }

    if(ch == '<' && ( position + 1 < input.size() ) && input[position + 1] == '=') {
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
        case '*' : return Token(TokenType::TOK_STAR, "*", tokenLine, tokenColumn);
        case ':' : return Token(TokenType::TOK_COLON, ":", tokenLine, tokenColumn);
        case ';' : return Token(TokenType::TOK_SEMICOLON, ";", tokenLine, tokenColumn);
        case '.' : return Token(TokenType::TOK_DOT, ".", tokenLine, tokenColumn);
    }

    return Token(TokenType::UNKOWN, std::string(1, ch), tokenLine, tokenColumn);
}