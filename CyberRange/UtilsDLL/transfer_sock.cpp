#include "pch.h"
#include "transfer_sock.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <atomic>
#include <queue>
#include <vector>
#include <algorithm>

// Global variables for managing transfer threads
static std::mutex transferMutex;
static std::condition_variable transferCV;
static std::atomic<bool> downloadRunning = false;
static std::atomic<bool> uploadRunning = false;

// Size of buffer used for transfer
constexpr size_t BUFFER_SIZE = 8192;

// Client constructor
transfer_sock::transfer_sock(const std::string& addr, int port)
    : TCPSock(addr, port),
    transferProgress(0),
    filePath(""),
    fileSize(0),
    aborted(false),
    thisServerRunning(false),
    priority(0),
    currentTransfer(TransferType::NONE),
    rootDirectory("./files") {
}

// Server constructor
transfer_sock::transfer_sock(int port)
    : TCPSock(port),
    transferProgress(0),
    filePath(""),
    thisServerRunning(false),
    fileSize(0),
    aborted(false),
    priority(0),
    currentTransfer(TransferType::NONE),
    rootDirectory("./files") {
}

// Socket created from accept()
transfer_sock::transfer_sock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort)
    : TCPSock(std::move(sock), clientAddr, clientPort),
    transferProgress(0),
    filePath(""),
	thisServerRunning(false),
    fileSize(0),
    aborted(false),
    priority(0),
    currentTransfer(TransferType::NONE),
    rootDirectory("./files") {
}

transfer_sock::~transfer_sock() {
    cancelTransfer();
    stopServer();
}

