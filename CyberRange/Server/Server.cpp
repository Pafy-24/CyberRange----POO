#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include "TCPSock.h"
#include "UDPSock.h"
#include "transfer_sock.h"

std::atomic<bool> serverRunning(true);
std::mutex consoleMutex;

void printMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "[" << std::this_thread::get_id() << "] " << message << std::endl;
}

void runTCPServer(int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    printMessage("Starting " + serverType + " Server on port " + std::to_string(port));

    TCPSock server(port);
    if (useTLS && !server.enableTLS()) {
        printMessage("Failed to enable TLS for " + serverType);
        return;
    }

    if (!server.runServer()) {
        printMessage("Failed to start " + serverType + " server on port " + std::to_string(port));
        return;
    }

    printMessage(serverType + " Server running on port " + std::to_string(port) + ", TLS: " +
        (server.isTLSEnabled() ? "enabled" : "disabled"));

    while (serverRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevent CPU hogging
    }

    server.stopServer();
    printMessage(serverType + " Server stopped");
}

// UDP Server function
void runUDPServer(int port) {
    printMessage("Starting UDP Server on port " + std::to_string(port));

    UDPSock server("0.0.0.0", port);
    if (!server.runServer()) {
        printMessage("Failed to start UDP server on port " + std::to_string(port));
        return;
    }

    printMessage("UDP Server running on port " + std::to_string(port));

    while (serverRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevent CPU hogging
    }

    server.stopServer();
    printMessage("UDP Server stopped");
}

// Download Server function
void runDownloadServer(int port) {
    printMessage("Starting Download Server on port " + std::to_string(port));

    transfer_sock server(port);
    if (!server.runServer("./server_files")) {
        printMessage("Failed to start Download server on port " + std::to_string(port));
        return;
    }

    printMessage("Download Server running on port " + std::to_string(port));

    while (serverRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevent CPU hogging
    }

    server.stopServer();
    printMessage("Download Server stopped");
}

int main() {
    printMessage("Network Test Server Starting");

    // Server ports
    int tcpPort = 1337;       // Standard TCP
    int securePort = 1338;    // Secure TCP with TLS
    int udpPort = 8081;       // UDP
    int downloadPort = 8082;  // Download server

    // Start server threads
    std::thread tcpServerThread(runTCPServer, tcpPort, false);
    std::thread secureTcpServerThread(runTCPServer, securePort, true);
    std::thread udpServerThread(runUDPServer, udpPort);
    std::thread downloadServerThread(runDownloadServer, downloadPort);

    // Keep server running until user terminates
    printMessage("All servers started. Press Enter to stop servers.");
    std::cin.get();

    // Signal all threads to stop
    serverRunning = false;

    // Wait for all threads to finish
    tcpServerThread.join();
    secureTcpServerThread.join();
    udpServerThread.join();
    downloadServerThread.join();

    printMessage("All servers stopped. Test complete.");
    return 0;
}