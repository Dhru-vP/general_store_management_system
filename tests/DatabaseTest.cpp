
#include <gtest/gtest.h>
#include "Database.h"
#include "Authentication.h"
#include <sqlite3.h>
#include <filesystem>
#include <sstream>
#include <iostream>

class DatabaseTest : public ::testing::Test {
protected:
    Database db{"database/test_store.db"};

    void SetUp() override {
        std::filesystem::create_directories("database");
        std::filesystem::remove("database/test_store.db");
        ASSERT_TRUE(db.connect());
        ASSERT_TRUE(db.initializeTables());
    }

    void TearDown() override {
        db.disconnect();
        std::filesystem::remove("database/test_store.db");
    }
};

TEST_F(DatabaseTest, DatabaseConnectionWorks) {
    ASSERT_NE(db.getConnection(), nullptr);
}

TEST_F(DatabaseTest, RequiredTablesAreCreated) {
    const char* sql =
        "SELECT COUNT(*) FROM sqlite_master "
        "WHERE type='table' AND name IN "
        "('users','products','orders','order_items','payments');";

    sqlite3_stmt* stmt = nullptr;

    ASSERT_EQ(sqlite3_prepare_v2(
        db.getConnection(), sql, -1, &stmt, nullptr), SQLITE_OK);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    EXPECT_EQ(sqlite3_column_int(stmt, 0), 5);

    sqlite3_finalize(stmt);
}

TEST_F(DatabaseTest, CustomerRegistrationWorks) {
    Authentication auth(db);
    std::istringstream input("alice\nPassword123\n");
    auto* old = std::cin.rdbuf(input.rdbuf());

    bool result = auth.registerCustomer();

    std::cin.rdbuf(old);
    std::cin.clear();

    EXPECT_TRUE(result);

    sqlite3_stmt* stmt = nullptr;
    ASSERT_EQ(sqlite3_prepare_v2(
        db.getConnection(),
        "SELECT role FROM users WHERE username='alice'",
        -1, &stmt, nullptr), SQLITE_OK);

    ASSERT_EQ(sqlite3_step(stmt), SQLITE_ROW);
    EXPECT_STREQ(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
        "customer");

    sqlite3_finalize(stmt);
}

TEST_F(DatabaseTest, CustomerLoginWorks) {
    Authentication auth(db);

    std::istringstream registration("alice\nPassword123\n");
    auto* old = std::cin.rdbuf(registration.rdbuf());
    bool registered = auth.registerCustomer();
    std::cin.rdbuf(old);
    std::cin.clear();

    ASSERT_TRUE(registered);

    UserSession session{};
    std::istringstream login("alice\nPassword123\n");
    old = std::cin.rdbuf(login.rdbuf());

    bool result = auth.login(session);

    std::cin.rdbuf(old);
    std::cin.clear();

    EXPECT_TRUE(result);
    EXPECT_TRUE(session.loggedIn);
    EXPECT_EQ(session.username, "alice");
    EXPECT_EQ(session.role, "customer");
}

TEST_F(DatabaseTest, IncorrectPasswordIsRejected) {
    Authentication auth(db);

    std::istringstream registration("alice\nPassword123\n");
    auto* old = std::cin.rdbuf(registration.rdbuf());
    bool registered = auth.registerCustomer();
    std::cin.rdbuf(old);
    std::cin.clear();

    ASSERT_TRUE(registered);

    UserSession session{};
    std::istringstream login("alice\nWrongPass123\n");
    old = std::cin.rdbuf(login.rdbuf());

    bool result = auth.login(session);

    std::cin.rdbuf(old);
    std::cin.clear();

    EXPECT_FALSE(result);
    EXPECT_FALSE(session.loggedIn);
}
