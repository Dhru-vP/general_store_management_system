
#include "Database.h"
#include <iostream>

using namespace std;

// Constructor
Database::Database(const std::string& path)
    : db(nullptr), dbPath(path)
{
}

// Destructor
Database::~Database()
{
    disconnect();
}

// Connect to SQLite database
bool Database::connect()
{
   int result = sqlite3_open(dbPath.c_str(), &db);

    if (result != SQLITE_OK)
    {
        cout << "Database connection failed: "
             << sqlite3_errmsg(db) << endl;
        return false;
    }

    cout << "Database connected successfully!" << endl;

    // Enable foreign key support
    return execute("PRAGMA foreign_keys = ON;");
}

// Execute SQL command
bool Database::execute(const string& sql)
{
    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql.c_str(),
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK)
    {
        cout << "SQL Error: " << errorMessage << endl;
        sqlite3_free(errorMessage);
        return false;
    }

    return true;
}

// Create all database tables
bool Database::initializeTables()
{
    string sql = R"(

    CREATE TABLE IF NOT EXISTS users
    (
        user_id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT NOT NULL UNIQUE,
        password_hash TEXT NOT NULL,
        role TEXT NOT NULL CHECK(role IN ('admin', 'customer')),
        is_active INTEGER DEFAULT 1,
        created_at TEXT DEFAULT CURRENT_TIMESTAMP
    );

    CREATE TABLE IF NOT EXISTS products
    (
        product_id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        quantity INTEGER NOT NULL CHECK(quantity >= 0),
        price REAL NOT NULL CHECK(price >= 0),
        is_active INTEGER DEFAULT 1,
        created_at TEXT DEFAULT CURRENT_TIMESTAMP
    );

    CREATE TABLE IF NOT EXISTS orders
    (
        order_id INTEGER PRIMARY KEY AUTOINCREMENT,
        user_id INTEGER NOT NULL,
        total_amount REAL NOT NULL DEFAULT 0,
        status TEXT DEFAULT 'completed',
        order_date TEXT DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY(user_id)
        REFERENCES users(user_id)
    );

    CREATE TABLE IF NOT EXISTS order_items
    (
        item_id INTEGER PRIMARY KEY AUTOINCREMENT,
        order_id INTEGER NOT NULL,
        product_id INTEGER NOT NULL,
        product_name TEXT NOT NULL,
        quantity INTEGER NOT NULL CHECK(quantity > 0),
        unit_price REAL NOT NULL,
        line_total REAL NOT NULL,

        FOREIGN KEY(order_id)
        REFERENCES orders(order_id),

        FOREIGN KEY(product_id)
        REFERENCES products(product_id)
    );

    CREATE TABLE IF NOT EXISTS payments
    (
        payment_id INTEGER PRIMARY KEY AUTOINCREMENT,
        order_id INTEGER NOT NULL,
        amount REAL NOT NULL,
        payment_method TEXT NOT NULL,
        payment_status TEXT DEFAULT 'completed',
        payment_date TEXT DEFAULT CURRENT_TIMESTAMP,

        FOREIGN KEY(order_id)
        REFERENCES orders(order_id)
    );

    CREATE INDEX IF NOT EXISTS idx_orders_user
    ON orders(user_id);

    CREATE INDEX IF NOT EXISTS idx_order_items_order
    ON order_items(order_id);

    )";

    if (execute(sql))
    {
        cout << "All database tables created successfully!"
             << endl;
        return true;
    }

    return false;
}

// Return database connection
sqlite3* Database::getConnection()
{
    return db;
}

// Close database
void Database::disconnect()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
        db = nullptr;

        cout << "Database disconnected." << endl;
    }
}
