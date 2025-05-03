#include "pch.h"
#include "Common.h"
#include <algorithm>

Common::Common(std::string username, std::string email, int id)
    : CUser(username, email, id), score(0), teamId(0)
{
    SetAccessLevel(1);  // utilizator standard
}

bool Common::challengeAlreadySolved(const std::string& challId) const
{
    return std::find(solvedChallenges.begin(), solvedChallenges.end(), challId) != solvedChallenges.end();
}

void Common::AddSolvedChallenge(const std::string& challId)
{
    if (!challengeAlreadySolved(challId))
    {
        solvedChallenges.push_back(challId);
    }
}

const std::vector<std::string>& Common::GetSolvedChallenges() const
{
    return solvedChallenges;
}

void Common::SetTeam(int teamId)
{
    this->teamId = teamId;
}

int Common::GetTeam() const
{
    return teamId;
}

void Common::AddScore(int points)
{
    score += points;
}

int Common::GetScore() const
{
    return score;
}