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
    int challengeId;

    bool isOVF() const { return imageType == "ovf"; }

public:
    VM(int userId, int resourceId, const std::string& name,
        const std::string& image, int challId);
    bool start() override;
    bool stop() override;
    bool deploy() override;
    bool undeploy() override;
    std::string getStatus() override;
    void setMemory(int mb);
    void setCPU(int cores);
    std::string getAddress() override;
};