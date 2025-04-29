#include "pch.h"
#include "TCPSock.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <openssl/err.h>


TCPSock::TCPSock(const std::string& addr, int port)
    : address(addr), port(port), connected(false), isServer(false),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free) {

    std::string msg = "Client socket created for " + addr + ":" + std::to_string(port);
    printMessage(msg);
}

TCPSock::TCPSock(int port)
    : address("0.0.0.0"), port(port), connected(false), isServer(true),
    timeout(sf::seconds(30)), tlsEnabled(false), ssl(nullptr, SSL_free),
    sslCtx(nullptr, SSL_CTX_free) {

    std::string msg = "Server socket created on port " + std::to_string(port);
    printMessage(msg);
}

TCPSock::TCPSock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort)
    : address(clientAddr), port(clientPort),
    connected(false), isServer(false), timeout(sf::seconds(30)), tlsEnabled(false),
    ssl(nullptr, SSL_free), sslCtx(nullptr, SSL_CTX_free) {

    if (sock) {
        // Create our own socket and import socket handle from the provided one
        tcpSocket = std::make_unique<TcpSocketWithHandle>();

        // Store connection information
        printMessage("Client socket created for " + clientAddr + ":" + std::to_string(clientPort));

        // Since SFML doesn't provide a direct way to transfer socket ownership,
        // we'll adopt the raw socket handle and configure our socket

        // The socket is already connected, we just need to mark it as such
        connected = true;

        // We're taking ownership of the socket itself, so swap it into our class
        // This ensures that the socket handle isn't closed when sock is destroyed
        auto* rawSocket = sock.release();

        // Dynamic cast to access the handle if possible
        if (auto* socketWithHandle = dynamic_cast<TcpSocketWithHandle*>(rawSocket)) {
            // We need to duplicate the socket with a new handle
            tcpSocket.reset(socketWithHandle);
        }
        else {
            // Fall back to creating a new connection
            printMessage("Failed to transfer socket handle - creating new connection");

            // Connect to the address/port directly
            sf::Socket::Status status = tcpSocket->connect(
                sf::IpAddress(clientAddr),
                static_cast<unsigned short>(clientPort),
                timeout
            );

            if (status != sf::Socket::Status::Done) {
                printMessage("Failed to create client connection to " + clientAddr + ":" + std::to_string(clientPort));
                connected = false;
            }

            // Clean up the original socket
            delete rawSocket;
        }
    }
    else {
        printMessage("Invalid socket provided for client connection");
        connected = false;
    }
}

TCPSock::~TCPSock() {
    disconnect();
    stopServer();
    printMessage("Socket destroyed for " + address + ":" + std::to_string(port));
}

