#include "pch.h"
#include "transfer_sock.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <filesystem>

static std::mutex transferMutex;
static std::condition_variable transferCV;
static std::atomic<bool> transferRunning = false;

constexpr size_t BUFFER_SIZE = 8192;

transfer_sock::transfer_sock(const std::string& addr, int port)
    : TCPSock(addr, port),
    transferProgress(0),
    filePath(""),
    fileSize(0),
    aborted(false),
    thisServerRunning(false),
    rootDirectory("./files") {
}

transfer_sock::transfer_sock(int port)
    : TCPSock(port),
    transferProgress(0),
    filePath(""),
    fileSize(0),
    aborted(false),
    thisServerRunning(false),
    rootDirectory("./files") {
}

transfer_sock::transfer_sock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort)
    : TCPSock(std::move(sock), clientAddr, clientPort),
    transferProgress(0),
    filePath(""),
    fileSize(0),
    aborted(false),
    thisServerRunning(false),
    rootDirectory("./files") {
}

transfer_sock::~transfer_sock() {
    stopServer();
}

void transfer_sock::setRootDirectory(const std::string& dir) {
    rootDirectory = dir;
    try {
        std::filesystem::create_directories(rootDirectory);
    }
    catch (const std::exception& e) {
        print("Failed to create root directory: " + std::string(e.what()) );
    }
}

std::string transfer_sock::getRootDirectory() const {
    return rootDirectory;
}

