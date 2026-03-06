#pragma once
#ifndef PARSER_H
#define PARSER_H

#include "core.h"
#include "tokenizer.h"
#include "databaseManager.h"

class Parser {
private :
    Tokenizer tokenizer;
    Token current;

    void advance();
    void consume(TokenType expected);
    bool match(TokenType type);

    void parseUse(DatabaseManager& manager);
    void parseCreate(DatabaseManager& manager);
    void parseDrop(DatabaseManager& manager);
    void parseShow(DatabaseManager& manager);
    void parseInsert(DatabaseManager& manager);
    void parseSelect(DatabaseManager& manager);
    void parseUpdate(DatabaseManager& manager);
    void parseDelete(DatabaseManager& manager);

    DataType parseDataType();
    Cell parseValue();
    int getColumnIndex(Table* table, const std::string& columnName);

public :
    Parser(const std::string& input, int startLine);
    void parse(DatabaseManager& manager);
};

#endif