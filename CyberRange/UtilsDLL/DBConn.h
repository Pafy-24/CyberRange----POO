#pragma once
#include <string>
#include "Connection.h"

class DBConn : public Connection {
private:
    std::string connectionString;
    bool connected;
    std::string dbType;
    int timeout;

public:
    DBConn(std::string connStr);
    bool connect() override;
    bool disconnect() override;
    bool isConnected() override;
    int send(std::string query) override;
    std::string receive() override;
    void setTimeout(int ms);
    std::string getAddress() override;
};
