#include "pch.h"
#include "CTeam.h"
#include <algorithm>

CTeam::CTeam(std::string name, int id, int contestId)
    : name(name), id(id), contestId(contestId), logo(""), active(true)
{
}

int CTeam::GetId()
{
    return id;
}

int CTeam::GetContestId()
{
    return contestId;
}

std::string CTeam::GetName()
{
    return name;
}

std::vector<User*> CTeam::GetMembers()
{
    return users;
}

void CTeam::SetLogo(std::string logoPath)
{
    logo = logoPath;
}

std::string CTeam::GetLogo()
{
    return logo;
}

void CTeam::SetId(int id)
{
    this->id = id;
}

void CTeam::SetContestId(int contestId)
{
    this->contestId = contestId;
}

void CTeam::SetName(std::string name)
{
    this->name = name;
}

void CTeam::AddMember(User* user)
{
    if (user == nullptr)
        return;

    for (auto* u : users) {
        if (u->GetId() == user->GetId())
            return;
    }
    users.push_back(user);
}

void CTeam::RemoveMember(int userId)
{
    users.erase(std::remove_if(users.begin(), users.end(),
        [userId](User* user) { return user->GetId() == userId; }), users.end());
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

void CTeam::SetActive(bool active)
{
    this->active = active;
}

bool CTeam::IsActive() const
{
    return active;
}