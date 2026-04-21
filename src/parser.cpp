#include "parser.h"
#include "helper.h"
#include "error.h"

#include <iostream>
#include <filesystem>

Parser::Parser(const std::string& input, int startLine) : tokenizer(input, startLine) {
    current = tokenizer.nextToken();
}

void Parser::advance() {
    current = tokenizer.nextToken();
}

bool Parser::match(TokenType type) {
    return current.getType() == type;
}

void Parser::consume(TokenType expected) {
    if(!match(expected)) throw SyntaxException(SyntaxError::UNEXPECTED_TOKEN, current.getLine(), current.getColumn(), current.getLexeme(), tokenTypeToString(expected));
    advance();
    return;
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
        throw TypeException(TypeError::NEGATIVE_LIMIT, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    return limit;
}

std::vector<int> Parser::parseLikeMatches(Table* table, int colIndex) {
    if(!(table->getColumnType(colIndex) == DataType::STRING)) {
        throw TypeException(TypeError::LIKE_REQUIRES_STRING, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    consume(TokenType::TOK_LIKE);

    bool isString = match(TokenType::TOK_STRING);

    if(!isString) {
        throw SyntaxException(SyntaxError::EXPECTED_STRING_AFTER_LIKE, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    std::string wordLike = current.getLexeme();
    consume(TokenType::TOK_STRING);

    std::vector<int> result;

    if(wordLike.empty() || wordLike.length() < 2) {
        throw RuntimeException(RuntimeError::LIKE_PATTERN_TOO_SHORT, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }

    if(wordLike.length() == 2) {
        if(wordLike[0] == '%') {
            char findWordWith = wordLike[1];
            std::vector<Row> rows = table->selectAll();

            for(int i = 0; i < static_cast<int>(rows.size()); i++) {
                Cell val = rows[i].getCell(colIndex);
                std::string s = val.getString();

                if(!s.empty() && findWordWith == s[s.length() - 1]) result.push_back(i);
            }
        }
        else if(wordLike[1] == '%') {
            char findWordWith = wordLike[0];
            std::vector<Row> rows = table->selectAll();

            for(int i = 0; i < static_cast<int>(rows.size()); i++) {
                Cell val = rows[i].getCell(colIndex);
                std::string s = val.getString();

                if(!s.empty() && findWordWith == s[0]) result.push_back(i);
            }
        }
        else throw SyntaxException(SyntaxError::INVALID_LIKE_PATTERN, current.getLine(), current.getColumn(), current.getLexeme(), "");
    }
    else if(wordLike[0] == '%' && wordLike[wordLike.length() - 1] == '%') {
        std::string findWord = wordLike.substr(1, wordLike.length() - 2);
        std::vector<Row> rows = table->selectAll();

        for(int i = 0; i < static_cast<int>(rows.size()); i++) {
            Cell val = rows[i].getCell(colIndex);

            if(val.getString().find(findWord) != std::string::npos) result.push_back(i);
        }
    }
    else throw SyntaxException(SyntaxError::INVALID_LIKE_PATTERN, current.getLine(), current.getColumn(), current.getLexeme(), "");

    return result;
}

std::vector<int> Parser::parseWhereClause(Table* table) {
    consume(TokenType::TOK_WHERE);

    std::string columnName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    int colNumber = getColumnIndex(table, columnName);

    if(colNumber == -1) {
        throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), columnName, table->getName());
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
                throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), columnName, table->getName());
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

    throw SyntaxException(SyntaxError::EXPECTED_COMPARISON_OPERATOR, current.getLine(), current.getColumn(), current.getLexeme(), "");
}

void Parser::parse(DatabaseManager& manager) {
    switch(current.getType()) {
        case TokenType::TOK_CREATE : parseCreate(manager); break;
        case TokenType::TOK_DROP : parseDrop(manager); break;
        case TokenType::TOK_USE : parseUse(manager); break;
        case TokenType::TOK_SHOW : parseShow(manager); break;
        case TokenType::TOK_INSERT : parseInsert(manager); break;
        case TokenType::TOK_SELECT : parseSelect(manager); break;
        case TokenType::TOK_UPDATE : parseUpdate(manager); break;
        case TokenType::TOK_DELETE : parseDelete(manager); break;
        case TokenType::TOK_TRUNCATE : parseTruncate(manager); break;
        case TokenType::TOK_RENAME : parseRename(manager); break;

        case TokenType::TOK_SAVE   : {
            Database* db = manager.getCurrentDatabase();
            if(!db) throw RuntimeException(RuntimeError::NO_DATABASE_SELECTED, current.getLine(), current.getColumn(), current.getLexeme(), "");

            db->saveDatabase();
            advance();
            consume(TokenType::TOK_SEMICOLON);
            break;
        }

        case TokenType::TOK_LOAD   : {
            Database* db = manager.getCurrentDatabase();
            if(!db) throw RuntimeException(RuntimeError::NO_DATABASE_SELECTED, current.getLine(), current.getColumn(), current.getLexeme(), "");
            
            db->loadDatabase();
            advance();
            consume(TokenType::TOK_SEMICOLON);
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

        bool created = db->createTable(tableName);
        if(!created) throw RuntimeException(RuntimeError::TABLE_ALREADY_EXISTS, current.getLine(), current.getColumn(), tableName, db->getName());

        Table* table = db->getTable(tableName);

        consume(TokenType::TOK_LPAREN);

        std::vector<std::string> columnNames;
        while(!match(TokenType::TOK_RPAREN)) {
            std::string columnName = current.getLexeme();

            for(int i = 0; i < static_cast<int>(columnNames.size()); i++) {
                if(columnName == columnNames[i]) {
                    throw RuntimeException(RuntimeError::DUPLICATE_COLUMN_NAME, current.getLine(), current.getColumn(), columnName, tableName);
                }
            }

            columnNames.push_back(columnName);

            consume(TokenType::TOK_IDENTIFIER);

            DataType type = parseDataType();
            table->addColumn(Column(columnName, type));

            if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
        }

        consume(TokenType::TOK_RPAREN);
        consume(TokenType::TOK_SEMICOLON);
    }
    else throw SyntaxException(SyntaxError::UNKNOWN_CREATE_KEYWORD, current.getLine(), current.getColumn(), current.getLexeme(), "");
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
    else throw SyntaxException(SyntaxError::UNKNOWN_DROP_KEYWORD, current.getLine(), current.getColumn(), current.getLexeme(), "");
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
    else throw SyntaxException(SyntaxError::UNKNOWN_SHOW_KEYWORD, current.getLine(), current.getColumn(), current.getLexeme(), "");
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
        throw RuntimeException(RuntimeError::TABLE_NOT_FOUND, current.getLine(), current.getColumn(), tableName, db->getName());
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

void Parser::parseOrderBy(Table* table, std::vector<int> whereRows, std::vector<int> columnIndexes, std::vector<std::string> aliasNames, bool distinct) {
    consume(TokenType::TOK_BY);

    std::vector<Row> rows = table->selectAll();

    std::string colName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    int colNumber = getColumnIndex(table, colName);

    if(colNumber == -1) throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), colName, table->getName());

    TokenType order = TokenType::TOK_ASC;
    if(match(TokenType::TOK_ASC) || match(TokenType::TOK_DESC)) {
        order = current.getType();
        consume(order);
    }

    std::vector<int> sortedRows;
    if(!whereRows.empty()) sortedRows = whereRows;
    else {
        for (int i = 0; i < (int)rows.size(); i++) sortedRows.push_back(i);
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

    if(columnIndexes.empty()) {
        for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);
    }

    printTableResult(table, columnIndexes, sortedRows, aliasNames, distinct);

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

        ValidationResult result = table->insertRow(row);

        if(result == ValidationResult::COUNT_MISMATCH) {
            throw RuntimeException(RuntimeError::COLUMN_COUNT_MISMATCH, current.getLine(), current.getColumn(), std::to_string(row.getCellCount()), std::to_string(table->getColumnCount()));
        }
        else if(result == ValidationResult::TYPE_MISMATCH) {
            throw RuntimeException(RuntimeError::INSERT_TYPE_MISMATCH, current.getLine(), current.getColumn(), tableName, "");
        }

        if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
        else break;
    }

    consume(TokenType::TOK_SEMICOLON);
}

void Parser::parseSelect(DatabaseManager& manager) {
    consume(TokenType::TOK_SELECT);

    if(match(TokenType::TOK_COUNT) || match(TokenType::TOK_AVG) || match(TokenType::TOK_SUM) || match(TokenType::TOK_MIN) || match(TokenType::TOK_MAX)) {
        parseAggregateSelect(manager, current.getType());

        return;
    }

    bool distinct = false;
    if(match(TokenType::TOK_DISTINCT)) {
        consume(TokenType::TOK_DISTINCT);
        distinct = true;
    }

    std::vector<std::string> selectedColumns;
    bool selectAll = false;
    
    std::vector<std::string> aliasColumnNames;
    bool alias = false;

    if(match(TokenType::TOK_STAR)) {
        consume(TokenType::TOK_STAR);
        selectAll = true;
    }
    else {
        while(true) {
            std::string colName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);
            selectedColumns.push_back(colName);

            if(match(TokenType::TOK_AS)) {
                consume(TokenType::TOK_AS);
                alias = true;

                std::string aliasName = current.getLexeme();
                consume(TokenType::TOK_IDENTIFIER);
                aliasColumnNames.push_back(aliasName);
            }

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

    if(match(TokenType::TOK_JOIN) || match(TokenType::TOK_INNER)) {
        parseInnerJoin(db, table, selectedColumns, selectAll);
        return;
    }

    if(match(TokenType::TOK_LEFT)) {
        parseLeftJoin(db, table, selectedColumns, selectAll);
        return;
    }

    if(match(TokenType::TOK_RIGHT)) {
        parseRightJoin(db, table, selectedColumns, selectAll);
        return;
    }

    if(match(TokenType::TOK_FULL)) {
        parseOutterJoin(db, table, selectedColumns, selectAll);
        return;
    }

    std::vector<int> whereRows;
    if(match(TokenType::TOK_WHERE)) whereRows = parseWhereClause(table);

    if(match(TokenType::TOK_ORDER)) {
        consume(TokenType::TOK_ORDER);

        std::vector<int> columnIndexes;
        if(selectAll) for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);
        else {
            for(const std::string& colName : selectedColumns) {
                columnIndexes.push_back(getColumnIndex(table, colName));
            }
        }

        if(match(TokenType::TOK_BY)) parseOrderBy(table, whereRows, columnIndexes, aliasColumnNames, distinct);
        return;
    }
    
    consume(TokenType::TOK_SEMICOLON);

    std::vector<int> columnIndexes;

    if(selectAll) for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);
    else {
        for(const std::string& colName : selectedColumns) {

            int colIndex = getColumnIndex(table, colName);

            if(colIndex == -1) {
                throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), colName, table->getName()); 
            }

            columnIndexes.push_back(colIndex);
        } 
    }

    printTableResult(table, columnIndexes, whereRows, alias ? aliasColumnNames : std::vector<std::string>{}, distinct);
}

