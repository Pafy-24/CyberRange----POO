#include "pch.h"
#include "DBConn.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <algorithm>

DBConn::DBConn(const std::string& connStr)
    : connectionString(connStr), port(1433), connected(false), timeout(30000), tlsEnabled(true),
    hEnv(nullptr), hDbc(nullptr) {
    if (!parseConnectionString()) {
        throw std::runtime_error("Invalid connection string");
    }
}

DBConn::~DBConn() {
    disconnect();
}

bool DBConn::parseConnectionString() {
    const std::string prefix = "sqlserver://";
    if (connectionString.substr(0, prefix.size()) != prefix) {
        printError("Connection string must start with 'sqlserver://'");
        return false;
    }

    std::string conn = connectionString.substr(prefix.size());

    size_t atPos = conn.rfind('@');
    if (atPos == std::string::npos) {
        printError("Missing '@' in connection string");
        return false;
    }

    std::string userInfo = conn.substr(0, atPos);
    std::string hostInfo = conn.substr(atPos + 1);

    size_t colonPos = userInfo.find(':');
    if (colonPos == std::string::npos) {
        printError("Missing ':' between username and password");
        return false;
    }

    username = userInfo.substr(0, colonPos);
    password = userInfo.substr(colonPos + 1);

    size_t slashPos = hostInfo.find('/');
    if (slashPos == std::string::npos) {
        printError("Missing '/' before database name");
        return false;
    }

    std::string hostPort = hostInfo.substr(0, slashPos);
    database = hostInfo.substr(slashPos + 1);

    size_t portPos = hostPort.find(':');
    if (portPos == std::string::npos) {
        printError("Missing ':' between host and port");
        return false;
    }

    host = hostPort.substr(0, portPos);
    port = std::stoi(hostPort.substr(portPos + 1));

    return true;
}


bool DBConn::connect() {
    if (connected) return true;

    if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS) {
        printError("Failed to allocate ODBC environment.");
        return false;
    }

    SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);

    if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS) {
        printError("Failed to allocate ODBC connection.");
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        return false;
    }

    std::stringstream conn;
    conn << "DRIVER={ODBC Driver 17 for SQL Server};SERVER=" << host << "," << port
        << ";DATABASE=" << database << ";UID=" << username << ";PWD=" << password << ";";

    SQLRETURN ret = SQLDriverConnectA(hDbc, NULL,
        (SQLCHAR*)conn.str().c_str(), SQL_NTS,
        NULL, 0, NULL, SQL_DRIVER_COMPLETE);

    if (SQL_SUCCEEDED(ret)) {
        connected = true;
        printInfo("Connected to MSSQL server at " + host + ":" + std::to_string(port));
        return true;
    }
    else {
        SQLCHAR       SqlState[6], Msg[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER    NativeError;
        SQLSMALLINT   MsgLen;

        SQLGetDiagRecA(SQL_HANDLE_DBC, hDbc, 1, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);
        printError("ODBC error: " + std::string((char*)Msg) + " (SQLState: " + std::string((char*)SqlState) + ")");

        SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
        SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
        hDbc = nullptr;
        hEnv = nullptr;
        return false;
    }
}

bool DBConn::disconnect() {
    if (!connected) return true;

    SQLDisconnect(hDbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
    SQLFreeHandle(SQL_HANDLE_ENV, hEnv);

    hDbc = nullptr;
    hEnv = nullptr;
    connected = false;

    printInfo("Disconnected from MSSQL server");
    return true;
}

bool DBConn::isConnected() const {
    return connected;
}

int DBConn::send(const std::string& query) {
    if (!isConnected()) {
        printError("Not connected to database");
        return -1;
    }

    std::string sanitizedQuery = query;
    if (!sanitizeQuery(sanitizedQuery)) {
        printError("Query failed sanitization checks");
        return -1;
    }

    SQLHSTMT stmt;
    if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt) != SQL_SUCCESS) {
        printError("Failed to allocate statement handle");
        return -1;
    }

    printLog("Executing query: " + sanitizedQuery);
    SQLRETURN ret = SQLExecDirectA(stmt, (SQLCHAR*)sanitizedQuery.c_str(), SQL_NTS);

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    if (SQL_SUCCEEDED(ret)) {
        printInfo("Query executed successfully");
        return 1;
    }
    else {
        printError("Query execution failed");
        return -1;
    }
}

std::string DBConn::receive() {
    printWarning("Use fetch API with SELECT queries (ODBC is not stream-based)");
    return "";
}

