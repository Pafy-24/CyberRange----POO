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
        logger->log("Potential SQL injection detected in input: " + input);
        return false;
    }
    return true;
}

bool DBController::connect() {
    try {
        if (!isConnected) {
            isConnected = dbConnection->connect();
            if (isConnected) {
                logger->log("Database connected successfully");
            }
            else {
                logger->log("Failed to connect to database");
            }
        }
        return isConnected;
    }
    catch (const std::exception& e) {
        logger->log("Database connection error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::disconnect() {
    try {
        if (isConnected) {
            isConnected = !dbConnection->disconnect();
            logger->log("Database disconnected");
        }
        return !isConnected;
    }
    catch (const std::exception& e) {
        logger->log("Database disconnection error: " + std::string(e.what()));
        return false;
    }
}

std::vector<std::map<std::string, std::string>> DBController::executeQuery(const std::string& query, const std::vector<std::string>& params) {
    try {
        if (!isConnected && !connect()) {
            logger->log("Query failed: No database connection");
            return {};
        }

        std::string finalQuery = query;
        if (!params.empty()) {
            size_t pos = 0;
            std::string tempQuery = query;
            for (const auto& param : params) {
                if (!sanitizeInput(param)) {
                    logger->log("Query rejected due to unsafe parameter: " + param);
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
                    logger->log("Query parameter count mismatch");
                    return {};
                }
                tempQuery.replace(pos, 1, "'" + escapedParam + "'");
                pos += escapedParam.length() + 2;
            }
            finalQuery = tempQuery;
        }

        if (!dbConnection->sanitizeQuery(finalQuery)) {
            logger->log("Query failed sanitization: " + finalQuery);
            return {};
        }

        auto results = dbConnection->fetchAll(finalQuery);
        logger->log("Query executed: " + finalQuery);
        return results;
    }
    catch (const std::exception& e) {
        logger->log("Query error: " + std::string(e.what()));
        return {};
    }
}

bool DBController::executeUpdate(const std::string& query, const std::vector<std::string>& params) {
    try {
        if (!isConnected && !connect()) {
            logger->log("Update failed: No database connection");
            return false;
        }

        std::string finalQuery = query;
        if (!params.empty()) {
            size_t pos = 0;
            std::string tempQuery = query;
            for (const auto& param : params) {
                if (!sanitizeInput(param)) {
                    logger->log("Update rejected due to unsafe parameter: " + param);
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
                    logger->log("Update parameter count mismatch");
                    return false;
                }
                tempQuery.replace(pos, 1, "'" + escapedParam + "'");
                pos += escapedParam.length() + 2;
            }
            finalQuery = tempQuery;
        }

        if (!dbConnection->sanitizeQuery(finalQuery)) {
            logger->log("Update failed sanitization: " + finalQuery);
            return false;
        }

        bool success = dbConnection->send(finalQuery) == 1;
        logger->log(success ? "Update executed: " + finalQuery : "Update failed: " + finalQuery);
        return success;
    }
    catch (const std::exception& e) {
        logger->log("Update error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::beginTransaction() {
    try {
        if (!isConnected && !connect()) {
            logger->log("Transaction failed: No database connection");
            return false;
        }
        bool success = dbConnection->send("BEGIN TRANSACTION;") == 1;
        logger->log(success ? "Transaction begun" : "Failed to begin transaction");
        return success;
    }
    catch (const std::exception& e) {
        logger->log("Transaction begin error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::commitTransaction() {
    try {
        if (!isConnected) {
            logger->log("Transaction commit failed: No database connection");
            return false;
        }
        bool success = dbConnection->send("COMMIT;") == 1;
        logger->log(success ? "Transaction committed" : "Failed to commit transaction");
        return success;
    }
    catch (const std::exception& e) {
        logger->log("Transaction commit error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::rollbackTransaction() {
    try {
        if (!isConnected) {
            logger->log("Transaction rollback failed: No database connection");
            return false;
        }
        bool success = dbConnection->send("ROLLBACK;") == 1;
        logger->log(success ? "Transaction rolled back" : "Failed to roll back transaction");
        return success;
    }
    catch (const std::exception& e) {
        logger->log("Transaction rollback error: " + std::string(e.what()));
        return false;
    }
}

bool DBController::testConnection() {
    try {
        if (!isConnected && !connect()) {
            logger->log("Connection test failed: Unable to connect");
            return false;
        }
        // Test with a simple query
        auto results = dbConnection->fetchAll("SELECT 1;");
        bool success = !results.empty();
        logger->log(success ? "Database connection test passed" : "Database connection test failed");
        return success;
    }
    catch (const std::exception& e) {
        logger->log("Connection test error: " + std::string(e.what()));
        return false;
    }
}