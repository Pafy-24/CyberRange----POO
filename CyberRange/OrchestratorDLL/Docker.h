#pragma once
#include <string>
#include <map>
#include "COrchestrator.h"

class Docker : public COrchestrator {
private:
    std::string imageName;
    std::string containerId;
    std::map<std::string, std::string> env;
    int port;

public:
    Docker(std::string image);
    bool start();
    bool stop();
    bool restart();
    std::string getContainerId();
    void setEnv(std::string key, std::string value);
    void setPort(int port);
    std::string getLogs();
};
