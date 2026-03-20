#include "parser.h"
#include "helper.h"
#include "error.h"
using namespace std;

#include <iostream>

Parser::Parser(const std::string& input, int startLine) : tokenizer(input, startLine) {
    current = tokenizer.nextToken();
}

void Parser::advance() {
    current = tokenizer.nextToken();
}

bool Parser::match(TokenType type) {
    return current.getType() == type;
}

bool Parser::consume(TokenType expected) {
    if(!match(expected)) return false;
    advance();
    return true;
}

DataType Parser::parseDataType() {
    if(match(TokenType::TOK_INT)) {
        advance();
        return DataType::INT;
    }

    if(match(TokenType::TOK_DOUBLE)) {
        advance();
        return DataType::DOUBLE;
    }

    if(match(TokenType::TOK_STRING_TYPE)) {
        advance();
        return DataType::STRING;
    }

    if(match(TokenType::TOK_BOOL)) {
        advance();
        return DataType::BOOL;
    }

    throw SyntaxException(SyntaxError::UNRECOGNIZED_DATA_TYPE, current.getLine(), current.getColumn(), current.getLexeme(), "");
}

Cell Parser::parseValue() {
    if(match(TokenType::TOK_NUMBER)) {
        std::string num = current.getLexeme();
        advance();

        if(num.find('.') != std::string::npos) return Cell(std::stod(num));
        else return Cell(std::stoi(num));
    }

    if(match(TokenType::TOK_STRING)) {
        std::string str = current.getLexeme();
        advance();

        return Cell(str);
    }

    if(match(TokenType::TOK_TRUE)) {
        advance();
        return Cell(true);
    }

    if(match(TokenType::TOK_FALSE)) {
        advance();
        return Cell(false);
    }
    if(match(TokenType::TOK_NULL)) {
        advance();
        return Cell();
    }

    throw SyntaxException(SyntaxError::UNRECOGNIZED_VALUE, current.getLine(), current.getColumn(), current.getLexeme(), "");
}

