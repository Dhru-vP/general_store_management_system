
#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "Database.h"
#include <string>

struct UserSession
{
    int userId;
    std::string username;
    std::string role;
    bool loggedIn;
};

class Authentication
{
private:
    Database& database;

    std::string hashPassword(const std::string& password);

    bool verifyPassword(
        const std::string& password,
        const std::string& storedHash
    );

    bool createUser(
        const std::string& username,
        const std::string& password,
        const std::string& role
    );

public:
    Authentication(Database& db);

    bool createInitialAdmin();

    bool registerCustomer();

    bool login(UserSession& session);
};

#endif
