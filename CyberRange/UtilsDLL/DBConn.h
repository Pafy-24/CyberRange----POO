#pragma once
#include "Connection.h"
#include "TCPSock.h"
#include <string>
#include <stdexcept> // For std::runtime_error

class UTILS_API DBConn : public Connection {
private:
    TCPSock* dbSocket;
    std::string connectionString;
    std::string host;
    int port;
    std::string username;
    std::string password;
    std::string database;
    std::string dbType;
    bool connected;
    int timeout;
    bool tlsEnabled;
    bool serverRunning; // Added for interface compatibility

protected:
    virtual bool sanitizeQuery(std::string& query);
    bool parseConnectionString();

public:
    DBConn(const std::string& connStr);
    virtual ~DBConn();

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

    // These methods are not applicable for DB connections and will throw exceptions
    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;

    // Server operations
    bool runServer() override;
    void stopServer() override;
    bool isServerRunning() const override;

    // DB specific methods
    void setDbType(const std::string& type);
    std::string getDbType() const { return dbType; }
};