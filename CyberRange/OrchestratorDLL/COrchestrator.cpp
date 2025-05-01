#include "pch.h"
#include "COrchestrator.h"
#include <sstream>
#include <stdexcept>
#include <array>
#include <windows.h>

COrchestrator::COrchestrator(const std::string& userId, const std::string& resourceId,int timeout)
    :  timeoutSeconds(timeout), userId(userId), resourceId(resourceId){
}

std::string COrchestrator::executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    FILE* pipe = _popen(("cmd.exe /c " + command).c_str(), "r");

    if (!pipe) {
        throw std::runtime_error("Command execution failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    int returnCode = _pclose(pipe);
    if (returnCode != 0) {
        result += "Command failed with exit code: " + std::to_string(returnCode);
    }

    return result;
}

bool COrchestrator::isValidId(const std::string& id) {
    return !id.empty() && id.find(' ') == std::string::npos && id.find('"') == std::string::npos;
}

bool COrchestrator::start() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return isValidId(userId) && isValidId(resourceId);
}

bool COrchestrator::stop() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return isValidId(userId) && isValidId(resourceId);
}

bool COrchestrator::deploy() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return isValidId(userId) && isValidId(resourceId);
}

bool COrchestrator::undeploy() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return isValidId(userId) && isValidId(resourceId);
}

std::string COrchestrator::getStatus() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    if (!isValidId(userId) || !isValidId(resourceId)) {
        return "Invalid user or resource ID";
    }

    std::stringstream status;
    status << "User: " << userId << "\nResource: " << resourceId << "\n";
    return status.str();
}

void COrchestrator::setTimeout(int seconds) {
    if (seconds > 0) {
        timeoutSeconds = seconds;
    }
}

std::string COrchestrator::getAddress()
{
    return std::string("Not applicable");
}
