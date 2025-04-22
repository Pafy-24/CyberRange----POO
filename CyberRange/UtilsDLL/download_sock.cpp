#include "download_sock.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>

// Global variables for managing download thread
static std::mutex downloadMutex;
static std::condition_variable downloadCV;
static bool downloadRunning = false;

download_sock::download_sock(std::string addr, int port)
    : TCPSock(addr, port), downloadProgress(0), filePath(""), fileSize(0),
    aborted(false), priority(0) {
    // Enable TLS by default for download sockets
    enableTLS();
}

download_sock::~download_sock() {
    cancelDownload();
}

bool download_sock::downloadFile(std::string url, std::string destination) {
    if (!isConnected()) {
        if (!connect()) {
            std::cerr << "Failed to connect for download" << std::endl;
            return false;
        }
    }

    filePath = destination;
    aborted = false;
    downloadProgress = 0;

    // Parse URL to extract host, path, etc.
    std::string host;
    std::string path = "/";

    // Simple URL parsing (would be more robust in a real implementation)
    size_t protocolEnd = url.find("://");
    if (protocolEnd != std::string::npos) {
        url = url.substr(protocolEnd + 3);
    }

    size_t pathStart = url.find('/');
    if (pathStart != std::string::npos) {
        host = url.substr(0, pathStart);
        path = url.substr(pathStart);
    }
    else {
        host = url;
    }

    // Create HTTP GET request
    std::stringstream request;
    request << "GET " << path << " HTTP/1.1\r\n";
    request << "Host: " << host << "\r\n";
    request << "Connection: close\r\n";
    request << "User-Agent: DownloadSock/1.0\r\n";
    request << "\r\n";

    // Send the request
    if (send(request.str()) < 0) {
        std::cerr << "Failed to send download request" << std::endl;
        return false;
    }

    // Start download in a separate thread
    std::thread downloadThread([this]() {
        std::unique_lock<std::mutex> lock(downloadMutex);
        downloadRunning = true;

        // Open file for writing
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            downloadRunning = false;
            downloadCV.notify_all();
            return;
        }

        // Receive HTTP response
        bool headerDone = false;
        long contentLength = -1;
        long bytesReceived = 0;
        std::string buffer;

        while (!aborted) {
            std::string chunk = TCPSock::receive();
            if (chunk.empty()) {
                break;  // End of data
            }

            buffer += chunk;

            // Process HTTP headers if not done yet
            if (!headerDone) {
                size_t headerEnd = buffer.find("\r\n\r\n");
                if (headerEnd != std::string::npos) {
                    std::string headers = buffer.substr(0, headerEnd);
                    buffer = buffer.substr(headerEnd + 4);
                    headerDone = true;

                    // Parse Content-Length
                    size_t contentLengthPos = headers.find("Content-Length:");
                    if (contentLengthPos != std::string::npos) {
                        size_t valueStart = headers.find_first_not_of(" ", contentLengthPos + 15);
                        size_t valueEnd = headers.find_first_of("\r\n", valueStart);
                        std::string lengthStr = headers.substr(valueStart, valueEnd - valueStart);
                        contentLength = std::stol(lengthStr);
                        fileSize = contentLength;
                    }
                }
            }

            // Write data to file if we're past headers
            if (headerDone && !buffer.empty()) {
                outFile.write(buffer.data(), buffer.size());
                bytesReceived += buffer.size();
                buffer.clear();

                // Update progress
                if (contentLength > 0) {
                    downloadProgress = static_cast<int>((bytesReceived * 100) / contentLength);
                }
            }

            // Release lock temporarily to allow checking abortion status
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            lock.lock();
        }

        outFile.close();

        // Set progress to 100% if successful and not aborted
        if (!aborted && headerDone) {
            downloadProgress = 100;
        }

        downloadRunning = false;
        downloadCV.notify_all();
        });

    downloadThread.detach();
    return true;
}

int download_sock::getProgress() {
    return downloadProgress;
}

void download_sock::cancelDownload() {
    std::unique_lock<std::mutex> lock(downloadMutex);

    if (downloadRunning) {
        aborted = true;

        // Wait for download thread to finish
        downloadCV.wait(lock, [] { return !downloadRunning; });
    }

    aborted = false;  // Reset for future downloads
}

void download_sock::setPriority(int prio) {
    priority = prio;
    // In a real implementation, this might adjust thread priority
    // or network QoS settings
}

long download_sock::getFileSize() {
    return fileSize;
}