#pragma once
#include "DLL.h"
#include <string>

class Orchestrator {
public:
    virtual bool start(const std::string& userId, const std::string& resourceId) = 0;
    virtual bool stop(const std::string& userId, const std::string& resourceId) = 0;
    virtual bool deploy(const std::string& userId, const std::string& resourceId) = 0;
    virtual bool undeploy(const std::string& userId, const std::string& resourceId) = 0;
    virtual std::string getStatus(const std::string& userId, const std::string& resourceId) = 0;
    virtual ~Orchestrator() = default;
};