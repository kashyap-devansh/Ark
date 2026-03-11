#include "parser.h"

#include <iostream>

void printBorder(const std::vector<int>& widths, const std::vector<int>& columnIndexes) {
    for(int i = 0; i < columnIndexes.size(); i++) {
        std::cout << "+-";
        for(int j = 0; j < widths[i]; j++) std::cout << "-";
        std::cout << "-";
    }
    std::cout << "+\n";
}

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
    if(!match(expected)) {
        std::cerr << "Syntax error at line\n" << current.getLine() << ", column " << current.getColumn() << "\n";
        exit(1);
    }
    advance();
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

    std::cerr << "Invalid data type.\n";
    exit(1);
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

    std::cerr << "Invalid value.\n";
    exit(1);
}

int Parser::getColumnIndex(Table* table, const std::string& columnName) {
    for(int i = 0; i < table->getColumnCount(); i++) {
        if(table->getColumName(i) == columnName) return i;
    }

    return -1;
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

        default : std::cerr << "Unknown command.\n";
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
        std::cerr << "Table not found: " << tableName << "\n";
        return;
    }

    std::vector<int> columnIndexes;
    for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);

    std::vector<int> widths;
    for(int i = 0; i < columnIndexes.size(); i++) {
        widths.push_back(table->getColumName(columnIndexes[i]).length());
    }

    for(int i = 0; i < columnIndexes.size(); i++) {
        switch(table->getColumnType(columnIndexes[i])) {
            case DataType::INT : if(3 > widths[i]) widths.at(i) = 3; break;
            case DataType::DOUBLE : if(6 > widths[i]) widths.at(i) = 6; break;
            case DataType::STRING : if(6 > widths[i]) widths.at(i) = 6; break;
            case DataType::BOOL : if(4 > widths[i]) widths.at(i) = 4; break;
            case DataType::NONE : if(4 > widths[i]) widths.at(i) = 4; break; 
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
            case DataType::INT : std::cout << " " << "INT"; typeLength = 3; break;
            case DataType::DOUBLE : std::cout << " " << "DOUBLE"; typeLength = 6; break;
            case DataType::STRING : std::cout << " " << "STRING"; typeLength = 6; break;
            case DataType::BOOL : std::cout << " " << "BOOL"; typeLength = 4; break;
            case DataType::NONE : std::cout << " " << "NONE"; typeLength = 4; break;
        }

        for(int j = typeLength; j < widths[i]; j++) std::cout << " ";
        std::cout << " |";
    }
    
    std::cout << "\n";

    printBorder(widths, columnIndexes);
}