int Parser::parseLimitValue() {
    consume(TokenType::TOK_LIMIT);
    Cell cLimit = parseValue();

    if(!cLimit.isInt()) {
        throw TypeException(TypeError::LIMIT_NOT_INT, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    int limit = cLimit.getInt();

    if(limit < 0) {
        TypeException(TypeError::NEGATIVE_LIMIT, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    return limit;
}

std::vector<int> Parser::parseLikeMatches(Table* table, int colIndex) {
    if(!(table->getColumnType(colIndex) == DataType::STRING)) {
        std::cerr << "To use \"LIKE\" the column must be of type string\n";
        exit(EXIT_FAILURE);
    }

    consume(TokenType::TOK_LIKE);

    bool isString = match(TokenType::TOK_STRING);

    if(!isString) {
        throw SyntaxException(SyntaxError::EXPECTED_STRING_AFTER_LIKE, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    std::string wordLike = current.getLexeme();
    consume(TokenType::TOK_STRING);

    if(!(wordLike[1] == '%')) {
        throw SyntaxException(SyntaxError::INVALID_LIKE_PATTERN, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    char findWordWith = wordLike[0];
    std::vector<Row> rows = table->selectAll();

    std::vector<int> result;
    for(int i = 0; i < rows.size(); i++) {
        Cell val = rows[i].getCell(colIndex);
        char firstLetter = val.getString()[0];

        if(findWordWith == firstLetter) {
            result.push_back(i);
        }
    }

    return result;
}

std::vector<int> Parser::parseWhereClause(Table* table) {
    consume(TokenType::TOK_WHERE);

    std::string columnName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    int colNumber = getColumnIndex(table, columnName);

    if(colNumber == -1) {
        throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    if(match(TokenType::TOK_LIKE)) {
        return parseLikeMatches(table, colNumber);
    }

    bool matched = (match(TokenType::TOK_EQUAL_EQUAL) || match(TokenType::TOK_NOT_EQUAL) || match(TokenType::TOK_GREATER_EQUAL) || match(TokenType::TOK_LESS_EQUAL) || match(TokenType::TOK_GREATER) || match(TokenType::TOK_LESS));

    if(matched) {
        TokenType op = current.getType();
        consume(op);

        Cell compVal = parseValue();

        if(!match(TokenType::TOK_AND) && !match(TokenType::TOK_OR)) {
            return getMatchingRows(table, colNumber, op, compVal);
        }

        std::vector<Condition> conditions;
        std::vector<TokenType> logicalOps;

        conditions.push_back({colNumber, op, compVal});

        while(match(TokenType::TOK_AND) || match(TokenType::TOK_OR)) {
            TokenType logicOp = current.getType();
            consume(logicOp);
            logicalOps.push_back(logicOp);

            std::string nextCol = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            int nextColIndex = getColumnIndex(table, nextCol);
            if(nextColIndex == -1) {
                throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), current.getLexeme(), "");
            }

            TokenType nextOp = current.getType();
            consume(nextOp);

            Cell nextVal = parseValue();
            conditions.push_back({nextColIndex, nextOp, nextVal});
        }

        std::vector<int> result;
        std::vector<Row> rows = table->selectAll();

        for(int i = 0; i < rows.size(); i++) {
            if(evaluateLogicalConditions(&rows[i], conditions, logicalOps)) {
                result.push_back(i);
            }
        }

        return result;
    }

    return {};
}

void Parser::parse(DatabaseManager& manager) {
    switch(current.getType()) {
        case TokenType::TOK_CREATE : parseCreate(manager); break;
        case TokenType::TOK_DROP   : parseDrop(manager); break;
        case TokenType::TOK_USE    : parseUse(manager); break;
        case TokenType::TOK_SHOW   : parseShow(manager); break;
        case TokenType::TOK_INSERT : parseInsert(manager); break;
        case TokenType::TOK_SELECT : parseSelect(manager); break;
        case TokenType::TOK_UPDATE : parseUpdate(manager); break;
        case TokenType::TOK_DELETE : parseDelete(manager); break;

        case TokenType::TOK_SAVE   : {
            Database* db = manager.getCurrentDatabase();
            if(db) db->saveDatabase();
            advance();
            break;
        }

        case TokenType::TOK_LOAD   : {
            Database* db = manager.getCurrentDatabase();
            if(db) db->loadDatabase();
            advance();
            break;
        }

        default : throw SyntaxException(SyntaxError::UNKNOWN_COMMAND, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }
}

void Parser::parseCreate(DatabaseManager& manager) {
    consume(TokenType::TOK_CREATE);

    if(match(TokenType::TOK_DATABASE)) {
        consume(TokenType::TOK_DATABASE);

        std::string dbName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);
        consume(TokenType::TOK_SEMICOLON);

        manager.createDatabase(dbName);
        return;
    }

    if(match(TokenType::TOK_TABLE)) {
        consume(TokenType::TOK_TABLE);

        Database* db = manager.getCurrentDatabase();
        if(!db) return;

        std::string tableName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        db->createTable(tableName);
        Table* table = db->getTable(tableName);

        consume(TokenType::TOK_LPAREN);

        while(!match(TokenType::TOK_RPAREN)) {
            std::string columnName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            DataType type = parseDataType();
            table->addColumn(Column(columnName, type));

            if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
        }

        consume(TokenType::TOK_RPAREN);
        consume(TokenType::TOK_SEMICOLON);
    }
}

void Parser::parseDrop(DatabaseManager& manager) {
    consume(TokenType::TOK_DROP);

    if(match(TokenType::TOK_DATABASE)) {
        consume(TokenType::TOK_DATABASE);

        std::string dbName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);
        consume(TokenType::TOK_SEMICOLON);

        manager.dropDatabase(dbName);
        return;
    }

    if(match(TokenType::TOK_TABLE)) {
        consume(TokenType::TOK_TABLE);

        Database* db = manager.getCurrentDatabase();
        if(!db) return;

        std::string tableName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);
        consume(TokenType::TOK_SEMICOLON);

        db->dropTable(tableName);
    }
}

void Parser::parseUse(DatabaseManager& manager) {
    consume(TokenType::TOK_USE);

    std::string dbName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_SEMICOLON);
    
    manager.useDatabase(dbName);
}

void Parser::parseShow(DatabaseManager& manager) {
    consume(TokenType::TOK_SHOW);

    if(match(TokenType::TOK_TABLES)) {
        parseShowTables(manager);
    }
    else if(match(TokenType::TOK_DATABASES)) {
        parseShowDatabases(manager);
    }
    else if(match(TokenType::TOK_COLUMNS)) {
        parseShowColumns(manager);
    }
}

void Parser::parseShowTables(DatabaseManager& manager) {
    consume(TokenType::TOK_TABLES);
    consume(TokenType::TOK_SEMICOLON);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::vector<std::string> tables = db->listTables();

    for(const auto& name : tables) std::cout << name << "\n";
}

void Parser::parseShowDatabases(DatabaseManager& manager) {
    consume(TokenType::TOK_DATABASES);
    consume(TokenType::TOK_SEMICOLON);

    std::vector<std::string> dbs = manager.listDatabase();

    if(dbs.empty()) {
        std::cerr << "There is no database in the disk or memory.\n";
        return;
    }

    for(const auto& name : dbs) std::cout << name << "\n";
}

void Parser::parseShowColumns(DatabaseManager& manager) {
    consume(TokenType::TOK_COLUMNS);
    consume(TokenType::TOK_FROM);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_SEMICOLON);

    Table* table = db->getTable(tableName);
    if(!table) {
        throw RuntimeException(RuntimeError::TABLE_NOT_FOUND, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    std::vector<int> columnIndexes;
    for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);

    std::vector<int> widths = computeColWidths(table, columnIndexes);

    for(int i = 0; i < columnIndexes.size(); i++) {
        switch(table->getColumnType(columnIndexes[i])) {
            case DataType::INT : widths[i] = std::max(widths[i], 3); break;
            case DataType::DOUBLE : widths[i] = std::max(widths[i], 6); break;
            case DataType::STRING : widths[i] = std::max(widths[i], 6); break;
            case DataType::BOOL : widths[i] = std::max(widths[i], 4); break;
            case DataType::NONE : widths[i] = std::max(widths[i], 4); break;
        }
    }

    printBorder(widths, columnIndexes);

    std::cout << "|";
    for(int i = 0; i < columnIndexes.size(); i++) {
        std::string colName = table->getColumName(columnIndexes[i]);
        std::cout << " " << colName;

        for(int j = colName.length(); j < widths[i]; j++) std::cout << " ";
        std::cout << " |";
    }

    std::cout << "\n";

    printBorder(widths, columnIndexes);

    std::cout << "|";
    for(int i = 0; i < columnIndexes.size(); i++) {
        int typeLength = 0;
        
        switch(table->getColumnType(columnIndexes[i])) {
            case DataType::INT : std::cout << " INT";    typeLength = 3; break;
            case DataType::DOUBLE : std::cout << " DOUBLE"; typeLength = 6; break;
            case DataType::STRING : std::cout << " STRING"; typeLength = 6; break;
            case DataType::BOOL : std::cout << " BOOL";   typeLength = 4; break;
            case DataType::NONE : std::cout << " NONE";   typeLength = 4; break;
        }

        for(int j = typeLength; j < widths[i]; j++) std::cout << " ";
        std::cout << " |";
    }
    std::cout << "\n";

    printBorder(widths, columnIndexes);
}

// SELECT * FROM users ORDER BY score ASC;
void Parser::parseOrderBy(Table* table) {
    consume(TokenType::TOK_BY);

    std::vector<Row> rows = table->selectAll();

    std::string colName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    int colNumber = getColumnIndex(table, colName);

    if (colNumber == -1) {
        throw RuntimeException(RuntimeError::TABLE_NOT_FOUND, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    TokenType order = current.getType();
    if(match(TokenType::TOK_ASC) || match(TokenType::TOK_DESC)) consume(order);

    std::vector<int> sortedRows;
    for(int i = 0; i < (int)rows.size(); i++) {
        sortedRows.push_back(i);
    }

    for(int i = 0; i < (int)sortedRows.size() - 1; i++) {
        bool swapped = false;

        if(order == TokenType::TOK_ASC) {
            for(int j = 0; j < (int)sortedRows.size() - i - 1; j++) {
                if(rows[sortedRows[j]].getCell(colNumber) > rows[sortedRows[j + 1]].getCell(colNumber)) {
                    std::swap(sortedRows[j], sortedRows[j + 1]);
                    swapped = true;
                }
            }
        }
        else if(order == TokenType::TOK_DESC) {
            for(int j = 0; j < (int)sortedRows.size() - i - 1; j++) {
                if(rows[sortedRows[j]].getCell(colNumber) < rows[sortedRows[j + 1]].getCell(colNumber)) {
                    std::swap(sortedRows[j], sortedRows[j + 1]);
                    swapped = true;
                }
            }
        }

        if(!swapped) break;
    }

    std::vector<int> columnIndexes;
    for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);

    printTableResult(table, columnIndexes, sortedRows);

    consume(TokenType::TOK_SEMICOLON);
}

void Parser::parseInsert(DatabaseManager& manager) {
    consume(TokenType::TOK_INSERT);
    consume(TokenType::TOK_INTO);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);
    checkNotNull(table, tableName);

    consume(TokenType::TOK_VALUES);

    while(!match(TokenType::TOK_SEMICOLON)) {
        consume(TokenType::TOK_LPAREN);

        Row row;
        while(!match(TokenType::TOK_RPAREN)) {
            row.addCell(parseValue());
    
            if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
        }

        consume(TokenType::TOK_RPAREN);

        table->insertRow(row);

        if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
        else break;
    }

    consume(TokenType::TOK_SEMICOLON);
}

void Parser::parseSelect(DatabaseManager& manager) {
    consume(TokenType::TOK_SELECT);

    std::vector<std::string> selectedColumns;
    bool selectAll = false;

    if(match(TokenType::TOK_STAR)) {
        consume(TokenType::TOK_STAR);
        selectAll = true;
    }
    else {
        while(true) {
            std::string colName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);
            selectedColumns.push_back(colName);

            if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
            else break;
        }
    }

    consume(TokenType::TOK_FROM);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);
    checkNotNull(table, tableName);

    std::vector<int> whereRows;
    if(match(TokenType::TOK_WHERE)) {
        whereRows = parseWhereClause(table);
    }

    if(match(TokenType::TOK_ORDER)) {
        consume(TokenType::TOK_ORDER);

        if(match(TokenType::TOK_BY)) parseOrderBy(table);
        return;
    }
    
    consume(TokenType::TOK_SEMICOLON);

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);
    }
    else {
        for(const std::string& colName : selectedColumns) {

            int colIndex = getColumnIndex(table, colName);

            if(colIndex == -1) {
                std::cerr << "Column not found: " << colName << "\n";
                return;
            }

            columnIndexes.push_back(colIndex);
        } 
    }

    printTableResult(table, columnIndexes, whereRows);
}

