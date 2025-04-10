#pragma once
#include <string>
#include <map>
#include <vector>
#include "CObs.h"
#include "Logger.h"

class ChallObs : public CObs {
private:
    std::map<std::string, int> solveAttempts;
    std::map<std::string, std::vector<std::string>> solvedBy;

public:
    ChallObs(Logger& logger);
    void trackAttempt(std::string challId, std::string userId);
    void trackSolve(std::string challId, std::string userId);
    std::map<std::string, int> getChallStats();
    std::vector<std::string> getSolvedUsers(std::string challId);
};