bool transfer_sock::startListening(std::function<void(const std::string&, Connection*)> handler) {
    if (thisServerRunning) {
        print("Transfer server is already running" );
        return false;
    }
    try {
        std::filesystem::create_directories(rootDirectory);
    }
    catch (const std::exception& e) {
        print("Failed to create root directory: " +std::string(e.what()));
        return false;
    }
    if (!TCPSock::bind(getPort())) {
        print("Failed to bind transfer server socket" );
        return false;
    }
    if (!TCPSock::listen()) {
        print("Failed to start listening" );
        return false;
    }
    requestHandler = handler;
    thisServerRunning = true;
    std::thread serverThread([this]() {
        print("Transfer server started on port " + getPort() );
        print("Using root directory: " + rootDirectory );

        if (tcpListener) {
            tcpListener->setBlocking(false);
        }

        while (thisServerRunning) {
            auto* conn = dynamic_cast<TCPSock*>(TCPSock::accept());
            if (!conn) {
                if (!thisServerRunning) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            print("New client connection from " + conn->getAddress() + ":"
                + std::to_string(conn->getPort()) );

            try {
                auto* transferConn = dynamic_cast<transfer_sock*>(conn);
                if (transferConn) {
                    std::thread clientThread(&transfer_sock::handleClientRequest, this,
                        std::unique_ptr<TCPSock>(transferConn));
                    clientThread.detach();
                }
                else {
                    auto transferClient = std::make_unique<transfer_sock>(
                        std::unique_ptr<sf::TcpSocket>(dynamic_cast<TcpSocketWithHandle*>(
                            conn->tcpSocket.release())),
                        conn->getAddress(),
                        conn->getPort());

                    transferClient->setRootDirectory(rootDirectory);
                    std::thread clientThread(&transfer_sock::handleClientRequest, this,
                        std::move(transferClient));
                    clientThread.detach();

                }
            }
            catch (const std::exception& e) {
                print("Failed to create client thread: " +std::string(e.what()));
                delete conn;
            }
        }
        print("Transfer server stopped" );
        });
    serverThread.detach();
    return true;
}

void transfer_sock::stopServer() {
    if (thisServerRunning) {
        print("Stopping transfer server..." );
        thisServerRunning = false;
        TCPSock::disconnect();
        print("Transfer server stopped" );
    }
}

bool transfer_sock::isServerRunning() const {
    return thisServerRunning;
}

std::string transfer_sock::receiveLine() {
    std::string line;
    char c;
    while (receiveRaw(&c, 1) > 0) {
        if (c == '\n') {
            break;
        }
        line += c;
    }
    return line;
}

std::string transfer_sock::receive(int maxSize) {
    if (!isConnected()) {
        print("Cannot receive: not connected" );
        return "";
    }
    char* buffer = new char[maxSize + 1];
    int bytesRead = receiveRaw(buffer, maxSize);
    if (bytesRead <= 0) {
        delete[] buffer;
        return "";
    }
    buffer[bytesRead] = '\0';
    std::string result(buffer, bytesRead);
    delete[] buffer;
    return result;
}

int transfer_sock::receiveRaw(char* buffer, int size) {
    if (!isConnected()) {
        return -1;
    }
    std::string data = TCPSock::receive();
    if (data.empty()) {
        return 0;
    }
    int bytesToCopy = min(size, static_cast<int>(data.length()));
    std::copy(data.begin(), data.begin() + bytesToCopy, buffer);
    return bytesToCopy;
}

int transfer_sock::sendRaw(const std::string& buffer, int size) {
    if (!isConnected()) {
        return -1;
    }
    std::string data(buffer.c_str(), size);
    return TCPSock::send(data);
}

void transfer_sock::handleRequest(const std::string& data, Connection* client) {
    if (!client) {
        print("Invalid client for handling request" );
        return;
    }

    auto* transferClient = dynamic_cast<transfer_sock*>(client);
    if (!transferClient) {
        print("Client is not a transfer_sock instance" );
        client->send("ERROR Invalid client type\n");
        return;
    }

    std::vector<std::string> tokens = parseCommand(data);
    if (tokens.empty()) {
        client->send("ERROR Invalid request\n");
        return;
    }

    std::string command = tokens[0];
    if (command == "DOWNLOAD" && tokens.size() >= 2) {
        std::string remotePath = tokens[1];
        handleDownloadRequest(transferClient, remotePath);
    }
    else if (command == "UPLOAD" && tokens.size() >= 2) {
        std::string remotePath = tokens[1];
        client->send("OK\n");
        std::string sizeStr = transferClient->receive(64);
        client->send("OK\n");
        long fileSize = 0;
        try {
            fileSize = std::stol(sizeStr);
        }
        catch (const std::exception& e) {
            print("Error parsing file size: " +std::string(e.what()));
            client->send("ERROR Invalid file size\n");
            return;
        }
        client->send("OK\n");
        handleUploadRequest(transferClient, remotePath, fileSize);
    }
    else {
        client->send("ERROR Unknown command\n");
    }
}

void transfer_sock::handleClientRequest(std::unique_ptr<TCPSock> clientSock) {
    auto* transferClient = dynamic_cast<transfer_sock*>(clientSock.get());
    if (!transferClient || !transferClient->isConnected()) {
        print("Invalid client connection" );
        return;
    }

    transferClient->setRootDirectory(rootDirectory);

    print("Handling client request from " + transferClient->getAddress()
        + ":" + std::to_string(transferClient->getPort()));

    try {
        transferClient->setTimeout(30000);

        std::string request = transferClient->receive(1024);
        if (request.empty()) {
            print("Empty request received" );
            return;
        }

        if (requestHandler) {
            requestHandler(request, transferClient);
        }
        else {
            handleRequest(request, transferClient);
        }
    }
    catch (const std::exception& e) {
        print("Exception handling client request: " + std::string(e.what()));
    }

    if (transferClient->isConnected()) {
        transferClient->disconnect();
    }

    print("Client connection handled and closed: "
        + transferClient->getAddress() + ":" + std::to_string(transferClient->getPort()) );
}

bool transfer_sock::handleDownloadRequest(transfer_sock* clientSock, const std::string& remotePath) {
    std::string fullPath = normalizePath(remotePath);
    std::string filePath = rootDirectory + "/" + fullPath;
    print("Download request for: " + fullPath );

    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath)) {
        print("File not found: " + filePath );
        clientSock->send("ERROR File not found\n");
        return false;
    }

    try {
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile.is_open()) {
            print("Failed to open file for reading: " + filePath );
            clientSock->send("ERROR Failed to open file\n");
            return false;
        }

        long fileSize = std::filesystem::file_size(filePath);
        clientSock->send("OK\n" + std::to_string(fileSize) + "\n");

        char buffer[BUFFER_SIZE];
        long bytesSent = 0;

        while (!aborted && inFile.good() && bytesSent < fileSize) {
            inFile.read(buffer, BUFFER_SIZE);
            int bytesRead = static_cast<int>(inFile.gcount());
            if (bytesRead <= 0) {
                break;
            }

            if (clientSock->sendRaw(std::string(buffer, bytesRead), bytesRead) < 0) {
                print("Error sending file data" );
                break;
            }

            bytesSent += bytesRead;
            transferProgress = static_cast<int>((bytesSent * 100) / fileSize);

            // Throttle very large transfers to avoid overloading the socket
            if (bytesSent % (BUFFER_SIZE * 50) == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        inFile.close();

        if (bytesSent == fileSize) {
            print("Download complete: " + fullPath + " (" + std::to_string(bytesSent) + " bytes)" );
            return true;
        }
        else {
            print("Download incomplete: " + fullPath + " ("
                + std::to_string(bytesSent) + "/" + std::to_string(fileSize) + " bytes)" );
            return false;
        }
    }
    catch (const std::exception& e) {
        print("Exception during download: " +std::string(e.what()));
        clientSock->send("ERROR Server error\n");
        return false;
    }
}

