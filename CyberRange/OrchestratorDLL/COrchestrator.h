#pragma once
#include <string>
#include <mutex>
#include "Orchestrator.h"

class ORCHESTRATOR_API COrchestrator : public Orchestrator {
protected:
	int id;
    std::string baseAddr;
    int userId;
    std::mutex resourceMutex;
    int timeoutSeconds;

    std::string executeCommand(const std::string& command);

public:
    COrchestrator(int userId,int id, int timeout = 300);
    int getId() const override;
    void setId(int id) override;
    bool start() override;
    bool stop() override;
    bool deploy() override;
    bool undeploy() override;
    std::string getStatus() override;
    void setTimeout(int seconds);
    std::string getAddress() override;
};