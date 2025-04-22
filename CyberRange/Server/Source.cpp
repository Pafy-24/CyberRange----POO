#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include "TCPSock.h"
#include "UDPSock.h"
#include "DBConn.h"
#include "AdminConn.h"

// Global flag for controlling server shutdown
std::atomic<bool> serverRunning(true);

// Function to print thread-safe messages
void printMessage(const std::string& message) {
    std::cout << message << std::endl;
}

// TCP Server function
void runTCPServer(int port) {
    printMessage("Starting TCP Server on port " + std::to_string(port));

    TCPSock listenSock(port);
    listenSock.listen();

    printMessage("TCP Server listening on port " + std::to_string(port));
	printMessage("Is TLS enabled: " + std::to_string(listenSock.isTLSEnabled()));
    std::vector<TCPSock*> clientSockets;

    while (serverRunning) {
		TCPSock* clientSock = (TCPSock*)listenSock.accept();
		if (clientSock) {
			printMessage("Accepted new client connection from " + clientSock->getAddress());
			clientSockets.push_back(clientSock);
		}

        for (auto& client : clientSockets) {
            if (client->isConnected()) {
                std::string data = client->receive();
                if (!data.empty()) {
                    printMessage("TCP Server received: " + data);
                    client->send("Hello there!");
                }
            }
        }

        auto it = clientSockets.begin();
        while (it != clientSockets.end()) {
            if (!(*it)->isConnected()) {
                delete* it;
                it = clientSockets.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    // Clean up
    for (auto& client : clientSockets) {
        client->disconnect();
        delete client;
    }
    clientSockets.clear();

    printMessage("TCP Server stopped");
}

// UDP Server function
void runUDPServer(int port) {
    printMessage("Starting UDP Server on port " + std::to_string(port));

    // Create a UDP socket for receiving
    UDPSock udpSock("0.0.0.0", port);

    // Connect socket (in UDP this doesn't establish a connection,
    // but sets up the socket for sending and receiving)
    if (!udpSock.connect()) {
        printMessage("Failed to set up UDP server socket");
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

            // Echo back to sender
            // In a real implementation, we would parse the sender address
            // and send back to that specific client
            udpSock.send("Echo: " + data);
        }

        // Small delay to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    udpSock.disconnect();
    printMessage("UDP Server stopped");
}

// Database server function (simulation)
void runDBServer() {
    printMessage("Starting Database Server simulation");

    // Create a database connection string
    std::string connStr = "mysql://dbuser:password@localhost:3306/testdb";
    DBConn dbConn(connStr);

    // Enable TLS for secure connections
    dbConn.enableTLS();

    // Connect to the database (simulated)
    if (!dbConn.connect()) {
        printMessage("Database server failed to initialize");
        return;
    }

    printMessage("Database Server running at " + dbConn.getAddress());

    // Admin connection for database management
    AdminConn adminConn("mysql://admin:adminpass@localhost:3306/testdb");
    adminConn.setAdminKey("SuperSecretAdminKey");
    adminConn.connect();

    printMessage("Admin connection established with privilege level: " +
        std::to_string(adminConn.getPrivilegeLevel()));

    // Main server loop
    while (serverRunning) {
        // In a real implementation, we would accept connections and process queries
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Clean up
    adminConn.disconnect();
    dbConn.disconnect();

    printMessage("Database Server stopped");
}

// HTTP server for testing downloads (simulation)
void runHTTPServer(int port) {
    printMessage("Starting HTTP Server on port " + std::to_string(port));

    // Create a TCP socket for the HTTP server
    TCPSock httpSock("0.0.0.0", port);
    httpSock.setBlocking(true);

    // Enable TLS for HTTPS
    httpSock.enableTLS();

    // Set up server socket (simulated)
    printMessage("HTTP Server listening on port " + std::to_string(port));

    // Main server loop
    while (serverRunning) {
        // In a real implementation, we would accept connections and serve HTTP responses
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    httpSock.disconnect();
    printMessage("HTTP Server stopped");
}

int main() {
    printMessage("Network Test Server Starting");

    // Start server threads
    std::thread tcpServerThread(runTCPServer, 1337);
   // std::thread udpServerThread(runUDPServer, 8081);
    //std::thread dbServerThread(runDBServer);

    // Keep server running until user terminates
    printMessage("All servers started. Press Enter to stop servers.");
    std::cin.get();

    // Signal all threads to stop
    serverRunning = false;

    // Wait for all threads to finish
    tcpServerThread.join();
  //  udpServerThread.join();
  //  dbServerThread.join();
   // httpServerThread.join();

    printMessage("All servers stopped. Test complete.");
    return 0;
}