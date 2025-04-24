#include "pch.h"
#include "UDPSock.h"
#include <iostream>

UDPSock::UDPSock(const std::string& addr, int port)
    : udpSocket(nullptr), address(addr), port(port), connected(false),
    isServer(false), timeout(sf::seconds(30)), maxPacketSize(8192),
    tlsEnabled(false), dtlsContext(nullptr) {
}

UDPSock::~UDPSock() {
    if (connected) {
        disconnect();
    }

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

    // Pentru UDP, nu există o conexiune directă
    // În schimb, legăm socketul la un port disponibil
    sf::Socket::Status status = udpSocket->bind(sf::Socket::AnyPort);

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error binding UDP socket: " << static_cast<int>(status) << std::endl;
        delete udpSocket;
        udpSocket = nullptr;
        return false;
    }

    // Dacă DTLS este activat, configurăm conexiunea securizată
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

    // Curățați mai întâi DTLS dacă este activat
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

    std::size_t bytesSent = 0;

    // În cazul în care DTLS este activat, implementați logica de trimitere DTLS aici
    if (tlsEnabled && dtlsContext) {
        // Implementare trimitere DTLS
        std::cerr << "DTLS send not implemented yet" << std::endl;
        return -1;
    }
    else {
        // Trimitere normală
        sf::IpAddress recipient(address);
        sf::Packet Packet;
		Packet << data;
		
        sf::Socket::Status status = udpSocket->send(Packet, recipient, static_cast<unsigned short>(port));

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

    char buffer[8192];
    std::size_t bytesRead = 0;
    sf::IpAddress sender;
    unsigned short senderPort;

    // În cazul în care DTLS este activat, implementați logica de primire DTLS aici
    if (tlsEnabled && dtlsContext) {
        // Implementare primire DTLS
        std::cerr << "DTLS receive not implemented yet" << std::endl;
        return "";
    }
    else {
        // Primire normală
        sf::Socket::Status status = udpSocket->receive(buffer, maxPacketSize, bytesRead, sender, senderPort);

        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error receiving data: " << static_cast<int>(status) << std::endl;
            return "";
        }
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
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
    // Pentru UDP, nu există listen
    std::cerr << "UDP sockets do not listen for connections" << std::endl;
    return isServer && connected;
}

Connection* UDPSock::accept() {
    // Pentru UDP, nu există accept
    std::cerr << "UDP sockets do not accept connections" << std::endl;
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

    // Pentru UDP, utilizăm DTLS
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

void UDPSock::setMaxPacketSize(int size) {
    maxPacketSize = size;
}

bool UDPSock::broadcast(const std::string& data) {
    if (!connected || udpSocket == nullptr) {
        std::cerr << "Cannot broadcast: not connected" << std::endl;
        return false;
    }

    sf::IpAddress broadcastAddr = sf::IpAddress::Broadcast;
    std::size_t bytesSent = 0;
	sf::Packet Packet;Packet << data;
    sf::Socket::Status status = udpSocket->send(Packet, broadcastAddr, static_cast<unsigned short>(port));

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error broadcasting data: " << static_cast<int>(status) << std::endl;
        return false;
    }

    return true;
}

std::string UDPSock::receiveFrom(std::string& sender) {
    if (!connected || udpSocket == nullptr) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }

    char buffer[8192];
    std::size_t bytesRead = 0;
    sf::IpAddress senderAddr;
    unsigned short senderPort;

    sf::Socket::Status status = udpSocket->receive(buffer,
        static_cast<std::size_t>(maxPacketSize),
        bytesRead, senderAddr, senderPort);

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error receiving data: " << static_cast<int>(status) << std::endl;
        return "";
    }

    sender = senderAddr.toString() + ":" + std::to_string(senderPort);
    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

bool UDPSock::setupDTLS() {
    // Implementare simplificată pentru DTLS
    if (tlsEnabled && !dtlsContext) {
        std::cout << "Setting up DTLS connection..." << std::endl;
        // Placeholder pentru inițializarea DTLS
        dtlsContext = (void*)1; // Simulăm că am creat un context DTLS
        return true;
    }
    return tlsEnabled;
}

bool UDPSock::cleanupDTLS() {
    if (dtlsContext) {
        std::cout << "Cleaning up DTLS resources..." << std::endl;
        // Eliberarea resurselor DTLS
        dtlsContext = nullptr;
    }
    return true;
}