#pragma once
#ifndef DATAABSEMANAGER_H
#define DATABASEMANAGER_H

#include "core.h"
#include <string>

class DatabaseManager {
private :
    Database currentDatabase;
    bool hasActiveDatabase;

public :
    DatabaseManager();

    void createDatabase(const std::string& name);
    void dropDatabase(const std::string& name);
    void useDatabase(const std::string& name);

    Database* getCurrentDatabase();
};

#endif