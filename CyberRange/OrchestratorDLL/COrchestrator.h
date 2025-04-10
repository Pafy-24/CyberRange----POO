#pragma once
#include <string>
#include <vector>
#include <map>
#include "Orchestrator.h"
#include "Socketer.h"

class COrchestrator : public Orchestrator {
private:
    std::string addr;
    std::vector<Socketer*> sockets;
    std::map<std::string, std::string> usersData;
    bool running;
    int timeout;

public:
    COrchestrator(std::string address);
    void start() override;
    void stop() override;
    bool deploy(std::string resource) override;
    bool undeploy(std::string resource) override;
    std::string getStatus() override;
    void setTimeout(int seconds);
    void addSocket(Socketer* socket);
    void removeSocket(std::string socketId);
    std::map<std::string, std::string> getUsersData();
};
