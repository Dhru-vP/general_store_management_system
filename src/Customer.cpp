
#include "Customer.h"

#include <iostream>
#include <iomanip>
#include <limits>
#include <sqlite3.h>

using namespace std;

// Constructor
CustomerManager::CustomerManager(
    Database& db,
    int id,
    const string& name)
    : database(db), userId(id), username(name)
{
}

// Display available products
void CustomerManager::displayProducts()
{
    const char* sql =
        "SELECT product_id, name, quantity, price "
        "FROM products "
        "WHERE is_active = 1 AND quantity > 0 "
        "ORDER BY product_id;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql, -1, &stmt, nullptr
    );

    cout << "\n========== AVAILABLE PRODUCTS ==========\n";

    cout << left << setw(10) << "ID"
         << setw(22) << "Name"
         << setw(12) << "Stock"
         << "Price\n";

    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;

        cout << left
             << setw(10) << sqlite3_column_int(stmt, 0)
             << setw(22) << sqlite3_column_text(stmt, 1)
             << setw(12) << sqlite3_column_int(stmt, 2)
             << fixed << setprecision(2)
             << sqlite3_column_double(stmt, 3)
             << "\n";
    }

    if (!found)
        cout << "No products available!\n";

    sqlite3_finalize(stmt);
}

// Search products
void CustomerManager::searchProducts()
{
    string name;

    cout << "\nEnter product name: ";
    getline(cin >> ws, name);

    const char* sql =
        "SELECT product_id, name, quantity, price "
        "FROM products "
        "WHERE name LIKE ? AND is_active = 1 "
        "AND quantity > 0;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql, -1, &stmt, nullptr
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

        cout << "\nStock: "
             << sqlite3_column_int(stmt, 2);

        cout << "\nPrice: Rs. "
             << sqlite3_column_double(stmt, 3)
             << "\n";
    }

    if (!found)
        cout << "\nProduct not found!\n";

    sqlite3_finalize(stmt);
}

// Add product to cart
void CustomerManager::addToCart()
{
    int id, quantity;

    displayProducts();

    cout << "\nEnter Product ID: ";

    if (!(cin >> id) || id <= 0)
    {
        cout << "Invalid product ID!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    cout << "Enter Quantity: ";

    if (!(cin >> quantity) || quantity <= 0)
    {
        cout << "Invalid quantity!\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    const char* sql =
        "SELECT name, quantity, price FROM products "
        "WHERE product_id = ? AND is_active = 1;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql, -1, &stmt, nullptr
    );

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) != SQLITE_ROW)
    {
        cout << "\nProduct not found!\n";
        sqlite3_finalize(stmt);
        return;
    }

    string name = reinterpret_cast<const char*>(
        sqlite3_column_text(stmt, 0)
    );

    int stock = sqlite3_column_int(stmt, 1);

    double price = sqlite3_column_double(stmt, 2);

    sqlite3_finalize(stmt);

    int alreadyInCart = 0;

    for (const auto& item : cart)
    {
        if (item.productId == id)
            alreadyInCart += item.quantity;
    }

    if (quantity + alreadyInCart > stock)
    {
        cout << "\nInsufficient stock!\n";
        cout << "Available stock: " << stock - alreadyInCart << "\n";
        return;
    }

    // Add to existing cart item if present
    for (auto& item : cart)
    {
        if (item.productId == id)
        {
            item.quantity += quantity;

            cout << "\nCart quantity updated!\n";
            return;
        }
    }

    CartItem item;

    item.productId = id;
    item.name = name;
    item.quantity = quantity;
    item.price = price;

    cart.push_back(item);

    cout << "\nProduct added to cart successfully!\n";
}

// View cart
void CustomerManager::viewCart()
{
    if (cart.empty())
    {
        cout << "\nYour cart is empty!\n";
        return;
    }

    double total = 0;

    cout << "\n========== YOUR CART ==========\n";

    cout << left << setw(20) << "Product"
         << setw(10) << "Quantity"
         << setw(12) << "Price"
         << "Total\n";

    for (const auto& item : cart)
    {
        double itemTotal = item.quantity * item.price;

        cout << left << setw(20) << item.name
             << setw(10) << item.quantity
             << setw(12) << fixed << setprecision(2)
             << item.price
             << itemTotal << "\n";

        total += itemTotal;
    }

    cout << "\nGrand Total: Rs. "
         << fixed << setprecision(2) << total << "\n";
}

