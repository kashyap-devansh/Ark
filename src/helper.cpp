#include "helper.h"
#include <iostream>

bool evaluateCondition(const Cell& left, TokenType op, const Cell& right) {
    switch(op) {
        case TokenType::TOK_EQUAL_EQUAL : return left == right; 
        case TokenType::TOK_NOT_EQUAL : return left != right;
        case TokenType::TOK_GREATER : return left > right;
        case TokenType::TOK_LESS : return left < right;
        case TokenType::TOK_GREATER_EQUAL : return left >= right;
        case TokenType::TOK_LESS_EQUAL : return left <= right;
        default : return false;
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

bool evaluateLogicalConditions(const Row* row, const std::vector<Condition>& conditions, const std::vector<TokenType>& logicalOps) {

    bool result = evaluateCondition(row->getCell(conditions[0].colIndex), conditions[0].op, conditions[0].value);

    for(int i = 1; i < conditions.size(); i++) {
        bool next = evaluateCondition(row->getCell(conditions[i].colIndex), conditions[i].op, conditions[i].value);

        if(logicalOps[i - 1] == TokenType::TOK_AND) result = result && next;
        else if(logicalOps[i - 1] == TokenType::TOK_OR)  result = result || next;
    }

    return result;
}
