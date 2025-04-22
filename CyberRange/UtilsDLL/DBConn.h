#pragma once
#include <string>
#include "Connection.h"

class DBConn : public Connection {
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
    bool isConnected() override;
    int send(std::string query) override;
    std::string receive() override;
    void setTimeout(int ms);
    std::string getAddress() override;
    bool enableTLS() override;
    bool isTLSEnabled() override;

    void setDbType(std::string type);
};