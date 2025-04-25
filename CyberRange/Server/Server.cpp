#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <fstream>
#include "TCPSock.h"
#include "UDPSock.h"
#include "DBConn.h"
#include "AdminConn.h"

// Global flag for controlling server shutdown
std::atomic<bool> serverRunning(true);
std::mutex consoleMutex;

// Function to print thread-safe messages
void printMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "[" << std::this_thread::get_id() << "] " << message << std::endl;
}

// TCP Server function
void runTCPServer(int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    printMessage("Starting " + serverType + " Server on port " + std::to_string(port));

    TCPSock listenSock(port);

    // Enable TLS if requested
    if (useTLS) {
        if (listenSock.enableTLS()) {
            printMessage("TLS enabled successfully");
        }
        else {
            printMessage("Failed to enable TLS");
        }
    }

    if (!listenSock.bind(port)) {
        printMessage("Failed to bind " + serverType + " server to port " + std::to_string(port));
        return;
    }

    if (!listenSock.listen()) {
        printMessage("Failed to start listening on " + serverType + " server");
        return;
    }

    printMessage(serverType + " Server listening on port " + std::to_string(port));
    printMessage("Is TLS enabled: " + std::string(listenSock.isTLSEnabled() ? "true" : "false"));

    std::vector<TCPSock*> clientSockets;

    while (serverRunning) {
        TCPSock* clientSock = (TCPSock*)listenSock.accept();
        if (clientSock) {
            printMessage("Accepted new client connection from " + clientSock->getAddress() +
                ":" + std::to_string(clientSock->getPort()));
            clientSockets.push_back(clientSock);
        }

        // Process existing connections
        for (auto& client : clientSockets) {
            if (client && client->isConnected()) {
                std::string data = client->receive();
                if (!data.empty()) {
                    printMessage(serverType + " Server received: " + data);
                    client->send("Response from " + serverType + " server: " + data);
                }
            }
        }

        // Clean up disconnected clients
        auto it = clientSockets.begin();
        while (it != clientSockets.end()) {
            if (!(*it) || !(*it)->isConnected()) {
                delete* it;
                it = clientSockets.erase(it);
            }
            else {
                ++it;
            }
        }

        // Small delay to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean up
    for (auto& client : clientSockets) {
        if (client) {
            client->disconnect();
            delete client;
        }
    }
    clientSockets.clear();

    printMessage(serverType + " Server stopped");
}

// UDP Server function
void runUDPServer(int port) {
    printMessage("Starting UDP Server on port " + std::to_string(port));

    // Create a UDP socket for receiving
    UDPSock udpSock("0.0.0.0", port);

    // Bind UDP socket to specified port
    if (!udpSock.bind(port)) {
        printMessage("Failed to bind UDP server to port " + std::to_string(port));
        return;
    }

    printMessage("UDP Server listening on port " + std::to_string(port));

    // Main server loop
    while (serverRunning) {
        // Check for incoming data
        std::string sender;
        std::string data = udpSock.receiveFrom(sender);

        if (!data.empty()) {
            printMessage("UDP Server received from " + sender + ": " + data);

            // Extract sender address and port
            size_t colonPos = sender.find(':');
            if (colonPos != std::string::npos) {
                std::string senderAddr = sender.substr(0, colonPos);
                int senderPort = std::stoi(sender.substr(colonPos + 1));

                // Create a response socket
                UDPSock responseSock(senderAddr, senderPort);
                if (responseSock.connect()) {
                    responseSock.send("UDP Echo: " + data);
                    printMessage("UDP response sent to " + senderAddr + ":" +
                        std::to_string(senderPort));
                }
            }
        }

        // Small delay to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    udpSock.disconnect();
    printMessage("UDP Server stopped");
}

int main() {
    printMessage("Network Test Server Starting");

    // Server ports
    int tcpPort = 1337;
    int securePort = 1338;
    int udpPort = 8081;
  //  int downloadPort = 8082;

    // Start server threads
    std::thread tcpServerThread(runTCPServer, tcpPort, false); // Standard TCP
    std::thread secureTcpServerThread(runTCPServer, securePort, true); // Secure TCP with TLS
    std::thread udpServerThread(runUDPServer, udpPort);
   // std::thread downloadServerThread(runDownloadServer, downloadPort);

    // Keep server running until user terminates
    printMessage("All servers started. Press Enter to stop servers.");
    std::cin.get();

    // Signal all threads to stop
    serverRunning = false;

    // Wait for all threads to finish
    tcpServerThread.join();
    secureTcpServerThread.join();
    udpServerThread.join();
//    downloadServerThread.join();

    printMessage("All servers stopped. Test complete.");
    return 0;
}