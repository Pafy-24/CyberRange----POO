#pragma once
#include <string>
#include <vector>
#include <map>
#include "Team.h"
#include "CUser.h"

class UTILS_API CTeam : public Team {
private:
    int id;
    int contestId;
    std::string name;
    std::vector<User*> users;
    std::map<std::string, int> stats;
    std::string logo;
    bool active;

public:
    CTeam(std::string name, int id = 0, int contestId = 0);
    int GetId() override;
    int GetContestId() override;
    std::string GetName() override;
    std::vector<User*> GetMembers() override;
    void SetLogo(std::string logoPath) override;
    std::string GetLogo() override;
    void SetId(int id) override;
    void SetContestId(int contestId) override;
    void SetName(std::string name) override;
    void AddMember(User* user) override;
    void RemoveMember(int userId) override;
    int GetScore() override;
    void AddStats(std::string key, int value);
    std::map<std::string, int> GetStats();
    void SetActive(bool active);
    bool IsActive() const;
};