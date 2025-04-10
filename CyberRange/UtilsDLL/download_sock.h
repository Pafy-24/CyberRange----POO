#pragma once
#include <string>

class download_sock {
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
