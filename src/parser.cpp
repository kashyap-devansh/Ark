#include "parser.h"

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

void Parser::consume(TokenType expected) {
    if(current.getType() != expected) {
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

    std::cerr << "Invalid value.\n";
    exit(1);
}

int Parser::getColumnIndex(Table* table, const std::string& columnName) {
    for(int i = 0; i < table->selectAll()[0].getCellCount(); i++) {
        if(table->selectAll().at(0).getCell(i).toString() == columnName) return i;
    }

    return 0;
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
    consume(TokenType::TOK_TABLES);
    consume(TokenType::TOK_SEMICOLON);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    auto tables = db->listTables();
    
    for(const auto& name : tables) std::cout << name << "\n";
}

void Parser::parseInsert(DatabaseManager& manager) {
    consume(TokenType::TOK_INSERT);
    consume(TokenType::TOK_INTO);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);

    Table* table = db->getTable(tableName);

    consume(TokenType::TOK_VALUES);
    consume(TokenType::TOK_LPAREN);

    Row row;

    while(!match(TokenType::TOK_RPAREN)) {
        row.addCell(parseValue());

        if(match(TokenType::TOK_COMMA)) consume(TokenType::TOK_COMMA);
    }

    consume(TokenType::TOK_RPAREN);
    consume(TokenType::TOK_SEMICOLON);

    table->insertRow(row);
}

void printBorder(const std::vector<int>& widths, const std::vector<int>& columnIndexes) {
    for(int i = 0; i < columnIndexes.size(); i++) {
        std::cout << "+-";
        for(int j = 0; j < widths[i]; j++) std::cout << "-";
        std::cout << "-";
    }
    std::cout << "+\n";
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
    consume(TokenType::TOK_SEMICOLON);

    Table* table = db->getTable(tableName);
    if(!table) {
        std::cerr << "Table not found: " << tableName << "\n";
        return;
    }

    std::vector<int> columnIndexes;

    if(selectAll) {
        for(int i = 0; i < table->getColumnCount(); i++) 
            columnIndexes.push_back(i);
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

    for(const Row& row : table->selectAll()) {
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

    consume(TokenType::TOK_SET);

    std::string columnName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_EQUAL);

    Cell newvalue = parseValue();
    consume(TokenType::TOK_SEMICOLON);

    int colIndex = getColumnIndex(table, columnName);

    for(int i = 0; i < table->selectAll().size(); i++)
        table->updateCell(i, colIndex, newvalue);
}

void Parser::parseDelete(DatabaseManager& manager) {
    consume(TokenType::TOK_DELETE);
    consume(TokenType::TOK_FROM);

    Database* db = manager.getCurrentDatabase();
    if(!db) return;

    std::string tableName = current.getLexeme();
    consume(TokenType::TOK_IDENTIFIER);
    consume(TokenType::TOK_SEMICOLON);

    Table* table = db->getTable(tableName);

    while(!table->selectAll().empty())
        table->deleteRow(0);   
}