#include "VM.h"
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <fstream>


VM::VM(const std::string& name, const std::string& image, const std::string& address, const std::string& challId)
    : COrchestrator(address), vmName(name), baseImage(image), memoryMB(1024), cpuCores(1), challengeId(challId) {
    std::string ext = baseImage.substr(baseImage.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "ovf") {
        imageType = "ovf";
    }
    else if (ext == "vmdk") {
        imageType = "vmdk";
    }
    else if (ext == "vdi") {
        imageType = "vdi";
    }
    else {
        throw std::runtime_error("Unsupported image format: " + ext);
    }
}

bool VM::start(const std::string& userId, const std::string& resourceId) {
    if (!COrchestrator::start(userId, resourceId)) return false;

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" startvm \"" << vmName << "-" << userId << "-" << resourceId << "\" --type headless";
    std::string result = executeCommand(cmd.str());
    return result.find("ERROR") == std::string::npos;
}

bool VM::stop(const std::string& userId, const std::string& resourceId) {
    if (!COrchestrator::stop(userId, resourceId)) return false;

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" controlvm \"" << vmName << "-" << userId << "-" << resourceId << "\" poweroff";
    std::string result = executeCommand(cmd.str());

    // Delete files in Resources/[chall_id]
    std::filesystem::path targetDir = "./Resources/" + challengeId;
    if (std::filesystem::exists(targetDir)) {
        std::filesystem::remove_all(targetDir);
    }

    return result.find("ERROR") == std::string::npos;
}

bool VM::deploy(const std::string& userId, const std::string& resourceId) {
    if (!COrchestrator::deploy(userId, resourceId)) return false;

    // Create Resources/[chall_id] directory and copy image
    std::filesystem::path targetDir = "./Resources/" + challengeId;
    std::filesystem::create_directories(targetDir);
    std::filesystem::path srcPath = baseImage;
    std::filesystem::path dstPath = targetDir / srcPath.filename();
    std::filesystem::copy_file(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing);

    std::stringstream cmd;
    std::string vmId = vmName + "-" + userId + "-" + resourceId;

    if (isOVF()) {
        cmd << "\"VBoxManage.exe\" import \"" << dstPath.string() << "\" --vsys 0 --vmname \"" << vmId << "\"";
        cmd << " && \"VBoxManage.exe\" modifyvm \"" << vmId << "\" --memory " << memoryMB << " --cpus " << cpuCores;
        cmd << " --nic1 bridged --bridgeadapter1 eth0";
    }
    else {
        cmd << "\"VBoxManage.exe\" createvm --name \"" << vmId
            << "\" --ostype Linux --register && "
            << "\"VBoxManage.exe\" modifyvm \"" << vmId
            << "\" --memory " << memoryMB << " --cpus " << cpuCores
            << " --nic1 bridged --bridgeadapter1 eth0 && "
            << "\"VBoxManage.exe\" storagectl \"" << vmId
            << "\" --name \"SATA\" --add sata && "
            << "\"VBoxManage.exe\" storageattach \"" << vmId
            << "\" --storagectl \"SATA\" --port 0 --device 0 --type hdd --medium \""
            << dstPath.string() << "\"";
    }

    std::string result = executeCommand(cmd.str());
    return result.find("ERROR") == std::string::npos;
}

bool VM::undeploy(const std::string& userId, const std::string& resourceId) {
    if (!COrchestrator::undeploy(userId, resourceId)) return false;

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" unregistervm \"" << vmName << "-" << userId << "-" << resourceId << "\" --delete";
    std::string result = executeCommand(cmd.str());

    // Delete files in Resources/[chall_id]
    std::filesystem::path targetDir = "./Resources/" + challengeId;
    if (std::filesystem::exists(targetDir)) {
        std::filesystem::remove_all(targetDir);
    }

    return result.find("ERROR") == std::string::npos;
}

std::string VM::getStatus(const std::string& userId, const std::string& resourceId) {
    std::string baseStatus = COrchestrator::getStatus(userId, resourceId);

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" showvminfo \"" << vmName << "-" << userId << "-" << resourceId << "\" --machinereadable";
    std::string result = executeCommand(cmd.str());

    std::stringstream status;
    status << baseStatus;
    status << "VM Name: " << vmName << "-" << userId << "-" << resourceId << "\n";
    status << "Image Type: " << imageType << "\n";
    status << "Memory: " << memoryMB << "MB\n";
    status << "CPUs: " << cpuCores << "\n";
    status << "State: " << (result.find("running") != std::string::npos ? "Running" : "Stopped") << "\n";

    return status.str();
}

void VM::setMemory(int mb) {
    if (mb > 0) memoryMB = mb;
}

void VM::setCPU(int cores) {
    if (cores > 0) cpuCores = cores;
}

std::string VM::getAddress() {
    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" guestproperty enumerate \"" << vmName << "\" | findstr /C:\"/VirtualBox/GuestInfo/Net/0/V4/IP\"";
    std::string result = executeCommand(cmd.str());

    std::string ip = "unknown";
    size_t pos = result.find("value=");
    if (pos != std::string::npos) {
        ip = result.substr(pos + 6);
        ip = ip.substr(0, ip.find_first_of(","));
    }
    return ip + ":0"; // No specific port for VMs
}