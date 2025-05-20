#define SQL_NOUNICODEMAP
#include "pch.h"
#include "TCPSock.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <openssl/err.h>
#include <sstream>

TCPSock::TCPSock(const std::string& addr, int port)
    : address(addr), port(port), connected(false), isServer(false),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free) {
    print("Client socket created for " + addr + ":" + std::to_string(port));
}

TCPSock::TCPSock(int port)
    : address("0.0.0.0"), port(port), connected(false), isServer(true),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free) {
    print("Server socket created on port " + std::to_string(port));
}

TCPSock::TCPSock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort)
    : address(clientAddr), port(clientPort),
    connected(false), isServer(false), timeout(sf::seconds(30)), tlsEnabled(false),
    ssl(nullptr, SSL_free), sslCtx(nullptr, SSL_CTX_free) {
    if (sock) {
        tcpSocket = std::make_unique<TcpSocketWithHandle>();
        print("Client socket created for " + clientAddr + ":" + std::to_string(clientPort));
        connected = true;
        auto* rawSocket = sock.release();
        if (auto* socketWithHandle = dynamic_cast<TcpSocketWithHandle*>(rawSocket)) {
            tcpSocket.reset(socketWithHandle);
        }
        else {
            print("Failed to transfer socket handle - creating new connection");
            sf::Socket::Status status = tcpSocket->connect(
                sf::IpAddress(clientAddr),
                static_cast<unsigned short>(clientPort),
                timeout
            );
            if (status != sf::Socket::Status::Done) {
                print("Failed to create client connection to " + clientAddr + ":" + std::to_string(clientPort));
                connected = false;
            }
            delete rawSocket;
        }
    }
    else {
        print("Invalid socket provided for client connection");
        connected = false;
    }
}

TCPSock::~TCPSock() {
    disconnect();
    stopServer();
    print("Socket destroyed for " + address + ":" + std::to_string(port));
}

