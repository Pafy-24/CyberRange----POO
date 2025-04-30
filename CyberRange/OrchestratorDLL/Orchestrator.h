#pragma once
#include "DLL.h"
#include <string>

class Orchestrator {
public:
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool deploy() = 0;
    virtual bool undeploy() = 0;
    virtual std::string getStatus() = 0;
	virtual std::string getAddress() = 0;
    virtual ~Orchestrator() = default;
};