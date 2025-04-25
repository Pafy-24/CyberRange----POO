#include "pch.h"
#include "AdminConn.h"
#include <iostream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

AdminConn::AdminConn(int listenPort, const std::string& dbConnStr)
    : dbConnection(nullptr), clientSocket(nullptr), serverSocket(nullptr),
    privilege(0), secure(true), connected(false), port(listenPort),
    timeout(30000), tlsEnabled(false) {

    // Create database connection
    dbConnection = new DBConn(dbConnStr);

    // Admin connections should always use TLS
    enableTLS();
}

AdminConn::AdminConn(TCPSock* clientSock, const std::string& dbConnStr)
    : dbConnection(nullptr), clientSocket(clientSock), serverSocket(nullptr),
    privilege(0), secure(true), connected(false), port(0),
    timeout(30000), tlsEnabled(false) {

    // Create database connection
    dbConnection = new DBConn(dbConnStr);

    if (clientSocket) {
        connected = true;
        port = clientSocket->getPort();
    }

    // Admin connections should always use TLS
    enableTLS();
}

AdminConn::~AdminConn() {
    // Clear sensitive data
    adminKey.clear();

    // Disconnect and clean up
    if (connected) {
        disconnect();
    }

    if (dbConnection) {
        delete dbConnection;
        dbConnection = nullptr;
    }

    if (serverSocket) {
        delete serverSocket;
        serverSocket = nullptr;
    }

    // Note: clientSocket is handled by disconnect()
}

bool AdminConn::connect() {
    // If we're in server mode, bind and listen
    if (serverSocket) {
        return serverSocket->isConnected();
    }

    // If we have a direct client connection
    if (clientSocket) {
        if (!dbConnection->connect()) {
            std::cerr << "Failed to connect to database server" << std::endl;
            return false;
        }
        return clientSocket->isConnected() && dbConnection->isConnected();
    }

    return false;
}

bool AdminConn::disconnect() {
    connected = false;

    // Disconnect from database
    if (dbConnection) {
        dbConnection->disconnect();
    }

    // Close client connection if exists
    if (clientSocket) {
        clientSocket->disconnect();
        delete clientSocket;
        clientSocket = nullptr;
    }

    return true;
}

bool AdminConn::isConnected() const {
    return connected &&
        ((clientSocket && clientSocket->isConnected()) ||
            (serverSocket && serverSocket->isConnected()));
}

int AdminConn::send(const std::string& data) {
    if (!connected || !clientSocket) {
        std::cerr << "Cannot send: not connected to client" << std::endl;
        return -1;
    }

    return clientSocket->send(data);
}

std::string AdminConn::receive() {
    if (!connected || !clientSocket) {
        std::cerr << "Cannot receive: not connected to client" << std::endl;
        return "";
    }

    return clientSocket->receive();
}

std::string AdminConn::getAddress() const {
    if (clientSocket) {
        return clientSocket->getAddress();
    }
    return "0.0.0.0";
}

int AdminConn::getPort() const {
    return port;
}

bool AdminConn::enableTLS() {
    tlsEnabled = true;

    if (serverSocket) {
        serverSocket->enableTLS();
    }

    if (clientSocket) {
        clientSocket->enableTLS();
    }

    if (dbConnection) {
        dbConnection->enableTLS();
    }

    return true;
}

bool AdminConn::isTLSEnabled() const {
    return tlsEnabled;
}

void AdminConn::setTimeout(int ms) {
    timeout = ms;

    if (serverSocket) {
        serverSocket->setTimeout(ms);
    }

    if (clientSocket) {
        clientSocket->setTimeout(ms);
    }

    if (dbConnection) {
        dbConnection->setTimeout(ms);
    }
}

int AdminConn::getTimeout() const {
    return timeout;
}

bool AdminConn::bind(int bindPort) {
    if (connected || serverSocket != nullptr) {
        std::cerr << "Cannot bind: already in use" << std::endl;
        return false;
    }

    if (bindPort > 0) {
        port = bindPort;
    }

    serverSocket = new TCPSock(port);

    if (tlsEnabled) {
        serverSocket->enableTLS();
    }

    serverSocket->setTimeout(timeout);

    bool result = serverSocket->bind(port);
    if (!result) {
        delete serverSocket;
        serverSocket = nullptr;
    }

    return result;
}

bool AdminConn::listen(int backlog) {
    if (!serverSocket) {
        std::cerr << "Cannot listen: no server socket" << std::endl;
        return false;
    }

    return serverSocket->listen(backlog);
}