void Parser::parseInnerJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll) {

    if(match(TokenType::TOK_JOIN)) consume(TokenType::TOK_JOIN);
    else if(match(TokenType::TOK_INNER)) {
        consume(TokenType::TOK_INNER);
        consume(TokenType::TOK_JOIN);
    }

    std::string table2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table2 = db->getTable(table2Name);
    checkNotNull(table2, table2Name);

    consume(TokenType::TOK_ON);

    std::string t1Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col1 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    consume(TokenType::TOK_EQUAL);

    std::string t2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col2 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    if(t1Name != table1->getName() || t2Name != table2Name) {
        std::cerr << "Table name mismatch in JOIN\n";
        return;
    }

    int col1Index = getColumnIndex(table1, col1);
    int col2Index = getColumnIndex(table2, col2);

    if(col1Index == -1 || col2Index == -1) {
        std::cerr << "Column not found in JOIN\n";
        return;
    }

    std::vector<Row> rows1 = table1->selectAll();
    std::vector<Row> rows2 = table2->selectAll();

    Table tempTable("result");
    Table* result = &tempTable;

    for(int i = 0; i < table1->getColumnCount(); i++) {
        result->addColumn(table1->getColumn(i));
    }

    for(int i = 0; i < table2->getColumnCount(); i++) {
        result->addColumn(table2->getColumn(i));
    }

    for(int i = 0; i < rows1.size(); i++) {
        for(int j = 0; j < rows2.size(); j++) {

            if(rows1[i].getCell(col1Index) == rows2[j].getCell(col2Index)) {

                Row newRow;

                for(int c = 0; c < table1->getColumnCount(); c++) {
                    newRow.addCell(rows1[i].getCell(c));
                }

                for(int c = 0; c < table2->getColumnCount(); c++) {
                    newRow.addCell(rows2[j].getCell(c));
                }

                result->insertRow(newRow);
            }
        }
    }

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < result->getColumnCount(); i++) {
            columnIndexes.push_back(i);
        }
    }
    else {
        for(const std::string& colName : selectedColumns) {

            int idx = getColumnIndex(result, colName);

            if(idx == -1) {
                std::cerr << "Column " << colName << " not found\n";
                return;
            }

            columnIndexes.push_back(idx);
        }
    }

    printTableResult(result, columnIndexes, {}, {}, false);
}

