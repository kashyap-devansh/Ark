#include "core.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

//====================================================================== COLUMN ======================================================================

Column::Column(const std::string& name, DataType type) : columnName(name), type(type) {}

std::string Column::getName() const {
    return columnName;
}

DataType Column::getType() const {
    return type;
}

//====================================================================== CELL ======================================================================

Cell::Cell() : type(DataType::NONE), stringValue(""), isNull(true) {
    value.Double = 0.0;
}

Cell::Cell(int value) : type(DataType::INT) {
    this->value.Int = value;
    isNull = false;
}

Cell::Cell(double value) : type(DataType::DOUBLE) {
    this->value.Double = value;
    isNull = false;
}

Cell::Cell(const std::string& value) : type(DataType::STRING) {
    stringValue = value;
    isNull = false;
}

Cell::Cell(bool value) : type(DataType::BOOL) {
    this->value.Bool = value;
    isNull = false;
}

DataType Cell::getType() const {
    return type;
}

int Cell::getInt() const {
    if(isNull) return 0;

    return value.Int;
}

double Cell::getDouble() const {
    if(isNull) return 0.0;

    return value.Double;
}

std::string Cell::getString() const {
    if(isNull) return "";

    return stringValue;
}

bool Cell::getBool() const {
    if(isNull) return false;

    return value.Bool;
}

bool Cell::isInt() const {
    return type == DataType::INT && !isNull;
}

bool Cell::isDouble() const {
    return type == DataType::DOUBLE && !isNull;
}

bool Cell::isString() const {
    return type == DataType::STRING && !isNull;
}

bool Cell::isBool() const {
    return type == DataType::BOOL && !isNull;
}

void Cell::setInt(int value) {
    this->value.Int = value;
    isNull = false;
    type = DataType::INT;
}

void Cell::setDouble(double value) {
    this->value.Double = value;
    isNull = false;
    type = DataType::DOUBLE;
}

void Cell::setString(const std::string& value) {
    stringValue = value;
    isNull = false;
    type = DataType::STRING;
}

void Cell::setBool(bool value) {
    this->value.Bool = value;
    isNull = false;
    type = DataType::BOOL;
}

void Cell::setNull() {
    isNull = true;
    type = DataType::NONE;
    value.Double = 0.0;
    stringValue = "";
}

std::string Cell::toString() const {
    if(isNull) return "NULL";

    switch(type) {
        case DataType::INT :
            return std::to_string(value.Int);
        case DataType::DOUBLE :
            return std::to_string(value.Double);
        case DataType::STRING : 
            return stringValue;
        case DataType::BOOL :
            return value.Bool ? "true" : "false";
        case DataType::NONE :
        default : return "NONE";
    }
}

bool Cell::operator==(const Cell& other) const {
    if(isNull || other.isNull) return false;

    if(type != other.type) {
        return false;
    }

    switch(type) {
        case DataType::INT :
            return (value.Int == other.value.Int);
        case DataType::DOUBLE :
            return (value.Double == other.value.Double);
        case DataType::STRING :
            return (stringValue == other.stringValue);
        case DataType::BOOL :
            return (value.Bool == other.value.Bool);
        case DataType::NONE :
        default : return true;
    }
}

bool Cell::operator!=(const Cell& other) const {
    return !(*this == other);
}

bool Cell::operator<(const Cell& other) const {
    if(isNull || other.isNull) return false;

    if(type != other.type) {
        return (type < other.type);
    }

    switch(type) {
        case DataType::INT :
            return (value.Int < other.value.Int);
        case DataType::DOUBLE :
            return (value.Double < other.value.Double);
        case DataType::STRING :
            return (stringValue < other.stringValue);
        case DataType::BOOL :
            return (value.Bool < other.value.Bool);
        case DataType::NONE :
        default : return false;
    }
}

bool Cell::operator>(const Cell& other) const {
    return (other < *this);
}

bool Cell::operator<=(const Cell& other) const {
    return !(other < *this);
}

bool Cell::operator>=(const Cell& other) const {
    return !(*this < other);
}

//====================================================================== ROW ======================================================================

void Row::addCell(const Cell& cell) {
    cells.push_back(cell);
}

void Row::setCell(int index, const Cell& cell){
    cells[index] = cell;
}

Cell Row::getCell(int index) const {
    return cells[index];
}

int Row::getCellCount() const {
    return static_cast<int>(cells.size());
}

