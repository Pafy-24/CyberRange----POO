#pragma once
#include <string>
#include "Socketer.h"

class TCPSock : public Socketer {
private:
    int socketFD;
    bool blocking;
    void* sslHandle;  // SSL connection handle

public:
    TCPSock(std::string addr, int port);
    virtual ~TCPSock();

    bool connect() override;
    bool disconnect() override;
    int send(std::string data) override;
    std::string receive() override;

    void setBlocking(bool block);
    int setTimeout(int ms);
    bool isBlocking();
};