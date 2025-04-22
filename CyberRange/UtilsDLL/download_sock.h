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

    // Eliminăm HANDLE downloadThread deoarece folosim std::thread

public:
    download_sock(std::string addr, int port);
    virtual ~download_sock();

    bool downloadFile(std::string url, std::string destination);
    int getProgress();
    void cancelDownload();
    void setPriority(int prio);
    long getFileSize();
};