bool transfer_sock::handleUploadRequest(transfer_sock* clientSock, const std::string& remotePath, long fileSize) {
    std::string fullPath = normalizePath(remotePath);
    std::string filePath = rootDirectory + "/" + fullPath;
    print("Upload request for: " + fullPath + " (" + std::to_string(fileSize) + " bytes)" );

    try {
        std::filesystem::path destinationPath(filePath);
        std::filesystem::create_directories(destinationPath.parent_path());

        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            print("Failed to open file for writing: " + filePath );
            clientSock->send("ERROR Failed to create file\n");
            return false;
        }

        clientSock->send("OK\n");

        this->filePath = filePath;
        this->fileSize = fileSize;
        transferProgress = 0;
        aborted = false;

        char buffer[BUFFER_SIZE];
        long bytesReceived = 0;

        while (!aborted && bytesReceived < fileSize) {
            int bytesToRead = static_cast<int>(min(static_cast<long>(BUFFER_SIZE), fileSize - bytesReceived));

            int received = clientSock->receiveRaw(buffer, bytesToRead);
            if (received <= 0) {
                print("Error receiving data or connection closed" );
                break;
            }

            outFile.write(buffer, received);
            bytesReceived += received;
            transferProgress = static_cast<int>((bytesReceived * 100) / fileSize);

            if (bytesReceived % (BUFFER_SIZE * 50) == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        outFile.close();

        if (bytesReceived == fileSize) {
            clientSock->send("COMPLETE\n");
            print("Upload complete: " + fullPath + " (" + std::to_string(bytesReceived) + " bytes)" );
            return true;
        }
        else {
            clientSock->send("ERROR Incomplete transfer\n");
            std::filesystem::remove(filePath);
            print("Upload incomplete: " + fullPath + " ("
                + std::to_string(bytesReceived) + "/" + std::to_string(fileSize) + " bytes)" );
            return false;
        }
    }
    catch (const std::exception& e) {
        print("Exception during upload: " +std::string(e.what()));
        clientSock->send("ERROR Server error\n");
        return false;
    }
}

int transfer_sock::getTransferProgress() const {
    return transferProgress;
}

void transfer_sock::abortTransfer() {
    aborted = true;
}

std::string transfer_sock::normalizePath(const std::string& path) const {
    std::string result = path;

    while (!result.empty() && (result[0] == '/' || result[0] == '\\')) {
        result.erase(0, 1);
    }

    std::replace(result.begin(), result.end(), '\\', '/');

    std::vector<std::string> parts;
    std::istringstream iss(result);
    std::string part;

    while (std::getline(iss, part, '/')) {
        if (part == "..") {
            if (!parts.empty()) {
                parts.pop_back();
            }
        }
        else if (!part.empty() && part != ".") {
            parts.push_back(part);
        }
    }

    result.clear();
    for (const auto& p : parts) {
        if (!result.empty()) {
            result += "/";
        }
        result += p;
    }

    return result;
}

std::vector<std::string> transfer_sock::parseCommand(const std::string& command) const {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    return tokens;
}