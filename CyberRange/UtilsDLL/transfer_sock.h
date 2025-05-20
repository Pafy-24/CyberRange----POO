#pragma once
#include "TCPSock.h"
#include "Observable.h"
#include <string>
#include <atomic>
#include <vector>
#include <functional>

class UTILS_API transfer_sock : public TCPSock{
private:
    std::atomic<int> transferProgress;
    std::string filePath;
    std::atomic<long> fileSize;
    std::atomic<bool> aborted;
    std::atomic<bool> thisServerRunning;
    std::string rootDirectory;

    std::string receiveLine();
    std::string receive(int maxSize);
    int receiveRaw(char* buffer, int size);
    int sendRaw(const std::string& buffer, int size);

    bool handleDownloadRequest(transfer_sock* clientSock, const std::string& remotePath);
    bool handleUploadRequest(transfer_sock* clientSock, const std::string& remotePath, long fileSize);

    std::string normalizePath(const std::string& path) const;
    std::vector<std::string> parseCommand(const std::string& command) const;

protected:
    void handleClientRequest(std::unique_ptr<TCPSock> clientSock) override;

public:
    transfer_sock(const std::string& addr, int port);
    explicit transfer_sock(int port);
    transfer_sock(std::unique_ptr<sf::TcpSocket> sock, const std::string& clientAddr, int clientPort);
    ~transfer_sock() override;

    void setRootDirectory(const std::string& dir);
    std::string getRootDirectory() const;

    int getTransferProgress() const;
    void abortTransfer();

    std::string getType() const override { return "TRANSFER"; }
    void handleRequest(const std::string& data, Connection* client = nullptr) override;

    bool startListening(std::function<void(const std::string&, Connection*)> handler) override;
    void stopServer() override;
    bool isServerRunning() const override;
};