void Parser::parseInsert(DatabaseManager& manager) {
    consume(TokenType::TOK_INSERT);
    consume(TokenType::TOK_INTO);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);

    if(!table) {
        std::cerr << "No table found with name : " << tableName << "\n";
        exit(1);
    }

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
    if(!table) {
        std::cerr << "Table not found: " << tableName << "\n";
        return;
    }

    int printWhere = 0;
    std::vector<int> whereColumns;

    if(match(TokenType::TOK_WHERE)) {
        consume(TokenType::TOK_WHERE);

        std::string colName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        int colNumber;
        bool colFound = false;

        for(int i = 0; i < table->getColumnCount(); i++) {
            if(table->getColumName(i) == colName) {
                colNumber = i;
                colFound = true;
                break;
            }
        }

        if(!colFound) {
            std::cerr << "no column found with name : " << colName << std::endl;
            exit(1);
        }

        if(match(TokenType::TOK_LIKE)) {
            consume(TokenType::TOK_LIKE);

            if(!(table->getColumnType(colNumber) == DataType::STRING)) {
                std::cerr << "To use \"LIKE\" the column must be of type string\n";
                exit(1);
            }

            bool isString = match(TokenType::TOK_STRING);

            if(!isString) {
                std::cerr << "To select like the given values should be string \n";
                exit(1);
            }

            std::string wordLike = current.getLexeme();
            consume(TokenType::TOK_STRING);

            if(!(wordLike[1] == '%')) {
                std::cerr << "Invalid syntax\n";
                exit(1);
            }

            char findWordWith = wordLike[0];
            std::vector<Row> rows = table->selectAll();
            bool found = false;

            for(int i = 0; i < table->getRowCount(); i++) {
                Cell val = rows[i].getCell(colNumber);
                char firstLetter = val.getString()[0];

                if(findWordWith == firstLetter) {
                    found = true;
                    printWhere = 1;

                    whereColumns.push_back(i);
                }
            }
        }

        bool matched = (match(TokenType::TOK_EQUAL_EQUAL) || match(TokenType::TOK_NOT_EQUAL) || match(TokenType::TOK_GREATER_EQUAL) || match(TokenType::TOK_LESS_EQUAL) || match(TokenType::TOK_GREATER) || match(TokenType::TOK_LESS));

        if(matched) {
            TokenType op = current.getType();
            consume(op);

            Cell compVal = parseValue();

            auto rows = table->selectAll();

            for(int i = 0; i <table->getRowCount(); i++) {
                Cell val = rows[i].getCell(colNumber);

                bool condition = false;

                switch(op) {
                    case TokenType::TOK_EQUAL_EQUAL : condition = (val == compVal); break;
                    case TokenType::TOK_NOT_EQUAL : condition = (val != compVal); break;
                    case TokenType::TOK_GREATER : condition = (val > compVal); break;
                    case TokenType::TOK_LESS : condition = (val < compVal); break;
                    case TokenType::TOK_GREATER_EQUAL : condition = (val >= compVal); break;
                    case TokenType::TOK_LESS_EQUAL : condition = (val <= compVal); break;
                    default : break;
                }

                if(condition) {
                    printWhere = 1;
                    whereColumns.push_back(i);
                }
            }
        }
    }

    consume(TokenType::TOK_SEMICOLON);

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < table->getColumnCount(); i++) columnIndexes.push_back(i);
    }
    else {
        for(const std::string& colName : selectedColumns) {
            bool found = false;

            for(int i = 0; i < table->getColumnCount(); i++) {
                if(table->getColumName(i) == colName) {
                    columnIndexes.push_back(i);
                    found = true;
                    break;
                }
            }

            if(!found) {
                std::cerr << "Column not found: " << colName << "\n";
                return;
            }
        } 
    }

    std::vector<int> widths;
    for(int i = 0; i < columnIndexes.size(); i++) {
        widths.push_back(table->getColumName(columnIndexes.at(i)).length());
    }

    for(const Row& row : table->selectAll()) {
        for(int i = 0; i < columnIndexes.size(); i++) {
            int cellWidth = row.getCell(columnIndexes[i]).toString().length();

            if(cellWidth > widths[i]) widths[i] = cellWidth;
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

    for(int r = 0; r < table->getRowCount(); r++) {

        if(printWhere) {
            bool allowed = false;
            for(int idx : whereColumns) {
                if(idx == r) {
                    allowed = true;
                    break;
                }
            }
            if(!allowed) continue;
        }

        const Row row = table->selectAll()[r];

        std::cout << "|";

        for(int i = 0; i < columnIndexes.size(); i++) {
            std::string val = row.getCell(columnIndexes[i]).toString();
            std::cout << " " << val;

            for(int j = val.length(); j < widths[i]; j++) std::cout << " ";
            std::cout << " |";
        }

        std::cout << "\n";
    }

    printBorder(widths, columnIndexes);
}


















































































void Parser::parseUpdate(DatabaseManager& manager) {
    consume(TokenType::TOK_UPDATE);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);

    if(!table) {
        std::cerr << "No table found with name : " << tableName << "\n";
        exit(1);
    }

    consume(TokenType::TOK_SET);

    std::vector<std::string> columnNames;
    std::vector<int> updateColumnIndexes;
    std::vector<Cell> newValues;
    int columnCounter = 0;

    while(!(match(TokenType::TOK_WHERE) || match(TokenType::TOK_SEMICOLON))) {
        columnNames.push_back(current.getLexeme());
        updateColumnIndexes.push_back(getColumnIndex(table, columnNames[columnCounter]));
        columnCounter++;

        consume(TokenType::TOK_IDENTIFIER);
        consume(TokenType::TOK_EQUAL);

        newValues.push_back(parseValue());

        for(int i = 0; i < updateColumnIndexes.size(); i++) {
            if(updateColumnIndexes[i] == -1) {
                std::cerr << "No column found with name: " << columnNames[i] << "\n";
                return;
            }
        }

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
        consume(TokenType::TOK_WHERE);

        std::string colName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        int colNumber;
        bool colFound = false;

        for(int i = 0; i < table->getColumnCount(); i++) {
            if(table->getColumName(i) == colName) {
                colNumber = i;
                colFound = true;
                break;
            }
        }

        if(!colFound) {
            std::cerr << "no column found with name : " << colName << std::endl;
            exit(1);
        }

        if(match(TokenType::TOK_LIKE)) {
            if(!(table->getColumnType(colNumber) == DataType::STRING)) {
                std::cerr << "To use \"LIKE\" the column must be of type string\n";
                return;
            } 

            consume(TokenType::TOK_LIKE);

            bool isString = match(TokenType::TOK_STRING);

            if(!isString) {
                std::cerr << "To update using like the given value should be string\n";
                return;
            }

            std::string wordLike = current.getLexeme();
            consume(TokenType::TOK_STRING);

            if(!(wordLike[1] == '%')) {
                std::cerr << "Invalid syntax\n";
                return;
            }

            char findWordWith = wordLike[0];
            std::vector<Row> rows = table->selectAll();
            bool found = false;

            for(int i = 0; i < rows.size(); i++) {
                Cell val = rows[i].getCell(colNumber);
                char firstLetter = val.getString()[0];

                if(findWordWith == firstLetter) {
                    found = true;

                    for(int j = 0; j < updateColumnIndexes.size(); j++) {
                        table->updateCell(i, updateColumnIndexes[j], newValues[j]);
                    }
                }
            }

            consume(TokenType::TOK_SEMICOLON);

            return;
        }

        bool matched = ( match(TokenType::TOK_EQUAL_EQUAL) || match(TokenType::TOK_NOT_EQUAL) || match(TokenType::TOK_GREATER_EQUAL) || match(TokenType::TOK_LESS_EQUAL) || match(TokenType::TOK_GREATER) || match(TokenType::TOK_LESS));

        if(matched) {

            TokenType op = current.getType();
            consume(op);

            Cell compVal = parseValue();

            auto rows = table->selectAll();

            if(match(TokenType::TOK_LIMIT)) {
                consume(TokenType::TOK_LIMIT);

                Cell climit = parseValue();
                
                if(!climit.isInt()) {
                    std::cerr << "Limit value must be INT\n";
                    return;
                }

                int limit = climit.getInt();

                if(limit < 0) {
                    std::cerr << "Limit cannot be negative\n";
                    return;
                }

                int deleted = 0;

                for(int i = 0; i < rows.size(); i++) {

                    Cell val = rows[i].getCell(colNumber);

                    bool condition = false;

                    switch(op) {
                        case TokenType::TOK_EQUAL_EQUAL : condition = (val == compVal); break;
                        case TokenType::TOK_NOT_EQUAL : condition = (val != compVal); break;
                        case TokenType::TOK_GREATER : condition = (val > compVal); break;
                        case TokenType::TOK_LESS : condition = (val < compVal); break;
                        case TokenType::TOK_GREATER_EQUAL : condition = (val >= compVal); break;
                        case TokenType::TOK_LESS_EQUAL : condition = (val <= compVal); break;
                        default : break;
                    }

                    if(condition) {
                        for(int j = 0; j < updateColumnIndexes.size(); j++) {
                            table->updateCell(i, updateColumnIndexes[j], newValues[j]);

                            deleted++;

                            if(deleted == limit) {
                                consume(TokenType::TOK_SEMICOLON);
                                return; 
                            }
                        }
                    }
                }
            }
            else {

                for(int i = 0; i < rows.size(); i++) {

                    Cell val = rows[i].getCell(colNumber);

                    bool condition = false;

                    switch(op) {

                        case TokenType::TOK_EQUAL_EQUAL : condition = (val == compVal); break;
                        case TokenType::TOK_NOT_EQUAL : condition = (val != compVal); break;
                        case TokenType::TOK_GREATER : condition = (val > compVal); break;
                        case TokenType::TOK_LESS : condition = (val < compVal); break;
                        case TokenType::TOK_GREATER_EQUAL : condition = (val >= compVal); break;
                        case TokenType::TOK_LESS_EQUAL : condition = (val <= compVal); break;
                        default : break;
                    }

                    if(condition) {
                        for(int j = 0; j < updateColumnIndexes.size(); j++) {
                            table->updateCell(i, updateColumnIndexes[j], newValues[j]);
                        }
                    }
                } 
            }
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

    if(match(TokenType::TOK_SEMICOLON)) {
        consume(TokenType::TOK_SEMICOLON);

        while(!table->selectAll().empty()) table->deleteRow(0);
    }
    else if(match(TokenType::TOK_WHERE)) {
        consume(TokenType::TOK_WHERE);

        std::string columnName = current.getLexeme();
        consume(TokenType::TOK_IDENTIFIER);

        int colNumber;
        bool colFound = false;

        for(int i = 0; i < table->getColumnCount(); i++) {
            if(table->getColumName(i) == columnName) {
                colNumber = i;
                colFound = true;
                break;
            }
        }

        if(!colFound) {
            std::cerr << "no column found with name : " << columnName << std::endl;
            exit(1);
        }

        if(match(TokenType::TOK_LIKE)) {

            if(!(table->getColumnType(colNumber) == DataType::STRING)) {
                std::cerr << "To use \"LIKE\" the column must be of type string\n";
                exit(1);
            }

            consume(TokenType::TOK_LIKE);

            bool isString = match(TokenType::TOK_STRING);

            if(!isString) {
                std::cerr << "To delete like the given value should be string \n";
                exit(1);
            }

            std::string wordLike = current.getLexeme();
            consume(TokenType::TOK_STRING);

            if(!(wordLike[1] == '%')) {
                std::cerr << "Invalid syntax\n";
                exit(1);
            }

            char findWordWith = wordLike[0];
            std::vector<Row> rows = table->selectAll();
            bool found = false;

            for(int i = rows.size() - 1; i >= 0; i--) {
                Cell val = rows[i].getCell(colNumber);
                char firstLetter = val.getString()[0];

                if(findWordWith == firstLetter) {
                    found = true;

                    table->deleteRow(i);
                }
            }

            if(!found) {
                std::cerr << "No Cell found with first letter : " << findWordWith << "\n";
            }
        }

        bool matched = ( match(TokenType::TOK_EQUAL_EQUAL) || match(TokenType::TOK_NOT_EQUAL) || match(TokenType::TOK_GREATER_EQUAL) || match(TokenType::TOK_LESS_EQUAL) || match(TokenType::TOK_GREATER) || match(TokenType::TOK_LESS));

        if(matched) {
            TokenType op = current.getType();
            consume(op);

            Cell compVal = parseValue();

            std::vector<Row> rows = table->selectAll();

            if(match(TokenType::TOK_LIMIT)) {
                consume(TokenType::TOK_LIMIT);

                Cell climit = parseValue();

                if(!climit.isInt()) {
                    std::cerr << "LIMIT value must be INT\n";
                    exit(1);
                }

                int limit = climit.getInt();

                if(limit < 0) {
                    std::cerr << "LIMIT cannot be negative\n";
                    exit(1);
                }

                int deleted = 0;

                for(int i = rows.size() - 1; i >= 0; i--) {

                    Cell val = rows[i].getCell(colNumber);

                    bool condition = false;

                    switch(op) {
                        case TokenType::TOK_EQUAL_EQUAL: condition = (val == compVal); break;
                        case TokenType::TOK_NOT_EQUAL: condition = (val != compVal); break;
                        case TokenType::TOK_GREATER: condition = (val > compVal); break;
                        case TokenType::TOK_LESS: condition = (val < compVal); break;
                        case TokenType::TOK_GREATER_EQUAL: condition = (val >= compVal); break;
                        case TokenType::TOK_LESS_EQUAL: condition = (val <= compVal); break;
                        default: break;
                    }

                    if(condition) {
                        table->deleteRow(i);
                        deleted++;

                        if(deleted == limit) break;
                    }
                }
            }
            else {
                for(int i = rows.size() - 1; i >= 0; i--) {
                    Cell val = rows[i].getCell(colNumber);

                    bool condition = false;

                    switch(op) {
                        case TokenType::TOK_EQUAL_EQUAL : condition = (val == compVal); break;
                        case TokenType::TOK_NOT_EQUAL : condition = (val != compVal); break;
                        case TokenType::TOK_GREATER : condition = (val > compVal); break;
                        case TokenType::TOK_LESS : condition = (val < compVal); break;
                        case TokenType::TOK_GREATER_EQUAL : condition = (val >= compVal); break;
                        case TokenType::TOK_LESS_EQUAL : condition = (val <= compVal); break;
                        default : break;
                    }

                    if(condition) table->deleteRow(i);
                } 
            }
        }

        consume(TokenType::TOK_SEMICOLON);
    }
    else if(match(TokenType::TOK_LIMIT)) {
        consume(TokenType::TOK_LIMIT);

        Cell climit = parseValue();

        if(!climit.isInt()) {
            std::cerr << "LIMIT value must be INT\n";
            exit(1);
        }

        int limit = climit.getInt();

        if(limit < 0) {
            std::cerr << "LIMIT cannot be negative\n";
            exit(1);
        }

        limit = std::min(limit, table->getRowCount());

        for(int i = 0; i < limit; i++) table->deleteRow(0);
        
        consume(TokenType::TOK_SEMICOLON);
    }
}