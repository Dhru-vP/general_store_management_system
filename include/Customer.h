
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "Database.h"
#include <vector>
#include <string>

struct CartItem
{
    int productId;
    std::string name;
    int quantity;
    double price;
};

class CustomerManager
{
private:
    Database& database;
    int userId;
    std::string username;
    std::vector<CartItem> cart;

    bool checkout();

public:
    CustomerManager(Database& db, int id, const std::string& name);

    void displayProducts();

    void searchProducts();

    void addToCart();

    void viewCart();

    void viewOrderHistory();

    void customerMenu();
};

#endif
