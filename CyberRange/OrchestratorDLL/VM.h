#pragma once
#include <string>
#include "COrchestrator.h"

class ORCHESTRATOR_API VM : public COrchestrator {
private:
    std::string vmName;
    std::string baseImage;
    std::string imageType; // "vdi", "vmdk", or "ovf"
    int memoryMB;
    int cpuCores;
    std::string challengeId;

    bool isOVF() const { return imageType == "ovf"; }

public:
    VM(const std::string& userId, const std::string& resourceId, const std::string& name, 
        const std::string& image, const std::string& address, const std::string& challId);
    bool start() override;
    bool stop() override;
    bool deploy() override;
    bool undeploy() override;
    std::string getStatus() override;
    void setMemory(int mb);
    void setCPU(int cores);
    std::string getAddress() override;
};