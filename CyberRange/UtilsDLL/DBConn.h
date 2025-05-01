#pragma once
#include "Connection.h"
#include "TCPSock.h"
#include <memory>
#include <string>
#include <stdexcept>

class UTILS_API DBConn : public Connection {
private:
    std::unique_ptr<TCPSock> dbSocket;
    std::string connectionString;
    std::string host;
    int port;
    std::string username;
    std::string password;
    std::string database;
    bool connected;
    int timeout;
    bool tlsEnabled;

    bool parseConnectionString();
    bool sanitizeQuery(std::string& query);

public:
    DBConn(const std::string& connStr);
    ~DBConn() override;

    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    int send(const std::string& query) override;
    std::string receive() override;
    std::string getAddress() const override;
    int getPort() const override;
    bool enableTLS() override;
    bool isTLSEnabled() const override;
    void setTimeout(int ms) override;
    int getTimeout() const override;
    bool sendLogin(const std::string& username, const std::string& password, std::string& roleOut);

    // Unsupported operations
    bool bind(int port) override { throw std::runtime_error("Operation not supported"); }
    bool listen(int backlog = 5) override { throw std::runtime_error("Operation not supported"); }
    Connection* accept() override { throw std::runtime_error("Operation not supported"); }
    bool runServer() override { throw std::runtime_error("Operation not supported"); }
    void stopServer() override { throw std::runtime_error("Operation not supported"); }
    bool isServerRunning() const override { return false; }
};