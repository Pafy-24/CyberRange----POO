#include "pch.h"
#include "AdminConn.h"
#include <iostream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

AdminConn::AdminConn(int listenPort, const std::string& dbConnStr)
    : TCPSock(listenPort), DBConn(dbConnStr), privilege(0), secure(true), stopRequested(false) {
    enableTLS(); // Admin connections always use TLS
    printMessage("Admin server created on port " + std::to_string(listenPort));
}

AdminConn::AdminConn(std::unique_ptr<sf::TcpSocket> clientSock, const std::string& clientAddr, int clientPort, const std::string& dbConnStr)
    : TCPSock(std::move(clientSock), clientAddr, clientPort), DBConn(dbConnStr),
    privilege(0), secure(true), stopRequested(false) {
    enableTLS(); // Admin connections always use TLS
    printMessage("Admin client connection created for " + clientAddr + ":" + std::to_string(clientPort));
}

AdminConn::~AdminConn() {
    // Clear sensitive data
    adminKey.clear();

    // Clean up client connections
    for (auto* conn : clientConnections) {
        delete conn;
    }
    clientConnections.clear();

    // Stop server if running
    if (isServerRunning()) {
        stopServer();
    }

    // Disconnect
    disconnect();
}

bool AdminConn::connect() {
    // For server mode, rely on TCPSock's bind/listen
    if (TCPSock::isServerRunning()) {
        return TCPSock::isConnected();
    }

    // For client mode, connect both TCP and DB
    if (!TCPSock::isConnected() && !TCPSock::connect()) {
        std::cerr << "Failed to connect TCP socket" << std::endl;
        return false;
    }
    if (!DBConn::connect()) {
        std::cerr << "Failed to connect to database server" << std::endl;
        return false;
    }
    return true;
}

bool AdminConn::disconnect() {
    DBConn::disconnect();
    TCPSock::disconnect();
    return true;
}

bool AdminConn::isConnected() const {
    return TCPSock::isConnected() || DBConn::isConnected();
}

int AdminConn::send(const std::string& data) {
    return TCPSock::send(data); // Use TCPSock for client communication
}

std::string AdminConn::receive() {
    return TCPSock::receive(); // Use TCPSock for client communication
}

std::string AdminConn::getAddress() const {
    return TCPSock::getAddress(); // Use TCPSock address
}

int AdminConn::getPort() const {
    return TCPSock::getPort(); // Use TCPSock port
}

bool AdminConn::enableTLS() {
    bool tcpSuccess = TCPSock::enableTLS();
    bool dbSuccess = DBConn::enableTLS();
    return tcpSuccess && dbSuccess;
}

bool AdminConn::isTLSEnabled() const {
    return TCPSock::isTLSEnabled() && DBConn::isTLSEnabled();
}

void AdminConn::setTimeout(int ms) {
    TCPSock::setTimeout(ms);
    DBConn::setTimeout(ms);
}

int AdminConn::getTimeout() const {
    return TCPSock::getTimeout(); // Assume both have same timeout
}

bool AdminConn::bind(int bindPort) {
    return TCPSock::bind(bindPort);
}

bool AdminConn::listen(int backlog) {
    return TCPSock::listen(backlog);
}

Connection* AdminConn::accept() {
    Connection* newConn = TCPSock::accept();
    if (!newConn) {
        return nullptr;
    }

    TCPSock* tcpSock = dynamic_cast<TCPSock*>(newConn);
    if (!tcpSock) {
        delete newConn;
        return nullptr;
    }

    // Create a new AdminConn for this client connection
    //AdminConn* newAdminConn = new AdminConn(
    //    std::unique_ptr<sf::TcpSocket>(tcpSock->tcpSocket.release()),
    //    tcpSock->getAddress(),
    //    tcpSock->getPort(),
    //    DBConn::getAddress()
    //);

    //// Transfer settings
    //newAdminConn->setAdminKey(adminKey);
    //if (TCPSock::isTLSEnabled()) {
    //    newAdminConn->enableTLS();
    //}
    //if (!secure) {
    //    newAdminConn->disableSecureMode();
    //}

    //delete newConn; // Delete the TCPSock instance as we created a new AdminConn
    //return newAdminConn;
	return nullptr;
}

