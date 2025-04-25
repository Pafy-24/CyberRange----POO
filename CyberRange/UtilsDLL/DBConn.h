#pragma once
#include "Connection.h"
#include "TCPSock.h"
#include <string>

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

    // These methods are not applicable for DB connections but required by interface
    bool bind(int port) override { return false; }
    bool listen(int backlog = 5) override { return false; }
    Connection* accept() override { return nullptr; }

    // DB specific methods
    void setDbType(const std::string& type);
    std::string getDbType() const { return dbType; }
};