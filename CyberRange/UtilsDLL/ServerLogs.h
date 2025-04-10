#pragma once
#include <string>
#include <vector>
#include "CLogger.h"
class ServerLogs: public CLogger {
private:
    std::string serverName;
    bool appendTimestamp;

public:
    ServerLogs(std::string file, std::string server);
    void logServerEvent(std::string event, int severity);
    std::vector<std::string> getServerEvents(std::string filter);
    void archiveLogs(std::string destination);
};
