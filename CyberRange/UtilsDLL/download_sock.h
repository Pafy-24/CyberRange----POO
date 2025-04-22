#pragma once
#include <string>
#include "TCPSock.h"

class download_sock : public TCPSock {
private:
    int downloadProgress;
    std::string filePath;
    long fileSize;
    bool aborted;
    int priority;

public:
    download_sock(std::string addr, int port);
    virtual ~download_sock();

    bool downloadFile(std::string url, std::string destination);
    int getProgress();
    void cancelDownload();
    void setPriority(int prio);
    long getFileSize();
};