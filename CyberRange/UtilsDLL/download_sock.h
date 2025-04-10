#pragma once
#include <string>
#include "TCPSock.h"

class download_sock: public TCPSock {
private:
    int downloadProgress;
    std::string filePath;
    long fileSize;

public:
    download_sock(std::string addr, int port);
    bool downloadFile(std::string url, std::string destination);
    int getProgress();
    void cancelDownload();
    void setPriority(int prio);
};
