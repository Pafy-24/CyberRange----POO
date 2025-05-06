#include "pch.h"
#include "CTeam.h"
#include <algorithm>

CTeam::CTeam(std::string name, int id, int contestId)
    : name(name), id(id), contestId(contestId), logo(""), active(true)
{
}

int CTeam::GetId() const
{
    return id;
}

int CTeam::GetContestId() const
{
    return contestId;
}

std::string CTeam::GetName() const
{
    return name;
}

std::vector<User*> CTeam::GetMembers() const
{
    return users;
}

void CTeam::SetLogo(std::string logoPath)
{
    logo = logoPath;
}

std::string CTeam::GetLogo() const
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

int CTeam::GetScore() const
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

std::map<std::string, int> CTeam::GetStats() const
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

void CTeam::SetLeader(int id)
{
	LeaderID = id;
}

int CTeam::GetLeader() const
{
	return LeaderID;
}

User* CTeam::GetMember(int id) const
{
    for (auto* mem : users) {
        if (mem->GetId() == id)return mem;
    }
    return nullptr;
}