// Checkout and generate bill
bool CustomerManager::checkout()
{
    if (cart.empty())
    {
        cout << "\nYour cart is empty!\n";
        return false;
    }

    double grandTotal = 0;

    for (const auto& item : cart)
        grandTotal += item.quantity * item.price;

    char confirm;

    viewCart();

    cout << "\nConfirm Purchase? (Y/N): ";
    cin >> confirm;

    if (confirm != 'Y' && confirm != 'y')
    {
        cout << "\nPurchase cancelled!\n";
        return false;
    }

    // Start transaction
    if (!database.execute("BEGIN IMMEDIATE;"))
    {
        cout << "Could not start checkout!\n";
        return false;
    }

    bool success = true;

    // Create order
    const char* orderSQL =
        "INSERT INTO orders(user_id, total_amount, status) "
        "VALUES(?, ?, 'completed');";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            database.getConnection(),
            orderSQL, -1, &stmt, nullptr) != SQLITE_OK)
    {
        success = false;
    }

    if (success)
    {
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_bind_double(stmt, 2, grandTotal);

        if (sqlite3_step(stmt) != SQLITE_DONE)
            success = false;
    }

    sqlite3_finalize(stmt);

    sqlite3_int64 orderId = 0;

    if (success)
        orderId = sqlite3_last_insert_rowid(database.getConnection());

    // Update stock and insert order items
    for (const auto& item : cart)
    {
        if (!success)
            break;

        const char* stockSQL =
            "UPDATE products SET quantity = quantity - ? "
            "WHERE product_id = ? "
            "AND is_active = 1 AND quantity >= ?;";

        stmt = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                stockSQL, -1, &stmt, nullptr) != SQLITE_OK)
        {
            success = false;
            break;
        }

        sqlite3_bind_int(stmt, 1, item.quantity);
        sqlite3_bind_int(stmt, 2, item.productId);
        sqlite3_bind_int(stmt, 3, item.quantity);

        int result = sqlite3_step(stmt);

        bool stockUpdated =
            result == SQLITE_DONE &&
            sqlite3_changes(database.getConnection()) == 1;

        sqlite3_finalize(stmt);

        if (!stockUpdated)
        {
            success = false;
            break;
        }

        const char* itemSQL =
            "INSERT INTO order_items "
            "(order_id, product_id, product_name, quantity, "
            "unit_price, line_total) "
            "VALUES(?, ?, ?, ?, ?, ?);";

        stmt = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                itemSQL, -1, &stmt, nullptr) != SQLITE_OK)
        {
            success = false;
            break;
        }

        sqlite3_bind_int64(stmt, 1, orderId);
        sqlite3_bind_int(stmt, 2, item.productId);

        sqlite3_bind_text(
            stmt, 3, item.name.c_str(), -1, SQLITE_TRANSIENT
        );

        sqlite3_bind_int(stmt, 4, item.quantity);
        sqlite3_bind_double(stmt, 5, item.price);

        sqlite3_bind_double(
            stmt, 6, item.quantity * item.price
        );

        if (sqlite3_step(stmt) != SQLITE_DONE)
            success = false;

        sqlite3_finalize(stmt);
    }

    // Record cash payment
    if (success)
    {
        const char* paymentSQL =
            "INSERT INTO payments "
            "(order_id, amount, payment_method, payment_status) "
            "VALUES(?, ?, 'Cash', 'completed');";

        stmt = nullptr;

        if (sqlite3_prepare_v2(
                database.getConnection(),
                paymentSQL, -1, &stmt, nullptr) != SQLITE_OK)
        {
            success = false;
        }
        else
        {
            sqlite3_bind_int64(stmt, 1, orderId);
            sqlite3_bind_double(stmt, 2, grandTotal);

            if (sqlite3_step(stmt) != SQLITE_DONE)
                success = false;
        }

        sqlite3_finalize(stmt);
    }

    // Commit or rollback
    if (success)
    {
        success = database.execute("COMMIT;");
    }

    if (!success)
    {
        database.execute("ROLLBACK;");
        cout << "\nPurchase failed! Database changes rolled back.\n";
        return false;
    }

    // Generate bill
    cout << "\n========== FINAL BILL ==========\n";
    cout << "Order ID: " << orderId << "\n";
    cout << "Customer: " << username << "\n";

    cout << "--------------------------------\n";

    for (const auto& item : cart)
    {
        cout << item.name
             << " x " << item.quantity
             << " = Rs. "
             << item.quantity * item.price
             << "\n";
    }

    cout << "--------------------------------\n";

    cout << "Grand Total: Rs. "
         << fixed << setprecision(2)
         << grandTotal << "\n";

    cout << "Payment: Cash\n";
    cout << "Status: Successful\n";

    cout << "\nThank you for shopping!\n";

    cart.clear();

    return true;
}

// Customer order history
void CustomerManager::viewOrderHistory()
{
    const char* sql =
        "SELECT order_id, total_amount, status, order_date "
        "FROM orders WHERE user_id = ? "
        "ORDER BY order_id DESC;";

    sqlite3_stmt* stmt = nullptr;

    sqlite3_prepare_v2(
        database.getConnection(),
        sql, -1, &stmt, nullptr
    );

    sqlite3_bind_int(stmt, 1, userId);

    cout << "\n========== MY ORDER HISTORY ==========\n";

    bool found = false;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = true;

        cout << "\nOrder ID: "
             << sqlite3_column_int(stmt, 0);

        cout << "\nAmount: Rs. "
             << sqlite3_column_double(stmt, 1);

        cout << "\nStatus: "
             << sqlite3_column_text(stmt, 2);

        cout << "\nDate: "
             << sqlite3_column_text(stmt, 3);

        cout << "\n";
    }

    if (!found)
        cout << "No previous orders!\n";

    sqlite3_finalize(stmt);
}

// Customer menu
void CustomerManager::customerMenu()
{
    int choice;

    do
    {
        cout << "\n========== CUSTOMER DASHBOARD ==========\n";

        cout << "1. View Products\n";
        cout << "2. Search Products\n";
        cout << "3. Add to Cart\n";
        cout << "4. View Cart\n";
        cout << "5. Checkout\n";
        cout << "6. Order History\n";
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
                displayProducts();
                break;

            case 2:
                searchProducts();
                break;

            case 3:
                addToCart();
                break;

            case 4:
                viewCart();
                break;

            case 5:
                checkout();
                break;

            case 6:
                viewOrderHistory();
                break;

            case 0:
                cart.clear();
                cout << "\nCustomer Logged Out!\n";
                break;

            default:
                cout << "\nInvalid Choice!\n";
        }

    } while (choice != 0);
}
