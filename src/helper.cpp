#include "helper.h"
#include <iostream>

bool evaluateCondition(const Cell& left, TokenType op, const Cell& right) {
    switch(op) {
        case TokenType::TOK_EQUAL_EQUAL:
            return left == right;

        case TokenType::TOK_NOT_EQUAL:
            return left != right;

        case TokenType::TOK_GREATER:
            return left > right;

        case TokenType::TOK_LESS:
            return left < right;

        case TokenType::TOK_GREATER_EQUAL:
            return left >= right;

        case TokenType::TOK_LESS_EQUAL:
            return left <= right;

        default:
            return false;
    }
}

std::vector<int> getMatchingRows(Table* table, int colIndex, TokenType op, const Cell& compVal) {
    std::vector<int> mathingRows;

    for(int i = 0; i < table->getRowCount(); i++) {
        Cell val = table->selectAll()[i].getCell(colIndex);

        if(evaluateCondition(val, op, compVal)) mathingRows.push_back(i);
    }

    return mathingRows;
}

int getColumnIndex(Table* table, const std::string& columnName) {
    for(int i = 0; i < table->getColumnCount(); i++) {
        if(table->getColumName(i) == columnName) return i;
    }

    return -1;
}

void checkNotNull(const void* table, const std::string& tableName) {
    if (!table) {
        std::cerr << "No table found with name : " << tableName << "\n";
        exit(EXIT_FAILURE);
    }
}

