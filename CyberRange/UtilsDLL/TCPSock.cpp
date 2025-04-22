#include "pch.h"
#include "TCPSock.h"
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Link with OpenSSL libraries
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

// Client constructor
TCPSock::TCPSock(const std::string& addr, int port)
    : Socketer(addr, port), blocking(true), sslHandle(nullptr) {
    socketType = SOCK_STREAM;
}

// Server constructor
TCPSock::TCPSock(int port=0)
    : Socketer(port), blocking(true), sslHandle(nullptr) {
    socketType = SOCK_STREAM;
}

// Accept constructor
TCPSock::TCPSock(SOCKET sock, const std::string& clientAddr, int clientPort)
    : Socketer(sock, clientAddr, clientPort), blocking(true), sslHandle(nullptr) {
    socketType = SOCK_STREAM;
}

TCPSock::~TCPSock() {
    if (isConnected()) {
        disconnect();
    }
}

bool TCPSock::connect() {
    // Create socket and connect using base class logic
    if (!Socketer::connect()) {
        return false;
    }

    // Set blocking mode if needed
    if (!blocking) {
        setNonBlocking(true);
    }

    // If TLS is enabled, setup the SSL connection
    if (isTLSEnabled()) {
        if (!setupTLS()) {
            disconnect();
            return false;
        }
    }

    return true;
}

bool TCPSock::disconnect() {
    // Clean up SSL if it's enabled
    if (sslHandle) {
        teardownTLS();
    }

    // Disconnect socket using base class logic
    return Socketer::disconnect();
}

int TCPSock::send(std::string data) {
    if (!isConnected()) {
        std::cerr << "Cannot send: not connected" << std::endl;
        return -1;
    }

    int bytesSent;

    if (sslHandle) {
        bytesSent = SSL_write(static_cast<SSL*>(sslHandle), data.c_str(), (int)data.length());
        if (bytesSent <= 0) {
            int sslError = SSL_get_error(static_cast<SSL*>(sslHandle), bytesSent);
            std::cerr << "SSL send error: " << sslError << std::endl;
            ERR_print_errors_fp(stderr);
            return -1;
        }
    }
    else {
        bytesSent = ::send(socketFD, data.c_str(), (int)data.length(), 0);
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
            return -1;
        }
    }

    return bytesSent;
}

std::string TCPSock::receive() {
    if (!isConnected()) {
        std::cerr << "Cannot receive: not connected" << std::endl;
        return "";
    }

    char buffer[4096];
    int bytesRead;

    if (sslHandle) {
        // Receive through SSL if TLS is enabled
        bytesRead = SSL_read(static_cast<SSL*>(sslHandle), buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0) {
            int sslError = SSL_get_error(static_cast<SSL*>(sslHandle), bytesRead);
            if (sslError == SSL_ERROR_WANT_READ || sslError == SSL_ERROR_WANT_WRITE) {
                // Non-blocking operation would block
                return "";
            }
            std::cerr << "SSL receive error: " << sslError << std::endl;
            ERR_print_errors_fp(stderr);
            return "";
        }
    }
    else {
        // Regular receive
        bytesRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                // Non-blocking operation would block
                return "";
            }
            std::cerr << "Error receiving data: " << error << std::endl;
            return "";
        }
        else if (bytesRead == 0) {
            // Connection closed
            connected = false;
            return "";
        }
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

Connection* TCPSock::accept() {
    if (!isServer || !isConnected()) {
        std::cerr << "Cannot accept: not a listening server socket" << std::endl;
        return nullptr;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    SOCKET clientSocket = ::accept(socketFD, (struct sockaddr*)&clientAddr, &clientAddrLen);

    if (clientSocket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) {
            // Non-blocking operation would block
            return nullptr;
        }
        std::cerr << "Error accepting connection: " << error << std::endl;
        return nullptr;
    }

    // Get client address as string
    char clientIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
    int clientPort = ntohs(clientAddr.sin_port);

    // Create new TCP socket object for client
    TCPSock* clientSock = new TCPSock(clientSocket, clientIP, clientPort);

    // If server has TLS enabled, the client should setup TLS too
    if (tlsEnabled) {
        clientSock->enableTLS();
        clientSock->setupTLS();
    }

    return clientSock;
}

void TCPSock::setBlocking(bool block) {
    blocking = block;

    if (socketFD != INVALID_SOCKET) {
        setNonBlocking(!block);
    }
}

bool TCPSock::isBlocking() const {
    return blocking;
}

bool TCPSock::enableTLS() {
    if (Socketer::enableTLS()) {
        // If already connected, setup TLS immediately
        if (isConnected() && !sslHandle) {
            return setupTLS();
        }
        return true;
    }
    return false;
}

bool TCPSock::setupTLS() {
    if (!tlsEnabled || sslHandle) {
        return tlsEnabled;
    }

    SSL_CTX* ctx = static_cast<SSL_CTX*>(tlsContext);
    if (!ctx) {
        if (!initializeTLS()) {
            std::cerr << "Failed to initialize TLS context" << std::endl;
            return false;
        }
        ctx = static_cast<SSL_CTX*>(tlsContext);
    }

    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        std::cerr << "Error creating SSL structure" << std::endl;
        return false;
    }

    SSL_set_fd(ssl, (int)socketFD);

    // Setup as server or client based on role
    int result;
    if (isServer) {
        result = SSL_accept(ssl);
    }
    else {
        result = SSL_connect(ssl);
    }

    if (result <= 0) {
        int sslError = SSL_get_error(ssl, result);
        std::cerr << "Error establishing SSL connection: " << sslError << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        return false;
    }

    sslHandle = ssl;
    std::cout << "TLS connection established" << std::endl;
    return true;
}

bool TCPSock::teardownTLS() {
    if (!sslHandle) {
        return true;
    }

    SSL* ssl = static_cast<SSL*>(sslHandle);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    sslHandle = nullptr;

    return true;
}