#pragma once
#include <string>
#include "Socketer.h"


class TCPSock : public Socketer {
private:
    int socketFD;
    bool blocking;

public:
    TCPSock(std::string addr, int port);
    void setBlocking(bool block);
    int setTimeout(int ms);
    bool isBlocking();
};
