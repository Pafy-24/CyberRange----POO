#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class UTILS_API Common : public CUser {
private:
    int score;
    std::vector<std::string> solvedChallenges;
    std::string teamId;

    bool challengeAlreadySolved(const std::string& challId) const;

public:
    Common(std::string username, std::string email);

    void AddSolvedChallenge(const std::string& challId);
    const std::vector<std::string>& GetSolvedChallenges() const;

    void SetTeam(const std::string& teamId);
    std::string GetTeam() const;

    void AddScore(int points);
    int GetScore() const;
};