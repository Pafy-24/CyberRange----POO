#include "pch.h"
#include "Docker.h"
#include <CustomSerial.h>
#include <sstream>
#include <filesystem>
#include <fstream>

#include <string>
#include <algorithm>

Docker::Docker(const std::string& userId, const std::string& resourceId,const std::string& image, 
    const std::string& challId)
    : COrchestrator(userId,resourceId), imageName(image), port(8080), challengeId(challId) {
}

bool Docker::start() {
    if (!COrchestrator::start()) return false;

    std::stringstream cmd;
    cmd << "docker start \"" << userId << "-" << resourceId << "-" << imageName << "\"";
    std::string result = executeCommand(cmd.str());
    return result.find("ERROR") == std::string::npos;
}

bool Docker::stop() {
    if (!COrchestrator::stop()) return false;

    std::stringstream cmd;
    cmd << "docker stop \"" << userId << "-" << resourceId << "-" << imageName << "\"";
    std::string result = executeCommand(cmd.str());

    // Delete files in Resources/[chall_id]
    std::filesystem::path targetDir = "./Resources/" + challengeId;
    if (std::filesystem::exists(targetDir)) {
        std::filesystem::remove_all(targetDir);
    }

    return result.find("ERROR") == std::string::npos;
}

bool Docker::deploy() {
    if (!COrchestrator::deploy()) return false;

    // Create Resources/[chall_id] directory and copy image (for Docker, we assume image is pulled)
    std::filesystem::path targetDir = "./Resources/" + challengeId;
    std::filesystem::create_directories(targetDir);
    // Note: Docker images are managed by Docker daemon, not copied as files

    std::stringstream cmd;
    cmd << "docker run -d --name \"" << userId << "-" << resourceId << "-" << imageName
        << "\" --network bridge -p " << port << ":" << port;

    for (const auto& env : envVars) {
        cmd << " -e \"" << env.first << "=" << env.second << "\"";
    }

    cmd << " \"" << imageName << "\"";
    std::string result = executeCommand(cmd.str());
    return result.find("ERROR") == std::string::npos;
}

bool Docker::undeploy() {
    if (!COrchestrator::undeploy()) return false;

    std::stringstream cmd;
    cmd << "docker rm -f \"" << userId << "-" << resourceId << "-" << imageName << "\"";
    std::string result = executeCommand(cmd.str());

    // Delete files in Resources/[chall_id]
    std::filesystem::path targetDir = "./Resources/" + challengeId;
    if (std::filesystem::exists(targetDir)) {
        std::filesystem::remove_all(targetDir);
    }

    return result.find("ERROR") == std::string::npos;
}

std::string Docker::getStatus() {
    std::string baseStatus = COrchestrator::getStatus();

    std::stringstream cmd;
    cmd << "docker inspect \"" << userId << "-" << resourceId << "-" << imageName << "\"";
    std::string result = executeCommand(cmd.str());

    std::stringstream status;
    status << baseStatus;
    status << "Container: " << userId << "-" << resourceId << "-" << imageName << "\n";
    status << "Port: " << port << "\n";
    status << "State: " << (result.find("\"Running\": true") != std::string::npos ? "Running" : "Stopped") << "\n";

    return status.str();
}

void Docker::setEnv(const std::string& key, const std::string& value) {
    if (!key.empty() && !value.empty()) {
        envVars[key] = value;
    }
}

void Docker::setPort(int portNum) {
    if (portNum > 0) {
        port = portNum;
    }
}

std::string Docker::getLogs() {
    std::stringstream cmd;
    cmd << "docker logs \"" << userId << "-" << resourceId << "-" << imageName << "\"";
    return executeCommand(cmd.str());
}


std::string Docker::getAddress() {
    std::stringstream cmd;
    cmd << "docker inspect \"" << userId << "-" << resourceId << "-" << imageName << "\"";
    std::string result = executeCommand(cmd.str());

    if (result.empty())
        return "unknown";

    try {
        json data = json::parse(result);
        // docker inspect returns a list of containers, so access the first
        std::string ip = data[0]["NetworkSettings"]["IPAddress"];

        // Clean the IP (just in case)
        ip.erase(std::remove(ip.begin(), ip.end(), '\n'), ip.end());

        if (ip.empty()) return "unknown";

        return ip + ":" + std::to_string(port);
    }
    catch (const std::exception& e) {
        return "unknown";
    }
}