void Parser::parseLeftJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll) {
    consume(TokenType::TOK_LEFT);
    consume(TokenType::TOK_JOIN);

    std::string table2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table2 = db->getTable(table2Name);
    checkNotNull(table2, table2Name);

    consume(TokenType::TOK_ON);

    std::string t1Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col1 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    
    consume(TokenType::TOK_EQUAL);

    std::string t2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col2 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    if(t1Name != table1->getName() || t2Name != table2Name) {
        std::cerr << "Table name mismatch in LEFT JOIN\n";
        return;
    }

    int col1Index = getColumnIndex(table1, col1);
    int col2Index = getColumnIndex(table2, col2);

    if(col1Index == -1 || col2Index == -1) {
        std::cerr << "Column not found in LEFT JOIN\n";
        return;
    }

    std::vector<Row> rows1 = table1->selectAll();
    std::vector<Row> rows2 = table2->selectAll();

    Table tempTable("result");
    Table* result = &tempTable;

    for(int i = 0; i < table1->getColumnCount(); i++) result->addColumn(table1->getColumn(i));

    for(int i = 0; i < table2->getColumnCount(); i++) result->addColumn(table2->getColumn(i));

    for(int i = 0; i < (int)rows1.size(); i++) {
        bool matched = false;

        for(int j = 0; j < (int)rows2.size(); j++) {
            if(rows1[i].getCell(col1Index) == rows2[j].getCell(col2Index)) {

                Row newRow;

                for(int c = 0; c < table1->getColumnCount(); c++) {
                    newRow.addCell(rows1[i].getCell(c));
                }

                for(int c = 0; c < table2->getColumnCount(); c++) {
                    newRow.addCell(rows2[j].getCell(c));
                }

                result->insertRow(newRow);

                matched = true;
            }
        }

        if(!matched) {
            Row newRow;

            for(int c = 0; c < table1->getColumnCount(); c++) {
                newRow.addCell(rows1[i].getCell(c));
            }

            for(int c = 0; c < table2->getColumnCount(); c++) {
                newRow.addCell(Cell());
            }

            result->insertRow(newRow);
        }
    }

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < result->getColumnCount(); i++) {
            columnIndexes.push_back(i);
        }
    }
    else {
        for(const std::string& colName : selectedColumns) {

            int idx = getColumnIndex(result, colName);

            if(idx == -1) {
                std::cerr << "Column " << colName << " not found\n";
                return;
            }

            columnIndexes.push_back(idx);
        }
    }

    printTableResult(result, columnIndexes, {}, {}, false);
}

