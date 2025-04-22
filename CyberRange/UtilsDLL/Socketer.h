#pragma once
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "Connection.h"

// Link with Winsock library
#pragma comment(lib, "ws2_32.lib")

/**
 * Base class for socket-based connections
 */
class UTILS_API Socketer : public Connection {
protected:
    // Socket properties
    SOCKET socketFD;
    std::string address;
    int port;
    bool connected;
    bool isServer;
    int socketType;
    int timeout;

    // TLS/Security properties
    bool tlsEnabled;
    void* tlsContext;  // Opaque pointer for TLS context

    // Static Winsock initialization
    static bool winsockInitialized;

public:
    // Client constructor
    Socketer(const std::string& addr, int port);

    // Server constructor
    explicit Socketer(int port);

    // Socket created from accept()
    Socketer(SOCKET sock, const std::string& clientAddr, int clientPort);

    virtual ~Socketer();

    // Connection interface implementation
    bool connect() override;
    bool disconnect() override;
    bool isConnected() const override;

    int send(std::string data) override;
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
    // Helper methods
    bool initializeTLS();
    bool cleanupTLS();
    static bool initializeWinsock();
    static void cleanupWinsock();

    // Utility methods for derived classes
    bool setSocketOption(int level, int optname, const char* optval, int optlen);
    bool setNonBlocking(bool nonBlocking);
};