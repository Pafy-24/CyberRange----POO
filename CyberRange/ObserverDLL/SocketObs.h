#pragma once
#include <string>
#include <map>
#include <ctime>
#include "CObs.h"
#include "Logger.h"

class SocketObs : public CObs {
private:
    std::map<std::string, time_t> connectionTimes;

public:
    SocketObs(Logger& logger);
    void trackConnection(std::string connId);
    void trackDisconnection(std::string connId);
    void trackDataTransfer(std::string connId, int bytes);
    std::map<std::string, time_t> getActiveConnections();
};
