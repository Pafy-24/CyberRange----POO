#pragma once
#include <string>
#include "TCPSock.h"

class UTILS_API download_sock : public TCPSock {
private:
    int downloadProgress;
    std::string filePath;
    long fileSize;
    bool aborted;
    int priority;

public:
    download_sock(const std::string& addr, int port);
    virtual ~download_sock();

    bool downloadFile(std::string url, const std::string& destination);

    int getProgress() const;
    void cancelDownload();
    void setPriority(int prio);
    long getFileSize() const;
};