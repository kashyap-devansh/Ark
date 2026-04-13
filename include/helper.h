#ifndef HELPER_H
#define HELPER_H

#include "parser.h"

struct Condition {
    int colIndex;
    TokenType op;
    Cell value;
};

std::string tokenTypeToString(TokenType type);

void printBorder(const std::vector<int>& widths, const std::vector<int>& columnIndexes);

bool evaluateCondition(const Cell& left, TokenType op, const Cell& right);

std::vector<int> getMatchingRows(Table* table, int colIndex, TokenType op, const Cell& compVal);

int getColumnIndex(Table* table, const std::string& columnName);

void checkNotNull(const void* table, const std::string& tableName);

bool checkOperator(TokenType operatorType);

std::vector<int> computeColWidths(Table* table, const std::vector<int>& colIndexes, const std::vector<std::string>& aliasNames = {});

void printTableResult(Table* table, const std::vector<int>& colIndexes, const std::vector<int>& rowIndexes = {}, const std::vector<std::string>& aliasNames = {}, bool distinct = false);

bool evaluateLogicalConditions(const Row* row, const std::vector<Condition>& conditions, const std::vector<TokenType>& logicalOps);

#endif