#pragma once
#include "TCPSock.h"
#include <string>

class UTILS_API transfer_sock : public TCPSock {
public:
    transfer_sock(const std::string& addr, int port);
    ~transfer_sock();

    // Download operations
    bool downloadFile(const std::string& remotePath, const std::string& destination);

    // Upload operations
    bool uploadFile(const std::string& localFile, const std::string& remotePath);

    // Transfer management
    int getProgress() const;
    void cancelTransfer();
    void setPriority(int prio);
    long getFileSize() const;

private:
    int transferProgress;
    std::string filePath;
    long fileSize;
    bool aborted;
    int priority;

    std::string receiveLine();
    std::string receive(int maxSize);
    int receiveRaw(char* buffer, int size);
    int sendRaw(const char* buffer, int size);
};