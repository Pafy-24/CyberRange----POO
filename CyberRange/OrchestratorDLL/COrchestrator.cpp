#include "pch.h"
#include "COrchestrator.h"
#include <sstream>
#include <stdexcept>
#include <array>
#include <windows.h>

COrchestrator::COrchestrator(int userId, int id,int timeout)
    :  timeoutSeconds(timeout), userId(userId), id(id){
}

int COrchestrator::getId() const
{
    return id;
}

void COrchestrator::setId(int id)
{
	this->id = id;
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


bool COrchestrator::start() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return 1;
}

bool COrchestrator::stop() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return 1;
}

bool COrchestrator::deploy() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return 1;
}

bool COrchestrator::undeploy() {
    std::lock_guard<std::mutex> lock(resourceMutex);
    return 1;
}

std::string COrchestrator::getStatus() {
    std::lock_guard<std::mutex> lock(resourceMutex);

    std::stringstream status;
    status << "User: " << userId << "\nResource: " << id << "\n";
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
