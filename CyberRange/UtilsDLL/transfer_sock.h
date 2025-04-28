#pragma once
#include "TCPSock.h"
#include <string>
#include <atomic>
#include <vector>
#include <functional>

enum class TransferType {
    UPLOAD,
    DOWNLOAD,
    NONE
};

struct TransferRequest {
    std::string path;
    std::string destination;
    TransferType type;
    long size;
    int priority;
};

using TransferProgressCallback = std::function<void(int progress, const std::string& filePath, long fileSize)>;
using TransferCompleteCallback = std::function<void(bool success, const std::string& filePath, TransferType type)>;

class UTILS_API transfer_sock : public TCPSock {
private:
    std::atomic<int> transferProgress;
    std::string filePath;
    std::atomic<long> fileSize;
    std::atomic<bool> aborted;
    std::atomic<bool> serverRunning;
    int priority;
    TransferType currentTransfer;
    std::string rootDirectory;

    TransferProgressCallback progressCallback;
    TransferCompleteCallback completeCallback;

    std::string receiveLine();
    std::string receive(int maxSize);
    int receiveRaw(char* buffer, int size);
    int sendRaw(const char* buffer, int size);

    // Server request handling
    void handleClientRequest(transfer_sock* clientSock);
    bool handleDownloadRequest(transfer_sock* clientSock, const std::string& remotePath);
    bool handleUploadRequest(transfer_sock* clientSock, const std::string& remotePath, long fileSize);

    // Utility functions
    std::string normalizePath(const std::string& path) const;
    std::vector<std::string> parseCommand(const std::string& command) const;
public:
    transfer_sock(const std::string& addr, int port);

    explicit transfer_sock(int port);

    transfer_sock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort);

    ~transfer_sock();

    bool downloadFile(const std::string& remotePath, const std::string& destination);

    bool uploadFile(const std::string& localFile, const std::string& remotePath);

    bool runServer(const std::string& rootDirectory = "./files");
    void stopServer();

    // Transfer management
    int getProgress() const;
    void cancelTransfer();
    void setPriority(int prio);
    long getFileSize() const;
    TransferType getCurrentTransferType() const;

    // Event callbacks
    void setProgressCallback(TransferProgressCallback callback);
    void setCompleteCallback(TransferCompleteCallback callback);

};