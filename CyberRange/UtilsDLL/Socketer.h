#pragma once
/*
#include "Connection.h"
#include <string>
#include <SFML/Network.hpp>

class UTILS_API Socketer : public Connection {
protected:
    // Socket properties
    sf::Socket* socket;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    sf::Socket::Type socketType;
    sf::Time timeout;

    // TLS/Security properties
    bool tlsEnabled;
    void* tlsContext;  // Opaque pointer for TLS context

public:
    // Client constructor
    Socketer(const std::string& addr, int port);

    // Server constructor
    explicit Socketer(int port);

    // Socket created from accept()
    Socketer(sf::Socket* sock, const std::string& clientAddr, int clientPort);

    virtual ~Socketer();

    // Connection interface implementation
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

protected:
    // Helper methods for TLS
    virtual bool initializeTLS();
    virtual bool cleanupTLS();

    // Utility methods for derived classes
    bool setSocketOption(int option, int value);
    bool setBlocking(bool blocking);
};
*/