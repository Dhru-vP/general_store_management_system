
#include <iostream>

#include "Database.h"
#include "Authentication.h"
#include "Product.h"
#include "Customer.h"

using namespace std;

int main()
{
    Database database;

    if (!database.connect())
        return 1;

    if (!database.initializeTables())
        return 1;

    Authentication auth(database);

    ProductManager productManager(database);

    if (!auth.createInitialAdmin())
        return 1;

    int choice;

    do
    {
        cout << "\n===== GENERAL STORE MANAGEMENT =====\n";
        cout << "1. Login\n";
        cout << "2. Customer Registration\n";
        cout << "0. Exit\n";

        cout << "\nEnter Choice: ";

        if (!(cin >> choice))
        {
            cout << "Invalid input!\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (choice)
        {
            case 1:
            {
                UserSession session{};

                if (auth.login(session))
                {
                    if (session.role == "admin")
                    {
                        productManager.adminMenu();
                    }
                    else if (session.role == "customer")
                    {
                        CustomerManager customerManager(
                            database,
                            session.userId,
                            session.username
                        );

                        customerManager.customerMenu();
                    }

                    session.loggedIn = false;

                    cout << "\nLogged Out Successfully!\n";
                }

                break;
            }

            case 2:
                auth.registerCustomer();
                break;

            case 0:
                cout << "\nThank you for visiting!\n";
                break;

            default:
                cout << "\nInvalid Choice!\n";
        }

    } while (choice != 0);

    return 0;
}
