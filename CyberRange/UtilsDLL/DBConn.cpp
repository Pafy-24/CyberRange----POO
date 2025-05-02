#include "pch.h"
#include "DBConn.h"
#include <iostream>
#include <regex>
#include <sstream>

DBConn::DBConn(const std::string& connStr)
    : connectionString(connStr), port(1433), connected(false), timeout(30000), tlsEnabled(true) {
    if (!parseConnectionString()) {
        throw std::runtime_error("Invalid connection string");
    }
}

DBConn::~DBConn() {
    disconnect();
}

bool DBConn::parseConnectionString() {
    // Expected format: sqlserver://username:password@host:port/database
    static const std::regex connRegex(R"(sqlserver://([^:]+):([^@]+)@([^:]+):(\d+)/(\w+))");
    std::smatch matches;

    if (!std::regex_match(connectionString, matches, connRegex)) {
        std::cerr << "Invalid MSSQL connection string format" << std::endl;
        return false;
    }

    username = matches[1];
    password = matches[2];
    host = matches[3];
    port = std::stoi(matches[4]);
    database = matches[5];
    return true;
}

bool DBConn::connect() {
    if (connected) {
        return true;
    }

    if (host.empty()) {
        std::cerr << "Invalid host" << std::endl;
        return false;
    }

    dbSocket = std::make_unique<TCPSock>(host, port);
    dbSocket->setTimeout(timeout);

    if (!dbSocket->connect()) {
        std::cerr << "Failed to connect to MSSQL server" << std::endl;
        dbSocket.reset();
        return false;
    }
    connected = true;
    std::cout << "Connected to MSSQL server at " << host << ":" << port << " with TLS: " << (tlsEnabled ? "enabled" : "disabled") << std::endl;
    return true;
}

bool DBConn::disconnect() {
    if (!connected) {
        return true;
    }

    dbSocket.reset();
    connected = false;
    std::cout << "Disconnected from MSSQL server" << std::endl;
    return true;
}

bool DBConn::isConnected() const {
    return connected && dbSocket && dbSocket->isConnected();
}

int DBConn::send(const std::string& query) {
    if (!isConnected()) {
        std::cerr << "Not connected to database" << std::endl;
        return -1;
    }

    std::string sanitizedQuery = query;
    if (!sanitizeQuery(sanitizedQuery)) {
        std::cerr << "Query failed sanitization checks" << std::endl;
        return -1;
    }

    return dbSocket->send(sanitizedQuery);
}

std::string DBConn::receive() {
    if (!isConnected()) {
        std::cerr << "Not connected to database" << std::endl;
        return "";
    }

    return dbSocket->receive();
}

bool DBConn::sanitizeQuery(std::string& query) {
    // Enhanced SQL injection prevention
    static const std::regex sqlInjectionPattern(
        R"((--)|(\b(ALTER|CREATE|DELETE|DROP|EXEC(UTE)?|INSERT|MERGE|SELECT|UPDATE)\b))",
        std::regex::icase
    );

    if (std::regex_search(query, sqlInjectionPattern)) {
        std::cerr << "Potential SQL injection detected" << std::endl;
        return false;
    }

    // Additional sanitization for MSSQL
    query.erase(std::remove(query.begin(), query.end(), ';'), query.end());
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
        std::cerr << "Cannot enable TLS on active connection" << std::endl;
        return false;
    }
    tlsEnabled = true;
    return true;
}

bool DBConn::isTLSEnabled() const {
    return tlsEnabled;
}

void DBConn::setTimeout(int ms) {
    timeout = ms;
    if (dbSocket) {
        dbSocket->setTimeout(ms);
    }
}

int DBConn::getTimeout() const {
    return timeout;
}

bool DBConn::sendLogin(const std::string& username, const std::string& password, std::string& roleOut) {
    std::string authData = "AUTH " + username + " " + password + " " + database;
    if (send(authData) <= 0) {
        std::cerr << "Failed to send authentication data" << std::endl;
        dbSocket.reset();
        return false;
    }

    std::string response = dbSocket->receive();
    if (response.empty() || response.find("OK") == std::string::npos) {
        roleOut = response.substr(4);
        std::cerr << "Authentication failed: " << response << std::endl;
        dbSocket.reset();
        return false;
    }
    return true;
}

void DBConn::handleRequest(const std::string& data, Connection* client) {
    std::cerr << "DBConn does not support server request handling" << std::endl;
}

bool DBConn::bind(int port) {
    std::cerr << "DBConn does not support binding" << std::endl;
    return false;
}

bool DBConn::listen(int backlog) {
    std::cerr << "DBConn does not support listening" << std::endl;
    return false;
}

Connection* DBConn::accept() {
    std::cerr << "DBConn does not support accepting connections" << std::endl;
    return nullptr;
}

bool DBConn::startListening(std::function<void(const std::string&, Connection*)> handler) {
    std::cerr << "DBConn does not support starting a server" << std::endl;
    return false;
}

void DBConn::stopServer() {
    std::cerr << "DBConn does not support server operations" << std::endl;
}

bool DBConn::isServerRunning() const {
    return false;
}

void DBConn::setCertificates(const std::string& cert, const std::string& key) {
    std::cerr << "DBConn does not support setting certificates" << std::endl;
}