#pragma once
#include <string>
#include "Connection.h"

class UTILS_API DBConn : public Connection {
private:
    std::string connectionString;
    bool connected;
    std::string dbType;
    int timeout;
    void* dbHandle;        // Database connection handle
    void* sslContext;      // SSL context for DB connection
    bool tlsEnabled;

protected:
    virtual bool sanitizeQuery(std::string& query);

public:
    DBConn(std::string connStr);
    virtual ~DBConn();

    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    int send(std::string query) override;
    std::string receive() override;
    std::string getAddress() const override;
    bool enableTLS() override;
    bool isTLSEnabled() const override;


    int getPort() const override;
    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;
    int getTimeout() const override;


    // Metode specifice pentru DBConn
    void setTimeout(int ms);
    void setDbType(std::string type);
    std::string getDbType() const { return dbType; }
};