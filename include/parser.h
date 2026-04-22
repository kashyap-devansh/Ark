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
    bool match(TokenType type);
    void consume(TokenType expected);

    void parseUse(DatabaseManager& manager);
    void parseCreate(DatabaseManager& manager);
    void parseDrop(DatabaseManager& manager);
    void parseShow(DatabaseManager& manager);
    void parseInsert(DatabaseManager& manager);
    void parseAlter(DatabaseManager& manager);
    void parseSelect(DatabaseManager& manager);
    void parseAggregateSelect(DatabaseManager& manager, TokenType aggregator);
    void parseUpdate(DatabaseManager& manager);
    void parseDelete(DatabaseManager& manager);
    void parseTruncate(DatabaseManager& manager);

    void parseInnerJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll);
    void parseLeftJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll);
    void parseRightJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll);
    void parseOutterJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll);

    void parseRename(DatabaseManager& manager);
    void parseShowTables(DatabaseManager& manager);
    void parseShowDatabases(DatabaseManager& manager);
    void parseShowColumns(DatabaseManager& manager);

    void parseDeleteLogical(DatabaseManager& manager, Table* table, int colNumber, std::string colName, TokenType firstOp, Cell firstVal);

    DataType parseDataType();
    Cell parseValue();

    int parseLimitValue();

    void parseOrderBy(Table* table, std::vector<int> whereRows, std::vector<int> columnIndexes, std::vector<std::string> aliasNames, bool distinct = false);

    std::vector<int> parseLikeMatches(Table* table, int colIndex);

    std::vector<int> parseWhereClause(Table* table);

public :
    Parser(const std::string& input, int startLine);
    void parse(DatabaseManager& manager);
};

#endif