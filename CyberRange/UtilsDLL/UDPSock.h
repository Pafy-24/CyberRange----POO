#pragma once
#include <string>
#include "Socketer.h"

class UDPSock: public Socketer{
private:
    int socketFD;
    int maxPacketSize;

public:
    UDPSock(std::string addr, int port);
    void setMaxPacketSize(int size);
    bool broadcast(std::string data);
    std::string receiveFrom(std::string& sender);
};
