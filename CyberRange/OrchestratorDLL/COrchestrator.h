#pragma once
#include <string>
#include <mutex>
#include "Orchestrator.h"

class ORCHESTRATOR_API COrchestrator : public Orchestrator {
protected:
    std::string baseAddr;
    std::string userId;
    std::string resourceId;
    std::mutex resourceMutex;
    int timeoutSeconds;

    std::string executeCommand(const std::string& command);
    bool isValidId(const std::string& id);

public:
    COrchestrator(const std::string& userId,const std::string& resourceId,const std::string& address, int timeout = 300);
    bool start() override;
    bool stop() override;
    bool deploy() override;
    bool undeploy() override;
    std::string getStatus() override;
    void setTimeout(int seconds);
    std::string getAddress() override;
};