#pragma once
#include <string>
#include <vector>
#include <map>
#include "Team.h"
#include "CUser.h"

class UTILS_API CTeam : public Team {
private:
    std::string id;
    std::string name;
    std::vector<CUser*> users;
    std::map<std::string, int> stats;
    std::string logo;
    bool active;

public:
    CTeam(std::string name);
    std::string GetId() override;
    std::string GetName() override;
    std::vector<CUser*> GetMembers() override;
    void AddMember(CUser* user) override;
    void RemoveMember(std::string userId) override;
    int GetScore() override;
    void AddStats(std::string key, int value);
    std::map<std::string, int> GetStats();
    void SetLogo(std::string logoPath);
    std::string GetLogo();
    void SetActive(bool active);
};
