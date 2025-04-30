#pragma once
#include <string>
#include <map>
#include "COrchestrator.h"

class ORCHESTRATOR_API Docker : public COrchestrator {
private:
    std::string imageName;
    std::map<std::string, std::string> envVars;
    int port;
    std::string challengeId;

public:
    Docker(const std::string& image, const std::string& address, const std::string& challId);
    bool start(const std::string& userId, const std::string& resourceId) override;
    bool stop(const std::string& userId, const std::string& resourceId) override;
    bool deploy(const std::string& userId, const std::string& resourceId) override;
    bool undeploy(const std::string& userId, const std::string& resourceId) override;
    std::string getStatus(const std::string& userId, const std::string& resourceId) override;
    void setEnv(const std::string& key, const std::string& value);
    void setPort(int portNum);
    std::string getLogs(const std::string& userId, const std::string& resourceId);
    std::string getAddress() override;
};