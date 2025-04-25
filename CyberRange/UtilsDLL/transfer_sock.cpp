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

// Global variables for managing transfer threads
static std::mutex transferMutex;
static std::condition_variable transferCV;
static bool downloadRunning = false;
static bool uploadRunning = false;

// Size of buffer used for transfer
constexpr size_t BUFFER_SIZE = 8192;

transfer_sock::transfer_sock(const std::string& addr, int port)
    : TCPSock(addr, port),
    transferProgress(0),
    filePath(""),
    fileSize(0),
    aborted(false),
    priority(0) {
}

transfer_sock::~transfer_sock() {
    cancelTransfer();
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

    // Format and send download request command
    std::string request = "DOWNLOAD " + remotePath + "\n";
    if (send(request) < 0) {
        std::cerr << "Failed to send download request" << std::endl;
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
                transferCV.notify_all();
                return;
            }

            // First receive file size
            std::string sizeStr = receiveLine();
            try {
                fileSize = std::stol(sizeStr);
            }
            catch (const std::exception& e) {
                std::cerr << "Error parsing file size: " << e.what() << std::endl;
                outFile.close();
                std::lock_guard<std::mutex> guard(transferMutex);
                downloadRunning = false;
                transferCV.notify_all();
                return;
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
            }
            else if (aborted) {
                std::cerr << "Download aborted" << std::endl;
            }

        }
        catch (const std::exception& e) {
            std::cerr << "Exception during download: " << e.what() << std::endl;
        }

        std::lock_guard<std::mutex> guard(transferMutex);
        downloadRunning = false;
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

    // Check if file exists and get size
    std::ifstream inFile(localFile, std::ios::binary | std::ios::ate);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << localFile << std::endl;
        return false;
    }

    fileSize = static_cast<long>(inFile.tellg());
    inFile.close();

    // Format and send upload request command
    std::string request = "UPLOAD " + remotePath + "\n" + std::to_string(fileSize) + "\n";
    if (send(request) < 0) {
        std::cerr << "Failed to send upload request" << std::endl;
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
            }
            else if (aborted) {
                std::cerr << "Upload aborted" << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception during upload: " << e.what() << std::endl;
        }

        std::lock_guard<std::mutex> guard(transferMutex);
        uploadRunning = false;
        transferCV.notify_all();
        });

    uploadThread.detach();
    return true;
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
}

void transfer_sock::setPriority(int prio) {
    priority = prio;
    // Note: actual thread priority setting would require platform-specific code
}

long transfer_sock::getFileSize() const {
    return fileSize;
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