bool AdminConn::runServer() {
    if (!TCPSock::bind(TCPSock::getPort())) {
        std::cerr << "Failed to bind server socket" << std::endl;
        return false;
    }
    if (!TCPSock::listen()) {
        std::cerr << "Failed to listen on server socket" << std::endl;
        return false;
    }
    if (TCPSock::isServerRunning()) {
        std::cerr << "Server is already running" << std::endl;
        return true;
    }

    TCPSock::startListening([this](const std::string& data, Connection* client) {
        AdminConn* adminClient = dynamic_cast<AdminConn*>(client);
        if (adminClient) {
            adminClient->processClientRequest();
        }
        });

    stopRequested = false;
    std::cout << "Admin server started on port " << TCPSock::getPort() << std::endl;
    return true;
}

void AdminConn::stopServer() {
    if (!TCPSock::isServerRunning()) {
        std::cerr << "Server is not running" << std::endl;
        return;
    }
    stopRequested = true;
    TCPSock::stopServer();

    // Clean up client connections
    for (auto* conn : clientConnections) {
        conn->disconnect();
        delete conn;
    }
    clientConnections.clear();

    std::cout << "Server stopped" << std::endl;
}

bool AdminConn::isServerRunning() const {
    return TCPSock::isServerRunning();
}

void AdminConn::setCertificates(const std::string& cert, const std::string& key) {
    TCPSock::setCertificates(cert, key);
}

void AdminConn::setAdminKey(const std::string& key) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return;
    }
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, key.c_str(), key.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, nullptr) != 1) {
        std::cerr << "Failed to compute SHA256 hash" << std::endl;
        EVP_MD_CTX_free(ctx);
        return;
    }
    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    adminKey = ss.str();

    privilege = key.length() >= 16 ? 2 : key.length() >= 8 ? 1 : 0;
    std::cout << "Admin key set with privilege level " << privilege << std::endl;
}

bool AdminConn::verifyPrivilege(int level) const {
    if (privilege >= level) {
        return true;
    }
    std::cerr << "Insufficient privileges. Required: " << level << ", Current: " << privilege << std::endl;
    return false;
}

void AdminConn::enableSecureMode() {
    secure = true;
    std::cout << "Secure mode enabled" << std::endl;
}

void AdminConn::disableSecureMode() {
    if (verifyPrivilege(2)) {
        secure = false;
        std::cout << "Secure mode disabled" << std::endl;
    }
    else {
        std::cerr << "Insufficient privileges to disable secure mode" << std::endl;
    }
}

bool AdminConn::sanitizeQuery(std::string& query) {
    if (!secure) {
        std::cout << "Warning: Executing unsanitized query in admin mode" << std::endl;
        return true;
    }
    return DBConn::sanitizeQuery(query);
}

bool AdminConn::verifyAdminCommand(const std::string& command) {
    if (command.find("SHUTDOWN") == 0) {
        return verifyPrivilege(2);
    }
    if (command.find("CONFIG") == 0) {
        return verifyPrivilege(1);
    }
    if (command.find("STATUS") == 0) {
        return verifyPrivilege(0);
    }
    return true;
}

bool AdminConn::processClientRequest() {
    if (!TCPSock::isConnected()) {
        std::cerr << "Cannot process request: not connected" << std::endl;
        return false;
    }

    std::string request = TCPSock::receive();
    if (request.empty()) {
        return false;
    }

    if (request.find("ADMIN:") == 0) {
        std::string adminCommand = request.substr(6);
        if (!verifyAdminCommand(adminCommand)) {
            TCPSock::send("ERROR: Insufficient privileges for admin command");
            return true;
        }

        std::string response = "OK: Admin command processed";
        if (adminCommand.find("STATUS") == 0) {
            response = "Status: Connected to DB server " + DBConn::getAddress() +
                " with privilege level " + std::to_string(privilege);
        }
        return TCPSock::send(response) > 0;
    }

    if (!forwardToDBServer(request)) {
        TCPSock::send("ERROR: Failed to forward request to DB server");
        return false;
    }

    std::string dbResponse = DBConn::receive();
    return sendResponseToClient(dbResponse);
}

bool AdminConn::forwardToDBServer(const std::string& request) {
    if (!DBConn::isConnected() && !DBConn::connect()) {
        std::cerr << "Cannot forward request: not connected to DB server" << std::endl;
        return false;
    }

    std::string sanitizedRequest = request;
    if (secure && !sanitizeQuery(sanitizedRequest)) {
        std::cerr << "Request failed sanitization" << std::endl;
        return false;
    }
    return DBConn::send(sanitizedRequest) > 0;
}

bool AdminConn::sendResponseToClient(const std::string& response) {
    return TCPSock::send(response) > 0;
}

void AdminConn::handleRequest(const std::string& data, Connection* client) {
    if (client) {
        AdminConn* adminClient = dynamic_cast<AdminConn*>(client);
        if (adminClient) {
            adminClient->processClientRequest();
        }
    }
}