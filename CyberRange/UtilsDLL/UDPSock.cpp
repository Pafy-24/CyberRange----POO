#include "pch.h"
#include "UDPSock.h"
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>

// Link with OpenSSL libraries
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

UDPSock::UDPSock(std::string addr, int port)
    : Socketer(addr, port), socketFD(INVALID_SOCKET), maxPacketSize(8192), dtlsContext(nullptr) {
}

UDPSock::~UDPSock() {
    if (socketFD != INVALID_SOCKET) {
        disconnect();
    }
}

bool UDPSock::connect() {
    struct addrinfo hints, * result = NULL;

    // Create socket
    socketFD = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socketFD == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Resolve hostname
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

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

    // For UDP, we don't actually connect in the traditional sense
    // But we can use connect() to set the default destination for send()
    if (::connect(socketFD, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Error connecting UDP socket: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }

    freeaddrinfo(result);

    // If DTLS (TLS for UDP) is enabled, set it up
    if (isTLSEnabled() && dtlsContext != nullptr) {
        // DTLS setup would go here
        // This is a simplified version as DTLS is complex
        std::cout << "Setting up DTLS connection" << std::endl;
        // In a real implementation, you would initialize DTLS here
    }

    Socketer::connect(); // Call parent's method to set the connected flag
    return true;
}

bool UDPSock::disconnect() {
    if (socketFD == INVALID_SOCKET) {
        return true;
    }

    // Clean up DTLS if it's enabled
    if (dtlsContext) {
        // Clean up DTLS context
        // This would be properly implemented in a real system
        dtlsContext = nullptr;
    }

    closesocket(socketFD);
    socketFD = INVALID_SOCKET;

    return Socketer::disconnect(); // Call parent's method to reset the connected flag
}

int UDPSock::send(std::string data) {
    if (!isConnected() || socketFD == INVALID_SOCKET) {
        return -1;
    }

    int bytesSent;

    if (isTLSEnabled() && dtlsContext != nullptr) {
        // In a real implementation, you would use DTLS_write or equivalent
        std::cout << "Sending data via DTLS" << std::endl;
        // Placeholder for DTLS send
        bytesSent = (int)data.length(); // Pretend we sent it all
    }
    else {
        // Regular UDP send
        bytesSent = ::send(socketFD, data.c_str(), (int)data.length(), 0);
    }

    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error sending data: " << WSAGetLastError() << std::endl;
    }

    return bytesSent;
}

std::string UDPSock::receive() {
    if (!isConnected() || socketFD == INVALID_SOCKET) {
        return "";
    }

    char buffer[65536]; // Maximum UDP packet size
    int bytesRead;

    if (isTLSEnabled() && dtlsContext != nullptr) {
        // In a real implementation, you would use DTLS_read or equivalent
        std::cout << "Receiving data via DTLS" << std::endl;
        // Placeholder for DTLS receive
        bytesRead = 0; // Pretend we received nothing for now
    }
    else {
        // Regular UDP receive
        bytesRead = recv(socketFD, buffer, min(maxPacketSize, 65536), 0);
    }

    if (bytesRead == SOCKET_ERROR) {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
        return "";
    }

    return std::string(buffer, bytesRead);
}

bool UDPSock::broadcast(std::string data) {
    if (!isConnected() || socketFD == INVALID_SOCKET) {
        return false;
    }

    // Enable broadcast option
    BOOL broadcastEnable = TRUE;
    if (setsockopt(socketFD, SOL_SOCKET, SO_BROADCAST,
        (const char*)&broadcastEnable, sizeof(broadcastEnable)) == SOCKET_ERROR) {
        std::cerr << "Error enabling broadcast: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Create broadcast address
    struct sockaddr_in broadcastAddr;
    ZeroMemory(&broadcastAddr, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons((u_short)port);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    // Send the broadcast
    int bytesSent = sendto(socketFD, data.c_str(), (int)data.length(), 0,
        (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error broadcasting data: " << WSAGetLastError() << std::endl;
        return false;
    }

    return true;
}

std::string UDPSock::receiveFrom(std::string& sender) {
    if (!isConnected() || socketFD == INVALID_SOCKET) {
        return "";
    }

    char buffer[65536]; // Maximum UDP packet size
    struct sockaddr_in senderAddr;
    int senderLen = sizeof(senderAddr);

    int bytesRead = recvfrom(socketFD, buffer, min(maxPacketSize, 65536), 0,
        (struct sockaddr*)&senderAddr, &senderLen);

    if (bytesRead == SOCKET_ERROR) {
        std::cerr << "Error receiving data: " << WSAGetLastError() << std::endl;
        return "";
    }

    // Get sender address as string
    char senderIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP, INET_ADDRSTRLEN);
    sender = std::string(senderIP) + ":" + std::to_string(ntohs(senderAddr.sin_port));

    return std::string(buffer, bytesRead);
}

void UDPSock::setMaxPacketSize(int size) {
    maxPacketSize = size;
}

bool UDPSock::enableTLS() {
    // For UDP, we use DTLS instead of TLS
    // Initialize DTLS context
    // This is a simplified version

    if (isConnected()) {
        std::cerr << "Cannot enable DTLS on already connected socket" << std::endl;
        return false;
    }

    // Initialize OpenSSL for DTLS
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // Create a new DTLS context
    const SSL_METHOD* method = DTLS_client_method();
    SSL_CTX* ctx = SSL_CTX_new(method);

    if (!ctx) {
        std::cerr << "Failed to create DTLS context" << std::endl;
        ERR_print_errors_fp(stderr);
        return false;
    }

    dtlsContext = ctx;
    return Socketer::enableTLS(); // Call parent's method to set the tlsEnabled flag
}