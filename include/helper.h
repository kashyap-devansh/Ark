#ifndef HELPER_H
#define HELPER_H

#include "parser.h"

bool evaluateCondition(const Cell& left, TokenType op, const Cell& right);

std::vector<int> getMatchingRows(Table* table, int colIndex, TokenType op, const Cell& compVal);

int getColumnIndex(Table* table, const std::string& columnName);

void checkNotNull(const void* table, const std::string& tableName);

bool checkOperator(TokenType operatorType);

#endif