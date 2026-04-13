#include "databaseManager.h"

#include <filesystem>
#include <fstream>
#include <iostream>

DatabaseManager::DatabaseManager() : hasActiveDatabase(false) {
    std::string base = "./databases/";
    std::string registryPath = base + "databases.meta";

    if(!std::filesystem::exists(registryPath)) return;

    std::ifstream fin(registryPath);
    if(!fin) return;

    std::string name;
    while(std::getline(fin, name)) {
        if(!name.empty()) dbNames.push_back(name);
    }

    fin.close();
}

void DatabaseManager::saveRegistry() {
    std::string registryPath = "./databases/databases.meta";
    std::ofstream fout(registryPath);

    if(!fout) {
        std::cerr << "Failed to save databases registry.\n";
        return;
    }

    for(const auto& name : dbNames) fout << name << "\n";

    fout.close();
}

void DatabaseManager::createDatabase(const std::string& name) {
    std::string base = "./databases/";
    std::string path = base + name;

    if(std::filesystem::exists(path)) {
        std::cerr << "Database already exists.\n";
        return;
    }

    std::filesystem::create_directories(base);
    std::filesystem::create_directories(path);

    dbNames.push_back(name);
    saveRegistry();

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
        if(dbNames[i] == name) {
            dbNames.erase(dbNames.begin() + i);
            break;
        }
    }

    if(hasActiveDatabase && currentDatabase.getName() == name) {
        hasActiveDatabase = false;
    }

    saveRegistry();

    std::cout << "Database dropped: " << name << "\n";
}

void DatabaseManager::useDatabase(const std::string& name) {
    std::string path = "./databases/" + name;

    if(!std::filesystem::exists(path)) {
        std::cerr << "Database does not exist.\n";
        return;
    }

    currentDatabase = Database(name, path);
    currentDatabase.loadDatabase();
    hasActiveDatabase = true;

    std::cout << "Using database: " << name << "\n";
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