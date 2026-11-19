
#include "Authentication.h"

#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

using namespace std;

namespace
{
    const int ITERATIONS = 210000;
    const int SALT_SIZE = 16;
    const int HASH_SIZE = 32;

    // Password masking function
    string getPasswordMasked()
    {
        string password;
        struct termios oldt, newt;

        // Clear the leftover newline after username input
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
       // Allow automated Google Tests to provide password input
if (!isatty(STDIN_FILENO))
{
    cin >> password;
    return password;
}
        // Get current terminal settings
        if (tcgetattr(STDIN_FILENO, &oldt) != 0)
        {
            cout << "Terminal error!\n";
            return "";
        }

        newt = oldt;

        // Disable normal character display and line buffering
        newt.c_lflag &= ~(ECHO | ICANON);

        newt.c_cc[VMIN] = 1;
        newt.c_cc[VTIME] = 0;

        if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0)
        {
            cout << "Terminal configuration failed!\n";
            return "";
        }

        char ch;

        while (cin.get(ch))
        {
            // Enter key
            if (ch == '\n' || ch == '\r')
                break;

            // Backspace handling
            if (ch == 127 || ch == 8)
            {
                if (!password.empty())
                {
                    password.pop_back();
                    cout << "\b \b" << flush;
                }
            }
            else
            {
                password += ch;
                cout << "*" << flush;
            }
        }

        // Restore original terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

        cout << endl;

        return password;
    }

    // Convert binary data into hexadecimal
    string toHex(const unsigned char* data, int length)
    {
        stringstream ss;

        ss << hex << setfill('0');

        for (int i = 0; i < length; i++)
        {
            ss << setw(2) << static_cast<int>(data[i]);
        }

        return ss.str();
    }

    // Convert hexadecimal into binary
    bool fromHex(
        const string& hexString,
        unsigned char* output,
        int outputSize)
    {
        if (hexString.size() != outputSize * 2)
            return false;

        for (int i = 0; i < outputSize; i++)
        {
            try
            {
                output[i] = static_cast<unsigned char>(
                    stoi(hexString.substr(i * 2, 2), nullptr, 16)
                );
            }
            catch (...)
            {
                return false;
            }
        }

        return true;
    }
}

// Constructor
Authentication::Authentication(Database& db)
    : database(db)
{
}

// Generate secure password hash
string Authentication::hashPassword(const string& password)
{
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];

    if (RAND_bytes(salt, SALT_SIZE) != 1)
        return "";

    if (PKCS5_PBKDF2_HMAC(
            password.c_str(),
            static_cast<int>(password.size()),
            salt,
            SALT_SIZE,
            ITERATIONS,
            EVP_sha256(),
            HASH_SIZE,
            hash) != 1)
    {
        return "";
    }

    return toHex(salt, SALT_SIZE) + "$" +
           toHex(hash, HASH_SIZE);
}

// Verify password
bool Authentication::verifyPassword(
    const string& password,
    const string& storedHash)
{
    size_t separator = storedHash.find('$');

    if (separator == string::npos)
        return false;

    string saltHex = storedHash.substr(0, separator);
    string hashHex = storedHash.substr(separator + 1);

    unsigned char salt[SALT_SIZE];
    unsigned char originalHash[HASH_SIZE];
    unsigned char newHash[HASH_SIZE];

    if (!fromHex(saltHex, salt, SALT_SIZE) ||
        !fromHex(hashHex, originalHash, HASH_SIZE))
    {
        return false;
    }

    if (PKCS5_PBKDF2_HMAC(
            password.c_str(),
            static_cast<int>(password.size()),
            salt,
            SALT_SIZE,
            ITERATIONS,
            EVP_sha256(),
            HASH_SIZE,
            newHash) != 1)
    {
        return false;
    }

    return CRYPTO_memcmp(
        originalHash,
        newHash,
        HASH_SIZE
    ) == 0;
}

// Create user account
bool Authentication::createUser(
    const string& username,
    const string& password,
    const string& role)
{
    if (username.empty() ||
        username.size() > 50 ||
        password.size() < 8 ||
        password.size() > 128)
    {
        cout << "Invalid username or password length!\n";
        return false;
    }

    string passwordHash = hashPassword(password);

    if (passwordHash.empty())
    {
        cout << "Password hashing failed!\n";
        return false;
    }

    const char* sql =
        "INSERT INTO users(username, password_hash, role) "
        "VALUES(?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    int result = sqlite3_prepare_v2(
        database.getConnection(),
        sql,
        -1,
        &stmt,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        cout << "Database error!\n";
        return false;
    }

    sqlite3_bind_text(
        stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        stmt, 3, role.c_str(), -1, SQLITE_TRANSIENT
    );

    result = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE)
    {
        cout << "Username already exists or database error!\n";
        return false;
    }

    return true;
}

// Create first admin account
bool Authentication::createInitialAdmin()
{
    const char* sql =
        "SELECT COUNT(*) FROM users WHERE role = 'admin';";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            database.getConnection(),
            sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Database error!\n";
        return false;
    }

    int count = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    if (count > 0)
        return true;

    string username, password;

    cout << "\n===== INITIAL ADMIN SETUP =====\n";

    cout << "Create Admin Username: ";
    cin >> username;

    cout << "Create Admin Password: ";
    password = getPasswordMasked();

    if (createUser(username, password, "admin"))
    {
        cout << "\nAdmin account created successfully!\n";
        return true;
    }

    return false;
}

// Customer registration
bool Authentication::registerCustomer()
{
    string username, password;

    cout << "\n===== CUSTOMER REGISTRATION =====\n";

    cout << "Enter Username: ";
    cin >> username;

    cout << "Enter Password (minimum 8 characters): ";
    password = getPasswordMasked();

    if (createUser(username, password, "customer"))
    {
        cout << "\nCustomer registered successfully!\n";
        return true;
    }

    return false;
}

// Login
bool Authentication::login(UserSession& session)
{
    string username, password;

    cout << "\n===== LOGIN =====\n";

    cout << "Enter Username: ";
    cin >> username;

    cout << "Enter Password: ";
    password = getPasswordMasked();

    const char* sql =
        "SELECT user_id, username, password_hash, role, is_active "
        "FROM users WHERE username = ?;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(
            database.getConnection(),
            sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Database error!\n";
        return false;
    }

    sqlite3_bind_text(
        stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT
    );

    int result = sqlite3_step(stmt);

    if (result != SQLITE_ROW)
    {
        cout << "\nInvalid username or password!\n";
        sqlite3_finalize(stmt);
        return false;
    }

    int userId = sqlite3_column_int(stmt, 0);

    string storedUsername =
        reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 1)
        );

    string storedHash =
        reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 2)
        );

    string role =
        reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 3)
        );

    int active = sqlite3_column_int(stmt, 4);

    sqlite3_finalize(stmt);

    if (!active || !verifyPassword(password, storedHash))
    {
        cout << "\nInvalid username or password!\n";
        return false;
    }

    session.userId = userId;
    session.username = storedUsername;
    session.role = role;
    session.loggedIn = true;

    cout << "\nLogin Successful!\n";
    cout << "Welcome, " << session.username << "!\n";
    cout << "Role: " << session.role << "\n";

    return true;
}