void Parser::parseRightJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll) {
    consume(TokenType::TOK_RIGHT);
    consume(TokenType::TOK_JOIN);

    std::string table2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table2 = db->getTable(table2Name);
    checkNotNull(table2, table2Name);

    consume(TokenType::TOK_ON);

    std::string t1Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col1 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    consume(TokenType::TOK_EQUAL);

    std::string t2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col2 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    if(t1Name != table1->getName() || t2Name != table2Name) {
        std::cerr << "Table name mismatch in RIGHT JOIN\n";
        return;
    }

    int col1Index = getColumnIndex(table1, col1);
    int col2Index = getColumnIndex(table2, col2);

    if(col1Index == -1 || col2Index == -1) {
        std::cerr << "Column not found in RIGHT JOIN\n";
        return;
    }

    std::vector<Row> rows1 = table1->selectAll();
    std::vector<Row> rows2 = table2->selectAll();

    Table tempTable("result");
    Table* result = &tempTable;

    for(int i = 0; i < table1->getColumnCount(); i++) result->addColumn(table1->getColumn(i));

    for(int i = 0; i < table2->getColumnCount(); i++) result->addColumn(table2->getColumn(i));

    for(int i = 0; i < (int)rows2.size(); i++) {
        bool matched = false;

        for(int j = 0; j < (int)rows1.size(); j++) {
            if(rows1[j].getCell(col1Index) == rows2[i].getCell(col2Index)) {

                Row newRow;
                
                for(int c = 0; c < table1->getColumnCount(); c++) newRow.addCell(rows1[j].getCell(c));

                for(int c = 0; c < table2->getColumnCount(); c++) newRow.addCell(rows2[i].getCell(c));

                result->insertRow(newRow);
                matched = true;
            }
        }

        if(!matched) {
            Row newRow;

            for(int c = 0; c < table1->getColumnCount(); c++) newRow.addCell(Cell());

            for(int c = 0; c < table2->getColumnCount(); c++) newRow.addCell(rows2[i].getCell(c));

            result->insertRow(newRow);
        }
    }

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < result->getColumnCount(); i++) {
            columnIndexes.push_back(i);
        }
    }
    else {
        for(const std::string& colName : selectedColumns) {

            int idx = getColumnIndex(result, colName);

            if(idx == -1) {
                std::cerr << "Column " << colName << " not found\n";
                return;
            }

            columnIndexes.push_back(idx);
        }
    }

    printTableResult(result, columnIndexes, {}, {}, false);
}

