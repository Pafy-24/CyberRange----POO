#pragma once
#include "Connection.h"
#include <string>
#include <SFML/Network.hpp>

class UTILS_API UDPSock : public Connection {
protected:
    sf::UdpSocket* udpSocket;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    sf::Time timeout;
    int maxPacketSize;

    // Pentru DTLS (echivalentul UDP al TLS)
    bool tlsEnabled;
    void* dtlsContext;

public:
    UDPSock(const std::string& addr, int port);
    virtual ~UDPSock();

    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;

    int send(const std::string& data) override;
    std::string receive() override;

    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;

    std::string getAddress() const override;
    int getPort() const override;

    bool enableTLS() override;
    bool isTLSEnabled() const override;

    void setTimeout(int ms) override;
    int getTimeout() const override;

    void setMaxPacketSize(int size);
    bool broadcast(const std::string& data);
    std::string receiveFrom(std::string& sender);

protected:
    bool setupDTLS();
    bool cleanupDTLS();
};