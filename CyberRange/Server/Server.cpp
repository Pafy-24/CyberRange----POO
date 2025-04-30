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
#include "VM.h"
#include "Docker.h"


void runTCPServer(int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    printMessage("Starting " + serverType + " Server on port " + std::to_string(port));

    TCPSock server(port);
    if (useTLS && !server.enableTLS()) {
        printMessage("Failed to enable TLS for " + serverType);
        return;
    }
	if (server.isTLSEnabled())server.setCertificates("server.crt", "server.key");

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


void testConns(){
    printMessage("Network Test Server Starting");

    // Server ports
    int tcpPort = 1337;       // Standard TCP
    int securePort = 1338;    // Secure TCP with TLS
    int udpPort = 8081;       // UDP
    int downloadPort = 8082;  // Download server
    serverRunning = true;
    // Start server threads
  //  std::thread tcpServerThread(runTCPServer, tcpPort, false);
//    std::thread secureTcpServerThread(runTCPServer, securePort, true);
 //   std::thread udpServerThread(runUDPServer, udpPort);
    std::thread downloadServerThread(runDownloadServer, downloadPort);

    // Keep server running until user terminates
    printMessage("All servers started. Press Enter to stop servers.");
    std::cin.get();

    // Signal all threads to stop
    serverRunning = false;

    // Wait for all threads to finish
 //   tcpServerThread.join();
 //   secureTcpServerThread.join();
  //  udpServerThread.join();
    downloadServerThread.join();

    serverRunning = false;
    printMessage("All servers stopped. Test complete.");
}

void testVM() {
    std::cout << "\n=== Testing VM Functions ===\n";

    // Initialize VM orchestrator
    VM vm("test-vm", "C:\\CTF\\Images\\base.vdi", "192.168.1.100");

    // Set configuration
    vm.setMemory(2048);
    vm.setCPU(2);
    vm.setTimeout(600);

    std::string userId = "alice";
    std::string resourceId = "challenge1";

    // Test deploy
    std::cout << "Deploying VM for " << userId << "/" << resourceId << "... ";
    bool deployResult = vm.deploy(userId, resourceId);
    std::cout << (deployResult ? "Success" : "Failed") << "\n";

    // Test status
    std::cout << "VM Status:\n" << vm.getStatus(userId, resourceId) << "\n";

    // Test start
    std::cout << "Starting VM... ";
    bool startResult = vm.start(userId, resourceId);
    std::cout << (startResult ? "Success" : "Failed") << "\n";

    // Test status again
    std::cout << "VM Status:\n" << vm.getStatus(userId, resourceId) << "\n";

    // Test stop
    std::cout << "Stopping VM... ";
    bool stopResult = vm.stop(userId, resourceId);
    std::cout << (stopResult ? "Success" : "Failed") << "\n";

    // Test undeploy
    std::cout << "Undeploying VM... ";
    bool undeployResult = vm.undeploy(userId, resourceId);
    std::cout << (undeployResult ? "Success" : "Failed") << "\n";

    // Final status
    std::cout << "Final VM Status:\n" << vm.getStatus(userId, resourceId) << "\n";
}

void testDocker() {
    std::cout << "\n=== Testing Docker Functions ===\n";

    // Initialize Docker orchestrator
    Docker docker("nginx", "192.168.1.100");

    // Set configuration
    docker.setPort(8080);
    docker.setEnv("APP_ENV", "production");
    docker.setTimeout(600);

    std::string userId = "bob";
    std::string resourceId = "challenge2";

    // Test deploy
    std::cout << "Deploying Docker for " << userId << "/" << resourceId << "... ";
    bool deployResult = docker.deploy(userId, resourceId);
    std::cout << (deployResult ? "Success" : "Failed") << "\n";

    // Test status
    std::cout << "Docker Status:\n" << docker.getStatus(userId, resourceId) << "\n";

    // Test start
    std::cout << "Starting Docker... ";
    bool startResult = docker.start(userId, resourceId);
    std::cout << (startResult ? "Success" : "Failed") << "\n";

    // Test status again
    std::cout << "Docker Status:\n" << docker.getStatus(userId, resourceId) << "\n";

    // Test logs
    std::cout << "Docker Logs:\n" << docker.getLogs(userId, resourceId) << "\n";

    // Test stop
    std::cout << "Stopping Docker... ";
    bool stopResult = docker.stop(userId, resourceId);
    std::cout << (stopResult ? "Success" : "Failed") << "\n";

    // Test undeploy
    std::cout << "Undeploying Docker... ";
    bool undeployResult = docker.undeploy(userId, resourceId);
    std::cout << (undeployResult ? "Success" : "Failed") << "\n";

    // Final status
    std::cout << "Final Docker Status:\n" << docker.getStatus(userId, resourceId) << "\n";
}


int main() {
	testVM();
	testDocker();
    return 0;
}