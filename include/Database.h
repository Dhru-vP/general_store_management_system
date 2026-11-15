
#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

class Database
{
private:
    sqlite3* db;

public:
    Database();

    ~Database();

    bool connect();

    bool initializeTables();

    bool execute(const std::string& sql);

    sqlite3* getConnection();

    void disconnect();
};

#endif