void Parser::parseOutterJoin(Database* db, Table* table1, const std::vector<std::string>& selectedColumns, bool selectAll) {
    consume(TokenType::TOK_FULL);
    if(match(TokenType::TOK_OUTER)) consume(TokenType::TOK_OUTER);
    consume(TokenType::TOK_JOIN);

    std::string table2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table2 = db->getTable(table2Name);

    consume(TokenType::TOK_ON);

    std::string t1Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col1 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    consume(TokenType::TOK_EQUAL);

    std::string t2Name = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_DOT);

    std::string col2 = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    if(t1Name != table1->getName() || t2Name != table2Name) {
        std::cerr << "Table name mismatch in RIGHT JOIN\n";
        return;
    }

    int col1Index = getColumnIndex(table1, col1);
    int col2Index = getColumnIndex(table2, col2);

    if(col1Index == -1 || col2Index == -1) {
        std::cerr << "Column not found in RIGHT JOIN\n";
        return;
    }

    std::vector<Row> rows1 = table1->selectAll();
    std::vector<Row> rows2 = table2->selectAll();

    Table tempTable("result");
    Table* result = &tempTable;

    for(int i = 0; i < table1->getColumnCount(); i++) result->addColumn(table1->getColumn(i));

    for(int i = 0; i < table2->getColumnCount(); i++) result->addColumn(table2->getColumn(i));

    std::vector<bool> leftMatched(rows1.size(), false);

    for(int i = 0; i < (int)rows2.size(); i++) {
        bool matched = false;

        for(int j = 0; j < (int)rows1.size(); j++) {
            if(rows1[j].getCell(col1Index) == rows2[i].getCell(col2Index)) {

                Row newRow;

                for(int c = 0; c < table1->getColumnCount(); c++) newRow.addCell(rows1[j].getCell(c));

                for(int c = 0; c < table2->getColumnCount(); c++) newRow.addCell(rows2[i].getCell(c));

                result->insertRow(newRow);

                matched = true;
                leftMatched[j] = true;
            }
        }

        if(!matched) {
            Row newRow;
            for(int c = 0; c < table1->getColumnCount(); c++) newRow.addCell(Cell());

            for(int c = 0; c < table2->getColumnCount(); c++) newRow.addCell(rows2[i].getCell(c));

            result->insertRow(newRow);
        }
    }

    for(int j = 0; j < (int)rows1.size(); j++) {
        if(!leftMatched[j]) {

            Row newRow;
            for(int c = 0; c < table1->getColumnCount(); c++) newRow.addCell(rows1[j].getCell(c));

            for(int c = 0; c < table2->getColumnCount(); c++) newRow.addCell(Cell());

            result->insertRow(newRow);
        }
    }

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < result->getColumnCount(); i++) {
            columnIndexes.push_back(i);
        }
    }
    else {
        for(const std::string& colName : selectedColumns) {

            int idx = getColumnIndex(result, colName);

            if(idx == -1) {
                std::cerr << "Column " << colName << " not found\n";
                return;
            }

            columnIndexes.push_back(idx);
        }
    }

    printTableResult(result, columnIndexes, {}, {}, false);
}