std::string Row::toString() const {
    std::string result;

    for(size_t i = 0; i < cells.size(); i++) {
        result += cells[i].toString();

        if(i != cells.size() - 1) result += " | ";
    }

    return result;
}

//====================================================================== TABLE ======================================================================

Table::Table(const std::string& name, const std::string& structurePath, const std::string& dataPath) : tableName(name), structureFilePath(structurePath), dataFilePath(dataPath) {}

std::string Table::getName() const {
    return tableName;
}

void Table::addColumn(const Column& column) {
    columns.push_back(column);   
}

int Table::getColumnCount() const {
    return static_cast<int>(columns.size());
}

std::string Table::getColumName(int index) const {
    return columns.at(index).getName();
}

DataType Table::getColumnType(int index) const {
    return columns.at(index).getType();
}

int Table::getRowCount() const {
    return static_cast<int>(rows.size());
}

ValidationResult Table::validateRow(const Row& row) const {
    if(row.getCellCount() != static_cast<int>(columns.size())) return ValidationResult::COUNT_MISMATCH;

    for(size_t i = 0; i < columns.size(); i++) {
        if(row.getCell(i).getType() != columns.at(i).getType()) return ValidationResult::TYPE_MISMATCH;
    }

    return ValidationResult::OK;
}

ValidationResult Table::insertRow(const Row& row) {
    ValidationResult result = validateRow(row);
    if (result != ValidationResult::OK) return result;

    rows.push_back(row);
    return ValidationResult::OK;
}

Row* Table::getRow(int rowIndex) {
    if(rowIndex >= 0 && rowIndex < rows.size()) return &rows.at(rowIndex);

    return nullptr;
}

void Table::deleteRow(int rowIndex) {
    if(rowIndex < 0 || rowIndex >= static_cast<int>(rows.size())) {
        std::cout << "Invalid row Index\n";
        return;
    }

    rows.erase(rows.begin() + rowIndex);
}

std::vector<Row> Table::selectAll() const {
    return rows;
}

void Table::updateCell(int rowIndex, int columnIndex, const Cell& newValue) {
    if(rowIndex < 0 || rowIndex >= static_cast<int>(rows.size())) {
        std::cerr << "Invalid Row Index.\n";
        return;
    }

    if(columnIndex < 0 || columnIndex >= static_cast<int>(columns.size())) {
        std::cerr << "Invalid Column Index.\n";
        return;
    }

    if(newValue.getType() != columns.at(columnIndex).getType()) {
        std::cerr << "Type mismatch. Update Failed.\n";
        return;
    }

    rows.at(rowIndex).setCell(columnIndex, newValue);

    std::cout << "Cell update successfully.\n";
}

bool Table::saveStructureToFile() const {
    std::ofstream fout(structureFilePath);
    if(!fout) {
        std::cerr << "Failed to open structure file.\n";
        return false;
    }

    fout << tableName << "\n";

    for(const auto& col : columns) {
        fout << col.getName() << " ";

        switch(col.getType()) {
            case DataType::INT : fout << "INT"; break;
            case DataType::DOUBLE : fout << "DOUBLE"; break;
            case DataType::STRING : fout << "STRING"; break;
            case DataType::BOOL : fout << "BOOL"; break;
        }

        fout << "\n";
    }

    fout.close();
    return true;
}

bool Table::loadStructureFromFile() {
    std::ifstream fin(structureFilePath);

    if(!fin) {
        std::cerr << "Failed to open structure file.\n";
        return false;
    }

    columns.clear();

    std::getline(fin,tableName);

    std::string colName, typeStr;
    while(fin >> colName >> typeStr) {
        DataType type;

        if(typeStr == "INT")
            type = DataType::INT;
        else if(typeStr == "DOUBLE")
            type = DataType::DOUBLE;
        else if(typeStr == "STRING")
            type = DataType::STRING;
        else if(typeStr == "BOOL") 
            type = DataType::BOOL;
        else {
            std::cerr << "Unknown data type: " << typeStr << '\n';
            return false;
        }

        columns.emplace_back(colName, type);
    }

    fin.close();
    return true;
}

bool Table::saveDataToFile() const {
    std::ofstream fout(dataFilePath);

    if(!fout) {
        std::cerr << "Failed to open data file.\n";
        return false;
    }

    for(const auto& row : rows) {
        for(size_t i = 0; i < row.getCellCount(); i++) {
            fout << row.getCell(i).toString();

            if(i != row.getCellCount() - 1) fout << ",";
        }

        fout << "\n";
    }

    fout.close();
    return true;
}

