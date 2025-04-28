#pragma once
#include "Connection.h"
#include <string>
#include <SFML/Network.hpp>
#include <atomic>

class UTILS_API TCPSock : public Connection {
protected:
    sf::TcpSocket* tcpSocket;
    sf::TcpListener* tcpListener;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    sf::Time timeout;
    bool tlsEnabled;
    void* tlsContext;
    std::atomic<bool> serverRunning;

public:
    TCPSock(const std::string& addr, int port);
    explicit TCPSock(int port);
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

    bool runServer() override;
    void stopServer() override;
    bool isServerRunning() const override;

    void setBlocking(bool blocking);
    bool isBlocking() const;

protected:
    virtual bool setupTLS();
    virtual bool cleanupTLS();
    virtual void handleClientRequest(TCPSock* clientSock);
};