bool transfer_sock::downloadFile(const std::string& remotePath, const std::string& destination) {
    if (!isConnected()) {
        if (!connect()) {
            std::cerr << "Failed to connect for download" << std::endl;
            return false;
        }
    }

    filePath = destination;
    aborted = false;
    transferProgress = 0;
    currentTransfer = TransferType::DOWNLOAD;

    // Format and send download request command
    std::string request = "DOWNLOAD " + remotePath + "\n";
    if (send(request) < 0) {
        std::cerr << "Failed to send download request" << std::endl;
        currentTransfer = TransferType::NONE;
        return false;
    }

    // Start download in a separate thread
    std::thread downloadThread([this]() {
        std::unique_lock<std::mutex> lock(transferMutex);
        downloadRunning = true;
        lock.unlock();

        try {
            // Create directories if needed
            std::filesystem::path destinationPath(filePath);
            std::filesystem::create_directories(destinationPath.parent_path());

            // Open file for writing
            std::ofstream outFile(filePath, std::ios::binary);
            if (!outFile.is_open()) {
                std::cerr << "Failed to open file for writing: " << filePath << std::endl;
                std::lock_guard<std::mutex> guard(transferMutex);
                downloadRunning = false;
                currentTransfer = TransferType::NONE;
                if (completeCallback) {
                    completeCallback(false, filePath, TransferType::DOWNLOAD);
                }
                transferCV.notify_all();
                return -1;
            }

            // First receive response status
            std::string response = receive(1024);
            if (response != "OK") {
                std::cerr << "Download request failed: " << response << std::endl;
                outFile.close();
                std::filesystem::remove(filePath);
                std::lock_guard<std::mutex> guard(transferMutex);
                downloadRunning = false;
                currentTransfer = TransferType::NONE;
                if (completeCallback) {
                    completeCallback(false, filePath, TransferType::DOWNLOAD);
                }
                transferCV.notify_all();
                return -1;
            }

            // Receive file size
            std::string sizeStr = receiveLine();
            try {
                fileSize = std::stol(sizeStr);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing file size: " << e.what() << std::endl;
                outFile.close();
                std::filesystem::remove(filePath);
                std::lock_guard<std::mutex> guard(transferMutex);
                downloadRunning = false;
                currentTransfer = TransferType::NONE;
                if (completeCallback) {
                    completeCallback(false, filePath, TransferType::DOWNLOAD);
                }
                transferCV.notify_all();
                return -1;
            }

            // Start receiving file data
            long bytesReceived = 0;
            char buffer[BUFFER_SIZE];

            while (!aborted && bytesReceived < fileSize) {
                int bytesToRead = static_cast<int>(min(static_cast<long>(BUFFER_SIZE),
                    fileSize - bytesReceived));

                int received = receiveRaw(buffer, bytesToRead);
                if (received <= 0) {
                    std::cerr << "Error receiving data or connection closed" << std::endl;
                    break;  // Error or connection closed
                }

                outFile.write(buffer, received);
                bytesReceived += received;

                // Update progress
                if (fileSize > 0) {
                    transferProgress = static_cast<int>((bytesReceived * 100) / fileSize);
                    if (progressCallback) {
                        progressCallback(transferProgress, filePath, fileSize);
                    }
                }

                // Check abortion status periodically
                if (bytesReceived % (BUFFER_SIZE * 10) == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            outFile.close();

            // Set progress to 100% if successful and not aborted
            if (!aborted && bytesReceived == fileSize) {
                transferProgress = 100;
                if (progressCallback) {
                    progressCallback(100, filePath, fileSize);
                }
                if (completeCallback) {
                    completeCallback(true, filePath, TransferType::DOWNLOAD);
                }
            }
            else if (aborted) {
                std::cerr << "Download aborted" << std::endl;
                std::filesystem::remove(filePath);
                if (completeCallback) {
                    completeCallback(false, filePath, TransferType::DOWNLOAD);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during download: " << e.what() << std::endl;
            if (completeCallback) {
                completeCallback(false, filePath, TransferType::DOWNLOAD);
            }
        }

        std::lock_guard<std::mutex> guard(transferMutex);
        downloadRunning = false;
        currentTransfer = TransferType::NONE;
        transferCV.notify_all();
        });

    downloadThread.detach();
    return true;
}

bool transfer_sock::uploadFile(const std::string& localFile, const std::string& remotePath) {
    if (!isConnected()) {
        if (!connect()) {
            std::cerr << "Failed to connect for upload" << std::endl;
            return false;
        }
    }

    filePath = localFile;
    aborted = false;
    transferProgress = 0;
    currentTransfer = TransferType::UPLOAD;

    // Check if file exists and get size
    if (!std::filesystem::exists(localFile)) {
        std::cerr << "File does not exist: " << localFile << std::endl;
        currentTransfer = TransferType::NONE;
        return false;
    }

    try {
        fileSize = std::filesystem::file_size(localFile);
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting file size: " << e.what() << std::endl;
        currentTransfer = TransferType::NONE;
        return false;
    }

    // Format and send upload request command
    std::string request = "UPLOAD " + remotePath + "\n" + std::to_string(fileSize) + "\n";
    if (send(request) < 0) {
        std::cerr << "Failed to send upload request" << std::endl;
        currentTransfer = TransferType::NONE;
        return false;
    }

    // Get server response
    std::string response = receiveLine();
    if (response != "OK") {
        std::cerr << "Upload request rejected: " << response << std::endl;
        currentTransfer = TransferType::NONE;
        return false;
    }

    // Start upload in a separate thread
    std::thread uploadThread([this, localFile]() {
        std::unique_lock<std::mutex> lock(transferMutex);
        uploadRunning = true;
        lock.unlock();

        try {
            // Open file for reading
            std::ifstream inFile(localFile, std::ios::binary);
            if (!inFile.is_open()) {
                std::cerr << "Failed to open file for reading: " << localFile << std::endl;
                std::lock_guard<std::mutex> guard(transferMutex);
                uploadRunning = false;
                currentTransfer = TransferType::NONE;
                if (completeCallback) {
                    completeCallback(false, filePath, TransferType::UPLOAD);
                }
                transferCV.notify_all();
                return;
            }

            // Start sending file data
            long bytesSent = 0;
            char buffer[BUFFER_SIZE];

            while (!aborted && inFile.good() && bytesSent < fileSize) {
                inFile.read(buffer, BUFFER_SIZE);
                int bytesRead = static_cast<int>(inFile.gcount());

                if (bytesRead <= 0) {
                    break;  // End of file or error
                }

                if (sendRaw(buffer, bytesRead) < 0) {
                    std::cerr << "Error sending file data" << std::endl;
                    break;
                }

                bytesSent += bytesRead;

                // Update progress
                if (fileSize > 0) {
                    transferProgress = static_cast<int>((bytesSent * 100) / fileSize);
                    if (progressCallback) {
                        progressCallback(transferProgress, filePath, fileSize);
                    }
                }

                // Check abortion status periodically
                if (bytesSent % (BUFFER_SIZE * 10) == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            }

            inFile.close();

            // Set progress to 100% if successful and not aborted
            if (!aborted && bytesSent == fileSize) {
                transferProgress = 100;
                if (progressCallback) {
                    progressCallback(100, filePath, fileSize);
                }

                // Wait for server confirmation
                std::string confirmResponse = receiveLine();
                bool success = (confirmResponse == "COMPLETE");

                if (completeCallback) {
                    completeCallback(success, filePath, TransferType::UPLOAD);
                }
            }
            else if (aborted) {
                std::cerr << "Upload aborted" << std::endl;
                if (completeCallback) {
                    completeCallback(false, filePath, TransferType::UPLOAD);
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during upload: " << e.what() << std::endl;
            if (completeCallback) {
                completeCallback(false, filePath, TransferType::UPLOAD);
            }
        }

        std::lock_guard<std::mutex> guard(transferMutex);
        uploadRunning = false;
        currentTransfer = TransferType::NONE;
        transferCV.notify_all();
        });

    uploadThread.detach();
    return true;
}

bool transfer_sock::runServer(const std::string& rootDir) {
    if (thisServerRunning) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }

    // Set root directory for file operations
    rootDirectory = rootDir;

    // Create root directory if it doesn't exist
    try {
        std::filesystem::create_directories(rootDirectory);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to create root directory: " << e.what() << std::endl;
        return false;
    }

    // Bind the socket to listen for incoming connections
    if (!bind(getPort())) {
        std::cerr << "Failed to bind server socket" << std::endl;
        return false;
    }

    if (!listen()) {
        std::cerr << "Failed to start listening" << std::endl;
        return false;
    }

    thisServerRunning = true;

    // Start server thread
    std::thread serverThread([this]() {
        std::cout << "Transfer server started on port " << getPort() << std::endl;
        std::cout << "Using root directory: " << rootDirectory << std::endl;

        while (thisServerRunning) {
            // Accept incoming connection
            Connection* conn = accept();
            if (!conn) {
                // Check if server was stopped
                if (!thisServerRunning) {
                    break;
                }
                std::cerr << "Failed to accept client connection" << std::endl;
                continue;
            }


            // Handle client in a separate thread
            std::thread clientThread(&transfer_sock::handleClientRequest, this, (transfer_sock*)conn);
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
        disconnect(); // Force accept() to return with an error
    }
}

int transfer_sock::getProgress() const {
    return transferProgress;
}

void transfer_sock::cancelTransfer() {
    std::unique_lock<std::mutex> lock(transferMutex);

    if (downloadRunning || uploadRunning) {
        aborted = true;
        transferCV.wait(lock, [] { return !downloadRunning && !uploadRunning; });
    }

    aborted = false;  // Reset for future transfers
    currentTransfer = TransferType::NONE;
}

void transfer_sock::setPriority(int prio) {
    priority = prio;
    // Note: actual thread priority setting would require platform-specific code
}

long transfer_sock::getFileSize() const {
    return fileSize;
}

TransferType transfer_sock::getCurrentTransferType() const {
    return currentTransfer;
}

void transfer_sock::setProgressCallback(TransferProgressCallback callback) {
    progressCallback = callback;
}

void transfer_sock::setCompleteCallback(TransferCompleteCallback callback) {
    completeCallback = callback;
}

// Helper function to receive a line terminated with '\n'
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

// Helper function to receive raw data with specified size limit
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

// Helper function to receive raw data
int transfer_sock::receiveRaw(char* buffer, int size) {
    if (!isConnected()) {
        return -1;
    }

    // Use the base class socket to receive data
    std::string data = TCPSock::receive();

    if (data.empty()) {
        return 0;
    }

    // Copy at most 'size' bytes to the buffer
    int bytesToCopy = min(size, static_cast<int>(data.length()));
    std::copy(data.begin(), data.begin() + bytesToCopy, buffer);

    return bytesToCopy;
}

// Helper function to send raw data
int transfer_sock::sendRaw(const char* buffer, int size) {
    if (!isConnected()) {
        return -1;
    }

    // Use the base class socket to send data
    std::string data(buffer, size);
    return TCPSock::send(data);
}

// Server request handling
void transfer_sock::handleClientRequest(transfer_sock* clientSock) {
    if (!clientSock || !clientSock->isConnected()) {
        std::cerr << "Invalid client connection" << std::endl;
        delete clientSock;
        return;
    }

    std::cout << "New client connected: " << clientSock->getAddress() << ":" << clientSock->getPort() << std::endl;

    try {
        // Receive command
        std::string request = clientSock->receive(1024);
        std::vector<std::string> tokens = parseCommand(request);

        if (tokens.empty()) {
            clientSock->send("ERROR Invalid request\n");
            delete clientSock;
            return;
        }

        std::string command = tokens[0];

        if (command == "DOWNLOAD" && tokens.size() >= 2) {
            // Handle download request (client wants to download a file from server)
            std::string remotePath = tokens[1];
            handleDownloadRequest(clientSock, remotePath);
        }
        else if (command == "UPLOAD" && tokens.size() >= 2) {
            // Handle upload request (client wants to upload a file to server)
            std::string remotePath = tokens[1];

            // Get file size
            std::string sizeStr = clientSock->receiveLine();
            long fileSize = 0;

            try {
                fileSize = std::stol(sizeStr);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing file size: " << e.what() << std::endl;
                clientSock->send("ERROR Invalid file size\n");
                delete clientSock;
                return;
            }

            handleUploadRequest(clientSock, remotePath, fileSize);
        }
        else {
            clientSock->send("ERROR Unknown command\n");
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception handling client request: " << e.what() << std::endl;
    }

    // Clean up
    delete clientSock;
}

bool transfer_sock::handleDownloadRequest(transfer_sock* clientSock, const std::string& remotePath) {
    // Normalize and validate path
    std::string fullPath = normalizePath(remotePath);
    std::string filePath = rootDirectory + "/" + fullPath;

    std::cout << "Download request for: " << fullPath << std::endl;

    // Check if file exists
    if (!std::filesystem::exists(filePath) || std::filesystem::is_directory(filePath)) {
        std::cerr << "File not found: " << filePath << std::endl;
        clientSock->send("ERROR File not found\n");
        return false;
    }

    try {
        // Open file for reading
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            clientSock->send("ERROR Failed to open file\n");
            return false;
        }

        // Get file size
        long fileSize = std::filesystem::file_size(filePath);

        // Send OK response with file size
        clientSock->send("OK\n" + std::to_string(fileSize) + "\n");

        // Send file data
        char buffer[BUFFER_SIZE];
        long bytesSent = 0;

        while (!aborted && inFile.good() && bytesSent < fileSize) {
            inFile.read(buffer, BUFFER_SIZE);
            int bytesRead = static_cast<int>(inFile.gcount());

            if (bytesRead <= 0) {
                break;  // End of file or error
            }

            if (clientSock->sendRaw(buffer, bytesRead) < 0) {
                std::cerr << "Error sending file data" << std::endl;
                break;
            }

            bytesSent += bytesRead;

            // Throttle transmission slightly to avoid overwhelming the network
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
    // Normalize and validate path
    std::string fullPath = normalizePath(remotePath);
    std::string filePath = rootDirectory + "/" + fullPath;

    std::cout << "Upload request for: " << fullPath << " (" << fileSize << " bytes)" << std::endl;

    try {
        // Create directories if needed
        std::filesystem::path destinationPath(filePath);
        std::filesystem::create_directories(destinationPath.parent_path());

        // Open file for writing
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            clientSock->send("ERROR Failed to create file\n");
            return false;
        }

        // Send OK response
        clientSock->send("OK\n");

        // Receive file data
        char buffer[BUFFER_SIZE];
        long bytesReceived = 0;

        while (!aborted && bytesReceived < fileSize) {
            int bytesToRead = static_cast<int>(min(static_cast<long>(BUFFER_SIZE),
                fileSize - bytesReceived));

            int received = clientSock->receiveRaw(buffer, bytesToRead);
            if (received <= 0) {
                std::cerr << "Error receiving data or connection closed" << std::endl;
                break;  // Error or connection closed
            }

            outFile.write(buffer, received);
            bytesReceived += received;

            // Throttle reception slightly to avoid overwhelming memory
            if (bytesReceived % (BUFFER_SIZE * 50) == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        outFile.close();

        // Send completion confirmation if fully received
        if (bytesReceived == fileSize) {
            clientSock->send("COMPLETE\n");
            std::cout << "Upload complete: " << fullPath << " (" << bytesReceived << " bytes)" << std::endl;
            return true;
        }
        else {
            clientSock->send("ERROR Incomplete transfer\n");
            // Clean up partial file
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

// Utility functions
std::string transfer_sock::normalizePath(const std::string& path) const {
    // Remove leading slashes and backslashes
    std::string result = path;
    while (!result.empty() && (result[0] == '/' || result[0] == '\\')) {
        result.erase(0, 1);
    }

    // Replace backslashes with forward slashes
    std::replace(result.begin(), result.end(), '\\', '/');

    // Remove any ".." to prevent directory traversal attacks
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

    // Reconstruct path
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