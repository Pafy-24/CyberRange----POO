#pragma once
#include "DLL.h"
#include <string>
#include <functional>

class UTILS_API Connection {
public:
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual int send(const std::string& data) = 0;
    virtual std::string receive() = 0;

    virtual bool enableTLS() = 0;
    virtual bool isTLSEnabled() const = 0;

    virtual void setTimeout(int ms) = 0;
    virtual int getTimeout() const = 0;

    virtual std::string getAddress() const = 0;
    virtual int getPort() const = 0;

    // Get the type of connection (e.g., "DB", "TCP", "UDP", "TRANSFER")
    virtual std::string getType() const = 0;

    // Handle incoming request (for server connections)
    virtual void handleRequest(const std::string& data, Connection* client = nullptr) = 0;


    virtual bool bind(int port) = 0;
    virtual bool listen(int backlog = 5) = 0;
    virtual Connection* accept() = 0;
    virtual bool startListening(std::function<void(const std::string&, Connection*)> handler) = 0;
    virtual void stopServer() = 0;
    virtual bool isServerRunning() const = 0;

    virtual  void setCertificates(const std::string& cert, const std::string& key) = 0;

    virtual ~Connection() = default;
};