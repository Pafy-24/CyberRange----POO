#pragma once
#include <string>
#include <map>
#include "COrchestrator.h"

class ORCHESTRATOR_API Docker : public COrchestrator {
private:
    std::string imageName;
    std::map<std::string, std::string> envVars;
    int port;
    int challengeId;

public:
    Docker(int userId, int resourceId,const std::string& image, 
        int challId);
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