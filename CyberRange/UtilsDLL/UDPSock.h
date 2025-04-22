#pragma once
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Socketer.h"

class UTILS_API UDPSock : public Socketer {
private:
    SOCKET socketFD;
    int maxPacketSize;
    void* dtlsContext;  // For DTLS (UDP equivalent of TLS)

public:
    UDPSock(std::string addr, int port);
    virtual ~UDPSock();

    bool connect() override;
    bool disconnect() override;
    int send(std::string data) override;
    std::string receive() override;
    bool enableTLS() override;  // Will use DTLS instead of TLS

    void setMaxPacketSize(int size);
    bool broadcast(std::string data);
    std::string receiveFrom(std::string& sender);
};