bool TCPSock::connect() {
    if (isServer || connected || tcpSocket) {
        print("Cannot connect: socket already in use or is a server");
        return false;
    }
    tcpSocket = std::make_unique<TcpSocketWithHandle>();
    tcpSocket->setBlocking(true);
    print("Connecting to " + address + ":" + std::to_string(port) + "...");
    sf::Socket::Status status = tcpSocket->connect(address, static_cast<unsigned short>(port), timeout);
    if (status != sf::Socket::Status::Done) {
        print("Error connecting to server: " + std::to_string(static_cast<int>(status)));
        tcpSocket.reset();
        return false;
    }
    if (tlsEnabled && !setupTLS()) {
        print("TLS setup failed");
        disconnect();
        return false;
    }
    connected = true;
    print("Successfully connected to " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::disconnect() {
    if (!connected) {
        return true;
    }
    print("Disconnecting from " + address + ":" + std::to_string(port));
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
        print("Cannot send: not connected");
        return -1;
    }
    if (tlsEnabled && ssl) {
        int bytesSent = SSL_write(ssl.get(), data.c_str(), static_cast<int>(data.length()));
        if (bytesSent <= 0) {
            print("SSL write error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return -1;
        }
        print("Sent " + std::to_string(bytesSent) + " bytes via SSL");
        return bytesSent;
    }
    std::size_t bytesSent = 0;
    sf::Socket::Status status = tcpSocket->send(data.c_str(), data.length(), bytesSent);
    if (status != sf::Socket::Status::Done) {
        print("Error sending data: " + std::to_string(static_cast<int>(status)));
        return -1;
    }
    print("Sent " + std::to_string(bytesSent) + " bytes");
    return static_cast<int>(bytesSent);
}

std::string TCPSock::receive() {
    if (!isConnected()) {
        print("Cannot receive: not connected");
        return "";
    }
    char buffer[4096];
    std::size_t bytesRead = 0;
    if (tlsEnabled && ssl) {
        int result = SSL_read(ssl.get(), buffer, sizeof(buffer) - 1);
        if (result <= 0) {
            int error = SSL_get_error(ssl.get(), result);
            if (error == SSL_ERROR_ZERO_RETURN) {
                print("SSL connection closed");
                connected = false;
            }
            else {
                print("SSL read error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            }
            return "";
        }
        bytesRead = static_cast<std::size_t>(result);
    }
    else {
        sf::Socket::Status status = tcpSocket->receive(buffer, sizeof(buffer) - 1, bytesRead);
        if (status == sf::Socket::Status::Disconnected) {
            print("Connection closed by peer");
            connected = false;
            return "";
        }
        if (status != sf::Socket::Status::Done) {
            print("Error receiving data: " + std::to_string(static_cast<int>(status)));
            return "";
        }
    }
    buffer[bytesRead] = '\0';
    print("Received " + std::to_string(bytesRead) + " bytes");
    return std::string(buffer, bytesRead);
}

bool TCPSock::bind(int bindPort) {
    if (connected || tcpListener) {
        print("Cannot bind: socket already in use");
        return false;
    }
    if (bindPort > 0) {
        port = bindPort;
    }
    tcpListener = std::make_unique<sf::TcpListener>();
    tcpListener->setBlocking(true);
    print("Binding to port " + std::to_string(port) + "...");
    sf::Socket::Status status = tcpListener->listen(static_cast<unsigned short>(port));
    if (status != sf::Socket::Status::Done) {
        print("Error binding socket: " + std::to_string(static_cast<int>(status)));
        tcpListener.reset();
        return false;
    }
    isServer = true;
    print("Successfully bound to port " + std::to_string(port));
    return true;
}

bool TCPSock::listen(int backlog) {
    if (!isServer || !tcpListener) {
        print("Cannot listen: not a server socket");
        return false;
    }
    connected = true;
    print("Listening for connections on port " + std::to_string(port));
    return true;
}

Connection* TCPSock::accept() {
    if (!isServer || !connected || !tcpListener) {
        print("Cannot accept: not a listening server socket");
        return nullptr;
    }
    auto clientSocket = std::make_unique<TcpSocketWithHandle>();
    sf::Socket::Status status = tcpListener->accept(*clientSocket);
    if (status != sf::Socket::Status::Done) {
        if (thisServerRunning && status != sf::Socket::Status::NotReady) {
            print("Error accepting connection: " + std::to_string(static_cast<int>(status)));
        }
        return nullptr;
    }
    sf::IpAddress clientAddr = clientSocket->getRemoteAddress();
    unsigned short clientPort = clientSocket->getRemotePort();
    print("Accepted connection from " + clientAddr.toString() + ":" + std::to_string(clientPort));
    auto clientSock = new TCPSock(std::move(clientSocket), clientAddr.toString(), clientPort);
    if (tlsEnabled) {
        clientSock->tlsEnabled = true;
        if (!clientSock->setupTLS()) {
            print("Failed to set up TLS for client connection");
            delete clientSock;
            return nullptr;
        }
        print("TLS successfully set up for client connection");
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
    if (connected) {
        print("Cannot enable TLS on already connected socket");
        return false;
    }
    tlsEnabled = true;
    print("TLS enabled for " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::isTLSEnabled() const {
    return tlsEnabled;
}

void TCPSock::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);
    print("Timeout set to " + std::to_string(ms) + " ms");
}

int TCPSock::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

bool TCPSock::setupTLS() {
    if (!tlsEnabled) {
        return false;
    }
    if (ssl) {
        return true;
    }
    print("Setting up TLS for " + address + ":" + std::to_string(port) + "...");
    static bool sslInitialized = false;
    static std::mutex sslInitMutex;
    {
        std::lock_guard<std::mutex> lock(sslInitMutex);
        if (!sslInitialized) {
            SSL_load_error_strings();
            OpenSSL_add_ssl_algorithms();
            sslInitialized = true;
        }
    }
    const SSL_METHOD* method = isServer ? TLS_server_method() : TLS_client_method();
    sslCtx = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>(
        SSL_CTX_new(method), SSL_CTX_free);
    if (!sslCtx) {
        print("Failed to create SSL context: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }
    SSL_CTX_set_options(sslCtx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);
    if (isServer) {
        const char* cert = certFile.empty() ? "server.crt" : certFile.c_str();
        const char* key = keyFile.empty() ? "server.key" : keyFile.c_str();
        print("Loading certificate from: " + std::string(cert));
        print("Loading private key from: " + std::string(key));
        if (SSL_CTX_use_certificate_file(sslCtx.get(), cert, SSL_FILETYPE_PEM) <= 0) {
            print("Failed to load certificate: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }
        if (SSL_CTX_use_PrivateKey_file(sslCtx.get(), key, SSL_FILETYPE_PEM) <= 0) {
            print("Failed to load private key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }
        if (SSL_CTX_check_private_key(sslCtx.get()) != 1) {
            print("Private key does not match certificate: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }
        print("Certificate and private key loaded successfully");
    }
    SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_NONE, nullptr);
    ssl = std::unique_ptr<SSL, decltype(&SSL_free)>(SSL_new(sslCtx.get()), SSL_free);
    if (!ssl) {
        print("Failed to create SSL object: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }
    if (!tcpSocket) {
        print("No valid socket for TLS setup");
        return false;
    }
    auto _tcpSocket = dynamic_cast<TcpSocketWithHandle*>(tcpSocket.get());
    if (!_tcpSocket) {
        print("Failed to get socket handle for TLS");
        return false;
    }
    int sock = _tcpSocket->getHandle();
    if (sock < 0) {
        print("Invalid socket handle: " + std::to_string(sock));
        return false;
    }
    if (SSL_set_fd(ssl.get(), sock) != 1) {
        print("Failed to set SSL file descriptor: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }
    print("Performing SSL handshake...");
    int result;
    int retry_count = 0;
    const int max_retries = 3;
    while (retry_count < max_retries) {
        if (isServer) {
            result = SSL_accept(ssl.get());
        }
        else {
            result = SSL_connect(ssl.get());
        }
        if (result == 1) {
            break;
        }
        int error = SSL_get_error(ssl.get(), result);
        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
            retry_count++;
            print("Retrying SSL handshake, attempt " + std::to_string(retry_count));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        unsigned long errCode = ERR_get_error();
        char errBuffer[256];
        ERR_error_string_n(errCode, errBuffer, sizeof(errBuffer));
        print("SSL handshake failed: " + std::string(errBuffer) + " (code: " + std::to_string(error) + ")");
        if (error == SSL_ERROR_SSL) {
            print("Protocol error. Check if both sides are using SSL/TLS.");
        }
        else if (error == SSL_ERROR_SYSCALL) {
            print("I/O error. Check if the connection was closed unexpectedly.");
        }
        return false;
    }
    if (result != 1) {
        print("SSL handshake failed after " + std::to_string(max_retries) + " attempts");
        return false;
    }
    print("SSL connection established using " +
        std::string(SSL_get_version(ssl.get())) +
        " with cipher " +
        std::string(SSL_get_cipher(ssl.get())));
    print("TLS setup completed successfully for " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::cleanupTLS() {
    if (ssl) {
        print("Shutting down SSL connection...");
        SSL_shutdown(ssl.get());
    }
    ssl.reset();
    sslCtx.reset();
    return true;
}

bool TCPSock::startListening(std::function<void(const std::string&, Connection*)> handler) {
    if (thisServerRunning) {
        print("Server is already running");
        return false;
    }
    if (!bind(port) || !listen()) {
        print("Failed to start server");
        return false;
    }
    requestHandler = handler;
    thisServerRunning = true;
    std::thread serverThread([this]() {
        print("TCP server started on port " + std::to_string(port));
        if (tcpListener) {
            tcpListener->setBlocking(false);
        }
        while (thisServerRunning) {
            auto conn = dynamic_cast<TCPSock*>(accept());
            if (!conn) {
                if (!thisServerRunning) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
            print("New client connection established from " + conn->getAddress() + ":" + std::to_string(conn->getPort()));
            std::unique_ptr<TCPSock> connPtr(conn);
            try {
                std::thread clientThread(&TCPSock::handleClientRequest, this, std::move(connPtr));
                clientThread.detach();
            }
            catch (const std::exception& e) {
                print("Failed to create client thread: " + std::string(e.what()));
            }
        }
        print("TCP server stopped");
        });
    serverThread.detach();
    return true;
}

void TCPSock::stopServer() {
    if (thisServerRunning) {
        print("Stopping server...");
        thisServerRunning = false;
        disconnect();
        tcpListener.reset();
        print("Server stopped");
    }
}

bool TCPSock::isServerRunning() const {
    return thisServerRunning;
}

void TCPSock::setCertificates(const std::string& cert, const std::string& key) {
    certFile = cert;
    keyFile = key;
    print("Certificate paths set: " + cert + ", " + key);
}

void TCPSock::handleClientRequest(std::unique_ptr<TCPSock> clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        print("Invalid client connection");
        return;
    }
    std::string clientInfo = clientSock->getAddress() + ":" + std::to_string(clientSock->getPort());
    print("Handling client request from " + clientInfo);
    clientSock->setTimeout(10000);
    try {
        while (clientSock->isConnected()) {
            std::string request = clientSock->receive();
            if (request.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
            if (requestHandler) {
                requestHandler(request, clientSock.get());
            }
            else {
                handleRequest(request, clientSock.get());
            }
        }
    }
    catch (const std::exception& e) {
        print("Exception handling client connection " + clientInfo + ": " + std::string(e.what()));
    }
    clientSock->disconnect();
    print("Client connection from " + clientInfo + " handled and closed");
}

void TCPSock::handleRequest(const std::string& data, Connection* client) {
    client->send("ERROR: Unknown protocol\n");
}
