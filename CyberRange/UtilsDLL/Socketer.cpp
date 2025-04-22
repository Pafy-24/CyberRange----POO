#include "pch.h"
#include "Socketer.h"
#include <iostream>
#include <cstring>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Static initialization
bool Socketer::winsockInitialized = false;

// Client constructor
Socketer::Socketer(const std::string& addr, int port)
    : socketFD(INVALID_SOCKET), address(addr), port(port), connected(false),
    isServer(false), socketType(SOCK_STREAM), timeout(30000),
    tlsEnabled(false), tlsContext(nullptr) {
    // Initialize Winsock
    initializeWinsock();
}

// Server constructor
Socketer::Socketer(int port)
    : socketFD(INVALID_SOCKET), address("0.0.0.0"), port(port), connected(false),
    isServer(true), socketType(SOCK_STREAM), timeout(30000),
    tlsEnabled(false), tlsContext(nullptr) {
    // Initialize Winsock
    initializeWinsock();
	bind(port);
}

// Accept constructor
Socketer::Socketer(SOCKET sock, const std::string& clientAddr, int clientPort)
    : socketFD(sock), address(clientAddr), port(clientPort), connected(true),
    isServer(false), socketType(SOCK_STREAM), timeout(30000),
    tlsEnabled(false), tlsContext(nullptr) {
    // Socket is already connected
}

Socketer::~Socketer() {
    if (connected) {
        disconnect();
    }
    cleanupTLS();
}

bool Socketer::initializeWinsock() {
    if (!winsockInitialized) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cerr << "WSAStartup failed: " << result << std::endl;
            return false;
        }
        winsockInitialized = true;
    }
    return true;
}

void Socketer::cleanupWinsock() {
    if (winsockInitialized) {
        WSACleanup();
        winsockInitialized = false;
    }
}

