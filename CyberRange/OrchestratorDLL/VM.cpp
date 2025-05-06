#include "pch.h"
#include "VM.h"
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <fstream>


VM::VM(int userId, int id, const std::string& name, 
    const std::string& image, int challId)
    : COrchestrator(userId,id), vmName(name), baseImage(image), memoryMB(1024), cpuCores(1), challengeId(challId) {
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

bool VM::start() {
    if (!COrchestrator::start()) return false;

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" startvm \"" << vmName << "-" << userId << "-" << id << "\" --type headless";
    std::string result = executeCommand(cmd.str());
    return result.find("ERROR") == std::string::npos;
}

bool VM::stop() {
    if (!COrchestrator::stop()) return false;

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" controlvm \"" << vmName << "-" << userId << "-" << id << "\" poweroff";
    std::string result = executeCommand(cmd.str());

    std::filesystem::path targetDir = "./Resources/" + challengeId;
    if (std::filesystem::exists(targetDir)) {
        std::filesystem::remove_all(targetDir);
    }

    return result.find("ERROR") == std::string::npos;
}

bool VM::deploy() {
    if (!COrchestrator::deploy()) return false;

    std::filesystem::path targetDir = "./Resources/" + challengeId;
    std::filesystem::create_directories(targetDir);
    std::filesystem::path srcPath = baseImage;
    std::filesystem::path dstPath = targetDir / srcPath.filename();
    std::filesystem::copy_file(srcPath, dstPath, std::filesystem::copy_options::overwrite_existing);

    std::stringstream cmd;
    std::string vmId = vmName + "-" + std::to_string(userId) + "-" + std::to_string(id);

    if (isOVF()) {
        cmd << "VBoxManage import \"" << dstPath.string() << "\" --vsys 0 --vmname \"" << vmId << "\"";
        cmd << " && VBoxManage modifyvm \"" << vmId << "\" --memory " << memoryMB << " --cpus " << cpuCores;
        cmd << " --nic1 bridged --bridgeadapter1 eth0";
    }
    else {
        cmd << "VBoxManage createvm --name \"" << vmId
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

bool VM::undeploy() {
    if (!COrchestrator::undeploy()) return false;

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" unregistervm \"" << vmName << "-" << userId << "-" << id << "\" --delete";
    std::string result = executeCommand(cmd.str());

    std::filesystem::path targetDir = "./Resources/" + challengeId;
    if (std::filesystem::exists(targetDir)) {
        std::filesystem::remove_all(targetDir);
    }

    return result.find("ERROR") == std::string::npos;
}

std::string VM::getStatus() {
    std::string baseStatus = COrchestrator::getStatus();

    std::stringstream cmd;
    cmd << "\"VBoxManage.exe\" showvminfo \"" << vmName << "-" << userId << "-" << id << "\" --machinereadable";
    std::string result = executeCommand(cmd.str());

    std::stringstream status;
    status << baseStatus;
    status << "VM Name: " << vmName << "-" << userId << "-" << id << "\n";
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
    return ip + ":0"; 
}