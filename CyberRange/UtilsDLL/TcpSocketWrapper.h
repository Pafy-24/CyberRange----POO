#pragma once
#include <SFML/Network.hpp>
#include <atomic>
#include <mutex>
#include <string>
#include <iostream>

// Enhanced TCP socket wrapper with explicit socket handle access
class TcpSocketWithHandle : public sf::TcpSocket {
public:
    TcpSocketWithHandle() : sf::TcpSocket() {}

    // Get the native socket handle
    int getHandle() const {
        return sf::Socket::getHandle();
    }

    // Additional utility functions for debugging
    std::string getRemoteInfo() const {
        return getRemoteAddress().toString() + ":" + std::to_string(getRemotePort());
    }

    // Set socket to non-blocking mode with timeout
    void setNonBlocking(sf::Time timeout) {
        setBlocking(false);
    }

    // Override send to add more error handling
    sf::Socket::Status safeSend(const void* data, std::size_t size, std::size_t& sent) {
        sf::Socket::Status status = send(data, size, sent);

        if (status == sf::Socket::Status::Disconnected) {
            printMessage("Connection lost during send operation");
        }
        else if (status == sf::Socket::Status::Error) {
            printMessage("Error during send operation");
        }

        return status;
    }

    // Override receive to add more error handling 
    sf::Socket::Status safeReceive(void* data, std::size_t size, std::size_t& received) {
        sf::Socket::Status status = receive(data, size, received);

        if (status == sf::Socket::Status::Disconnected) {
            printMessage("Connection lost during receive operation");
        }
        else if (status == sf::Socket::Status::Error) {
            printMessage("Error during receive operation");
        }

        return status;
    }
};