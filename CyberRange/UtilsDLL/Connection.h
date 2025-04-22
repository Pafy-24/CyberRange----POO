#pragma once
#include "DLL.h"
#include <string>
#include <vector>

/**
 * Base interface for all connection types
 */
class UTILS_API Connection {
public:
    // Common connection operations
    virtual bool connect() = 0;
    virtual bool disconnect() = 0;
    virtual bool isConnected() const = 0;

    // Data transfer operations
    virtual int send(std::string data) = 0;
    virtual std::string receive() = 0;

    // Server mode operations
    virtual bool bind(int port) = 0;
    virtual bool listen(int backlog = 5) = 0;
    virtual Connection* accept() = 0;

    // Connection info
    virtual std::string getAddress() const = 0;
    virtual int getPort() const = 0;

    // Security
    virtual bool enableTLS() = 0;
    virtual bool isTLSEnabled() const = 0;

    // Timeout configuration
    virtual void setTimeout(int ms) = 0;
    virtual int getTimeout() const = 0;

    virtual ~Connection() = default;
};