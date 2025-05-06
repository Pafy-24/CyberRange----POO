#pragma once
#include <SFML/Network.hpp>
#include <atomic>
#include <mutex>
#include <string>
#include <iostream>

class TcpSocketWithHandle : public sf::TcpSocket {
public:
    TcpSocketWithHandle() : sf::TcpSocket() {}

    int getHandle() const {
        return sf::Socket::getHandle();
    }

    std::string getRemoteInfo() const {
        return getRemoteAddress().toString() + ":" + std::to_string(getRemotePort());
    }

    void setNonBlocking(sf::Time timeout) {
        setBlocking(false);
    }

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