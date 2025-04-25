#include "Client.h"
#include "pch.h"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <fstream>
#include "TCPSock.h"
#include "UDPSock.h"
#include "DBConn.h"
#include "AdminConn.h"
#include "download_sock.h"

// Function to print thread-safe messages
void Client::printMessage(const std::string& message) {
	QString qMessage = QString::fromStdString(message);
    ui.plainTextEdit->textCursor().insertText(qMessage +"\n");
}

// Test TCP client functionality
void Client::testTCPClient(const std::string& serverAddress, int port) {
    printMessage("Testing TCP Client to " + serverAddress + ":" + std::to_string(port));

    // Create TCP socket
    TCPSock tcpClient(serverAddress, port);

    // Set socket options
    tcpClient.setBlocking(true);
    tcpClient.setTimeout(5000); // 5 seconds timeout

    // Connect to server
    printMessage("Connecting to TCP server...");
    if (!tcpClient.connect()) {
        printMessage("Failed to connect to TCP server");
        return;
    }

    printMessage("Connected to TCP server at " + tcpClient.getAddress());

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

    // Test TLS functionality
    printMessage("Testing TLS connection...");
    tcpClient.disconnect();


    printMessage("TCP client test completed");
}

// Test UDP client functionality
void Client::testUDPClient(const std::string& serverAddress, int port) {
    printMessage("Testing UDP Client to " + serverAddress + ":" + std::to_string(port));

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
    printMessage("Sending: " + testMessage);

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

    udpClient.disconnect();
    printMessage("UDP client test completed");
}

// Test database client functionality
void Client::testDBClient() {
    printMessage("Testing Database Client");

    // Create database connection
    std::string connStr = "mysql://user:pass@localhost:3306/testdb";
    DBConn dbClient(connStr);

    // Connect to database
    printMessage("Connecting to database...");
    if (!dbClient.connect()) {
        printMessage("Failed to connect to database");
        return;
    }

    printMessage("Connected to database at " + dbClient.getAddress());

    // Execute a sample query
    std::string query = "SELECT * FROM users LIMIT 10";
    printMessage("Executing query: " + query);

    int result = dbClient.send(query);
    if (result < 0) {
        printMessage("Query failed");
    }
    else {
        printMessage("Query executed successfully");
        std::string response = dbClient.receive();
        printMessage("Query result: " + response);
    }

    // Test SQL injection prevention
    printMessage("Testing SQL injection prevention...");
    std::string maliciousQuery = "SELECT * FROM users WHERE id = 1 OR 1=1";
    result = dbClient.send(maliciousQuery);
    if (result < 0) {
        printMessage("SQL injection correctly prevented");
    }
    else {
        printMessage("Warning: SQL injection not caught");
    }

    // Test admin connection
    printMessage("Testing admin database connection...");
    AdminConn adminClient("mysql://admin:adminpass@localhost:3306/testdb");
    adminClient.setAdminKey("SuperSecretAdminKey");

    if (!adminClient.connect()) {
        printMessage("Failed to connect with admin credentials");
    }
    else {
        printMessage("Admin connection successful");
        printMessage("Admin privilege level: " + std::to_string(adminClient.getPrivilegeLevel()));

        // Test admin privilege verification
        if (adminClient.verifyPrivilege(1)) {
            printMessage("Admin has sufficient privileges for level 1 operations");
        }

        if (adminClient.verifyPrivilege(2)) {
            printMessage("Admin has sufficient privileges for level 2 operations");
            adminClient.disableSecureMode();
            printMessage("Secure mode disabled: " + std::string(adminClient.isSecureModeEnabled() ? "true" : "false"));
        }

        adminClient.disconnect();
    }

    dbClient.disconnect();
    printMessage("Database client test completed");
}

// Test download functionality
void Client::testDownload(const std::string& serverAddress, int port) {
    printMessage("Testing Download Client");

    // Create download socket
    download_sock downloader(serverAddress, port);

    // Test file download
    std::string testUrl = "https://" + serverAddress + ":" + std::to_string(port) + "/testfile.txt";
    std::string destination = "downloaded_testfile.txt";

    printMessage("Starting download from " + testUrl);
    printMessage("Downloading to " + destination);

    if (!downloader.downloadFile(testUrl, destination)) {
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

        // Simulate cancelling and restarting download halfway
        if (currentProgress >= 50 && currentProgress < 55 && lastProgress < 50) {
            printMessage("Testing download cancellation at 50%");
            downloader.cancelDownload();

            // Wait a moment
            std::this_thread::sleep_for(std::chrono::seconds(1));

            // Restart download
            printMessage("Restarting download after cancellation");
            if (!downloader.downloadFile(testUrl, destination)) {
                printMessage("Failed to restart download");
                return;
            }
            lastProgress = -1;  // Reset progress tracking
        }
    }

    printMessage("Download complete!");
    printMessage("File size: " + std::to_string(downloader.getFileSize()) + " bytes");

    // Check if file exists
    std::ifstream downloadedFile(destination);
    if (downloadedFile.good()) {
        printMessage("File successfully saved to " + destination);
        downloadedFile.close();
    }
    else {
        printMessage("Error: Downloaded file not found or cannot be opened");
    }

    // Test download priority
    printMessage("Testing download priority setting");
    downloader.setPriority(2);  // Higher priority

    // Test another download with higher priority
    std::string secondUrl = "https://" + serverAddress + ":" + std::to_string(port) + "/largefile.dat";
    std::string secondDestination = "downloaded_largefile.dat";

    printMessage("Starting high-priority download from " + secondUrl);
    if (!downloader.downloadFile(secondUrl, secondDestination)) {
        printMessage("Failed to start second download");
    }
    else {
        // Just demonstrate starting the download, then cancel it
        std::this_thread::sleep_for(std::chrono::seconds(2));
        printMessage("Cancelling second download for demonstration");
        downloader.cancelDownload();
    }

    printMessage("Download client test completed");
}

Client::Client(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

Client::~Client()
{}

void Client::on_pushButton_clicked()
{
	ui.plainTextEdit->clear();
    printMessage("Refreshing...");
	ui.plainTextEdit->update();
    ui.plainTextEdit->repaint();
    ui.plainTextEdit->clear();

    // Test settings
    std::string serverAddress = "127.0.0.1";
    int tcpPort = 1337;
    int udpPort = 8081;
    int httpPort = 8082;

    // Run tests
    testTCPClient(serverAddress, tcpPort);
    std::this_thread::sleep_for(std::chrono::seconds(1));

  //  testUDPClient(serverAddress, udpPort);
 //   std::this_thread::sleep_for(std::chrono::seconds(1));

  //  testDBClient();
  //  std::this_thread::sleep_for(std::chrono::seconds(1));

   // testDownload(serverAddress, httpPort);

    printMessage("All client tests completed");

	ui.plainTextEdit->textCursor().insertText("Button clicked!\n");
}

