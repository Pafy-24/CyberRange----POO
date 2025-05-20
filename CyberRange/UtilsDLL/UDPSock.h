#pragma once
#include "Connection.h"
#include "Observable.h" 
#include <string>
#include <SFML/Network.hpp>
#include <atomic>
#include <functional>

class UTILS_API UDPSock : public Connection, public Observable { 
protected:
    sf::UdpSocket* udpSocket;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    sf::Time timeout;
    bool tlsEnabled;
    void* dtlsContext;
    std::atomic<bool> thisServerRunning;
    std::function<void(const std::string&, Connection*)> requestHandler;

public:
    UDPSock(const std::string& addr, int port);
    virtual ~UDPSock();

    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;
    int send(const std::string& data) override;
    std::string receive() override;
    std::string getAddress() const override;
    int getPort() const override;
    bool enableTLS() override;
    bool isTLSEnabled() const override;
    void setTimeout(int ms) override;
    int getTimeout() const override;
    std::string getType() const override { return "UDP"; }
    void handleRequest(const std::string& data, Connection* client = nullptr) override;

    bool bind(int port) override;
    bool listen(int backlog = 5) override;
    Connection* accept() override;

    bool startListening(std::function<void(const std::string&, Connection*)> handler) override;
    void stopServer() override;
    bool isServerRunning() const override;
    void setCertificates(const std::string& cert, const std::string& key) override;

protected:
    bool setupDTLS();
    bool cleanupDTLS();
    virtual void handleClientRequest();
};