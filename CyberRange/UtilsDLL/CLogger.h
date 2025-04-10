#pragma once
#include <string>
#include "Logger.h"

class CLogger : public Logger {
private:
    int logLevel;
    std::string logFile;
    bool console;
    std::string dateFormat;

public:
    CLogger(std::string file, int level);
    void log(std::string message, int level) override;
    void setLogLevel(int level) override;
    void setOutputFile(std::string path) override;
    std::string getLastLog() override;
    void enableConsole(bool enable);
    void setDateFormat(std::string format);
};
