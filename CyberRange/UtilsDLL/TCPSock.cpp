#include "pch.h"
#include "TCPSock.h"
#include <iostream>
#include <thread>
#include <filesystem>

TCPSock::TCPSock(const std::string& addr, int port)
    : tcpSocket(nullptr), tcpListener(nullptr), address(addr), port(port),
    connected(false), isServer(false), timeout(sf::seconds(30)), tlsEnabled(false), tlsContext(nullptr),
    serverRunning(false) {
}

TCPSock::TCPSock(int port)
    : tcpSocket(nullptr), tcpListener(nullptr), address("0.0.0.0"), port(port),
    connected(false), isServer(true), timeout(sf::seconds(30)), tlsEnabled(false), tlsContext(nullptr),
    serverRunning(false) {
}

TCPSock::TCPSock(sf::TcpSocket* sock, const std::string& clientAddr, int clientPort)
    : tcpSocket(sock), tcpListener(nullptr), address(clientAddr), port(clientPort),
    connected(true), isServer(false), timeout(sf::seconds(30)), tlsEnabled(false), tlsContext(nullptr),
    serverRunning(false) {
}

TCPSock::~TCPSock() {
    if (connected) {
        disconnect();
    }

    stopServer();

    if (tcpSocket) {
        delete tcpSocket;
        tcpSocket = nullptr;
    }

    if (tcpListener) {
        delete tcpListener;
        tcpListener = nullptr;
    }

    cleanupTLS();
}

bool TCPSock::connect() {
    if (isServer || connected || tcpSocket != nullptr) {
        std::cerr << "Cannot connect: socket already in use or is a server" << std::endl;
        return false;
    }

    tcpSocket = new sf::TcpSocket();
    tcpSocket->setBlocking(true);

    sf::Socket::Status status = tcpSocket->connect(address, static_cast<unsigned short>(port), timeout);

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error connecting to server: " << static_cast<int>(status) << std::endl;
        delete tcpSocket;
        tcpSocket = nullptr;
        return false;
    }

    if (tlsEnabled) {
        if (!setupTLS()) {
            disconnect();
            return false;
        }
    }

    connected = true;
    return true;
}

bool TCPSock::disconnect() {
    if (!connected) {
        return true;
    }

    if (tlsEnabled && tlsContext) {
        cleanupTLS();
    }

    if (tcpSocket) {
        tcpSocket->disconnect();
        delete tcpSocket;
        tcpSocket = nullptr;
    }

    connected = false;
    return true;
}

bool TCPSock::isConnected() const {
    return connected;
}

int TCPSock::send(const std::string& data) {
    if (!connected || tcpSocket == nullptr) {
        std::cerr << "Cannot send: not connected" << std::endl;
        return -1;
    }

    std::size_t bytesSent = 0;

    if (tlsEnabled && tlsContext) {
        std::cerr << "TLS send not implemented yet" << std::endl;
        return -1;
    }
    else {
        sf::Socket::Status status = tcpSocket->send(data.c_str(), data.length(), bytesSent);

        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error sending data: " << static_cast<int>(status) << std::endl;
            return -1;
        }
    }

    return static_cast<int>(bytesSent);
}

std::string TCPSock::receive() {
    if (!connected || tcpSocket == nullptr) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }

    char buffer[4096];
    std::size_t bytesRead = 0;

    if (tlsEnabled && tlsContext) {
        std::cerr << "TLS receive not implemented yet" << std::endl;
        return "";
    }
    else {
        sf::Socket::Status status = tcpSocket->receive(buffer, sizeof(buffer) - 1, bytesRead);

        if (status == sf::Socket::Status::Disconnected) {
            connected = false;
            return "";
        }
        else if (status != sf::Socket::Status::Done) {
            std::cerr << "Error receiving data: " << static_cast<int>(status) << std::endl;
            return "";
        }
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

bool TCPSock::bind(int bindPort) {
    if (connected || tcpListener != nullptr) {
        std::cerr << "Cannot bind: socket already in use" << std::endl;
        return false;
    }

    if (bindPort > 0) {
        port = bindPort;
    }

    tcpListener = new sf::TcpListener();
    tcpListener->setBlocking(true);

    sf::Socket::Status status = tcpListener->listen(static_cast<unsigned short>(port));

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error binding socket: " << static_cast<int>(status) << std::endl;
        delete tcpListener;
        tcpListener = nullptr;
        return false;
    }

    isServer = true;
    return true;
}