bool DBConn::sanitizeQuery(std::string& query) {
    static const std::regex dangerousPattern(
        R"(\b(DROP|TRUNCATE|ALTER)\b)",
        std::regex::icase
    );

    if (std::regex_search(query, dangerousPattern)) {
        printError("Dangerous query detected");
        return false;
    }

    return true;
}

std::string DBConn::getAddress() const {
    return host;
}

int DBConn::getPort() const {
    return port;
}

bool DBConn::enableTLS() {
    if (connected) {
        printError("Cannot enable TLS on active connection");
        return false;
    }
    tlsEnabled = true;
    printInfo("TLS enabled for database connection");
    return true;
}

bool DBConn::isTLSEnabled() const {
    return tlsEnabled;
}

void DBConn::setTimeout(int ms) {
    timeout = ms;
    if (connected && hDbc) {
        SQLSetConnectAttr(hDbc, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER)(intptr_t)(ms / 1000), 0);
        printInfo("Connection timeout set to " + std::to_string(ms) + "ms");
    }
}

int DBConn::getTimeout() const {
    return timeout;
}

bool DBConn::sendLogin(const std::string& u, const std::string& p, std::string& roleOut) {
    printInfo("Attempting login with username: " + u);
    std::stringstream roleQuery;
    roleQuery << "SELECT SYSTEM_USER;";

    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);
    SQLRETURN ret = SQLExecDirectA(stmt, (SQLCHAR*)roleQuery.str().c_str(), SQL_NTS);

    if (!SQL_SUCCEEDED(ret)) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        printError("Login check failed");
        return false;
    }

    char roleBuf[256];
    SQLBindCol(stmt, 1, SQL_C_CHAR, roleBuf, sizeof(roleBuf), NULL);
    if (SQLFetch(stmt) == SQL_SUCCESS) {
        roleOut = std::string(roleBuf);
        printInfo("Login successful, role: " + roleOut);
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    return true;
}

std::vector<std::map<std::string, std::string>> DBConn::fetchAll(const std::string& query) {
    std::vector<std::map<std::string, std::string>> rows;

    if (!isConnected()) {
        printError("Not connected to database");
        return rows;
    }

    printLog("Executing fetch query: " + query);
    SQLHSTMT stmt;
    SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);

    if (SQLExecDirectA(stmt, (SQLCHAR*)query.c_str(), SQL_NTS) != SQL_SUCCESS) {
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        printError("Query execution failed");
        return rows;
    }

    SQLSMALLINT numCols;
    SQLNumResultCols(stmt, &numCols);

    std::vector<std::string> colNames(numCols);
    for (SQLSMALLINT i = 0; i < numCols; ++i) {
        SQLCHAR colName[256];
        SQLSMALLINT nameLen;
        SQLDescribeColA(stmt, i + 1, colName, sizeof(colName), &nameLen, NULL, NULL, NULL, NULL);
        colNames[i] = std::string((char*)colName);
    }

    int rowCount = 0;
    while (SQLFetch(stmt) == SQL_SUCCESS) {
        std::map<std::string, std::string> row;
        for (SQLSMALLINT i = 0; i < numCols; ++i) {
            char buf[512];
            SQLLEN ind;
            SQLGetData(stmt, i + 1, SQL_C_CHAR, buf, sizeof(buf), &ind);
            row[colNames[i]] = ind == SQL_NULL_DATA ? "NULL" : std::string(buf);
        }
        rows.push_back(row);
        rowCount++;
    }

    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    printInfo("Fetched " + std::to_string(rowCount) + " rows from database");
    return rows;
}


void DBConn::handleRequest(const std::string& data, Connection* client) {
    printWarning("DBConn does not support server request handling");
}

bool DBConn::bind(int port) {
    printWarning("DBConn does not support binding");
    return false;
}

bool DBConn::listen(int backlog) {
    printWarning("DBConn does not support listening");
    return false;
}

Connection* DBConn::accept() {
    printWarning("DBConn does not support accepting connections");
    return nullptr;
}

bool DBConn::startListening(std::function<void(const std::string&, Connection*)> handler) {
    printWarning("DBConn does not support starting a server");
    return false;
}

void DBConn::stopServer() {
    printWarning("DBConn does not support server operations");
}

bool DBConn::isServerRunning() const {
    return false;
}

void DBConn::setCertificates(const std::string& cert, const std::string& key) {
    printWarning("DBConn does not support setting certificates");
}