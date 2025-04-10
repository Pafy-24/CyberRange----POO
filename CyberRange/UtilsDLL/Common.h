#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class Common : public CUser {
private:
    int score;
    std::vector<std::string> solvedChallenges;
    std::string teamId;

public:
    Common(std::string username, std::string email);
    void addSolvedChallenge(std::string challId);
    std::vector<std::string> getSolvedChallenges();
    void setTeam(std::string teamId);
    std::string getTeam();
    void addScore(int points);
    int getScore();
};
