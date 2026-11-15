
#include "Product.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <sqlite3.h>

using namespace std;

// Constructor
ProductManager::ProductManager(Database& db)
    : database(db)
{
}

// CREATE PRODUCT
void ProductManager::addProduct()
{
    string name;
    int quantity;
    double price;

    cout << "\n===== ADD PRODUCT =====\n";

    cout << "Enter Product Name: ";
    getline(cin >> ws, name);

    if (name.empty())
    {
        cout << "Product name cannot be empty!\n";
        return;
    }

    cout << "Enter Quantity: ";
    if (!(cin >> quantity) || quantity < 0)
    {
        cout << "Invalid quantity!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter Price: ";
    if (!(cin >> price) || price < 0)
    {
        cout << "Invalid price!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    const char* sql =
        "INSERT INTO products(name, quantity, price) "
        "VALUES(?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    sqlite3_bind_text(
        stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_int(stmt, 2, quantity);
    sqlite3_bind_double(stmt, 3, price);

    int result = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (result == SQLITE_DONE)
    {
        cout << "\nProduct Added Successfully!\n";
        cout << "Product ID: "
             << sqlite3_last_insert_rowid(database.getConnection())
             << "\n";
    }
    else
    {
        cout << "Failed to add product!\n";
    }
}

// READ PRODUCTS
void ProductManager::displayProducts()
{
    cout << "\n========== STORE INVENTORY ==========\n";

    const char* sql =
        "SELECT product_id, name, quantity, price "
        "FROM products WHERE is_active = 1 "
        "ORDER BY product_id;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    cout << left << setw(10) << "ID"
         << setw(22) << "Product"
         << setw(12) << "Quantity"
         << setw(12) << "Price"
         << "Stock Value\n";

    cout << "----------------------------------------------------------\n";

    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;

        int id = sqlite3_column_int(stmt, 0);

        string name = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 1)
        );

        int quantity = sqlite3_column_int(stmt, 2);
        double price = sqlite3_column_double(stmt, 3);

        cout << left << setw(10) << id
             << setw(22) << name
             << setw(12) << quantity
             << setw(12) << fixed << setprecision(2) << price
             << quantity * price << "\n";
    }

    if (!found)
        cout << "No products found!\n";

    sqlite3_finalize(stmt);
}

// SEARCH PRODUCT
void ProductManager::searchProduct()
{
    string name;

    cout << "\nEnter product name to search: ";
    getline(cin >> ws, name);

    const char* sql =
        "SELECT product_id, name, quantity, price "
        "FROM products "
        "WHERE name LIKE ? AND is_active = 1;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    string search = "%" + name + "%";

    sqlite3_bind_text(
        stmt, 1, search.c_str(), -1, SQLITE_TRANSIENT
    );

    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;

        cout << "\nProduct ID: "
             << sqlite3_column_int(stmt, 0);

        cout << "\nName: "
             << sqlite3_column_text(stmt, 1);

        cout << "\nQuantity: "
             << sqlite3_column_int(stmt, 2);

        cout << "\nPrice: Rs. "
             << sqlite3_column_double(stmt, 3)
             << "\n";
    }

    if (!found)
        cout << "\nProduct not found!\n";

    sqlite3_finalize(stmt);
}

// UPDATE PRODUCT
void ProductManager::updateProduct()
{
    int id;
    string name;
    int quantity;
    double price;

    cout << "\nEnter Product ID to update: ";

    if (!(cin >> id) || id <= 0)
    {
        cout << "Invalid ID!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter New Product Name: ";
    getline(cin >> ws, name);

    cout << "Enter New Quantity: ";
    if (!(cin >> quantity) || quantity < 0)
    {
        cout << "Invalid quantity!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter New Price: ";
    if (!(cin >> price) || price < 0)
    {
        cout << "Invalid price!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    const char* sql =
        "UPDATE products "
        "SET name = ?, quantity = ?, price = ? "
        "WHERE product_id = ? AND is_active = 1;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    sqlite3_bind_text(
        stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_int(stmt, 2, quantity);
    sqlite3_bind_double(stmt, 3, price);
    sqlite3_bind_int(stmt, 4, id);

    int result = sqlite3_step(stmt);

    int changes = sqlite3_changes(database.getConnection());

    sqlite3_finalize(stmt);

    if (result == SQLITE_DONE && changes > 0)
        cout << "\nProduct Updated Successfully!\n";
    else
        cout << "\nProduct Not Found or No Changes Made!\n";
}

// DELETE PRODUCT
void ProductManager::deleteProduct()
{
    int id;

    cout << "\nEnter Product ID to delete: ";

    if (!(cin >> id) || id <= 0)
    {
        cout << "Invalid ID!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    // Soft delete preserves order history
    const char* sql =
        "UPDATE products SET is_active = 0 "
        "WHERE product_id = ? AND is_active = 1;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    sqlite3_bind_int(stmt, 1, id);

    int result = sqlite3_step(stmt);

    int changes = sqlite3_changes(database.getConnection());

    sqlite3_finalize(stmt);

    if (result == SQLITE_DONE && changes > 0)
        cout << "\nProduct Deleted Successfully!\n";
    else
        cout << "\nProduct Not Found!\n";
}

// ADMIN MENU
void ProductManager::adminMenu()
{
    int choice;

    do
    {
        cout << "\n========== ADMIN DASHBOARD ==========\n";

        cout << "1. Add Product\n";
        cout << "2. Display Products\n";
        cout << "3. Search Product\n";
        cout << "4. Update Product\n";
        cout << "5. Delete Product\n";
        cout << "0. Logout\n";

        cout << "\nEnter Choice: ";

        if (!(cin >> choice))
        {
            cout << "Invalid input!\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        switch (choice)
        {
            case 1:
                addProduct();
                break;

            case 2:
                displayProducts();
                break;

            case 3:
                searchProduct();
                break;

            case 4:
                updateProduct();
                break;

            case 5:
                deleteProduct();
                break;

            case 0:
                cout << "\nAdmin Logged Out!\n";
                break;

            default:
                cout << "\nInvalid Choice!\n";
        }

    } while (choice != 0);
}
