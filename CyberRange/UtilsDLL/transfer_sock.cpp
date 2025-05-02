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

bool transfer_sock::connect() {
    return TCPSock::connect();
}

bool transfer_sock::disconnect() {
    return TCPSock::disconnect();
}

bool transfer_sock::isConnected() const {
    return TCPSock::isConnected();
}

int transfer_sock::send(const std::string& data) {
    return TCPSock::send(data);
}

std::string transfer_sock::receive() {
    return TCPSock::receive();
}

std::string transfer_sock::getAddress() const {
    return TCPSock::getAddress();
}

int transfer_sock::getPort() const {
    return TCPSock::getPort();
}

bool transfer_sock::enableTLS() {
    return TCPSock::enableTLS();
}

bool transfer_sock::isTLSEnabled() const {
    return TCPSock::isTLSEnabled();
}

void transfer_sock::setTimeout(int ms) {
    TCPSock::setTimeout(ms);
}

int transfer_sock::getTimeout() const {
    return TCPSock::getTimeout();
}

bool transfer_sock::bind(int port) {
    return TCPSock::bind(port);
}

bool transfer_sock::listen(int backlog) {
    return TCPSock::listen(backlog);
}

Connection* transfer_sock::accept() {
    return TCPSock::accept();
}

bool transfer_sock::startListening(std::function<void(const std::string&, Connection*)> handler) {
    if (thisServerRunning) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }
    try {
        std::filesystem::create_directories(rootDirectory);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to create root directory: " << e.what() << std::endl;
        return false;
    }
    if (!bind(getPort())) {
        std::cerr << "Failed to bind server socket" << std::endl;
        return false;
    }
    if (!listen()) {
        std::cerr << "Failed to start listening" << std::endl;
        return false;
    }
    requestHandler = handler;
    thisServerRunning = true;
    std::thread serverThread([this]() {
        std::cout << "Transfer server started on port " << getPort() << std::endl;
        std::cout << "Using root directory: " << rootDirectory << std::endl;
        while (thisServerRunning) {
            Connection* conn = accept();
            if (!conn) {
                if (!thisServerRunning) {
                    break;
                }
                std::cerr << "Failed to accept client connection" << std::endl;
                continue;
            }
            std::thread clientThread(&transfer_sock::handleClientRequest, this, std::unique_ptr<TCPSock>(dynamic_cast<TCPSock*>(conn)));
            clientThread.detach();
        }
        std::cout << "Transfer server stopped" << std::endl;
        });
    serverThread.detach();
    return true;
}

void transfer_sock::stopServer() {
    if (thisServerRunning) {
        thisServerRunning = false;
        disconnect();
    }
}

bool transfer_sock::isServerRunning() const {
    return thisServerRunning;
}

void transfer_sock::setCertificates(const std::string& cert, const std::string& key) {
    TCPSock::setCertificates(cert, key);
}

void transfer_sock::handleRequest(const std::string& data, Connection* client) {
    if (!client) {
        std::cerr << "Invalid client for handling request" << std::endl;
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
        handleDownloadRequest(dynamic_cast<transfer_sock*>(client), remotePath);
    }
    else if (command == "UPLOAD" && tokens.size() >= 2) {
        std::string remotePath = tokens[1];
        std::string sizeStr = receiveLine();
        long fileSize = 0;
        try {
            fileSize = std::stol(sizeStr);
        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing file size: " << e.what() << std::endl;
            client->send("ERROR Invalid file size\n");
            return;
        }
        handleUploadRequest(dynamic_cast<transfer_sock*>(client), remotePath, fileSize);
    }
    else {
        client->send("ERROR Unknown command\n");
    }
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
        std::cerr << "Cannot receive: not connected" << std::endl;
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
    std::string data(buffer, size);
    return TCPSock::send(data);
}

void transfer_sock::handleClientRequest(std::unique_ptr<TCPSock> clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        std::cerr << "Invalid client connection" << std::endl;
        return;
    }
    std::cout << "New client connected: " << clientSock->getAddress() << ":" << clientSock->getPort() << std::endl;
    try {
        std::string request = clientSock->receive();
        if (requestHandler) {
            requestHandler(request, clientSock.get());
        }
        else {
            handleRequest(request, clientSock.get());
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception handling client request: " << e.what() << std::endl;
    }
}

bool transfer_sock::handleDownloadRequest(transfer_sock* clientSock, const std::string& remotePath) {
    std::string fullPath = normalizePath(remotePath);
    std::string filePath = rootDirectory + "/" + fullPath;
    std::cout << "Download request for: " << fullPath << std::endl;
    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath)) {
        std::cerr << "File not found: " << filePath << std::endl;
        clientSock->send("ERROR File not found\n");
        return false;
    }
    try {
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
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
                std::cerr << "Error sending file data" << std::endl;
                break;
            }
            bytesSent += bytesRead;
            if (bytesSent % (BUFFER_SIZE * 50) == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        inFile.close();
        std::cout << "Download complete: " << fullPath << " (" << bytesSent << " bytes)" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during download: " << e.what() << std::endl;
        clientSock->send("ERROR Server error\n");
        return false;
    }
}

bool transfer_sock::handleUploadRequest(transfer_sock* clientSock, const std::string& remotePath, long fileSize) {
    std::string fullPath = normalizePath(remotePath);
    std::string filePath = rootDirectory + "/" + fullPath;
    std::cout << "Upload request for: " << fullPath << " (" << fileSize << " bytes)" << std::endl;
    try {
        std::filesystem::path destinationPath(filePath);
        std::filesystem::create_directories(destinationPath.parent_path());
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            clientSock->send("ERROR Failed to create file\n");
            return false;
        }
        clientSock->send("OK\n");
        char buffer[BUFFER_SIZE];
        long bytesReceived = 0;
        while (!aborted && bytesReceived < fileSize) {
            int bytesToRead = static_cast<int>(min(static_cast<long>(BUFFER_SIZE),
                fileSize - bytesReceived));
            int received = clientSock->receiveRaw(buffer, bytesToRead);
            if (received <= 0) {
                std::cerr << "Error receiving data or connection closed" << std::endl;
                break;
            }
            outFile.write(buffer, received);
            bytesReceived += received;
            if (bytesReceived % (BUFFER_SIZE * 50) == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        outFile.close();
        if (bytesReceived == fileSize) {
            clientSock->send("COMPLETE\n");
            std::cout << "Upload complete: " << fullPath << " (" << bytesReceived << " bytes)" << std::endl;
            return true;
        }
        else {
            clientSock->send("ERROR Incomplete transfer\n");
            std::filesystem::remove(filePath);
            std::cerr << "Upload incomplete: " << fullPath << " (" << bytesReceived << "/" << fileSize << " bytes)" << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during upload: " << e.what() << std::endl;
        clientSock->send("ERROR Server error\n");
        return false;
    }
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