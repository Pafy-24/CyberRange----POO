#pragma once
#include "Connection.h"
#include "TCPSock.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <sql.h>
#include <sqlext.h>

class UTILS_API DBConn : public Connection {
protected:
    std::unique_ptr<TCPSock> dbSocket;
    std::string connectionString;
    std::string host;
    int port;
    std::string username;
    std::string password;
    std::string database;
    SQLHENV hEnv = nullptr;
    SQLHDBC hDbc = nullptr;
    bool connected;
    int timeout;
    bool tlsEnabled;

    bool parseConnectionString();
    virtual bool sanitizeQuery(std::string& query);

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
    std::string getType() const override { return "DB"; }
    void handleRequest(const std::string& data, Connection* client = nullptr) override;

    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;
    bool startListening(std::function<void(const std::string&, Connection*)> handler) override;
    void stopServer() override;
    bool isServerRunning() const override;
    void setCertificates(const std::string& cert, const std::string& key) override;

    bool sendLogin(const std::string& username, const std::string& password, std::string& roleOut);
    std::vector<std::map<std::string, std::string>> fetchAll(const std::string& query);
};