#pragma once
#include "Connection.h"
#include <string>
#include <SFML/Network.hpp>

class UTILS_API TCPSock : public Connection {
protected:
    // Socket properties
    sf::TcpSocket* tcpSocket;
    sf::TcpListener* tcpListener;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    sf::Time timeout;
    bool tlsEnabled;
    void* tlsContext; // Pentru implementarea TLS

public:
    // Client constructor
    TCPSock(const std::string& addr, int port);

    // Server constructor
    explicit TCPSock(int port);

    // Socket created from accept()
    TCPSock(sf::TcpSocket* sock, const std::string& clientAddr, int clientPort);

    virtual ~TCPSock();

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

    void setBlocking(bool blocking);
    bool isBlocking() const;

protected:
    // Helper pentru TLS
    virtual bool setupTLS();
    virtual bool cleanupTLS();
};