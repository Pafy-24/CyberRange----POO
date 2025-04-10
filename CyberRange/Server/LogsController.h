#pragma once
#include <string>
#include <vector>
#include "CController.h"
#include "CLogger.h"
#include "LogEntry.h"

class LogsController : public CController {
private:
    CLogger* logger;
    int maxEntries;

public:
    LogsController(CLogger& logger);
    std::vector<LogEntry> getLogs(std::string type, int count);
    void clearLogs(std::string type);
    void exportLogs(std::string format, std::string destination);
    bool rotateLogs();
};
