#include "pch.h"
#include "CTeam.h"
#include <algorithm>

CTeam::CTeam(std::string name) : name(name), logo(""), active(true)
{
    id = "TEAM_" + name;
}

std::string CTeam::GetId() 
{
    return id;
}

std::string CTeam::GetName() 
{
    return name;
}

std::vector<CUser*> CTeam::GetMembers() 
{
    return users;
}

void CTeam::AddMember(CUser* user) 
{
    if (user == nullptr)
        return;

    for (auto* u : users) {
        if (u->GetId() == user->GetId())
            return;
    }
    users.push_back(user);
}

void CTeam::RemoveMember(std::string userId) 
{
    users.erase(std::remove_if(users.begin(), users.end(), [&](CUser* user) {return user->GetId() == userId;}), users.end());
}

int CTeam::GetScore() 
{
    int total = 0;
    for (const auto& pair : stats) 
    {
        total += pair.second;
    }
    return total;
}

void CTeam::AddStats(std::string key, int value) 
{
    stats[key] += value;
}

std::map<std::string, int> CTeam::GetStats() 
{
    return stats;
}

void CTeam::SetLogo(std::string logoPath) 
{
    logo = logoPath;
}

std::string CTeam::GetLogo() 
{
    return logo;
}

void CTeam::SetActive(bool isActive) 
{
    active = isActive;
}
