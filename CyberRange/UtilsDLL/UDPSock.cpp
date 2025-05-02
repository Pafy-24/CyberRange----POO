#include "pch.h"
#include "UDPSock.h"
#include <iostream>
#include <thread>

UDPSock::UDPSock(const std::string& addr, int port)
    : udpSocket(nullptr), address(addr), port(port), connected(false),
    isServer(false), timeout(sf::seconds(30)), tlsEnabled(false), dtlsContext(nullptr) {
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
}

bool UDPSock::connect() {
    if (isServer || connected || udpSocket != nullptr) {
        std::cerr << "Cannot connect: socket already in use or is a server" << std::endl;
        return false;
    }
    udpSocket = new sf::UdpSocket();
    udpSocket->setBlocking(true);
    sf::Socket::Status status = udpSocket->bind(sf::Socket::AnyPort);
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error binding UDP socket: " << static_cast<int>(status) << std::endl;
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
    return true;
}

bool UDPSock::isConnected() const {
    return connected;
}

int UDPSock::send(const std::string& data) {
    if (!connected || udpSocket == nullptr) {
        std::cerr << "Cannot send: not connected" << std::endl;
        return -1;
    }
    std::size_t bytesSent = data.length();
    if (tlsEnabled && dtlsContext) {
        std::cerr << "DTLS send not implemented yet" << std::endl;
        return -1;
    }
    else {
        sf::IpAddress recipient(address);
        sf::Packet packet;
        packet << data;
        sf::Socket::Status status = udpSocket->send(packet, recipient, static_cast<unsigned short>(port));
        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error sending data: " << static_cast<int>(status) << std::endl;
            return -1;
        }
    }
    return static_cast<int>(bytesSent);
}

std::string UDPSock::receive() {
    if (!connected || udpSocket == nullptr) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }
    sf::Packet packet;
    sf::IpAddress sender;
    unsigned short senderPort;
    if (tlsEnabled && dtlsContext) {
        std::cerr << "DTLS receive not implemented yet" << std::endl;
        return "";
    }
    else {
        sf::Socket::Status status = udpSocket->receive(packet, sender, senderPort);
        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error receiving data: " << static_cast<int>(status) << std::endl;
            return "";
        }
    }
    std::string data;
    packet >> data;
    return data;
}

bool UDPSock::bind(int bindPort) {
    if (connected || udpSocket != nullptr) {
        std::cerr << "Cannot bind: socket already in use" << std::endl;
        return false;
    }
    if (bindPort > 0) {
        port = bindPort;
    }
    udpSocket = new sf::UdpSocket();
    sf::Socket::Status status = udpSocket->bind(static_cast<unsigned short>(port));
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error binding socket: " << static_cast<int>(status) << std::endl;
        delete udpSocket;
        udpSocket = nullptr;
        return false;
    }
    isServer = true;
    connected = true;
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
        std::cerr << "Cannot enable DTLS on already connected socket" << std::endl;
        return false;
    }
    std::cout << "Enabling DTLS for UDP socket" << std::endl;
    tlsEnabled = true;
    return true;
}

bool UDPSock::isTLSEnabled() const {
    return tlsEnabled;
}

void UDPSock::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);
}

int UDPSock::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

bool UDPSock::startListening(std::function<void(const std::string&, Connection*)> handler) {
    if (thisServerRunning) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }
    if (!bind(port)) {
        std::cerr << "Failed to bind server socket" << std::endl;
        return false;
    }
    requestHandler = handler;
    thisServerRunning = true;
    std::thread serverThread([this]() {
        std::cout << "UDP server started on port " << getPort() << std::endl;
        while (thisServerRunning) {
            handleClientRequest();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "UDP server stopped" << std::endl;
        });
    serverThread.detach();
    return true;
}

void UDPSock::stopServer() {
    if (thisServerRunning) {
        thisServerRunning = false;
        disconnect();
    }
}

bool UDPSock::isServerRunning() const {
    return thisServerRunning;
}

void UDPSock::setCertificates(const std::string& cert, const std::string& key) {
    std::cerr << "UDPSock does not support setting certificates" << std::endl;
}

bool UDPSock::setupDTLS() {
    if (tlsEnabled && !dtlsContext) {
        std::cout << "Setting up DTLS connection..." << std::endl;
        dtlsContext = (void*)1;
        return true;
    }
    return tlsEnabled;
}

bool UDPSock::cleanupDTLS() {
    if (dtlsContext) {
        std::cout << "Cleaning up DTLS resources..." << std::endl;
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
        std::cout << "Received from " << clientAddr.toString() << ":" << clientPort
            << " - " << data.length() << " bytes" << std::endl;
        if (requestHandler) {
            requestHandler(data, nullptr);
        }
        else {
            sf::Packet response;
            response << "Echo: " + data;
            udpSocket->send(response, clientAddr, clientPort);
        }
    }
}

void UDPSock::handleRequest(const std::string& data, Connection* client) {
    if (requestHandler) {
        requestHandler(data, client);
    }
    else {
        std::cout << "No request handler set for data: " << data << std::endl;
    }
}