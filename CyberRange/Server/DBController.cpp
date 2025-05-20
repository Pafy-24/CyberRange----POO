#include "DBController.h"
#include <stdexcept>
#include <regex>


DBController::DBController(std::string connString)
    : CController("DBController"), connectionString(connString), isConnected(false) {
    dbConnection = new DBConn(connectionString);
}

DBController::~DBController() {
    disconnect();
    delete dbConnection;
}

bool DBController::sanitizeInput(const std::string& input) {
    static const std::regex injectionPattern(R"([;'"-]|/\*|\*/|\b(OR|AND|UNION|SELECT|INSERT|DELETE|UPDATE)\b)", std::regex::icase);
    if (std::regex_search(input, injectionPattern)) {
        printError("Potential SQL injection detected in input: " + input);
        return false;
    }
    return true;
}

bool DBController::connect() {
    try {
        if (!isConnected) {
            isConnected = dbConnection->connect();
            if (isConnected) {
                printInfo("Database connected successfully");
            }
            else {
                printError("Failed to connect to database");
            }
        }
        return isConnected;
    }
    catch (const std::exception& e) {
        printError("Database connection error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::disconnect() {
    try {
        if (isConnected) {
            isConnected = !dbConnection->disconnect();
            printInfo("Database disconnected");
        }
        return !isConnected;
    }
    catch (const std::exception& e) {
        printError("Database disconnection error: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::map<std::string, std::string>> DBController::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    try {
        if (!isConnected && !connect()) {
            printError("Query failed: No database connection");
            return {};
        }

        std::string finalQuery = query;
        if (!params.empty()) {
            size_t pos = 0;
            std::string tempQuery = query;
            for (const auto& param : params) {
                if (!sanitizeInput(param)) {
                    printError("Query rejected due to unsafe parameter: " + param);
                    return {};
                }
                std::string escapedParam = param;
                size_t pos = 0;
                while ((pos = escapedParam.find('\'', pos)) != std::string::npos) {
                    escapedParam.replace(pos, 1, "\'\'");
                    pos += 2;
                }
                pos = tempQuery.find('?', pos);
                if (pos == std::string::npos) {
                    printError("Query parameter count mismatch");
                    return {};
                }
                tempQuery.replace(pos, 1, "'" + escapedParam + "'");
                pos += escapedParam.length() + 2;
            }
            finalQuery = tempQuery;
        }

        if (!dbConnection->sanitizeQuery(finalQuery)) {
            printError("Query failed sanitization: " + finalQuery);
            return {};
        }

        auto results = dbConnection->fetchAll(finalQuery);
        printInfo("Query executed: " + finalQuery);
        return results;
    }
    catch (const std::exception& e) {
        printError("Query error: " + std::string(e.what()));
        return {};
    }
}

bool DBController::executeUpdate(const std::string& query, const std::vector<std::string>& params) {
    try {
        if (!isConnected && !connect()) {
            printError("Update failed: No database connection");
            return false;
        }

        std::string finalQuery = query;
        if (!params.empty()) {
            size_t pos = 0;
            std::string tempQuery = query;
            for (const auto& param : params) {
                if (!sanitizeInput(param)) {
                    printError("Update rejected due to unsafe parameter: " + param);
                    return false;
                }
                std::string escapedParam = param;
                size_t pos = 0;
                while ((pos = escapedParam.find('\'', pos)) != std::string::npos) {
                    escapedParam.replace(pos, 1, "\'\'");
                    pos += 2;
                }
                pos = tempQuery.find('?', pos);
                if (pos == std::string::npos) {
                    printError("Update parameter count mismatch");
                    return false;
                }
                tempQuery.replace(pos, 1, "'" + escapedParam + "'");
                pos += escapedParam.length() + 2;
            }
            finalQuery = tempQuery;
        }

        if (!dbConnection->sanitizeQuery(finalQuery)) {
            printError("Update failed sanitization: " + finalQuery);
            return false;
        }

        bool success = dbConnection->send(finalQuery) == 1;
        if (success) {
            printInfo("Update executed: " + finalQuery);
        }
        else {
            printError("Update failed: " + finalQuery);
        }
        return success;
    }
    catch (const std::exception& e) {
        printError("Update error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::beginTransaction() {
    try {
        if (!isConnected && !connect()) {
            printError("Transaction failed: No database connection");
            return false;
        }
        bool success = dbConnection->send("BEGIN TRANSACTION;") == 1;
        if (success) {
            printInfo("Transaction begun");
        }
        else {
            printError("Failed to begin transaction");
        }
        return success;
    }
    catch (const std::exception& e) {
        printError("Transaction begin error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::commitTransaction() {
    try {
        if (!isConnected) {
            printError("Transaction commit failed: No database connection");
            return false;
        }
        bool success = dbConnection->send("COMMIT;") == 1;
        if (success) {
            printInfo("Transaction committed");
        }
        else {
            printError("Failed to commit transaction");
        }
        return success;
    }
    catch (const std::exception& e) {
        printError("Transaction commit error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::rollbackTransaction() {
    try {
        if (!isConnected) {
            printError("Transaction rollback failed: No database connection");
            return false;
        }
        bool success = dbConnection->send("ROLLBACK;") == 1;
        if (success) {
            printInfo("Transaction rolled back");
        }
        else {
            printError("Failed to roll back transaction");
        }
        return success;
    }
    catch (const std::exception& e) {
        printError("Transaction rollback error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::testConnection() {
    try {
        if (!isConnected && !connect()) {
            printError("Connection test failed: Unable to connect");
            return false;
        }
        // Test with a simple query
        auto results = dbConnection->fetchAll("SELECT 1;");
        bool success = !results.empty();
        if (success) {
            printInfo("Database connection test passed");
        }
        else {
            printError("Database connection test failed");
        }
        return success;
    }
    catch (const std::exception& e) {
        printError("Connection test error: " + std::string(e.what()));
        return false;
    }
}