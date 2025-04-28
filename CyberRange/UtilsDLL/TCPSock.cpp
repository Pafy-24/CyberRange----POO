#include "pch.h"
#include "TCPSock.h"
#include <iostream>
#include <thread>
#include <openssl/err.h>


class TcpSocketWithHandle : public sf::TcpSocket {
public:
    int getHandle() const {
        return sf::Socket::getHandle();
    }
};


TCPSock::TCPSock(const std::string& addr, int port)
    : address(addr), port(port), connected(false), isServer(false),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free), serverRunning(false) {
}

TCPSock::TCPSock(int port)
    : address("0.0.0.0"), port(port), connected(false), isServer(true),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free), serverRunning(false) {
}

TCPSock::TCPSock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort)
    : tcpSocket(std::move(sock)), address(clientAddr), port(clientPort),
    connected(true), isServer(false), timeout(sf::seconds(30)), tlsEnabled(false),
    ssl(nullptr, SSL_free), sslCtx(nullptr, SSL_CTX_free), serverRunning(false) {
}

TCPSock::~TCPSock() {
    disconnect();
    stopServer();
}

bool TCPSock::connect() {
    if (isServer || connected || tcpSocket) {
        std::cerr << "Cannot connect: socket already in use or is a server" << std::endl;
        return false;
    }

    tcpSocket = std::make_unique<sf::TcpSocket>();
    tcpSocket->setBlocking(true);

    sf::Socket::Status status = tcpSocket->connect(address, static_cast<unsigned short>(port), timeout);
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error connecting to server: " << static_cast<int>(status) << std::endl;
        tcpSocket.reset();
        return false;
    }

    if (tlsEnabled && !setupTLS()) {
        disconnect();
        return false;
    }

    connected = true;
    return true;
}

bool TCPSock::disconnect() {
    if (!connected) {
        return true;
    }

    cleanupTLS();
    tcpSocket.reset();
    connected = false;
    return true;
}

bool TCPSock::isConnected() const {
    return connected && tcpSocket;
}

int TCPSock::send(const std::string& data) {
    if (!isConnected()) {
        std::cerr << "Cannot send: not connected" << std::endl;
        return -1;
    }

    if (tlsEnabled && ssl) {
        int bytesSent = SSL_write(ssl.get(), data.c_str(), static_cast<int>(data.length()));
        if (bytesSent <= 0) {
            std::cerr << "SSL write error: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return -1;
        }
        return bytesSent;
    }

    std::size_t bytesSent = 0;
    sf::Socket::Status status = tcpSocket->send(data.c_str(), data.length(), bytesSent);
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error sending data: " << static_cast<int>(status) << std::endl;
        return -1;
    }
    return static_cast<int>(bytesSent);
}

std::string TCPSock::receive() {
    if (!isConnected()) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }

    char buffer[4096];
    std::size_t bytesRead = 0;

    if (tlsEnabled && ssl) {
        bytesRead = SSL_read(ssl.get(), buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            std::cerr << "SSL read error: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
            return "";
        }
    }
    else {
        sf::Socket::Status status = tcpSocket->receive(buffer, sizeof(buffer) - 1, bytesRead);
        if (status == sf::Socket::Status::Disconnected) {
            connected = false;
            return "";
        }
        if (status != sf::Socket::Status::Done) {
            std::cerr << "Error receiving data: " << static_cast<int>(status) << std::endl;
            return "";
        }
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

bool TCPSock::bind(int bindPort) {
    if (connected || tcpListener) {
        std::cerr << "Cannot bind: socket already in use" << std::endl;
        return false;
    }

    if (bindPort > 0) {
        port = bindPort;
    }

    tcpListener = std::make_unique<sf::TcpListener>();
    sf::Socket::Status status = tcpListener->listen(static_cast<unsigned short>(port));
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error binding socket: " << static_cast<int>(status) << std::endl;
        tcpListener.reset();
        return false;
    }

    isServer = true;
    return true;
}

bool TCPSock::listen(int backlog) {
    if (!isServer || !tcpListener) {
        std::cerr << "Cannot listen: not a server socket" << std::endl;
        return false;
    }
    connected = true;
    return true;
}

Connection* TCPSock::accept() {
    if (!isServer || !connected || !tcpListener) {
        std::cerr << "Cannot accept: not a listening server socket" << std::endl;
        return nullptr;
    }

    auto clientSocket = std::make_unique<sf::TcpSocket>();
    sf::Socket::Status status = tcpListener->accept(*clientSocket);
    if (status != sf::Socket::Status::Done) {
        std::cerr << "Error accepting connection: " << static_cast<int>(status) << std::endl;
        return nullptr;
    }

    sf::IpAddress clientAddr = clientSocket->getRemoteAddress();
    unsigned short clientPort = clientSocket->getRemotePort();

    auto clientSock = std::make_unique<TCPSock>(std::move(clientSocket), clientAddr.toString(), clientPort);
    if (tlsEnabled) {
        clientSock->enableTLS();
        if (!clientSock->setupTLS()) {
            return nullptr;
        }
    }

    return clientSock.release();
}

std::string TCPSock::getAddress() const {
    return address;
}

int TCPSock::getPort() const {
    return port;
}

bool TCPSock::enableTLS() {
    if (connected) {
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

bool TCPSock::setupTLS() {
    if (!tlsEnabled || ssl) {
        return tlsEnabled;
    }

    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // Create SSL context
    sslCtx = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>(
        SSL_CTX_new(TLS_client_method()), SSL_CTX_free);
    if (!sslCtx) {
        std::cerr << "Failed to create SSL context" << std::endl;
        return false;
    }

    // Configure SSL context (adjust as needed for MSSQL)
    SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_PEER, nullptr);
    SSL_CTX_set_options(sslCtx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);

    // Create SSL object
    ssl = std::unique_ptr<SSL, decltype(&SSL_free)>(SSL_new(sslCtx.get()), SSL_free);
    if (!ssl) {
        std::cerr << "Failed to create SSL object" << std::endl;
        return false;
    }


	auto _tcpSocket = dynamic_cast<TcpSocketWithHandle*>(tcpSocket.get());
    int sock = _tcpSocket->getHandle(); 
    if (SSL_set_fd(ssl.get(), sock) != 1) {
        std::cerr << "Failed to set SSL file descriptor" << std::endl;
        return false;
    }

    // Perform SSL handshake
    if (SSL_connect(ssl.get()) != 1) {
        std::cerr << "SSL handshake failed: " << ERR_error_string(ERR_get_error(), nullptr) << std::endl;
        return false;
    }

    return true;
}

bool TCPSock::cleanupTLS() {
    if (ssl) {
        SSL_shutdown(ssl.get());
    }
    ssl.reset();
    sslCtx.reset();
    return true;
}


bool TCPSock::runServer() {
    if (serverRunning) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }

    if (!bind(port) || !listen()) {
        std::cerr << "Failed to start server" << std::endl;
        return false;
    }

    serverRunning = true;
    std::thread serverThread([this]() {
        std::cout << "TCP server started on port " << port << std::endl;
        while (serverRunning) {
            auto conn = std::unique_ptr<TCPSock>(dynamic_cast<TCPSock*>(accept()));
            if (!conn) {
                if (!serverRunning) break;
                continue;
            }
            std::thread clientThread(&TCPSock::handleClientRequest, this, std::move(conn));
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
        tcpListener.reset();
    }
}

bool TCPSock::isServerRunning() const {
    return serverRunning;
}

void TCPSock::handleClientRequest(std::unique_ptr<TCPSock> clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        std::cerr << "Invalid client connection" << std::endl;
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
}