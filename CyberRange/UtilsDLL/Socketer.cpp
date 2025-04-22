#include "Socketer.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

Socketer::Socketer(std::string addr, int port)
    : address(addr), port(port), connected(false), socketType(SOCK_STREAM),
    timeout(30000), tlsEnabled(false), tlsContext(nullptr) {
}

Socketer::~Socketer() {
    if (connected) {
        disconnect();
    }
    cleanupTLS();
}

bool Socketer::connect() {
    // Basic socket connection will be implemented in derived classes
    // This is just a placeholder
    std::cout << "Connecting to " << address << ":" << port << std::endl;

    // If TLS is enabled, initialize it after establishing socket connection
    if (tlsEnabled) {
        if (!initializeTLS()) {
            std::cerr << "Failed to initialize TLS" << std::endl;
            return false;
        }
    }

    connected = true;
    return connected;
}

bool Socketer::disconnect() {
    if (!connected) {
        return true;
    }

    // Clean up TLS first if it's enabled
    if (tlsEnabled) {
        cleanupTLS();
    }

    // Socket disconnection will be implemented by derived classes
    connected = false;
    return true;
}

bool Socketer::isConnected() {
    return connected;
}

int Socketer::send(std::string data) {
    // To be implemented by derived classes
    return -1;
}

std::string Socketer::receive() {
    // To be implemented by derived classes
    return "";
}

void Socketer::setTimeout(int ms) {
    timeout = ms;
}

std::string Socketer::getAddress() {
    return address + ":" + std::to_string(port);
}

bool Socketer::enableTLS() {
    if (connected) {
        std::cerr << "Cannot enable TLS on already connected socket" << std::endl;
        return false;
    }

    tlsEnabled = true;
    return true;
}

bool Socketer::isTLSEnabled() {
    return tlsEnabled;
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