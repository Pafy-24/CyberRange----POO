#include "pch.h"
#include "Common.h"
#include <algorithm>

Common::Common(std::string username, std::string email) : CUser(username, email), score(0), teamId("") 
{
    SetAccessLevel(1);  // utilizator standard
}

bool Common::challengeAlreadySolved(const std::string& challId) const 
{
    return std::find(solvedChallenges.begin(), solvedChallenges.end(), challId) != solvedChallenges.end();
}

void Common::AddSolvedChallenge(const std::string& challId) {

    if (!challengeAlreadySolved(challId)) 
    {
        solvedChallenges.push_back(challId);
    }
}

const std::vector<std::string>& Common::GetSolvedChallenges() const 
{
    return solvedChallenges;
}

void Common::SetTeam(const std::string& teamId) 
{
    this->teamId = teamId;
}

std::string Common::GetTeam() const 
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