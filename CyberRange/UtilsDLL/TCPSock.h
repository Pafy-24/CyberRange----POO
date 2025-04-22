#pragma once
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Socketer.h"

/**
 * TCP Socket implementation
 */
class UTILS_API TCPSock : public Socketer {
private:
    bool blocking;
    void* sslHandle;  // SSL connection handle

public:
    // Client constructor
    TCPSock(const std::string& addr, int port);

    // Server constructor
    explicit TCPSock(int port);

    // Accept constructor (used internally)
    TCPSock(SOCKET sock, const std::string& clientAddr, int clientPort);

    virtual ~TCPSock();

    // Override specific connection methods to handle TLS
    bool connect() override;
    bool disconnect() override;
    int send(std::string data) override;
    std::string receive() override;
    Connection* accept() override;

    // TCP-specific methods
    void setBlocking(bool block);
    bool isBlocking() const;
    bool enableTLS() override;

protected:
    bool setupTLS();
    bool teardownTLS();
};