#pragma once
#include "DLL.h"
#include <string>
#include <mutex>
#include <vector>
#include <functional>

class UTILS_API Logger {
public:
    virtual void log(const std::string& message)=0;
    virtual void attachObserver(std::function<void(const std::string&)> observer)=0;
};
