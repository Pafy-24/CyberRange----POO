#include "pch.h"
#include "Client.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include "TCPSock.h"
#include "UDPSock.h"
#include "transfer_sock.h"

// Thread-safe UI message printing
void Client::printMessage(const std::string& message) {
    QString qMessage = QString::fromStdString(message);
    QMetaObject::invokeMethod(this, [this, qMessage]() {
        ui.plainTextEdit->appendPlainText(qMessage);
        ui.plainTextEdit->ensureCursorVisible();
        }, Qt::QueuedConnection);
    QApplication::processEvents();
}

// Test TCP client (standard and secure)
void Client::testTCPClient(const std::string& serverAddress, int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    printMessage("\n----- Testing " + serverType + " Client -----");
    printMessage("Connecting to " + serverAddress + ":" + std::to_string(port));

    TCPSock client(serverAddress, port);
    client.setTimeout(5000);

    if (useTLS && !client.enableTLS()) {
        printMessage("Failed to enable TLS");
        return;
    }

    if (!client.connect()) {
        printMessage("Failed to connect to " + serverType + " server");
        return;
    }

    printMessage("Connected to " + serverType + " server at " + client.getAddress() + ":" +
        std::to_string(client.getPort()));

    std::string testMessage = "Hello from " + serverType + " client!";
    printMessage("Sending: " + testMessage);

    if (client.send(testMessage) < 0) {
        printMessage("Failed to send message");
        client.disconnect();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::string response = client.receive();
    printMessage(response.empty() ? "No response received" : "Received: " + response);

    client.disconnect();
    printMessage(serverType + " client test completed");
}

// Test UDP client
void Client::testUDPClient(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing UDP Client -----");
    printMessage("Setting up UDP client for " + serverAddress + ":" + std::to_string(port));

    UDPSock client(serverAddress, port);
    if (!client.connect()) {
        printMessage("Failed to set up UDP client");
        return;
    }

    std::string testMessage = "Hello from UDP client!";
    printMessage("Sending: " + testMessage);

    if (client.send(testMessage) < 0) {
        printMessage("Failed to send UDP message");
        client.disconnect();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::string response = client.receive();
    printMessage(response.empty() ? "No UDP response received" : "Received: " + response);

    printMessage("Testing UDP broadcast...");
    if (client.broadcast("Broadcast message")) {
        printMessage("Broadcast sent");
    }
    else {
        printMessage("Failed to send broadcast");
    }

    std::string sender;
    std::string fromResponse = client.receiveFrom(sender);
    if (!fromResponse.empty()) {
        printMessage("Received from " + sender + ": " + fromResponse);
    }

    client.disconnect();
    printMessage("UDP client test completed");
}

// Test download functionality
void Client::testDownload(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing Download Client -----");
    printMessage("Setting up download from " + serverAddress + ":" + std::to_string(port));

    transfer_sock conn(serverAddress, port);
    if (!conn.connect()) {
        printMessage("Failed to connect download socket");
        return;
    }

    // Set progress and completion callbacks
    conn.setProgressCallback([this](int progress, const std::string& filePath, long fileSize) {
        printMessage("Download progress for " + filePath + ": " + std::to_string(progress) + "%");
        });

    conn.setCompleteCallback([this](bool success, const std::string& filePath, TransferType type) {
        printMessage(success ? "Download completed: " + filePath : "Download failed: " + filePath);
        });

    // Test small file download
    std::string smallFile = "server.key";
    std::string smallFileDest = "E:\\Users\\xxsho\\Documents\\downloaded_small.dat";
    printMessage("Starting download: " + smallFile + " to " + smallFileDest);

    if (!conn.downloadFile(smallFile, smallFileDest)) {
        printMessage("Failed to start small file download");
        return;
    }

    while (conn.getCurrentTransferType() != TransferType::NONE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Verify downloaded file
    std::ifstream check(smallFileDest, std::ios::binary);
    if (check.good()) {
        check.seekg(0, std::ios::end);
        printMessage("Small file size: " + std::to_string(check.tellg()) + " bytes");
        check.close();
    }
    else {
        printMessage("Error: Small file not found");
    }

    // Test large file download with cancellation
    std::string largeFile = "server.crt";
    std::string largeFileDest = "downloaded_large.dat";
    conn.setPriority(2);
    printMessage("\nStarting large file download: " + largeFile + " to " + largeFileDest + " (Priority: 2)");

    if (!conn.downloadFile(largeFile, largeFileDest)) {
        printMessage("Failed to start large file download");
        return;
    }

    int progress = 0;
    while (conn.getCurrentTransferType() != TransferType::NONE && progress < 50) {
        progress = conn.getProgress();
        if (progress >= 50) {
            printMessage("Cancelling download at " + std::to_string(progress) + "%");
            conn.cancelTransfer();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Restart large file download
    printMessage("\nRestarting large file download");
    conn.disconnect();
    if (!conn.connect()) {
        printMessage("Failed to reconnect for large file download");
        return;
    }

    if (!conn.downloadFile(largeFile, largeFileDest)) {
        printMessage("Failed to restart large file download");
        return;
    }

    while (conn.getCurrentTransferType() != TransferType::NONE) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Verify downloaded file
    check.open(largeFileDest, std::ios::binary);
    if (check.good()) {
        check.seekg(0, std::ios::end);
        printMessage("Large file size: " + std::to_string(check.tellg()) + " bytes");
        check.close();
    }
    else {
        printMessage("Error: Large file not found");
    }

    printMessage("Download tests completed");
}

Client::Client(QWidget* parent) : QMainWindow(parent) {
    ui.setupUi(this);
}

Client::~Client() {}

void Client::on_pushButton_clicked() {
    ui.plainTextEdit->clear();
    printMessage("Running network tests...");

    std::string serverAddress = "127.0.0.1";
    int tcpPort = 1337;
    int secureTcpPort = 1338;
    int udpPort = 8081;
    int downloadPort = 8082;

  //  testTCPClient(serverAddress, tcpPort, false);
 //   testTCPClient(serverAddress, secureTcpPort, true);
 //   testUDPClient(serverAddress, udpPort);
    testDownload(serverAddress, downloadPort);

    printMessage("\nAll client tests completed");
}