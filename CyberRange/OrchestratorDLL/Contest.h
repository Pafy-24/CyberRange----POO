#pragma once
#include <string>
#include <vector>
#include <ctime>
#include "CChallMng.h"

class Contest: public CChallMng {
private:
    std::string id;
    std::string name;
    time_t startTime;
    time_t endTime;
    std::vector<std::string> challIds;
    std::vector<std::string> teamIds;
    bool active;

public:
    Contest(std::string name);
    std::string getId();
    std::string getName();
    time_t getStartTime();
    time_t getEndTime();
    void setStartTime(time_t start);
    void setEndTime(time_t end);
    void addChallenge(std::string challId);
    void removeChallenge(std::string challId);
    std::vector<std::string> getChallenges();
    void addTeam(std::string teamId);
    void removeTeam(std::string teamId);
    std::vector<std::string> getTeams();
    void setActive(bool active);
    bool isActive();
};
