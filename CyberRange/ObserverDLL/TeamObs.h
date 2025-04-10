#pragma once
#include <string>
#include <map>
#include <vector>
#include "CObs.h"
#include "Logger.h"

class TeamObs : public CObs {
private:
    std::map<std::string, int> teamScores;
    std::map<std::string, std::vector<std::string>> teamSolves;

public:
    TeamObs(Logger& logger);
    void trackScore(std::string teamId, int points);
    void trackSolve(std::string teamId, std::string challId);
    std::map<std::string, int> getTeamScores();
    std::vector<std::string> getTeamSolves(std::string teamId);
};
