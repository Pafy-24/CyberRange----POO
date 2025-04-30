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
    VM(const std::string& name, const std::string& image, const std::string& address, const std::string& challId);
    bool start(const std::string& userId, const std::string& resourceId) override;
    bool stop(const std::string& userId, const std::string& resourceId) override;
    bool deploy(const std::string& userId, const std::string& resourceId) override;
    bool undeploy(const std::string& userId, const std::string& resourceId) override;
    std::string getStatus(const std::string& userId, const std::string& resourceId) override;
    void setMemory(int mb);
    void setCPU(int cores);
    std::string getAddress() override;
};