#include "pch.h"
#include "ConnsFactory.h"
#include "DBConn.h"
#include "TCPSock.h"
#include "UDPSock.h"
#include "transfer_sock.h"

std::unique_ptr<Connection> ConnsFactory::createConnection(
    ConnectionType type,
    const std::string& address,
    int port,
    const std::string& connStr,
    const std::string& rootDir
) {
    switch (type) {
    case ConnectionType::DB:
        if (connStr.empty()) {
            throw std::runtime_error("Connection string required for DB connection");
        }
        return std::make_unique<DBConn>(connStr);

    case ConnectionType::TCP:
        if (address.empty() && port == 0) {
            throw std::runtime_error("Address or port required for TCP connection");
        }
        if (address.empty()) {
            return std::make_unique<TCPSock>(port);
        }
        return std::make_unique<TCPSock>(address, port);

    case ConnectionType::UDP:
        if (address.empty() && port == 0) {
            throw std::runtime_error("Address or port required for UDP connection");
        }
        return std::make_unique<UDPSock>(address, port);

    case ConnectionType::TRANSFER:
        if (address.empty() && port == 0) {
            throw std::runtime_error("Address or port required for transfer connection");
        }
        if (address.empty()) {
            return std::make_unique<transfer_sock>(port);
        }
        return std::make_unique<transfer_sock>(address, port);

    default:
        throw std::runtime_error("Unknown connection type");
    }
}