void Parser::parseUpdate(DatabaseManager& manager) {
    consume(TokenType::TOK_UPDATE);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);
    checkNotNull(table, tableName);

    consume(TokenType::TOK_SET);

    std::vector<std::string> columnNames;
    std::vector<int> updateColumnIndexes;
    std::vector<Cell> newValues;
    int columnCounter = 0;

    while(!(match(TokenType::TOK_WHERE) || match(TokenType::TOK_SEMICOLON))) {
        columnNames.push_back(current.getLexeme());
        int colIndex = getColumnIndex(table, columnNames[columnCounter]);

        if(colIndex == -1) {
            std::cerr << "No column found with name: " << columnNames[columnCounter] << "\n";
            return;
        }
        updateColumnIndexes.push_back(getColumnIndex(table, columnNames[columnCounter]));
        columnCounter++;

        consume(TokenType::TOK_IDENTIFIER);
        consume(TokenType::TOK_EQUAL);

        newValues.push_back(parseValue());

        if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
    }

    if(match(TokenType::TOK_SEMICOLON)) {
        consume(TokenType::TOK_SEMICOLON);

        auto rows = table->selectAll();

        for(int i = 0; i < rows.size(); i++) {
            for(int j = 0; j < updateColumnIndexes.size(); j++) {
                table->updateCell(i, updateColumnIndexes[j], newValues[j]);
            }
        }
    }
    else if(match(TokenType::TOK_WHERE)) {
        std::vector<int> whereRows = parseWhereClause(table);

        int limit = -1;
        if(match(TokenType::TOK_LIMIT)) {
            limit = parseLimitValue();
        }

        int updated = 0;
        for(int i : whereRows) {
            for(int j = 0; j < updateColumnIndexes.size(); j++) table->updateCell(i, updateColumnIndexes[j], newValues[j]);

            updated++;
            if(limit != -1 && updated == limit) break;
        }

        consume(TokenType::TOK_SEMICOLON);
    }
}

void Parser::parseDelete(DatabaseManager& manager) {
    consume(TokenType::TOK_DELETE);
    consume(TokenType::TOK_FROM);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);
    checkNotNull(table, tableName);

    if(match(TokenType::TOK_SEMICOLON)) {
        consume(TokenType::TOK_SEMICOLON);

        while(!table->selectAll().empty()) table->deleteRow(0);
    }
    else if(match(TokenType::TOK_WHERE)) {
        std::vector<int> whereRows = parseWhereClause(table);

        int limit = -1;
        if(match(TokenType::TOK_LIMIT)) limit = parseLimitValue();

        int deleted = 0;
        for(int i = whereRows.size() - 1; i >= 0; i--) {
            table->deleteRow(whereRows[i]);
            
            deleted++;
            if(limit != -1 && deleted == limit) break;
        }
        
        consume(TokenType::TOK_SEMICOLON);
    }
}