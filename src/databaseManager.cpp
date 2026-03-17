#include "databaseManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>

DatabaseManager::DatabaseManager() : hasActiveDatabase(false) {}

void DatabaseManager::createDatabase(const std::string& name) {
    std::string path = "./databases/" + name;

    if(std::filesystem::exists(path)) {
        std::cerr << "Database already exists.\n";
        return;
    }

    std::filesystem::create_directories(path);
    
    std::ofstream fout(path + "/database.meta");
    fout << name << std::endl;
    dbNames.push_back(name);

    std::cout << "Database created: " << name << "\n";
}

void DatabaseManager::dropDatabase(const std::string& name) {
    std::string path = "./databases/" + name;

    if(!std::filesystem::exists(path)) {
        std::cerr << "Database not found.\n";
        return;
    }

    std::filesystem::remove_all(path);

    for(int i = 0; i < dbNames.size(); i++) {
        if(dbNames[i] == name) dbNames.erase(dbNames.begin() + i);
    }

    if(hasActiveDatabase && currentDatabase.getName() == name) {
        hasActiveDatabase = false;
    }

    std::cout << "Database Dropped: " << name << std::endl;
}

void DatabaseManager::useDatabase(const std::string& name) {
    std::string path = "./databases/" + name;

    if(!std::filesystem::exists(path)) {
        std::cerr << "Database does not exist.\n";
        return;
    }

    currentDatabase = Database(name, path);
    hasActiveDatabase = true;

    std::cout << "Using database : " << name << "\n";
}

Database* DatabaseManager::getCurrentDatabase() {
    if(!hasActiveDatabase) {
        std::cerr << "No database selected.\n";
        return nullptr;
    }

    return &currentDatabase;
}

std::vector<std::string> DatabaseManager::listDatabase() const {
    return dbNames;
}