#pragma once
#ifndef CORE_H
#define CORE_H

#include <string>
#include <vector>

//====================================================================== DATATYPES ======================================================================

enum class DataType {
    NONE,
    INT,
    DOUBLE,
    STRING,
    BOOL
};

union Value {
    int Int;
    double Double;
    bool Bool;
};

enum class ValidationResult { 
    OK, 
    COUNT_MISMATCH, 
    TYPE_MISMATCH 
};

//====================================================================== COLUMN ======================================================================

class Column {
private :
    std::string columnName;
    DataType type;

public :
    Column(const std::string& name, DataType type);

    std::string getName() const;
    void setName(std::string newName);
    DataType getType() const;
};

//================================ CELL ================================

class Cell {
private :
    DataType type;
    Value value;
    std::string stringValue;
    bool isNull;

public :
    Cell();
    Cell(int value);
    Cell(double value);
    Cell(const std::string& value);
    Cell(bool value);

    DataType getType() const;
    bool null() const;

    int getInt() const;
    double getDouble() const;
    std::string getString() const;
    bool getBool() const;

    bool isNullValue() const;
    bool isInt() const;
    bool isDouble() const;
    bool isString() const;
    bool isBool() const;

    void setInt(int value);
    void setDouble(double value);
    void setString(const std::string& value);
    void setBool(bool value);
    void setNull();

    std::string toString() const;

    bool operator==(const Cell& other) const;
    bool operator!=(const Cell& other) const;
    bool operator<(const Cell& other) const;
    bool operator>(const Cell& other) const;
    bool operator<=(const Cell& other) const;
    bool operator>=(const Cell& other) const;
};

//================================ ROW ================================

class Row {
private :
    std::vector<Cell> cells;
public :
    void addCell(const Cell& cell);

    void setCell(int index, const Cell& cell);

    Cell getCell(int index) const;

    int getCellCount() const;

    std::string toString() const;
};

//================================ ROW ================================

class Table {
private :
    std::string tableName;
    std::vector<Column> columns;
    std::vector<Row> rows;

    std::string structureFilePath;
    std::string dataFilePath;

    ValidationResult validateRow(const Row& row) const;

public :
    Table(const std::string& name);
    Table(const std::string& name, const std::string& structurePath, const std::string& dataPath);

    std::string getName() const;
    void setName(std::string newName);

    void addColumn(const Column& column);

    Column& getColumn(int index);
    int getColumnCount() const;
    std::string getColumName(int index) const;
    DataType getColumnType(int index) const;

    int getRowCount() const;

    ValidationResult insertRow(const Row& row);

    void updateCell(int rowIndex, int columnIndex, const Cell& newValue);

    Row* getRow(int rowIndex);

    void deleteRow(int rowIndex);
    void clearRows();

    std::vector<Row> selectAll() const;

    void setStructurePath(const std::string& path);
    void setDataPath(const std::string& path);

    bool saveStructureToFile() const;
    bool loadStructureFromFile();

    bool saveDataToFile() const;
    bool loadDataFromFile();
};

//====================================================================== DATABASE ======================================================================

class Database {
private :
    std::string databaseName;
    std::vector<Table> tables;
    std::string databaseFolderPath;
public :
    Database();
    Database(const std::string& name, const std::string& folderPath);
    
    std::string getName();
    Table* getTable(const std::string& tableName);

    bool createTable(const std::string& tableName);
    void dropTable(const std::string& tableName);

    std::vector<std::string> listTables() const;

    void loadDatabase();
    void saveDatabase();
};

#endif