bool Table::loadDataFromFile() {
    std::ifstream fin(dataFilePath);

    if(!fin) {
        std::cerr << "Failed to open data file.\n";
        return false;
    }

    rows.clear();

    std::string line;
    
    while(std::getline(fin, line)) {
        std::stringstream ss(line);
        std::string value;

        Row row;
        int columnIndex = 0;

        while(std::getline(ss, value, ',') && columnIndex < columns.size()) {
            DataType type = columns[columnIndex].getType();

            switch(type) {
                case DataType::INT : {
                    int intVal = std::stoi(value);
                    row.addCell(Cell(intVal));
                    break;
                }
                case DataType::DOUBLE : {
                    double doubleVal = std::stod(value);
                    row.addCell(Cell(doubleVal));
                    break;
                }
                case DataType::STRING : {
                    row.addCell(Cell(value));
                    break;
                }
                case DataType::BOOL : {
                    bool boolVal = (value == "1" || value == "true");
                    row.addCell(Cell(boolVal));
                    break;
                }
                default : break;
            }

            columnIndex++;
        }

        if(validateRow(row) == ValidationResult::OK) rows.push_back(row);
    }

    fin.close();
    return true;
}

//====================================================================== DATABASE ======================================================================

Database::Database() : databaseName(""), databaseFolderPath("") {}

Database::Database(const std::string& name, const std::string& folderPath) : databaseName(name), databaseFolderPath(folderPath) {
    std::filesystem::create_directories(databaseFolderPath + "/tables");
    std::filesystem::create_directories(databaseFolderPath + "/data");
}

std::string Database::getName() {
    return databaseName;
}

Table* Database::getTable(const std::string& tableName) {
    for(Table& table : tables) {
        if(table.getName() == tableName) {
            return &table;
        }
    }
    return nullptr;
}

bool Database::createTable(const std::string& tableName) {
    if(getTable(tableName) != nullptr) return false;

    std::string structurePath = databaseFolderPath + "/tables/" + tableName + ".tbl";
    std::string dataPath = databaseFolderPath + "/data/" + tableName + ".dat";

    if(std::filesystem::exists(structurePath) || std::filesystem::exists(dataPath)) return false;

    tables.emplace_back(Table(tableName, structurePath, dataPath));

    return true;
}

void Database::dropTable(const std::string& tableName) {
    for(size_t i = 0; i < tables.size(); i++) {
        if(tables.at(i).getName() == tableName) {

            std::string structurePath = databaseFolderPath + "/tables/" + tableName + ".tbl";
            std::string dataPath = databaseFolderPath + "/data/" + tableName + ".dat";

            std::filesystem::remove(structurePath);
            std::filesystem::remove(dataPath);

            tables.erase(tables.begin() + i);

            std::cout << "Table dropped: " << tableName << "\n";
            return;
        }
    }

    std::cerr << "Table not found.\n";
}

std::vector<std::string> Database::listTables() const {
    std::vector<std::string> names;

    for(size_t i = 0; i < tables.size(); i++) {
        names.push_back(tables.at(i).getName());
    }

    return names;
}

void Database::saveDatabase() {
    std::string registryPath = databaseFolderPath + "/tables.meta";
    std::ofstream fout(registryPath);

    if(!fout) {
        std::cerr << "Failed to save tables registry.\n";
        return;
    }

    for(size_t i = 0; i < tables.size(); i++) {
        fout << tables.at(i).getName() << "\n"; 

        tables.at(i).saveStructureToFile();
        tables.at(i).saveDataToFile();
    }

    std::cout << "Database saved.\n";

    fout.close();
}

void Database::loadDatabase() {
    tables.clear();

    std::string registryPath = databaseFolderPath + "/tables.meta";
    std::ifstream fin(registryPath);

    if(!fin) {
        std::cerr << "No tables registryf found.\n";
        return;
    }

    std::string tableName;

    while(std::getline(fin, tableName)) {
        if(tableName.empty()) continue;

        std::string structurePath = databaseFolderPath + "/tables/" + tableName + ".tbl";
        std::string dataPath = databaseFolderPath + "/data/" + tableName + ".dat";
        
        Table table(tableName, structurePath, dataPath);
        
        table.loadStructureFromFile();
        table.loadDataFromFile();

        tables.push_back(table);
    }

    std::cout << "Database loaded.\n";

    fin.close();
}