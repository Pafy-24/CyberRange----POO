#pragma once
#include <string>
#include <map>
#include "CObs.h"
#include "Logger.h"

class LogObs : public CObs {
private:
    int criticalCount;
    int warningCount;
    int errorCount;

public:
    LogObs(Logger& logger);
    void trackLog(std::string logType, int level);
    int getCriticalCount();
    int getWarningCount();
    int getErrorCount();
    std::map<std::string, int> getLogSummary();
};
