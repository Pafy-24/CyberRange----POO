#include "pch.h"
#include "UDPSock.h"
#include <iostream>
#include <thread>

UDPSock::UDPSock(const std::string& addr, int port)
    : udpSocket(nullptr), address(addr), port(port), connected(false),
    isServer(false), timeout(sf::seconds(30)), tlsEnabled(false), dtlsContext(nullptr) {
    printInfo("Created UDP socket for " + addr + ":" + std::to_string(port));
}

UDPSock::~UDPSock() {
    if (connected) {
        disconnect();
    }
    stopServer();
    if (udpSocket) {
        delete udpSocket;
        udpSocket = nullptr;
    }
    cleanupDTLS();
    printInfo("UDP socket destroyed");
}

bool UDPSock::connect() {
    if (isServer || connected || udpSocket != nullptr) {
        printError("Cannot connect: socket already in use or is a server");
        return false;
    }
    udpSocket = new sf::UdpSocket();
    udpSocket->setBlocking(true);
    sf::Socket::Status status = udpSocket->bind(sf::Socket::AnyPort);
    if (status != sf::Socket::Status::Done) {
        printError("Error binding UDP socket: " + std::to_string(static_cast<int>(status)));
        delete udpSocket;
        udpSocket = nullptr;
        return false;
    }
    if (tlsEnabled) {
        if (!setupDTLS()) {
            disconnect();
            return false;
        }
    }
    connected = true;
    printInfo("Connected UDP socket to " + address + ":" + std::to_string(port));
    return true;
}

bool UDPSock::disconnect() {
    if (!connected) {
        return true;
    }
    if (tlsEnabled && dtlsContext) {
        cleanupDTLS();
    }
    if (udpSocket) {
        udpSocket->unbind();
        delete udpSocket;
        udpSocket = nullptr;
    }
    connected = false;
    printInfo("Disconnected UDP socket from " + address + ":" + std::to_string(port));
    return true;
}

bool UDPSock::isConnected() const {
    return connected;
}

int UDPSock::send(const std::string& data) {
    if (!connected || udpSocket == nullptr) {
        printError("Cannot send: not connected");
        return -1;
    }
    std::size_t bytesSent = data.length();
    if (tlsEnabled && dtlsContext) {
        printError("DTLS send not implemented yet");
        return -1;
    }
    else {
        sf::IpAddress recipient(address);
        sf::Packet packet;
        packet << data;
        sf::Socket::Status status = udpSocket->send(packet, recipient, static_cast<unsigned short>(port));
        if (status != sf::Socket::Status::Done) {
            printError("Error sending data: " + std::to_string(static_cast<int>(status)));
            return -1;
        }
    }
    printInfo("Sent " + std::to_string(bytesSent) + " bytes to " + address + ":" + std::to_string(port));
    return static_cast<int>(bytesSent);
}

std::string UDPSock::receive() {
    if (!connected || udpSocket == nullptr) {
        printError("Cannot receive: not connected");
        return "";
    }
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short senderPort;
    if (tlsEnabled && dtlsContext) {
        printError("DTLS receive not implemented yet");
        return "";
    }
    else {
        sf::Socket::Status status = udpSocket->receive(packet, sender, senderPort);
        if (status != sf::Socket::Status::Done) {
            printError("Error receiving data: " + std::to_string(static_cast<int>(status)));
            return "";
        }
    }
    std::string data;
    packet >> data;
    printInfo("Received " + std::to_string(data.length()) + " bytes from " + sender.toString() + ":" + std::to_string(senderPort));
    return data;
}

bool UDPSock::bind(int bindPort) {
    if (connected || udpSocket != nullptr) {
        printError("Cannot bind: socket already in use");
        return false;
    }
    if (bindPort > 0) {
        port = bindPort;
    }
    udpSocket = new sf::UdpSocket();
    sf::Socket::Status status = udpSocket->bind(static_cast<unsigned short>(port));
    if (status != sf::Socket::Status::Done) {
        printError("Error binding socket: " + std::to_string(static_cast<int>(status)));
        delete udpSocket;
        udpSocket = nullptr;
        return false;
    }
    isServer = true;
    connected = true;
    printInfo("Bound UDP socket to port " + std::to_string(port));
    return true;
}

bool UDPSock::listen(int backlog) {
    return isServer && connected;
}

Connection* UDPSock::accept() {
    return nullptr;
}

std::string UDPSock::getAddress() const {
    return address;
}

int UDPSock::getPort() const {
    return port;
}

bool UDPSock::enableTLS() {
    if (connected && !tlsEnabled) {
        printError("Cannot enable DTLS on already connected socket");
        return false;
    }
    printInfo("Enabling DTLS for UDP socket");
    tlsEnabled = true;
    return true;
}

bool UDPSock::isTLSEnabled() const {
    return tlsEnabled;
}

void UDPSock::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);
    printInfo("Set timeout to " + std::to_string(ms) + " ms");
}

int UDPSock::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

bool UDPSock::startListening(std::function<void(const std::string&, Connection*)> handler) {
    if (thisServerRunning) {
        printError("Server is already running");
        return false;
    }
    if (!bind(port)) {
        printError("Failed to bind server socket");
        return false;
    }
    requestHandler = handler;
    thisServerRunning = true;
    std::thread serverThread([this]() {
        printInfo("UDP server started on port " + std::to_string(getPort()));
        while (thisServerRunning) {
            handleClientRequest();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        printInfo("UDP server stopped");
        });
    serverThread.detach();
    return true;
}

void UDPSock::stopServer() {
    if (thisServerRunning) {
        printInfo("Stopping UDP server...");
        thisServerRunning = false;
        disconnect();
    }
}

bool UDPSock::isServerRunning() const {
    return thisServerRunning;
}

void UDPSock::setCertificates(const std::string& cert, const std::string& key) {
    printWarning("UDPSock does not support setting certificates");
}

bool UDPSock::setupDTLS() {
    if (tlsEnabled && !dtlsContext) {
        printInfo("Setting up DTLS connection...");
        dtlsContext = (void*)1;
        return true;
    }
    return tlsEnabled;
}

bool UDPSock::cleanupDTLS() {
    if (dtlsContext) {
        printInfo("Cleaning up DTLS resources...");
        dtlsContext = nullptr;
    }
    return true;
}

void UDPSock::handleClientRequest() {
    if (!isServer || !connected || udpSocket == nullptr) {
        return;
    }
    sf::Packet packet;
    sf::IpAddress clientAddr;
    unsigned short clientPort;
    sf::Socket::Status status = udpSocket->receive(packet, clientAddr, clientPort);
    if (status != sf::Socket::Status::Done) {
        return;
    }
    std::string data;
    packet >> data;
    if (!data.empty()) {
        printInfo("Received from " + clientAddr.toString() + ":" + std::to_string(clientPort) + " - " + std::to_string(data.length()) + " bytes");
        
        if (requestHandler) {
            requestHandler(data, nullptr);
        }
        else {
            sf::Packet response;
            response << "Echo: " + data;
            udpSocket->send(response, clientAddr, clientPort);
            printInfo("Echoed message back to " + clientAddr.toString() + ":" + std::to_string(clientPort));
        }
    }
}

void UDPSock::handleRequest(const std::string& data, Connection* client) {
    if (requestHandler) {
        requestHandler(data, client);
    }
    else {
        printLog("No request handler set for data: " + data);
    }
}