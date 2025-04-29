#pragma once
#include "Connection.h"
#include "TcpSocketWrapper.h"
#include <SFML/Network.hpp>
#include <openssl/ssl.h>
#include <memory>
#include <string>
#include <atomic>

class UTILS_API TCPSock : public Connection {
protected:
    std::unique_ptr<TcpSocketWithHandle> tcpSocket;
    std::unique_ptr<sf::TcpListener> tcpListener;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    std::atomic<bool> thisServerRunning;
    sf::Time timeout;
    bool tlsEnabled;
    std::unique_ptr<SSL, decltype(&SSL_free)> ssl;
    std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)> sslCtx;

    std::string certFile; 
    std::string keyFile;

public:
    TCPSock(const std::string& addr, int port);
    explicit TCPSock(int port);
    TCPSock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort);
    ~TCPSock() override;

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

    void setCertificates(const std::string& cert, const std::string& key) {
        certFile = cert;
        keyFile = key;
        printMessage("Certificate paths set: " + cert + ", " + key);
    }

protected:
    bool setupTLS();
    bool cleanupTLS();
    virtual void handleClientRequest(std::unique_ptr<TCPSock> clientSock);
};