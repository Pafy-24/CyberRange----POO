#pragma once
#include <string>
#include <ctime>
#include "CObs.h"
#include "Logger.h"

class ContestObs : public CObs {
private:
    std::string contestId;
    time_t startTime;

public:
    ContestObs(Logger& logger, std::string contestId);
    void trackStart();
    void trackEnd();
    void trackTeamRegistration(std::string teamId);
    double getElapsedTime();
};
