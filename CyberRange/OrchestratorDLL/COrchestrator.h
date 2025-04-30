#pragma once
#include <string>
#include <mutex>
#include "Orchestrator.h"

class COrchestrator : public Orchestrator {
protected:
    std::string baseAddr;
    std::mutex resourceMutex;
    int timeoutSeconds;

    std::string executeCommand(const std::string& command);
    bool isValidId(const std::string& id);

public:
    COrchestrator(const std::string& address, int timeout = 300);
    bool start(const std::string& userId, const std::string& resourceId) override;
    bool stop(const std::string& userId, const std::string& resourceId) override;
    bool deploy(const std::string& userId, const std::string& resourceId) override;
    bool undeploy(const std::string& userId, const std::string& resourceId) override;
    std::string getStatus(const std::string& userId, const std::string& resourceId) override;
    void setTimeout(int seconds);
};