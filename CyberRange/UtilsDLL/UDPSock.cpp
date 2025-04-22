#include "UDPSock.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

UDPSock::UDPSock(std::string addr, int port)
    : Socketer(addr, port), socketFD(-1), maxPacketSize(8192), dtlsContext(nullptr) {
}

UDPSock::~UDPSock() {
    if (socketFD >= 0) {
        disconnect();
    }
}

bool UDPSock::connect() {
    struct sockaddr_in serverAddr;
    struct hostent* server;

    // Create socket
    socketFD = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFD < 0) {
        std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
        return false;
    }

    // Resolve hostname
    server = gethostbyname(getAddress().c_str());
    if (server == nullptr) {
        std::cerr << "Error resolving hostname" << std::endl;
        close(socketFD);
        socketFD = -1;
        return false;
    }

    // Setup server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);

    // For UDP, we don't actually connect in the traditional sense
    // But we can use connect() to set the default destination for send()
    if (::connect(socketFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error connecting UDP socket: " << strerror(errno) << std::endl;
        close(socketFD);
        socketFD = -1;
        return false;
    }

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
    if (socketFD < 0) {
        return true;
    }

    // Clean up DTLS if it's enabled
    if (dtlsContext) {
        // Clean up DTLS context
        // This would be properly implemented in a real system
        dtlsContext = nullptr;
    }

    close(socketFD);
    socketFD = -1;

    return Socketer::disconnect(); // Call parent's method to reset the connected flag
}

int UDPSock::send(std::string data) {
    if (!isConnected() || socketFD < 0) {
        return -1;
    }

    int bytesSent;

    if (isTLSEnabled() && dtlsContext != nullptr) {
        // In a real implementation, you would use DTLS_write or equivalent
        std::cout << "Sending data via DTLS" << std::endl;
        // Placeholder for DTLS send
        bytesSent = data.length(); // Pretend we sent it all
    }
    else {
        // Regular UDP send
        bytesSent = ::send(socketFD, data.c_str(), data.length(), 0);
    }

    if (bytesSent < 0) {
        std::cerr << "Error sending data: " << strerror(errno) << std::endl;
    }

    return bytesSent;
}

std::string UDPSock::receive() {
    if (!isConnected() || socketFD < 0) {
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
        bytesRead = recv(socketFD, buffer, std::min(maxPacketSize, 65536), 0);
    }

    if (bytesRead < 0) {
        std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
        return "";
    }

    buffer[bytesRead] = '\0';
    return std::string(buffer, bytesRead);
}

bool UDPSock::broadcast(std::string data) {
    if (!isConnected() || socketFD < 0) {
        return false;
    }

    // Enable broadcast option
    int broadcastEnable = 1;
    if (setsockopt(socketFD, SOL_SOCKET, SO_BROADCAST,
        &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        std::cerr << "Error enabling broadcast: " << strerror(errno) << std::endl;
        return false;
    }

    // Create broadcast address
    struct sockaddr_in broadcastAddr;
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    // Send the broadcast
    int bytesSent = sendto(socketFD, data.c_str(), data.length(), 0,
        (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));

    if (bytesSent < 0) {
        std::cerr << "Error broadcasting data: " << strerror(errno) << std::endl;
        return false;
    }

    return true;
}

std::string UDPSock::receiveFrom(std::string& sender) {
    if (!isConnected() || socketFD < 0) {
        return "";
    }

    char buffer[65536]; // Maximum UDP packet size
    struct sockaddr_in senderAddr;
    socklen_t senderLen = sizeof(senderAddr);

    int bytesRead = recvfrom(socketFD, buffer, std::min(maxPacketSize, 65536), 0,
        (struct sockaddr*)&senderAddr, &senderLen);

    if (bytesRead < 0) {
        std::cerr << "Error receiving data: " << strerror(errno) << std::endl;
        return "";
    }

    // Get sender address as string
    char senderIP[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(senderAddr.sin_addr), senderIP, INET_ADDRSTRLEN);
    sender = std::string(senderIP) + ":" + std::to_string(ntohs(senderAddr.sin_port));

    buffer[bytesRead] = '\0';
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