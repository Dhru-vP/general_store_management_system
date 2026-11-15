
#ifndef PRODUCT_H
#define PRODUCT_H

#include "Database.h"

class ProductManager
{
private:
    Database& database;

public:
    ProductManager(Database& db);

    void addProduct();

    void displayProducts();

    void searchProduct();

    void updateProduct();

    void deleteProduct();

    void adminMenu();
};

#endif
