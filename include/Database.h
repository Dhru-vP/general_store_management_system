
#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>

class Database
{
private:
    sqlite3* db;
    std::string dbPath;

public:
    // Constructor
    explicit Database(const std::string& path = "database/store.db");

    // Destructor
    ~Database();

    // Database operations
    bool connect();
    bool initializeTables();
    bool execute(const std::string& sql);

    // Get database connection
    sqlite3* getConnection();

    // Close database connection
    void disconnect();
};

#endif