bool TCPSock::listen(int backlog) {
    if (!isServer || tcpListener == nullptr) {
        std::cerr << "Cannot listen: not a server socket or socket not created" << std::endl;
        return false;
    }

    connected = true;
    return true;
}

Connection* TCPSock::accept() {
    if (!isServer || !connected || tcpListener == nullptr) {
        std::cerr << "Cannot accept: not a listening server socket" << std::endl;
        return nullptr;
    }

    sf::TcpSocket* clientSocket = new sf::TcpSocket();
    sf::Socket::Status status = tcpListener->accept(*clientSocket);

    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error accepting connection: " << static_cast<int>(status) << std::endl;
        delete clientSocket;
        return nullptr;
    }

    sf::IpAddress clientAddr = clientSocket->getRemoteAddress();
    unsigned short clientPort = clientSocket->getRemotePort();

    TCPSock* clientSock = new TCPSock(clientSocket, clientAddr.toString(), clientPort);

    if (tlsEnabled) {
        clientSock->enableTLS();
        clientSock->setupTLS();
    }

    return clientSock;
}

std::string TCPSock::getAddress() const {
    return address;
}

int TCPSock::getPort() const {
    return port;
}

bool TCPSock::enableTLS() {
    if (connected && !tlsEnabled) {
        std::cerr << "Cannot enable TLS on already connected socket" << std::endl;
        return false;
    }

    tlsEnabled = true;
    return true;
}

bool TCPSock::isTLSEnabled() const {
    return tlsEnabled;
}

void TCPSock::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);
}

int TCPSock::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

void TCPSock::setBlocking(bool blocking) {
    if (tcpSocket) {
        tcpSocket->setBlocking(blocking);
    }

    if (tcpListener) {
        tcpListener->setBlocking(blocking);
    }
}

bool TCPSock::isBlocking() const {
    if (tcpSocket) {
        return tcpSocket->isBlocking();
    }
    if (tcpListener) {
        return tcpListener->isBlocking();
    }
    return true;
}

bool TCPSock::setupTLS() {
    if (tlsEnabled && !tlsContext) {
        std::cout << "Setting up TLS connection..." << std::endl;
        tlsContext = (void*)1;
        return true;
    }
    return tlsEnabled;
}

bool TCPSock::cleanupTLS() {
    if (tlsContext) {
        std::cout << "Cleaning up TLS resources..." << std::endl;
        tlsContext = nullptr;
    }
    return true;
}

bool TCPSock::runServer() {
    if (serverRunning) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }


    if (!bind(port)) {
        std::cerr << "Failed to bind server socket" << std::endl;
        return false;
    }

    if (!listen()) {
        std::cerr << "Failed to start listening" << std::endl;
        return false;
    }

    serverRunning = true;

    std::thread serverThread([this]() {
        std::cout << "TCP server started on port " << getPort() << std::endl;

        while (serverRunning) {
            Connection* conn = accept();
            if (!conn) {
                if (!serverRunning) {
                    break;
                }
                std::cerr << "Failed to accept client connection" << std::endl;
                continue;
            }

            TCPSock* clientSock = dynamic_cast<TCPSock*>(conn);
            if (!clientSock) {
                std::cerr << "Invalid client connection type" << std::endl;
                delete conn;
                continue;
            }

            std::thread clientThread(&TCPSock::handleClientRequest, this, clientSock);
            clientThread.detach();
        }

        std::cout << "TCP server stopped" << std::endl;
        });

    serverThread.detach();
    return true;
}

void TCPSock::stopServer() {
    if (serverRunning) {
        serverRunning = false;
        disconnect();
    }
}

bool TCPSock::isServerRunning() const {
    return serverRunning;
}

void TCPSock::handleClientRequest(TCPSock* clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        std::cerr << "Invalid client connection" << std::endl;
        delete clientSock;
        return;
    }

    std::cout << "New client connected: " << clientSock->getAddress() << ":" << clientSock->getPort() << std::endl;

    try {
        std::string request = clientSock->receive();
        clientSock->send("Server response: Received " + std::to_string(request.length()) + " bytes");
    }
    catch (const std::exception& e) {
        std::cerr << "Exception handling client request: " << e.what() << std::endl;
    }

    delete clientSock;
}