# General Store Management System

A console-based General Store Management System developed using **C++, SQLite and OpenSSL**. The application provides role-based authentication, product inventory management, customer shopping, order processing, billing and payment record management.

## Features

### 1. Role-Based Authentication
- Admin and Customer login.
- Customer registration.
- Secure password hashing using PBKDF2 with SHA-256.
- Separate Admin and Customer dashboards.
- Role-based access to application features.

### 2. Admin Product Management
- Add new products.
- View available products.
- Search products by name.
- Update product information.
- Delete products using soft deletion.
- Track product quantity and price.

### 3. Customer Shopping
- Browse available products.
- Search for products.
- Add products to shopping cart.
- View cart and calculate total amount.
- Validate product availability.
- Confirm or cancel purchases.

### 4. Order and Billing Management
- Generate bills after successful checkout.
- Automatically update inventory.
- Store customer orders.
- Maintain order history.
- Record cash payment transactions.

### 5. Database Management
- SQLite database integration.
- Automatic table initialization.
- Persistent storage of users, products, orders, order items and payments.
- Prepared SQL statements for database operations.
- Database transactions for checkout consistency.

## Technologies Used

- **Programming Language:** C++
- **Database:** SQLite3
- **Cryptography:** OpenSSL
- **Build Tools:** GCC / G++ and CMake
- **Development Environment:** GitHub Codespaces / VS Code
- **Version Control:** Git and GitHub

## Project Structure

```text
general_store_management_system/
│
├── database/
│   └── store.db
│
├── include/
│   ├── Admin.h
│   ├── Authentication.h
│   ├── Billing.h
│   ├── Customer.h
│   ├── Database.h
│   └── Product.h
│
├── src/
│   ├── Admin.cpp
│   ├── Authentication.cpp
│   ├── Billing.cpp
│   ├── Customer.cpp
│   ├── Database.cpp
│   └── Product.cpp
│
├── reports/
│   ├── inventory.csv
│   └── sales.csv
│
├── main.cpp
├── CMakeLists.txt
├── README.md
└── .gitignore
```

## Prerequisites

Install the following dependencies on Ubuntu:

```bash
sudo apt update

sudo apt install g++ sqlite3 libsqlite3-dev libssl-dev cmake git
```

Verify installations:

```bash
g++ --version
sqlite3 --version
openssl version
cmake --version
```

## Compilation and Execution

Clone the repository:

```bash
git clone https://github.com/YOUR_USERNAME/general_store_management_system.git
```

Navigate into the project:

```bash
cd general_store_management_system
```

Compile the application:

```bash
g++ -std=c++17 -Iinclude main.cpp src/Database.cpp src/Authentication.cpp src/Product.cpp src/Customer.cpp -lsqlite3 -lcrypto -o store
```

Run:

```bash
./store
```

The application initializes the database tables and prompts for initial Admin account creation if no Admin account exists.

## Database Tables

| Table | Purpose |
|---|---|
| users | Stores user accounts and roles |
| products | Stores product and inventory information |
| orders | Stores customer orders |
| order_items | Stores individual purchased products |
| payments | Stores payment records |

## Security

- Passwords are stored as salted PBKDF2-SHA256 hashes.
- Password verification uses constant-time comparison.
- SQL prepared statements are used for database operations.
- Checkout operations use SQLite transactions.
- Product deletion uses soft deletion to preserve historical records.

## Future Improvements

- Excel report generation.
- Sales analytics and dashboard.
- Invoice export to PDF.
- Online payment gateway integration.
- Product categories and supplier management.
- Automated database backups.
- Unit testing and improved input validation.

---

*This project was developed to demonstrate C++ programming, object-oriented design, database management, authentication, SQL CRUD operations and transaction handling.*