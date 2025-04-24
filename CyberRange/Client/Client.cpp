#include "Client.h"

#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include "TCPSock.h"
#include "UDPSock.h"
#include "download_sock.h"

// Function to print thread-safe messages  
void Client::printMessage(const std::string& message) {  
   QString qMessage = QString::fromStdString(message);
   QMetaObject::invokeMethod(this, [this, qMessage]() {
       ui.plainTextEdit->appendPlainText(qMessage);
       ui.plainTextEdit->ensureCursorVisible();
       }, Qt::QueuedConnection);
   QApplication::processEvents(); 
   ui.plainTextEdit->update();
   ui.plainTextEdit->repaint();
}

// Test standard TCP client functionality
void Client::testTCPClient(const std::string& serverAddress, int port) {
    printMessage("----- Testing Standard TCP Client -----");
    printMessage("Connecting to " + serverAddress + ":" + std::to_string(port));

    // Create TCP socket
    TCPSock tcpClient(serverAddress, port);

    // Set socket options
    tcpClient.setBlocking(true);
    tcpClient.setTimeout(5000); // 5 seconds timeout

    // Connect to server
    if (!tcpClient.connect()) {
        printMessage("Failed to connect to TCP server");
        return;
    }

    printMessage("Connected to TCP server at " + tcpClient.getAddress() + ":" + std::to_string(tcpClient.getPort()));

    // Send test message
    std::string testMessage = "Hello from TCP client!";
    printMessage("Sending: " + testMessage);

    if (tcpClient.send(testMessage) < 0) {
        printMessage("Failed to send message");
        tcpClient.disconnect();
        return;
    }

    // Wait for response
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string response = tcpClient.receive();

    if (!response.empty()) {
        printMessage("Received response: " + response);
    }
    else {
        printMessage("No response received");
    }

    // Disconnect
    tcpClient.disconnect();
    printMessage("TCP client test completed");
}

// Test secure TCP client with TLS functionality
void Client::testSecureTCPClient(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing Secure TCP Client (TLS) -----");
    printMessage("Connecting to " + serverAddress + ":" + std::to_string(port));

    // Create TCP socket
    TCPSock tcpClient(serverAddress, port);

    // Set socket options
    tcpClient.setBlocking(true);
    tcpClient.setTimeout(5000); // 5 seconds timeout

    // Enable TLS before connecting
    if (!tcpClient.enableTLS()) {
        printMessage("Failed to enable TLS");
        return;
    }

    printMessage("TLS enabled: " + std::string(tcpClient.isTLSEnabled() ? "true" : "false"));

    // Connect to secure server
    if (!tcpClient.connect()) {
        printMessage("Failed to connect to secure TCP server");
        return;
    }

    printMessage("Connected securely to TCP server at " + tcpClient.getAddress() + ":" +
        std::to_string(tcpClient.getPort()));

    // Send test message over secure connection
    std::string testMessage = "Hello from secure TCP client!";
    printMessage("Sending encrypted: " + testMessage);

    if (tcpClient.send(testMessage) < 0) {
        printMessage("Failed to send secure message");
        tcpClient.disconnect();
        return;
    }

    // Wait for response
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string response = tcpClient.receive();

    if (!response.empty()) {
        printMessage("Received encrypted response: " + response);
    }
    else {
        printMessage("No response received");
    }

    // Disconnect
    tcpClient.disconnect();
    printMessage("Secure TCP client test completed");
}

// Test UDP client functionality
void Client::testUDPClient(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing UDP Client -----");
    printMessage("Setting up UDP client for " + serverAddress + ":" + std::to_string(port));

    // Create UDP socket
    UDPSock udpClient(serverAddress, port);

    // Connect to server (sets destination for send)
    if (!udpClient.connect()) {
        printMessage("Failed to set up UDP client");
        return;
    }

    printMessage("UDP client ready");

    // Send test message
    std::string testMessage = "Hello from UDP client!";
    printMessage("Sending UDP: " + testMessage);

    if (udpClient.send(testMessage) < 0) {
        printMessage("Failed to send UDP message");
        udpClient.disconnect();
        return;
    }

    // Wait for response
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::string response = udpClient.receive();

    if (!response.empty()) {
        printMessage("Received UDP response: " + response);
    }
    else {
        printMessage("No UDP response received");
    }

    // Test broadcast functionality
    printMessage("Testing UDP broadcast...");
    if (udpClient.broadcast("Broadcast message to all")) {
        printMessage("Broadcast sent");
    }
    else {
        printMessage("Failed to send broadcast");
    }

    // Test receiving from specific sender
    std::string sender;
    std::string fromResponse = udpClient.receiveFrom(sender);
    if (!fromResponse.empty()) {
        printMessage("Received UDP message from " + sender + ": " + fromResponse);
    }

    udpClient.disconnect();
    printMessage("UDP client test completed");
}

