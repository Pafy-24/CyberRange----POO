#pragma once
#include <string>

class Orchestrator {
public:
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool deploy(std::string resource) = 0;
    virtual bool undeploy(std::string resource) = 0;
    virtual std::string getStatus() = 0;
    virtual ~Orchestrator() = default;
};
