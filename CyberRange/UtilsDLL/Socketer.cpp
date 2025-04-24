#include "pch.h"
/*#include "Socketer.h"
#include <iostream>

// Client constructor
Socketer::Socketer(const std::string& addr, int port)
    : socket(nullptr), address(addr), port(port), connected(false),
    isServer(false), socketType(sf::Socket::Type::Stream),
    timeout(sf::seconds(30)), tlsEnabled(false), tlsContext(nullptr) {
}

// Server constructor
Socketer::Socketer(int port)
    : socket(nullptr), address("0.0.0.0"), port(port), connected(false),
    isServer(true), socketType(sf::Socket::Type::Stream),
    timeout(sf::seconds(30)), tlsEnabled(false), tlsContext(nullptr) {
}

// Accept constructor
Socketer::Socketer(sf::Socket* sock, const std::string& clientAddr, int clientPort)
    : socket(sock), address(clientAddr), port(clientPort), connected(true),
    isServer(false), socketType(sf::Socket::Type::Stream),
    timeout(sf::seconds(30)), tlsEnabled(false), tlsContext(nullptr) {
}

Socketer::~Socketer() {
    if (connected) {
        disconnect();
    }

    if (socket) {
        delete socket;
        socket = nullptr;
    }

    cleanupTLS();
}

bool Socketer::connect() {
    if (isServer || connected || socket != nullptr) {
        std::cerr << "Cannot connect: socket already in use or is a server" << std::endl;
        return false;
    }

    // Create the appropriate socket type
    if (socketType == sf::Socket::Type::Stream) {
        socket = new sf::TcpSocket();
    }
    else {
        socket = new sf::UdpSocket();
    }

    // Set the timeout
    socket->setBlocking(true);

    // Connect to server
    sf::Socket::Status status;
    if (socketType == sf::Socket::Type::Stream) {
        status = static_cast<sf::TcpSocket*>(socket)->connect(address, static_cast<unsigned short>(port), timeout);
    }
    else {
        // UdpSocket doesn't have direct connect method in SFML
        // We'll just bind it to any port
        status = static_cast<sf::UdpSocket*>(socket)->bind(sf::Socket::AnyPort);
    }

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error connecting to server: " << status << std::endl;
        delete socket;
        socket = nullptr;
        return false;
    }

    // If TLS is enabled, set up the secure connection
    if (tlsEnabled) {
        if (!initializeTLS()) {
            std::cerr << "Failed to initialize TLS" << std::endl;
            delete socket;
            socket = nullptr;
            return false;
        }
    }

    connected = true;
    return true;
}

bool Socketer::disconnect() {
    if (!connected) {
        return true;
    }

    // Clean up TLS first if it's enabled
    if (tlsEnabled && tlsContext) {
        cleanupTLS();
    }

    if (socket != nullptr) {
        if (socketType == sf::Socket::Type::Stream) {
            static_cast<sf::TcpSocket*>(socket)->disconnect();
        }
        delete socket;
        socket = nullptr;
    }

    connected = false;
    return true;
}

bool Socketer::isConnected() const {
    return connected;
}

int Socketer::send(const std::string& data) {
    if (!connected || socket == nullptr) {
        std::cerr << "Cannot send: not connected" << std::endl;
        return -1;
    }

    std::size_t bytesSent = 0;
    sf::Socket::Status status;

    if (socketType == sf::Socket::Type::Stream) {
        status = static_cast<sf::TcpSocket*>(socket)->send(data.c_str(), data.length(), bytesSent);
    }
    else {
        // For UDP, we need a recipient
        sf::IpAddress recipient(address);
        status = static_cast<sf::UdpSocket*>(socket)->send(data.c_str(), data.length(), recipient, static_cast<unsigned short>(port), bytesSent);
    }

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error sending data: " << status << std::endl;
        return -1;
    }

    return static_cast<int>(bytesSent);
}

std::string Socketer::receive() {
    if (!connected || socket == nullptr) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }

    char buffer[4096];
    std::size_t bytesRead = 0;
    sf::Socket::Status status;

    if (socketType == sf::Socket::Type::Stream) {
        status = static_cast<sf::TcpSocket*>(socket)->receive(buffer, sizeof(buffer) - 1, bytesRead);
    }
    else {
        sf::IpAddress sender;
        unsigned short senderPort;
        status = static_cast<sf::UdpSocket*>(socket)->receive(buffer, sizeof(buffer) - 1, bytesRead, sender, senderPort);
    }

    if (status == sf::Socket::Status::Disconnected) {
        connected = false;
        return "";
    }
    else if (status != sf::Socket::Status::Done) {
        std::cerr << "Error receiving data: " << status << std::endl;
        return "";
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

bool Socketer::bind(int bindPort) {
    if (connected || socket != nullptr) {
        std::cerr << "Cannot bind: socket already in use" << std::endl;
        return false;
    }

    // If a port was provided, use it instead of the default
    if (bindPort > 0) {
        port = bindPort;
    }

    // Create the appropriate socket
    if (socketType == sf::Socket::Type::Stream) {
        socket = new sf::TcpListener();
        sf::Socket::Status status = static_cast<sf::TcpListener*>(socket)->listen(static_cast<unsigned short>(port));

        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error binding socket: " << status << std::endl;
            delete socket;
            socket = nullptr;
            return false;
        }
    }
    else {
        socket = new sf::UdpSocket();
        sf::Socket::Status status = static_cast<sf::UdpSocket*>(socket)->bind(static_cast<unsigned short>(port));

        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error binding socket: " << status << std::endl;
            delete socket;
            socket = nullptr;
            return false;
        }
    }

    isServer = true;
    return true;
}

bool Socketer::listen(int backlog) {
    if (!isServer || socket == nullptr) {
        std::cerr << "Cannot listen: not a server socket or socket not created" << std::endl;
        return false;
    }

    if (socketType != sf::Socket::Type::Stream) {
        std::cerr << "Cannot listen: only TCP sockets can listen" << std::endl;
        return false;
    }

    // In SFML, listen is combined with bind in the TcpListener::listen method
    // So we don't need to do anything here if the socket is already bound

    connected = true;  // For a server, "connected" means ready to accept connections
    return true;
}

Connection* Socketer::accept() {
    if (!isServer || !connected || socket == nullptr) {
        std::cerr << "Cannot accept: not a listening server socket" << std::endl;
        return nullptr;
    }

    if (socketType != sf::Socket::Type::Stream) {
        std::cerr << "Cannot accept: only TCP sockets can accept connections" << std::endl;
        return nullptr;
    }

    sf::TcpSocket* clientSocket = new sf::TcpSocket();
    sf::Socket::Status status = static_cast<sf::TcpListener*>(socket)->accept(*clientSocket);

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error accepting connection: " << status << std::endl;
        delete clientSocket;
        return nullptr;
    }

    // Get client address as string
    sf::IpAddress clientAddr = clientSocket->getRemoteAddress();
    unsigned short clientPort = clientSocket->getRemotePort();

    // Create new socket object for client
    Socketer* clientSock = new Socketer(clientSocket, clientAddr.toString(), clientPort);

    // If server has TLS enabled, the client should too
    if (tlsEnabled) {
        clientSock->enableTLS();
    }

    return clientSock;
}

std::string Socketer::getAddress() const {
    return address;
}

int Socketer::getPort() const {
    return port;
}

bool Socketer::enableTLS() {
    if (connected && !tlsEnabled) {
        std::cerr << "Cannot enable TLS on already connected socket" << std::endl;
        return false;
    }

    // SFML doesn't have built-in TLS support
    // We're providing a stub here that would need to be implemented with OpenSSL or another library
    std::cerr << "TLS support requires OpenSSL integration with SFML" << std::endl;

    tlsEnabled = true;
    return true;
}

bool Socketer::isTLSEnabled() const {
    return tlsEnabled;
}

void Socketer::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);

    if (socket != nullptr) {
        if (socketType == sf::Socket::Type::Stream) {
            static_cast<sf::TcpSocket*>(socket)->setBlocking(true);
        }
        else {
            static_cast<sf::UdpSocket*>(socket)->setBlocking(true);
        }
    }
}

int Socketer::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

bool Socketer::initializeTLS() {
    // This would need to be implemented with OpenSSL or another library
    // SFML doesn't have built-in TLS support
    std::cerr << "TLS initialization requires OpenSSL integration with SFML" << std::endl;
    return true;
}

bool Socketer::cleanupTLS() {
    if (tlsContext) {
        // Clean up TLS resources
        tlsContext = nullptr;
    }

    tlsEnabled = false;
    return true;
}

bool Socketer::setSocketOption(int option, int value) {
    // SFML doesn't expose low-level socket options directly
    // This would need to be extended with platform-specific code
    std::cerr << "setSocketOption not implemented in SFML wrapper" << std::endl;
    return false;
}

bool Socketer::setBlocking(bool blocking) {
    if (socket == nullptr) {
        std::cerr << "Cannot set blocking mode: socket not created" << std::endl;
        return false;
    }

    if (socketType == sf::Socket::Type::Stream) {
        if (isServer) {
            static_cast<sf::TcpListener*>(socket)->setBlocking(blocking);
        }
        else {
            static_cast<sf::TcpSocket*>(socket)->setBlocking(blocking);
        }
    }
    else {
        static_cast<sf::UdpSocket*>(socket)->setBlocking(blocking);
    }

    return true;
}
*/