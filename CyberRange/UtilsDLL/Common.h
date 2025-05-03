#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class UTILS_API Common : public CUser {
private:
    int score;
    std::vector<std::string> solvedChallenges;
    int teamId;

    bool challengeAlreadySolved(const std::string& challId) const;

public:
    Common(std::string username, std::string email, int id = 0);
    void AddSolvedChallenge(const std::string& challId);
    const std::vector<std::string>& GetSolvedChallenges() const;
    void SetTeam(int teamId);
    int GetTeam() const;
    void AddScore(int points);
    int GetScore() const;
};