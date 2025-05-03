#include "pch.h"
#include "CLogger.h"
#include <iostream>

void CLogger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << "[LOG] " << message << std::endl;

    for (const auto& observer : observers) {
        observer(message);
    }
}

void CLogger::attachObserver(std::function<void(const std::string&)> observer) {
    std::lock_guard<std::mutex> lock(logMutex);
    observers.push_back(observer);
}