bool TCPSock::connect() {
    if (isServer || connected || tcpSocket) {
        printMessage("Cannot connect: socket already in use or is a server");
        return false;
    }

    tcpSocket = std::make_unique<TcpSocketWithHandle>();
    tcpSocket->setBlocking(true);

    printMessage("Connecting to " + address + ":" + std::to_string(port) + "...");
    sf::Socket::Status status = tcpSocket->connect(address, static_cast<unsigned short>(port), timeout);
    if (status != sf::Socket::Status::Done) {
        printMessage("Error connecting to server: " + std::to_string(static_cast<int>(status)));
        tcpSocket.reset();
        return false;
    }

    if (tlsEnabled && !setupTLS()) {
        printMessage("TLS setup failed");
        disconnect();
        return false;
    }

    connected = true;
    printMessage("Successfully connected to " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::disconnect() {
    if (!connected) {
        return true;
    }

    printMessage("Disconnecting from " + address + ":" + std::to_string(port));
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
        printMessage("Cannot send: not connected");
        return -1;
    }

    if (tlsEnabled && ssl) {
        int bytesSent = SSL_write(ssl.get(), data.c_str(), static_cast<int>(data.length()));
        if (bytesSent <= 0) {
            printMessage("SSL write error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return -1;
        }
        printMessage("Sent " + std::to_string(bytesSent) + " bytes via SSL");
        return bytesSent;
    }

    std::size_t bytesSent = 0;
    sf::Socket::Status status = tcpSocket->send(data.c_str(), data.length(), bytesSent);
    if (status != sf::Socket::Status::Done) {
        printMessage("Error sending data: " + std::to_string(static_cast<int>(status)));
        return -1;
    }
    printMessage("Sent " + std::to_string(bytesSent) + " bytes");
    return static_cast<int>(bytesSent);
}

std::string TCPSock::receive() {
    if (!isConnected()) {
        printMessage("Cannot receive: not connected");
        return "";
    }

    char buffer[4096];
    std::size_t bytesRead = 0;

    if (tlsEnabled && ssl) {
        int result = SSL_read(ssl.get(), buffer, sizeof(buffer) - 1);
        if (result <= 0) {
            int error = SSL_get_error(ssl.get(), result);
            if (error == SSL_ERROR_ZERO_RETURN) {
                printMessage("SSL connection closed");
                connected = false;
            }
            else {
                printMessage("SSL read error: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            }
            return "";
        }
        bytesRead = static_cast<std::size_t>(result);
    }
    else {
        sf::Socket::Status status = tcpSocket->receive(buffer, sizeof(buffer) - 1, bytesRead);
        if (status == sf::Socket::Status::Disconnected) {
            printMessage("Connection closed by peer");
            connected = false;
            return "";
        }
        if (status != sf::Socket::Status::Done) {
            printMessage("Error receiving data: " + std::to_string(static_cast<int>(status)));
            return "";
        }
    }

    buffer[bytesRead] = '\0';
    printMessage("Received " + std::to_string(bytesRead) + " bytes");
    return std::string(buffer, bytesRead);
}

bool TCPSock::bind(int bindPort) {
    if (connected || tcpListener) {
        printMessage("Cannot bind: socket already in use");
        return false;
    }

    if (bindPort > 0) {
        port = bindPort;
    }

    tcpListener = std::make_unique<sf::TcpListener>();
    tcpListener->setBlocking(true);

    printMessage("Binding to port " + std::to_string(port) + "...");
    sf::Socket::Status status = tcpListener->listen(static_cast<unsigned short>(port));
    if (status != sf::Socket::Status::Done) {
        printMessage("Error binding socket: " + std::to_string(static_cast<int>(status)));
        tcpListener.reset();
        return false;
    }

    isServer = true;
    printMessage("Successfully bound to port " + std::to_string(port));
    return true;
}

bool TCPSock::listen(int backlog) {
    if (!isServer || !tcpListener) {
        printMessage("Cannot listen: not a server socket");
        return false;
    }

    // Note: SFML's TcpListener::listen already includes listening functionality
    // so we just mark the socket as connected to indicate it's ready to accept
    connected = true;
    printMessage("Listening for connections on port " + std::to_string(port));
    return true;
}

Connection* TCPSock::accept() {
    if (!isServer || !connected || !tcpListener) {
        printMessage("Cannot accept: not a listening server socket");
        return nullptr;
    }

    auto clientSocket = std::make_unique<TcpSocketWithHandle>();
    sf::Socket::Status status = tcpListener->accept(*clientSocket);

    if (status != sf::Socket::Status::Done) {
        // Only log errors if there's an actual error and we're still running the server
        // Don't log for sf::Socket::Status::NotReady which just means no pending connection
        if (thisServerRunning && status != sf::Socket::Status::NotReady) {
            printMessage("Error accepting connection: " + std::to_string(static_cast<int>(status)));
        }
        return nullptr;
    }

    sf::IpAddress clientAddr = clientSocket->getRemoteAddress();
    unsigned short clientPort = clientSocket->getRemotePort();

    printMessage("Accepted connection from " + clientAddr.toString() + ":" + std::to_string(clientPort));

    // Create a new TCPSock for the client and transfer the socket
    auto clientSock = new TCPSock(std::move(clientSocket), clientAddr.toString(), clientPort);

    // Apply TLS if server has it enabled
    if (tlsEnabled) {
        // Since we're creating a new connection, we need to set TLS before the connection is established
        clientSock->tlsEnabled = true;

        // Setup TLS for the client connection
        if (!clientSock->setupTLS()) {
            printMessage("Failed to set up TLS for client connection");
            delete clientSock;
            return nullptr;
        }

        printMessage("TLS successfully set up for client connection");
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
        printMessage("Cannot enable TLS on already connected socket");
        return false;
    }
    tlsEnabled = true;
    printMessage("TLS enabled for " + address + ":" + std::to_string(port));
    return true;
}

bool TCPSock::isTLSEnabled() const {
    return tlsEnabled;
}

void TCPSock::setTimeout(int ms) {
    timeout = sf::milliseconds(ms);
    printMessage("Timeout set to " + std::to_string(ms) + " ms");
}

int TCPSock::getTimeout() const {
    return static_cast<int>(timeout.asMilliseconds());
}

bool TCPSock::setupTLS() {
    if (!tlsEnabled) {
        return false;
    }

    // If SSL is already set up, don't initialize again
    if (ssl) {
        return true;
    }

    printMessage("Setting up TLS for " + address + ":" + std::to_string(port) + "...");

    // Initialize OpenSSL libraries if needed
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

    // Create SSL context based on whether this is a server or client
    const SSL_METHOD* method = isServer ? TLS_server_method() : TLS_client_method();
    sslCtx = std::unique_ptr<SSL_CTX, decltype(&SSL_CTX_free)>(
        SSL_CTX_new(method), SSL_CTX_free);

    if (!sslCtx) {
        printMessage("Failed to create SSL context: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    // Configure SSL context with appropriate security settings
    SSL_CTX_set_options(sslCtx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1);

    // In server mode, we need to set up certificates
    if (isServer) {
        // Use the certificate and key files if they were specified
        const char* cert = certFile.empty() ? "server.crt" : certFile.c_str();
        const char* key = keyFile.empty() ? "server.key" : keyFile.c_str();

        printMessage("Loading certificate from: " + std::string(cert));
        printMessage("Loading private key from: " + std::string(key));

        if (SSL_CTX_use_certificate_file(sslCtx.get(), cert, SSL_FILETYPE_PEM) <= 0) {
            printMessage("Failed to load certificate: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }

        if (SSL_CTX_use_PrivateKey_file(sslCtx.get(), key, SSL_FILETYPE_PEM) <= 0) {
            printMessage("Failed to load private key: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }

        // Verify the private key matches the certificate
        if (SSL_CTX_check_private_key(sslCtx.get()) != 1) {
            printMessage("Private key does not match certificate: " +
                std::string(ERR_error_string(ERR_get_error(), nullptr)));
            return false;
        }

        printMessage("Certificate and private key loaded successfully");
    }

    // For simplicity in this example, we're using a less strict verification mode
    // In production, you'd want proper certificate verification
    SSL_CTX_set_verify(sslCtx.get(), SSL_VERIFY_NONE, nullptr);

    // Create SSL object
    ssl = std::unique_ptr<SSL, decltype(&SSL_free)>(SSL_new(sslCtx.get()), SSL_free);
    if (!ssl) {
        printMessage("Failed to create SSL object: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    // Ensure we have a valid socket and get its handle
    if (!tcpSocket) {
        printMessage("No valid socket for TLS setup");
        return false;
    }

    auto _tcpSocket = dynamic_cast<TcpSocketWithHandle*>(tcpSocket.get());
    if (!_tcpSocket) {
        printMessage("Failed to get socket handle for TLS");
        return false;
    }

    int sock = _tcpSocket->getHandle();
    if (sock < 0) {
        printMessage("Invalid socket handle: " + std::to_string(sock));
        return false;
    }

    if (SSL_set_fd(ssl.get(), sock) != 1) {
        printMessage("Failed to set SSL file descriptor: " + std::string(ERR_error_string(ERR_get_error(), nullptr)));
        return false;
    }

    // Perform SSL handshake based on server/client role
    printMessage("Performing SSL handshake...");
    int result;
    int retry_count = 0;
    const int max_retries = 3;

    while (retry_count < max_retries) {
        if (isServer) {
            // Server accepts SSL connection
            result = SSL_accept(ssl.get());
        }
        else {
            // Client initiates SSL connection
            result = SSL_connect(ssl.get());
        }

        if (result == 1) {
            // Success!
            break;
        }

        int error = SSL_get_error(ssl.get(), result);

        // Handle specific error cases
        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
            // Need to retry the operation
            retry_count++;
            printMessage("Retrying SSL handshake, attempt " + std::to_string(retry_count));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        // Any other error is fatal
        unsigned long errCode = ERR_get_error();
        char errBuffer[256];
        ERR_error_string_n(errCode, errBuffer, sizeof(errBuffer));

        printMessage("SSL handshake failed: " + std::string(errBuffer) + " (code: " + std::to_string(error) + ")");

        // Additional debugging information
        if (error == SSL_ERROR_SSL) {
            printMessage("Protocol error. Check if both sides are using SSL/TLS.");
        }
        else if (error == SSL_ERROR_SYSCALL) {
            printMessage("I/O error. Check if the connection was closed unexpectedly.");
        }

        return false;
    }

    if (result != 1) {
        printMessage("SSL handshake failed after " + std::to_string(max_retries) + " attempts");
        return false;
    }

    // Log the negotiated protocol and cipher
    printMessage("SSL connection established using " +
        std::string(SSL_get_version(ssl.get())) +
        " with cipher " +
        std::string(SSL_get_cipher(ssl.get())));

    printMessage("TLS setup completed successfully for " + address + ":" + std::to_string(port));
    return true;
}


bool TCPSock::cleanupTLS() {
    if (ssl) {
        printMessage("Shutting down SSL connection...");
        SSL_shutdown(ssl.get());
    }
    ssl.reset();
    sslCtx.reset();
    return true;
}

bool TCPSock::runServer() {
    if (thisServerRunning) {
        printMessage("Server is already running");
        return false;
    }

    if (!bind(port) || !listen()) {
        printMessage("Failed to start server");
        return false;
    }

    thisServerRunning = true;
    std::thread serverThread([this]() {
        printMessage("TCP server started on port " + std::to_string(port));

        // Set listener to non-blocking mode to allow clean shutdown
        if (tcpListener) {
            tcpListener->setBlocking(false);
        }

        while (thisServerRunning) {
            auto conn = dynamic_cast<TCPSock*>(accept());
            if (!conn) {
                if (!thisServerRunning) break;
                // Small sleep to prevent CPU spike if accept fails repeatedly
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            // Verify connection before creating thread
            if (!conn->isConnected()) {
                printMessage("Discarding invalid connection");
                delete conn;
                continue;
            }

            // Log connection info
            printMessage("New client connection established from " + conn->getAddress() + ":" + std::to_string(conn->getPort()));

            // Create a unique_ptr to manage the connection in the new thread
            std::unique_ptr<TCPSock> connPtr(conn);

            try {
                std::thread clientThread(&TCPSock::handleClientRequest, this, std::move(connPtr));
                clientThread.detach();
            }
            catch (const std::exception& e) {
                printMessage("Failed to create client thread: " + std::string(e.what()));
            }
        }

        printMessage("TCP server stopped");
        });

    serverThread.detach();
    return true;
}

void TCPSock::stopServer() {
    if (thisServerRunning) {
        printMessage("Stopping server...");
        thisServerRunning = false;
        disconnect();
        tcpListener.reset();
        printMessage("Server stopped");
    }
}

bool TCPSock::isServerRunning() const {
    return thisServerRunning;
}

void TCPSock::handleClientRequest(std::unique_ptr<TCPSock> clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        printMessage("Invalid client connection");
        return;
    }

    std::string clientInfo = clientSock->getAddress() + ":" + std::to_string(clientSock->getPort());
    printMessage("Handling client request from " + clientInfo);

    // Set a reasonable timeout for operations
    clientSock->setTimeout(10000);  // 10 seconds timeout

    try {
        // Keep connection alive to handle multiple requests
        while (clientSock->isConnected()) {
            std::string request = clientSock->receive();

            // Check if client disconnected
            if (!clientSock->isConnected()) {
                printMessage("Client disconnected: " + clientInfo);
                break;
            }

            // Process received data
            if (!request.empty()) {
                printMessage("Received " + std::to_string(request.length()) + " bytes from " + clientInfo);

                // Send response back to client
                std::string response = "Server response: Received " + std::to_string(request.length()) + " bytes";
                int sent = clientSock->send(response);

                if (sent > 0) {
                    printMessage("Sent " + std::to_string(sent) + " bytes to " + clientInfo);
                }
                else {
                    printMessage("Failed to send response to " + clientInfo);
                    break;
                }
            }
            else {
                // No data received but still connected - could be waiting for more data
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }
    catch (const std::exception& e) {
        printMessage("Exception handling client connection " + clientInfo + ": " + std::string(e.what()));
    }

    // Ensure clean disconnect
    clientSock->disconnect();
    printMessage("Client connection from " + clientInfo + " handled and closed");
}