Connection* AdminConn::accept() {
    if (!serverSocket) {
        std::cerr << "Cannot accept: no server socket" << std::endl;
        return nullptr;
    }

    TCPSock* newClientSocket = dynamic_cast<TCPSock*>(serverSocket->accept());
    if (!newClientSocket) {
        return nullptr;
    }

    // Create a new AdminConn for this client connection
    AdminConn* newAdminConn = new AdminConn(newClientSocket,
        dbConnection ? dbConnection->getAddress() : "");

    // Transfer settings
    newAdminConn->setAdminKey(adminKey);
    if (tlsEnabled) {
        newAdminConn->enableTLS();
    }

    if (!secure) {
        newAdminConn->disableSecureMode();
    }

    return newAdminConn;
}

void AdminConn::setAdminKey(const std::string& key) {
    // Hash the key for security using EVP API
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, key.c_str(), key.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, nullptr) != 1) {
        std::cerr << "Failed to compute SHA256 hash using EVP API" << std::endl;
        EVP_MD_CTX_free(ctx);
        return;
    }

    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    adminKey = ss.str();

    // Set privilege level based on key length (just a simple example)
    if (key.length() >= 16) {
        privilege = 2; // Full admin
    }
    else if (key.length() >= 8) {
        privilege = 1; // Limited admin
    }
    else {
        privilege = 0; // No privileges
    }

    std::cout << "Admin key set with privilege level " << privilege << std::endl;
}

bool AdminConn::verifyPrivilege(int level) const {
    if (privilege >= level) {
        return true;
    }

    std::cerr << "Insufficient privileges. Required level: " << level
        << ", Current level: " << privilege << std::endl;
    return false;
}

void AdminConn::enableSecureMode() {
    secure = true;
    std::cout << "Secure mode enabled" << std::endl;
}

void AdminConn::disableSecureMode() {
    if (verifyPrivilege(2)) { // Only highest privilege can disable secure mode
        secure = false;
        std::cout << "Secure mode disabled" << std::endl;
    }
    else {
        std::cerr << "Insufficient privileges to disable secure mode" << std::endl;
    }
}

bool AdminConn::sanitizeQuery(std::string& query) {
    // Admin connections bypass SQL injection checks if secure mode is disabled
    if (!secure) {
        std::cout << "Warning: Executing unsanitized query in admin mode" << std::endl;
        return true;
    }

    // Otherwise use the DBConn sanitization
    return dbConnection ? dbConnection->send(query) > 0 : false;
}

bool AdminConn::verifyAdminCommand(const std::string& command) {
    // Check if the command is an admin command and verify privileges

    // Example of admin commands
    if (command.find("SHUTDOWN") == 0) {
        return verifyPrivilege(2); // Require highest privilege
    }
    if (command.find("CONFIG") == 0) {
        return verifyPrivilege(1); // Require medium privilege
    }
    if (command.find("STATUS") == 0) {
        return verifyPrivilege(0); // Anyone can check status
    }

    // By default, allow the command
    return true;
}

bool AdminConn::processClientRequest() {
    if (!connected || !clientSocket || !dbConnection) {
        std::cerr << "Cannot process request: not properly connected" << std::endl;
        return false;
    }

    // Receive request from client
    std::string request = clientSocket->receive();
    if (request.empty()) {
        return false;
    }

    // Check if it's an admin command
    if (request.find("ADMIN:") == 0) {
        std::string adminCommand = request.substr(6); // Skip "ADMIN:"

        if (!verifyAdminCommand(adminCommand)) {
            std::string errorMsg = "ERROR: Insufficient privileges for admin command";
            clientSocket->send(errorMsg);
            return true;
        }

        // Process admin command locally
        std::string response = "OK: Admin command processed";

        if (adminCommand.find("STATUS") == 0) {
            response = "Status: Connected to DB server " + dbConnection->getAddress() +
                " with privilege level " + std::to_string(privilege);
        }

        // Send response back to client
        clientSocket->send(response);
        return true;
    }

    // It's a regular request, forward to DB server
    if (!forwardToDBServer(request)) {
        clientSocket->send("ERROR: Failed to forward request to DB server");
        return false;
    }

    // Get response from DB server
    std::string dbResponse = dbConnection->receive();

    // Send DB response back to client
    return sendResponseToClient(dbResponse);
}

bool AdminConn::forwardToDBServer(const std::string& request) {
    if (!dbConnection || !dbConnection->isConnected()) {
        if (!dbConnection->connect()) {
            std::cerr << "Cannot forward request: not connected to DB server" << std::endl;
            return false;
        }
    }

    std::string sanitizedRequest = request;
    if (secure && !sanitizeQuery(sanitizedRequest)) {
        std::cerr << "Request failed sanitization" << std::endl;
        return false;
    }

    return dbConnection->send(sanitizedRequest) > 0;
}

bool AdminConn::sendResponseToClient(const std::string& response) {
    if (!clientSocket || !clientSocket->isConnected()) {
        std::cerr << "Cannot send response: client not connected" << std::endl;
        return false;
    }

    return clientSocket->send(response) > 0;
}