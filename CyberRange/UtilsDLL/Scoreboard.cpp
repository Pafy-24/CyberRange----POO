#include "pch.h"
#include "Scoreboard.h"

Scoreboard::Scoreboard(std::string contestId)
{
}

void Scoreboard::update()
{
}

std::vector<std::pair<std::string, int>> Scoreboard::getScores()
{
    return std::vector<std::pair<std::string, int>>();
}

std::pair<std::string, int> Scoreboard::getTopTeam()
{
    return std::pair<std::string, int>();
}

void Scoreboard::setUpdateInterval(int seconds)
{
}

time_t Scoreboard::getLastUpdateTime()
{
    return time_t();
}

std::string Scoreboard::exportJSON()
{
    return std::string();
}

void Scoreboard::addScore(const std::string& teamId, int score)
{
    for (auto& pair : teamScores) 
    {
        if (pair.first == teamId) 
        {
            pair.second = score;  // actualizare scor existent
            return;
        }
    }
    teamScores.emplace_back(teamId, score);
}
