#pragma once
#include <string>
#include <vector>
#include "CController.h"
#include "CLogger.h"

class LogsController : public CController {
private:
    CLogger* logger;
    int maxEntries;

public:
    LogsController(CLogger& logger);
    void clearLogs(std::string type);
    void exportLogs(std::string format, std::string destination);
    bool rotateLogs();
};
