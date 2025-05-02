#include "Client.h"
#include "ConnsFactory.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>

Client::Client(QWidget* parent) : QMainWindow(parent) {
    ui.setupUi(this);
}

Client::~Client() {}

void Client::printMessage(const std::string& message) {
    QString qMessage = QString::fromStdString(message);
    QMetaObject::invokeMethod(this, [this, qMessage]() {
        ui.plainTextEdit->appendPlainText(qMessage);
        ui.plainTextEdit->ensureCursorVisible();
        }, Qt::QueuedConnection);
    QApplication::processEvents();
}

void Client::testTCPClient(const std::string& serverAddress, int port, bool useTLS) {
    std::string serverType = useTLS ? "Secure TCP" : "Standard TCP";
    printMessage("\n----- Testing " + serverType + " Client -----");
    printMessage("Connecting to " + serverAddress + ":" + std::to_string(port));

    auto client = ConnsFactory::createConnection(ConnectionType::TCP, serverAddress, port);
    if (!client) {
        printMessage("Failed to create TCP client");
        return;
    }

    client->setTimeout(5000);

    if (useTLS && !client->enableTLS()) {
        printMessage("Failed to enable TLS");
        return;
    }

    if (!client->connect()) {
        printMessage("Failed to connect to " + serverType + " server");
        return;
    }

    printMessage("Connected to " + serverType + " server at " + client->getAddress() + ":" +
        std::to_string(client->getPort()));

    std::string testMessage = "Hello from " + serverType + " client!";
    printMessage("Sending: " + testMessage);

    if (client->send(testMessage) < 0) {
        printMessage("Failed to send message");
        client->disconnect();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::string response = client->receive();
    printMessage(response.empty() ? "No response received" : "Received: " + response);

    client->disconnect();
    printMessage(serverType + " client test completed");
}

void Client::testUDPClient(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing UDP Client -----");
    printMessage("Setting up UDP client for " + serverAddress + ":" + std::to_string(port));

    auto client = ConnsFactory::createConnection(ConnectionType::UDP, serverAddress, port);
    if (!client) {
        printMessage("Failed to create UDP client");
        return;
    }

    if (!client->connect()) {
        printMessage("Failed to set up UDP client");
        return;
    }

    std::string testMessage = "Hello from UDP client!";
    printMessage("Sending: " + testMessage);

    if (client->send(testMessage) < 0) {
        printMessage("Failed to send UDP message");
        client->disconnect();
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::string response = client->receive();
    printMessage(response.empty() ? "No UDP response received" : "Received: " + response);

    client->disconnect();
    printMessage("UDP client test completed");
}

void Client::testDownload(const std::string& serverAddress, int port) {
    printMessage("\n----- Testing Download Client -----");
    printMessage("Setting up download from " + serverAddress + ":" + std::to_string(port));

    auto conn = ConnsFactory::createConnection(ConnectionType::TRANSFER, serverAddress, port);
    if (!conn) {
        printMessage("Failed to create download client");
        return;
    }

    if (!conn->connect()) {
        printMessage("Failed to connect download socket");
        return;
    }

    auto downloadFile = [&](const std::string& remoteFile, const std::string& localFile) {
        printMessage("Starting download: " + remoteFile + " to " + localFile);

        // Send download request
        std::string request = "DOWNLOAD " + remoteFile;
        if (conn->send(request) < 0) {
            printMessage("Failed to send download request");
            return false;
        }

        // Receive response (OK or ERROR)
        std::string response = conn->receive();
        if (response.empty() || response.find("OK") != 0) {
            printMessage("Server error: " + (response.empty() ? "No response" : response));
            return false;
        }

        // Parse file size from OK response (format: "OK\n<size>\n")
        size_t pos = response.find('\n');
        if (pos == std::string::npos) {
            printMessage("Invalid server response format");
            return false;
        }
        std::string sizeStr = response.substr(pos + 1);
        long fileSize = 0;
        try {
            fileSize = std::stol(sizeStr);
        }
        catch (const std::exception& e) {
            printMessage("Error parsing file size: " + std::string(e.what()));
            return false;
        }

        // Open local file for writing
        std::ofstream outFile(localFile, std::ios::binary);
        if (!outFile.is_open()) {
            printMessage("Failed to open local file: " + localFile);
            return false;
        }

        // Receive file data
        long bytesReceived = 0;
        char buffer[8192];
        while (bytesReceived < fileSize) {
            int bytesToRead = std::min(static_cast<long>(sizeof(buffer)), fileSize - bytesReceived);
            std::string data = conn->receive();
            if (data.empty()) {
                printMessage("Connection closed or no data received");
                outFile.close();
                std::filesystem::remove(localFile);
                return false;
            }
            outFile.write(data.data(), data.size());
            bytesReceived += data.size();
            int progress = static_cast<int>(bytesReceived * 100 / fileSize);
            printMessage("Download progress for " + remoteFile + ": " + std::to_string(progress) + "%");
        }

        outFile.close();
        printMessage("Download completed: " + localFile + " (" + std::to_string(bytesReceived) + " bytes)");
        return true;
        };

    // Test small file download
    std::string smallFile = "server.key";
    std::string smallFileDest = "downloaded_small.dat";
    downloadFile(smallFile, smallFileDest);

    // Test large file download
    std::string largeFile = "server.crt";
    std::string largeFileDest = "downloaded_large.dat";
    downloadFile(largeFile, largeFileDest);

    conn->disconnect();
    printMessage("Download tests completed");
}

void Client::on_pushButton_clicked() {
    ui.plainTextEdit->clear();
    printMessage("Running network tests...");

    std::string serverAddress = "127.0.0.1";
    int tcpPort = 1337;
    int secureTcpPort = 1338;
    int udpPort = 1339;
    int downloadPort = 1340;

    // Run tests in a separate thread to avoid blocking the UI
    std::thread testThread([this, serverAddress, tcpPort, secureTcpPort, udpPort, downloadPort]() {
        testTCPClient(serverAddress, tcpPort, false);
        testTCPClient(serverAddress, secureTcpPort, true);
        testUDPClient(serverAddress, udpPort);
        testDownload(serverAddress, downloadPort);
        printMessage("\nAll client tests completed");
        });
    testThread.detach();
}