#pragma once
#include <string>
#include "Connection.h"

class Socketer : public Connection {
private:
    std::string address;
    int port;
    bool connected;
    int socketType;
    int timeout;

public:
    Socketer(std::string addr, int port);
    bool connect() override;
    bool disconnect() override;
    bool isConnected() override;
    int send(std::string data) override;
    std::string receive() override;
    void setTimeout(int ms);
    std::string getAddress() override;
};
