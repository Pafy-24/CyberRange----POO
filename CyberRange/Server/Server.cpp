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

// File download server (simulates an FTP-like server using TCP)
void runDownloadServer(int port) {
    printMessage("Starting Download Server on port " + std::to_string(port));

    TCPSock listenSock(port);

    if (!listenSock.bind(port)) {
        printMessage("Failed to bind Download server to port " + std::to_string(port));
        return;
    }

    if (!listenSock.listen()) {
        printMessage("Failed to start listening on Download server");
        return;
    }

    printMessage("Download Server listening on port " + std::to_string(port));

    // Create test files for download
    std::string smallFilePath = "test_small.dat";
    std::string largeFilePath = "test_large.dat";

    // Create small test file (10KB)
    {
        std::ofstream smallFile(smallFilePath, std::ios::binary);
        if (smallFile.is_open()) {
            const int size = 10 * 1024;
            char* buffer = new char[size];
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(i % 256);
            }
            smallFile.write(buffer, size);
            smallFile.close();
            delete[] buffer;
            printMessage("Created small test file: " + smallFilePath + " (10KB)");
        }
    }

    // Create large test file (1MB)
    {
        std::ofstream largeFile(largeFilePath, std::ios::binary);
        if (largeFile.is_open()) {
            const int size = 1024 * 1024;
            char* buffer = new char[size];
            for (int i = 0; i < size; i++) {
                buffer[i] = static_cast<char>(i % 256);
            }
            largeFile.write(buffer, size);
            largeFile.close();
            delete[] buffer;
            printMessage("Created large test file: " + largeFilePath + " (1MB)");
        }
    }

    std::vector<TCPSock*> clientSockets;

    while (serverRunning) {
        TCPSock* clientSock = (TCPSock*)listenSock.accept();
        if (clientSock) {
            printMessage("Download server: Accepted new client connection from " +
                clientSock->getAddress() + ":" + std::to_string(clientSock->getPort()));
            clientSockets.push_back(clientSock);
        }

        // Process existing connections
        for (auto it = clientSockets.begin(); it != clientSockets.end();) {
            TCPSock* client = *it;
            if (client && client->isConnected()) {
                std::string request = client->receive();
                if (!request.empty()) {
                    printMessage("Download request received: " + request);

                    std::string filePath;
                    // Simple protocol: "GET filename"
                    if (request.find("GET test_small.dat") != std::string::npos) {
                        filePath = smallFilePath;
                    }
                    else if (request.find("GET test_large.dat") != std::string::npos) {
                        filePath = largeFilePath;
                    }

                    if (!filePath.empty()) {
                        // Open the requested file
                        std::ifstream file(filePath, std::ios::binary);
                        if (file.is_open()) {
                            // Get file size
                            file.seekg(0, std::ios::end);
                            std::streamsize fileSize = file.tellg();
                            file.seekg(0, std::ios::beg);

                            // Send file size first
                            client->send("SIZE " + std::to_string(fileSize));

                            // Read and send the file in chunks
                            const int bufferSize = 4096;
                            char buffer[bufferSize];
                            std::streamsize totalSent = 0;

                            while (file && totalSent < fileSize) {
                                file.read(buffer, bufferSize);
                                std::streamsize bytesRead = file.gcount();

                                if (bytesRead > 0) {
                                    std::string chunk(buffer, bytesRead);
                                    client->send(chunk);
                                    totalSent += bytesRead;

                                    // Simulate network delay for testing progress tracking
                                    std::this_thread::sleep_for(std::chrono::milliseconds(10));

                                    // Print progress every 10%
                                    int progress = static_cast<int>((totalSent * 100) / fileSize);
                                    if (progress % 10 == 0) {
                                        printMessage("Upload progress: " + std::to_string(progress) + "%");
                                    }
                                }
                            }

                            file.close();
                            printMessage("Sent file: " + filePath + " (" + std::to_string(totalSent) + " bytes)");
                        }
                        else {
                            // File not found
                            client->send("ERROR File not found");
                            printMessage("File not found: " + filePath);
                        }
                    }
                    else {
                        // Unknown request
                        client->send("ERROR Unknown command");
                        printMessage("Unknown download request");
                    }
                }
                ++it;
            }
            else {
                if (client) {
                    delete client;
                }
                it = clientSockets.erase(it);
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

    // Delete test files
    std::remove(smallFilePath.c_str());
    std::remove(largeFilePath.c_str());

    printMessage("Download Server stopped");
}

int main() {
    printMessage("Network Test Server Starting");

    // Server ports
    int tcpPort = 1337;
    int securePort = 1338;
    int udpPort = 8081;
    int downloadPort = 8082;

    // Start server threads
    std::thread tcpServerThread(runTCPServer, tcpPort, false); // Standard TCP
    std::thread secureTcpServerThread(runTCPServer, securePort, true); // Secure TCP with TLS
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