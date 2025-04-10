#pragma once
#include <string>
#include <vector>
#include <utility>
#include <ctime>

class Scoreboard {
private:
    std::string contestId;
    std::vector<std::pair<std::string, int>> teamScores;
    int updateInterval;
    time_t lastUpdate;

public:
    Scoreboard(std::string contestId);
    void update();
    std::vector<std::pair<std::string, int>> getScores();
    std::pair<std::string, int> getTopTeam();
    void setUpdateInterval(int seconds);
    time_t getLastUpdateTime();
    std::string exportJSON();
};
