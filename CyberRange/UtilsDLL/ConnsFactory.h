#pragma once
#include "Connection.h"
#include <memory>
#include <string>

enum class ConnectionType {
    DB,
    TCP,
    UDP,
    TRANSFER
};

class UTILS_API ConnsFactory {
public:
    // Create a connection based on type and configuration
    static std::unique_ptr<Connection> createConnection(
        ConnectionType type,
        const std::string& address = "",
        int port = 0,
        const std::string& connStr = "",
        const std::string& rootDir = "./files"
    );
};