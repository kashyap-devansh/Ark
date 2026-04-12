#include "helper.h"
#include <iostream>

std::string tokenTypeToString(TokenType type) {
    switch(type) {
        case TokenType::TOK_CREATE        : return "CREATE";
        case TokenType::TOK_DROP          : return "DROP";
        case TokenType::TOK_SHOW          : return "SHOW";
        case TokenType::TOK_SAVE          : return "SAVE";
        case TokenType::TOK_LOAD          : return "LOAD";
        case TokenType::TOK_TABLE         : return "TABLE";
        case TokenType::TOK_TABLES        : return "TABLES";
        case TokenType::TOK_COLUMN        : return "COLUMN";
        case TokenType::TOK_COLUMNS       : return "COLUMNS";
        case TokenType::TOK_DATABASE      : return "DATABASE";
        case TokenType::TOK_DATABASES     : return "DATABASES";
        case TokenType::TOK_USE           : return "USE";
        case TokenType::TOK_INSERT        : return "INSERT";
        case TokenType::TOK_INTO          : return "INTO";
        case TokenType::TOK_VALUES        : return "VALUES";
        case TokenType::TOK_SELECT        : return "SELECT";
        case TokenType::TOK_FROM          : return "FROM";
        case TokenType::TOK_DELETE        : return "DELETE";
        case TokenType::TOK_UPDATE        : return "UPDATE";
        case TokenType::TOK_SET           : return "SET";
        case TokenType::TOK_WHERE         : return "WHERE";
        case TokenType::TOK_LIMIT         : return "LIMIT";
        case TokenType::TOK_LIKE          : return "LIKE";
        case TokenType::TOK_ORDER         : return "ORDER";
        case TokenType::TOK_BY            : return "BY";
        case TokenType::TOK_ASC           : return "ASC";
        case TokenType::TOK_DESC          : return "DESC";
        case TokenType::TOK_INT           : return "INT";
        case TokenType::TOK_DOUBLE        : return "DOUBLE";
        case TokenType::TOK_STRING_TYPE   : return "STRING";
        case TokenType::TOK_BOOL          : return "BOOL";
        case TokenType::TOK_AND           : return "AND";
        case TokenType::TOK_OR            : return "OR";
        case TokenType::TOK_NOT           : return "NOT";
        case TokenType::TOK_TRUE          : return "TRUE";
        case TokenType::TOK_FALSE         : return "FALSE";
        case TokenType::TOK_NULL          : return "NULL";
        case TokenType::TOK_EQUAL         : return "'='";
        case TokenType::TOK_EQUAL_EQUAL   : return "'=='";
        case TokenType::TOK_NOT_EQUAL     : return "'!='";
        case TokenType::TOK_GREATER       : return "'>'";
        case TokenType::TOK_GREATER_EQUAL : return "'>='";
        case TokenType::TOK_LESS          : return "'<'";
        case TokenType::TOK_LESS_EQUAL    : return "'<='";
        case TokenType::TOK_LPAREN        : return "'('";
        case TokenType::TOK_RPAREN        : return "')'";
        case TokenType::TOK_COMMA         : return "','";
        case TokenType::TOK_STAR          : return "'*'";
        case TokenType::TOK_SEMICOLON     : return "';'";
        case TokenType::TOK_NUMBER        : return "NUMBER";
        case TokenType::TOK_STRING        : return "STRING";
        case TokenType::TOK_IDENTIFIER    : return "IDENTIFIER";
        case TokenType::TOK_END_OF_FILE   : return "END OF FILE";
        default                           : return "UNKNOWN";
    }
}

void printBorder(const std::vector<int>& widths, const std::vector<int>& columnIndexes) {
    for(int i = 0; i < columnIndexes.size(); i++) {
        std::cout << "+-";
        for(int j = 0; j < widths[i]; j++) std::cout << "-";
        std::cout << "-";
    }
    std::cout << "+\n";
}

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

std::vector<int> computeColWidths(Table* table, const std::vector<int>& colIndexes, const std::vector<std::string>& aliasNames) {
    std::vector<int> widths;

    for(int i = 0; i < colIndexes.size(); i++) {
        std::string headerName = (i < aliasNames.size() && !aliasNames[i].empty()) ? aliasNames[i] : table->getColumName(colIndexes[i]);

        widths.push_back(headerName.length());
    }

    for(const Row& row : table->selectAll()) {
        for(int i = 0; i < colIndexes.size(); i++) {
            int cellWidth = row.getCell(colIndexes[i]).toString().length();

            widths[i] = std::max(cellWidth, widths[i]);
        }
    }

    return widths;
}

void printTableResult(Table* table, const std::vector<int>& colIndexes, const std::vector<int>& rowIndexes, const std::vector<std::string>& aliasNames) {
    std::vector<int> widths = computeColWidths(table, colIndexes, aliasNames);

    printBorder(widths, colIndexes);

    std::cout << "|";
    for(int i = 0; i < colIndexes.size(); i++) {
        std::string colName = (i < aliasNames.size() && !aliasNames[i].empty()) ? aliasNames[i] : table->getColumName(colIndexes[i]);
        std::cout << " " << colName;

        for(int j = colName.length(); j < widths[i]; j++) std::cout << " ";
        std::cout << " |";
    }

    std::cout << "\n";

    printBorder(widths, colIndexes);

    std::vector<Row> allRows = table->selectAll();

    std::vector<int> order;
    
    if(rowIndexes.empty()) for(int i = 0; i < table->getRowCount(); i++) order.push_back(i); 
    else order = rowIndexes;

    for(int r : order) {
        const Row row = allRows[r];
        std::cout << "|";

        for(int i = 0; i < colIndexes.size(); i++) {
            std::string val = row.getCell(colIndexes[i]).toString();
            std::cout << " " << val;

            for(int j = val.length(); j < widths[i]; j++) std::cout << " ";
            std::cout << " |";
        }

        std::cout << "\n";
    }

    printBorder(widths, colIndexes);
}