void Parser::parseAggregateSelect(DatabaseManager& manager, TokenType aggregator) {
    consume(aggregator);

    int count = 0;
    std::string selectedColumn;
    int selectedColumnIndex = -1;

    switch(aggregator) {
        case TokenType::TOK_COUNT : {
            consume(TokenType::TOK_LPAREN);

            bool selectAll = false;

            if(match(TokenType::TOK_STAR)) {
                selectAll = true;
                consume(TokenType::TOK_STAR);
            }
            else {
                selectedColumn = current.getLexeme();
                consume(TokenType::TOK_IDENTIFIER);
            }
            consume(TokenType::TOK_RPAREN);

            consume(TokenType::TOK_FROM);

            Database* db = manager.getCurrentDatabase();
            if(!db) return;

            std::string tableName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            Table* table = db->getTable(tableName);
            checkNotNull(table, tableName);

            std::vector<int> whereRows;
            if(match(TokenType::TOK_WHERE)) whereRows = parseWhereClause(table);

            consume(TokenType::TOK_SEMICOLON);

            if(selectAll) {
                if(whereRows.empty()) count = table->getRowCount();
                else count = whereRows.size();

                printAggregateResult("COUNT(*)", std::to_string(count));
            }
            else {
                std::vector<Row> rows = table->selectAll();

                selectedColumnIndex = getColumnIndex(table, selectedColumn);

                if(whereRows.empty()) for(const Row& row : rows) if(!row.getCell(selectedColumnIndex).isNullValue()) count++;
                else for(int idx : whereRows) if(!rows[idx].getCell(selectedColumnIndex).isNullValue()) count++;

                std::string printColumnName = "COUNT(" + selectedColumn + ")";
                printAggregateResult(printColumnName, std::to_string(count));
            }

            break;
        }
        case TokenType::TOK_AVG : {
            consume(TokenType::TOK_LPAREN);

            std::string selectedColumn = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            consume(TokenType::TOK_RPAREN);
            consume(TokenType::TOK_FROM);

            Database* db = manager.getCurrentDatabase();
            if(!db) return;

            std::string tableName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            Table* table = db->getTable(tableName);
            checkNotNull(table, tableName);

            int selectedColumnIndex = getColumnIndex(table, selectedColumn);

            std::vector<int> whereRows;
            if(match(TokenType::TOK_WHERE)) whereRows = parseWhereClause(table);

            consume(TokenType::TOK_SEMICOLON);

            std::vector<Row> rows = table->selectAll();

            double sum = 0.0;

            if(whereRows.empty()) {
                for(const Row& row : rows) {
                    Cell cell = row.getCell(selectedColumnIndex);

                    if(cell.isNullValue()) continue;

                    if(cell.isInt()) sum += cell.getInt();
                    else if(cell.isDouble()) sum += cell.getDouble();

                    count++;
                }
            }
            else {
                for(int idx : whereRows) {
                    Cell cell = rows[idx].getCell(selectedColumnIndex);

                    if(cell.isNullValue()) continue;

                    if(cell.isInt()) sum += cell.getInt();
                    else if(cell.isDouble()) sum += cell.getDouble();

                    count++;
                }
            }

            double avg = (count == 0) ? 0 : sum / count;

            std::string printColumnName = "AVG(" + selectedColumn + ")";
            printAggregateResult(printColumnName, std::to_string(avg));

            break;
        }
        case TokenType::TOK_SUM : {
            consume(TokenType::TOK_LPAREN);

            std::string selectedColumn = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);
            
            consume(TokenType::TOK_RPAREN);
            consume(TokenType::TOK_FROM);

            Database* db = manager.getCurrentDatabase();
            if(!db) return;

            std::string tableName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            Table* table = db->getTable(tableName);
            checkNotNull(table, tableName);

            int selectedColumnIndex = getColumnIndex(table, selectedColumn);

            std::vector<int> whereRows;
            if(match(TokenType::TOK_WHERE)) whereRows = parseWhereClause(table);

            consume(TokenType::TOK_SEMICOLON);

            std::vector<Row> rows = table->selectAll();

            double sum = 0.0;
            if(whereRows.empty()) {
                for(const Row& row : rows) {
                    Cell cell = row.getCell(selectedColumnIndex);

                    if(cell.isNullValue()) continue;

                    if(cell.isInt()) sum += cell.getInt();
                    else if(cell.isDouble()) sum += cell.getDouble();
                }
            }
            else {
                for(int idx : whereRows) {
                    Cell cell = rows[idx].getCell(selectedColumnIndex);

                    if(cell.isNullValue()) continue;

                    if(cell.isInt()) sum += cell.getInt();
                    else if(cell.isDouble()) sum += cell.getDouble();
                }
            }

            std::string printColumnName = "SUM(" + selectedColumn + ")";
            printAggregateResult(printColumnName, std::to_string(sum));

            break;
        }
        case TokenType::TOK_MIN : {
            consume(TokenType::TOK_LPAREN);

            std::string selectedColumn = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            consume(TokenType::TOK_RPAREN);
            consume(TokenType::TOK_FROM);

            Database* db = manager.getCurrentDatabase();
            if(!db) return;

            std::string tableName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            Table* table = db->getTable(tableName);
            checkNotNull(table, tableName);

            int colIndex = getColumnIndex(table, selectedColumn);

            std::vector<int> whereRows;
            if(match(TokenType::TOK_WHERE)) whereRows = parseWhereClause(table);

            consume(TokenType::TOK_SEMICOLON);

            std::vector<Row> rows = table->selectAll();

            bool found = false;
            Cell minVal;

            if(whereRows.empty()) {
                for(const Row& row : rows) {
                    Cell cell = row.getCell(colIndex);

                    if(cell.isNullValue()) continue;

                    if(!found || cell < minVal) {
                        minVal = cell;
                        found = true;
                    }
                }
            }
            else {
                for(int idx : whereRows) {
                    Cell cell = rows[idx].getCell(colIndex);

                    if(cell.isNullValue()) continue;

                    if(!found || cell < minVal) {
                        minVal = cell;
                        found = true;
                    }
                }
            }

            std::string printColumnName = "MIN(" + selectedColumn + ")";

            if(found) printAggregateResult(printColumnName, minVal.toString());
            else printAggregateResult(printColumnName, "NULL");

            break;
        }
        case TokenType::TOK_MAX : {
            consume(TokenType::TOK_LPAREN);

            std::string selectedColumn = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            consume(TokenType::TOK_RPAREN);
            consume(TokenType::TOK_FROM);

            Database* db = manager.getCurrentDatabase();
            if(!db) return;

            std::string tableName = current.getLexeme();
            consume(TokenType::TOK_IDENTIFIER);

            Table* table = db->getTable(tableName);
            checkNotNull(table, tableName);

            int colIndex = getColumnIndex(table, selectedColumn);

            std::vector<int> whereRows;
            if(match(TokenType::TOK_WHERE)) whereRows = parseWhereClause(table);

            consume(TokenType::TOK_SEMICOLON);

            std::vector<Row> rows = table->selectAll();

            bool found = false;
            Cell maxVal;

            if(whereRows.empty()) {
                for(const Row& row : rows) {
                    Cell cell = row.getCell(colIndex);

                    if(cell.isNullValue()) continue;

                    if(!found || cell > maxVal) {
                        maxVal = cell;
                        found = true;
                    }
                }
            }
            else {
                for(int idx : whereRows) {
                    Cell cell = rows[idx].getCell(colIndex);

                    if(cell.isNullValue()) continue;

                    if(!found || cell > maxVal) {
                        maxVal = cell;
                        found = true;
                    }
                }
            }

            std::string printColumnName = "MAX(" + selectedColumn + ")";

            if(found) printAggregateResult(printColumnName, maxVal.toString());
            else printAggregateResult(printColumnName, "NULL");

            break;
        }
    }
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
            throw RuntimeException(RuntimeError::COLUMN_NOT_FOUND, current.getLine(), current.getColumn(), columnNames[columnCounter], table->getName());
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

        table->clearRows();
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

