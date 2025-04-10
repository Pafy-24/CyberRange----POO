#pragma once
#include <string>

class Logger {
public:
    virtual void log(std::string message, int level) = 0;
    virtual void setLogLevel(int level) = 0;
    virtual void setOutputFile(std::string path) = 0;
    virtual std::string getLastLog() = 0;
    virtual ~Logger() = default;
};
