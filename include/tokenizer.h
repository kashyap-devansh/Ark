#pragma once
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>

enum class TokenType {
    // ----- Literals & Identifiers -----
    TOK_IDENTIFIER,        // table names, column names
    TOK_NUMBER,            // 10, 20, -5
    TOK_STRING,            // "Dev"
    TOK_TRUE,              // true
    TOK_FALSE,             // false
    TOK_NULL,              // NULL

    // ----- Database Commands -----
    TOK_CREATE,            // CREATE
    TOK_DROP,              // DROP
    TOK_SHOW,              // SHOW
    TOK_SAVE,              // SAVE
    TOK_LOAD,              // LOAD
    TOK_DATABASE,          // DATABASE
    TOK_DATABASES,         // DATABASES
    TOK_USE,               // USE

    // ----- Table Keywords -----
    TOK_TABLE,             // TABLE
    TOK_TABLES,            // TABLES
    TOK_COLUMN,            // COLUMN
    TOK_COLUMNS,           // COLUMNS

    // ----- Data Operations -----
    TOK_INSERT,            // INSERT
    TOK_INTO,              // INTO
    TOK_VALUES,            // VALUES
    TOK_SELECT,            // SELECT
    TOK_FROM,              // FROM
    TOK_DELETE,            // DELETE
    TOK_UPDATE,            // UPDATE
    TOK_SET,               // SET
    TOK_WHERE,             // WHERE
    TOK_LIMIT,             // LIMIT
    TOK_LIKE,              // LIKE
    TOK_ORDER,             // ORDER 
    TOK_BY,                // BY
    TOK_ASC,               // ASCENDING
    TOK_DESC,              // DASCEDNING

    // ----- Data Types -----
    TOK_INT,               // INT
    TOK_DOUBLE,            // DOUBLE
    TOK_STRING_TYPE,       // STRING
    TOK_BOOL,              // BOOL

    // ----- Logical Operators -----
    TOK_EQUAL_EQUAL,       // ==
    TOK_NOT_EQUAL,         // !=
    TOK_EQUAL,             // =
    TOK_GREATER,           // >
    TOK_LESS,              // <
    TOK_GREATER_EQUAL,     // >=
    TOK_LESS_EQUAL,        // <=
    TOK_AND,               // AND
    TOK_OR,                // OR
    TOK_NOT,               // NOT

    // ----- Symbols -----
    TOK_LPAREN,            // (
    TOK_RPAREN,            // )
    TOK_COMMA,             // ,
    TOK_STAR,              // *
    TOK_SEMICOLON,         // ;

    // ----- End Marker -----
    TOK_END_OF_FILE,
    UNKOWN
};

struct Keyword {
    const std::string word;
    TokenType type;
};

class Token {
private :
    TokenType type;
    std::string lexeme;
    int line;
    int column;

public :
    Token();
    Token(const TokenType type, const std::string& lexeme, const int line, const int column);

    TokenType getType() const;
    std::string getLexeme() const;
    int getLine() const;
    int getColumn() const;
};

class Tokenizer {
private :
    std::string input;
    size_t position;
    int line;
    int column;

    char current();
    void advance();
    void skipWhiteSpace();

public :
    Tokenizer(const std::string& text, int startLine = 1);
    Token nextToken();
};

#endif