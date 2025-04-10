#pragma once
#include <string>
#include "COrchestrator.h"

class File: public COrchestrator{
private:
    std::string path;
    std::string content;
    bool exists;

public:
    File(std::string path);
    bool read();
    bool write(std::string content);
    bool append(std::string content);
    bool remove();
    std::string getContent();
    bool exists();
    long getSize();
};