bool Socketer::connect() {
    if (isServer || connected || socketFD != INVALID_SOCKET) {
        std::cerr << "Cannot connect: socket already in use or is a server" << std::endl;
        return false;
    }

    struct addrinfo hints, * result = NULL;

    // Create socket
    socketFD = socket(AF_INET, socketType, socketType == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP);
    if (socketFD == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Resolve hostname
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socketType;
    hints.ai_protocol = socketType == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP;

    // Extract hostname from address (which may include port)
    std::string hostname = address;
    size_t colonPos = hostname.find(':');
    if (colonPos != std::string::npos) {
        hostname = hostname.substr(0, colonPos);
    }

    // Convert port to string for getaddrinfo
    std::string portStr = std::to_string(port);

    if (getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &result) != 0) {
        std::cerr << "Error resolving hostname: " << WSAGetLastError() << std::endl;
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }

    // Connect to server
    if (::connect(socketFD, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            std::cerr << "Error connecting to server: " << err << std::endl;
            freeaddrinfo(result);
            closesocket(socketFD);
            socketFD = INVALID_SOCKET;
            return false;
        }
    }

    freeaddrinfo(result);

    // If TLS is enabled, set up the SSL connection
    if (tlsEnabled) {
        if (!initializeTLS()) {
            std::cerr << "Failed to initialize TLS" << std::endl;
            closesocket(socketFD);
            socketFD = INVALID_SOCKET;
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

    if (socketFD != INVALID_SOCKET) {
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
    }

    connected = false;
    return true;
}

bool Socketer::isConnected() const {
    return connected;
}


int Socketer::send(std::string data) {
    if (!connected || socketFD == INVALID_SOCKET) {
        std::cerr << "Cannot send: not connected" << std::endl;
        return -1;
    }

    // This base implementation should be overridden by derived classes
    // to handle TLS/DTLS correctly
    int bytesSent = ::send(socketFD, data.c_str(), (int)data.length(), 0);

    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
        return -1;
    }

    return bytesSent;
}

std::string Socketer::receive() {
    if (!connected || socketFD == INVALID_SOCKET) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }

    // This base implementation should be overridden by derived classes
    // to handle TLS/DTLS correctly
    char buffer[4096];
    int bytesRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead == SOCKET_ERROR) {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
        return "";
    }
    else if (bytesRead == 0) {
        // Connection closed
        return "";
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

bool Socketer::bind(int bindPort) {
    if (connected || socketFD != INVALID_SOCKET) {
        std::cerr << "Cannot bind: socket already in use" << std::endl;
        return false;
    }

    // If a port was provided, use it instead of the default
    if (bindPort > 0) {
        port = bindPort;
    }

    // Create socket
    socketFD = socket(AF_INET, socketType, socketType == SOCK_STREAM ? IPPROTO_TCP : IPPROTO_UDP);
    if (socketFD == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set SO_REUSEADDR option
    BOOL reuse = TRUE;
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        std::cerr << "Error setting SO_REUSEADDR: " << WSAGetLastError() << std::endl;
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }

    // Bind to address
    struct sockaddr_in serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;  // Bind to all interfaces

    if (::bind(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding socket: " << WSAGetLastError() << std::endl;
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }

    isServer = true;
    return true;
}

bool Socketer::listen(int backlog) {
    if (!isServer || socketFD == INVALID_SOCKET) {
        std::cerr << "Cannot listen: not a server socket or socket not created" << std::endl;
        return false;
    }

    if (socketType != SOCK_STREAM) {
        std::cerr << "Cannot listen: only TCP sockets can listen" << std::endl;
        return false;
    }

    if (::listen(socketFD, backlog) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket: " << WSAGetLastError() << std::endl;
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }

    connected = true;  // For a server, "connected" means ready to accept connections
    return true;
}

Connection* Socketer::accept() {
    if (!isServer || !connected || socketFD == INVALID_SOCKET) {
        std::cerr << "Cannot accept: not a listening server socket" << std::endl;
        return nullptr;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    SOCKET clientSocket = ::accept(socketFD, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error accepting connection: " << WSAGetLastError() << std::endl;
        return nullptr;
    }

    // Get client address as string
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);

    // Create new socket object for client
    Socketer* clientSock = new Socketer(clientSocket, clientIP, clientPort);

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

    tlsEnabled = true;
    return true;
}

bool Socketer::isTLSEnabled() const {
    return tlsEnabled;
}

void Socketer::setTimeout(int ms) {
    timeout = ms;

    if (socketFD != INVALID_SOCKET) {
        // Set receive timeout
        DWORD recvTimeout = ms;
        if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&recvTimeout, sizeof(recvTimeout)) == SOCKET_ERROR) {
            std::cerr << "Error setting receive timeout: " << WSAGetLastError() << std::endl;
        }

        // Set send timeout
        DWORD sendTimeout = ms;
        if (setsockopt(socketFD, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sendTimeout, sizeof(sendTimeout)) == SOCKET_ERROR) {
            std::cerr << "Error setting send timeout: " << WSAGetLastError() << std::endl;
        }
    }
}

int Socketer::getTimeout() const {
    return timeout;
}

bool Socketer::initializeTLS() {
    // Initialize OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // Create a new SSL context
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);

    if (!ctx) {
        std::cerr << "Failed to create SSL context" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    tlsContext = ctx;
    return true;
}

bool Socketer::cleanupTLS() {
    if (tlsContext) {
        SSL_CTX_free(static_cast<SSL_CTX*>(tlsContext));
        tlsContext = nullptr;
    }

    // Clean up OpenSSL
    EVP_cleanup();
    ERR_free_strings();

    tlsEnabled = false;
    return true;
}

bool Socketer::setSocketOption(int level, int optname, const char* optval, int optlen) {
    if (socketFD == INVALID_SOCKET) {
        std::cerr << "Cannot set socket option: socket not created" << std::endl;
        return false;
    }

    if (setsockopt(socketFD, level, optname, optval, optlen) == SOCKET_ERROR) {
        std::cerr << "Error setting socket option: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

bool Socketer::setNonBlocking(bool nonBlocking) {
    if (socketFD == INVALID_SOCKET) {
        std::cerr << "Cannot set non-blocking mode: socket not created" << std::endl;
        return false;
    }

    u_long mode = nonBlocking ? 1 : 0;  // 1 = non-blocking, 0 = blocking
    if (ioctlsocket(socketFD, FIONBIO, &mode) != 0) {
        std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}