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
    Docker(const std::string& userId, const std::string& resourceId,const std::string& image, 
        const std::string& challId);
    bool start() override;
    bool stop() override;
    bool deploy() override;
    bool undeploy() override;
    std::string getStatus() override;
    void setEnv(const std::string& key, const std::string& value);
    void setPort(int portNum);
    std::string getLogs();
    std::string getAddress() override;
};