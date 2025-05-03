#pragma once
#include <string>
#include <mutex>
#include <vector>
#include <functional>
#include "Logger.h"

class UTILS_API CLogger : public Logger {
private:
    std::mutex logMutex;
    std::vector<std::function<void(const std::string&)>> observers;

public:
    void log(const std::string& message) override;
    void attachObserver(std::function<void(const std::string&)> observer) override;
};