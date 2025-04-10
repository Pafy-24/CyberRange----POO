#pragma once
#include <string>

class Connection {
public:
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() = 0;
    virtual int send(std::string data) = 0;
    virtual std::string receive() = 0;
    virtual std::string getAddress() = 0;
    virtual ~Connection() = default;
};
