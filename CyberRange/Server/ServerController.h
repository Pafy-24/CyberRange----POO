#pragma once
#include <string>
#include <map>
#include "CController.h"
#include "ServerMng.h"

class ServerController : public CController {
private:
    ServerMng& serverMng;
    std::map<std::string, std::string> configs;

public:
    ServerController(ServerMng& serverMng);
    void restart();
    void shutdown();
    void updateConfig(std::string key, std::string value);
    std::string getConfig(std::string key);
    std::map<std::string, std::string> getStatus();
    bool backup(std::string destination);
};