// Test download functionality
void Client::testDownload(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing Download Client -----");
    printMessage("Setting up download from " + serverAddress + ":" + std::to_string(port));

    // Create download socket
    download_sock downloader(serverAddress, port);

    // Test first download - small file
    std::string smallFile = "test_small.dat";
    std::string smallFileDest = "downloaded_small.dat";

    printMessage("Starting small file download test");

    // Create custom URL for our protocol
    std::string smallFileUrl = serverAddress + ":" + std::to_string(port) + "/" + smallFile;

    // Connect the downloader first
    if (!downloader.connect()) {
        printMessage("Failed to connect download socket");
        return;
    }

    // Send GET request directly before starting the download
    std::string getRequest = "GET " + smallFile;
    if (downloader.send(getRequest) < 0) {
        printMessage("Failed to send download request");
        return;
    }

    printMessage("Starting download for: " + smallFile);
    printMessage("Saving to: " + smallFileDest);

    if (!downloader.downloadFile(smallFileUrl, smallFileDest)) {
        printMessage("Failed to start download");
        return;
    }

    // Monitor download progress
    int lastProgress = -1;
    while (downloader.getProgress() < 100) {
        int currentProgress = downloader.getProgress();
        if (currentProgress != lastProgress) {
            printMessage("Download progress: " + std::to_string(currentProgress) + "%");
            lastProgress = currentProgress;
        }

        // Small delay to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    printMessage("Small file download complete!");
    printMessage("File size: " + std::to_string(downloader.getFileSize()) + " bytes");

    // Verify the downloaded file exists
    std::ifstream smallFileCheck(smallFileDest, std::ios::binary);
    if (smallFileCheck.good()) {
        // Get file size
        smallFileCheck.seekg(0, std::ios::end);
        std::streamsize fileSize = smallFileCheck.tellg();
        smallFileCheck.close();
        printMessage("File successfully saved with size: " + std::to_string(fileSize) + " bytes");
    }
    else {
        printMessage("Error: Downloaded file not found or cannot be opened");
    }

    // Test second download - large file with cancellation
    std::string largeFile = "test_large.dat";
    std::string largeFileDest = "downloaded_large.dat";

    printMessage("\nStarting large file download test with cancellation");

    // Set priority for this download
    downloader.setPriority(2);  // Higher priority
    printMessage("Set download priority to 2");

    // Create custom URL for our protocol
    std::string largeFileUrl = serverAddress + ":" + std::to_string(port) + "/" + largeFile;

    // Disconnect and reconnect to ensure clean state
    downloader.disconnect();
    downloader.connect();

    // Send GET request
    getRequest = "GET " + largeFile;
    downloader.send(getRequest);

    printMessage("Starting download for: " + largeFile);
    printMessage("Saving to: " + largeFileDest);

    if (!downloader.downloadFile(largeFileUrl, largeFileDest)) {
        printMessage("Failed to start download");
        return;
    }

    // Monitor download progress and cancel at 50%
    lastProgress = -1;
    while (downloader.getProgress() < 100) {
        int currentProgress = downloader.getProgress();
        if (currentProgress != lastProgress) {
            printMessage("Download progress: " + std::to_string(currentProgress) + "%");
            lastProgress = currentProgress;

            // Cancel at approximately 50%
            if (currentProgress >= 50 && currentProgress < 55) {
                printMessage("Cancelling download at " + std::to_string(currentProgress) + "%");
                downloader.cancelDownload();
                break;
            }
        }

        // Small delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    printMessage("Download cancelled successfully");

    // Restart the download
    printMessage("\nRestarting the large file download");

    // Reconnect
    downloader.disconnect();
    downloader.connect();

    // Send GET request again
    downloader.send(getRequest);

    if (!downloader.downloadFile(largeFileUrl, largeFileDest)) {
        printMessage("Failed to restart download");
        return;
    }

    // Monitor the restarted download progress to completion
    lastProgress = -1;
    while (downloader.getProgress() < 100) {
        int currentProgress = downloader.getProgress();
        if (currentProgress != lastProgress && currentProgress % 10 == 0) { // Show every 10%
            printMessage("Restarted download progress: " + std::to_string(currentProgress) + "%");
            lastProgress = currentProgress;
        }

        // Small delay
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    printMessage("Download complete!");
    printMessage("File size: " + std::to_string(downloader.getFileSize()) + " bytes");

    // Verify the downloaded file exists
    std::ifstream largeFileCheck(largeFileDest, std::ios::binary);
    if (largeFileCheck.good()) {
        // Get file size
        largeFileCheck.seekg(0, std::ios::end);
        std::streamsize fileSize = largeFileCheck.tellg();
        largeFileCheck.close();
        printMessage("Large file successfully saved with size: " + std::to_string(fileSize) + " bytes");
    }
    else {
        printMessage("Error: Downloaded large file not found or cannot be opened");
    }

    downloader.disconnect();
    printMessage("Download client tests completed");
}

Client::Client(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

Client::~Client()
{
}

void Client::on_pushButton_clicked()
{
    ui.plainTextEdit->clear();
    printMessage("Running network tests...");

    // Test settings
    std::string serverAddress = "127.0.0.1";
    int tcpPort = 1337;        // Standard TCP
    int secureTcpPort = 1338;  // Secure TCP with TLS
    int udpPort = 8081;        // UDP
    int downloadPort = 8082;   // Download server

    // Run tests in separate threads
    std::thread tcpThread([this, serverAddress, tcpPort]() {
        testTCPClient(serverAddress, tcpPort);
    });

    std::thread secureTcpThread([this, serverAddress, secureTcpPort]() {
        testSecureTCPClient(serverAddress, secureTcpPort);
    });

    std::thread udpThread([this, serverAddress, udpPort]() {
        testUDPClient(serverAddress, udpPort);
    });

//    std::thread downloadThread([this, serverAddress, downloadPort]() {
 //       testDownload(serverAddress, downloadPort);
 //   });

    // Wait for all threads to complete
   
    tcpThread.join();
    secureTcpThread.join();
    udpThread.join();
  //  downloadThread.join();

    printMessage("\nAll client tests completed");
}