void Parser::parseTruncate(DatabaseManager& manager) {
    consume(TokenType::TOK_TRUNCATE);

    consume(TokenType::TOK_TABLE);

    std::string tableName = current.getLexeme();

    consume(TokenType::TOK_IDENTIFIER);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    Table* table = db->getTable(tableName);

    table->clearRows();

    std::cout << "Cleared All the rows from table " << tableName << "\n";

    consume(TokenType::TOK_SEMICOLON);
}

void Parser::parseRename(DatabaseManager& manager) {
    consume(TokenType::TOK_RENAME);

    if(match(TokenType::TOK_TABLE)) {
        consume(TokenType::TOK_TABLE);

        std::string currentTableName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        consume(TokenType::TOK_TO);

        std::string newTableName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);
        consume(TokenType::TOK_SEMICOLON);

        Database* db = manager.getCurrentDatabase();
        if(!db) return;

        Table* table = db->getTable(currentTableName);
        if(!table) {       
            std::cerr << "Table not found\n";
            return;
        }

        std::string base = "./databases/" + db->getName();

        std::string oldStruct = base + "/tables/" + currentTableName + ".tbl";
        std::string oldData   = base + "/data/"   + currentTableName + ".dat";

        std::string newStruct = base + "/tables/" + newTableName + ".tbl";
        std::string newData   = base + "/data/"   + newTableName + ".dat";

        if(!std::filesystem::exists(oldStruct)) {
            table->saveStructureToFile();
        }
        if(!std::filesystem::exists(oldData)) {
            table->saveDataToFile();
        }

        if(std::filesystem::exists(oldStruct)) std::filesystem::rename(oldStruct, newStruct);

        if(std::filesystem::exists(oldData)) std::filesystem::rename(oldData, newData);

        table->setName(newTableName);
        table->setStructurePath(newStruct);
        table->setDataPath(newData);

        table->saveStructureToFile();

        std::cout << "Table renamed successfully\n";
    }
    else if(match(TokenType::TOK_COLUMN)) {
        consume(TokenType::TOK_COLUMN); 

        std::string currentColumnName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        consume(TokenType::TOK_TO);

        std::string newColumnName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        consume(TokenType::TOK_FROM);

        std::string tableName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        Database* db = manager.getCurrentDatabase();
        if(!db) return;

        Table* table = db->getTable(tableName);
        if(!table) {
            std::cerr << "Table not found\n";
            return;
        }

        int columnIndex = getColumnIndex(table, currentColumnName);
        if(columnIndex == -1) {
            std::cerr << "Column not found\n";
            return;
        }

        for(int i = 0; i < table->getColumnCount(); i++) {
            if(table->getColumName(i) == newColumnName) {
                std::cerr << "Column already exists\n";
                return;
            }
        }

        table->getColumn(columnIndex).setName(newColumnName);

        table->saveStructureToFile();

        consume(TokenType::TOK_SEMICOLON);

        std::cout << "Column renamed successfully\n";
    }
}