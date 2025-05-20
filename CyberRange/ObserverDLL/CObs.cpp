#include "pch.h"
#include "CObs.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

CObs::CObs(OutputType type, const std::string& fileName)
    : outputType(type), logFileName(fileName) {

    if (outputType == OutputType::FILE || outputType == OutputType::BOTH) {

		std::filesystem::path logFilePath(logFileName);
		if (!std::filesystem::exists(logFilePath.parent_path())) {
			std::filesystem::create_directories(logFilePath.parent_path());
		}

        logFile.open(logFileName, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFileName << std::endl;
            outputType = OutputType::CONSOLE;
        }
    }
}

CObs::~CObs() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void CObs::update(const std::string& message) {
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &time);

    std::stringstream timestamp;
    timestamp << "[" << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S") << "] ";

    std::string logMessage = timestamp.str() + message;

    // Output based on the configuration
    if (outputType == OutputType::CONSOLE || outputType == OutputType::BOTH) {
        std::cout << logMessage << std::endl;
    }

    if ((outputType == OutputType::FILE || outputType == OutputType::BOTH) && logFile.is_open()) {
        logFile << logMessage << std::endl;
        logFile.flush();
    }
}

void CObs::setOutputType(OutputType type) {
    outputType = type;

    // Open log file if needed
    if ((type == OutputType::FILE || type == OutputType::BOTH) && !logFile.is_open()) {
        logFile.open(logFileName, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFileName << std::endl;
            outputType = OutputType::CONSOLE;
        }
    }
    // Close log file if not needed
    else if (type == OutputType::CONSOLE && logFile.is_open()) {
        logFile.close();
    }
}

void CObs::setLogFileName(const std::string& fileName) {
    if (logFile.is_open()) {
        logFile.close();
    }

    logFileName = fileName;

    if (outputType == OutputType::FILE || outputType == OutputType::BOTH) {
        logFile.open(logFileName, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Failed to open log file: " << logFileName << std::endl;
            outputType = OutputType::CONSOLE;
        }
    }
}