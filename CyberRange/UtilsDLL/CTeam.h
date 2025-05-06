#pragma once
#include <string>
#include <vector>
#include <map>
#include "Team.h"
#include "CUser.h"

class UTILS_API CTeam : public Team {
private:
    int id;
    int LeaderID;
    int contestId;
    std::string name;
    std::vector<User*> users;
    std::map<std::string, int> stats;
    std::string logo;
    bool active;

public:
    CTeam(std::string name, int id = 0, int contestId = 0);
    int GetId() const override;
    int GetContestId() const override;
    std::string GetName() const override;
    std::vector<User*> GetMembers() const override;
    void SetLogo(std::string logoPath) override;
    std::string GetLogo() const override;
    void SetId(int id) override;
    void SetContestId(int contestId) override;
    void SetName(std::string name) override;
    void AddMember(User* user) override;
    void RemoveMember(int userId) override;
    int GetScore() const override;
    void AddStats(std::string key, int value) override;
    std::map<std::string, int> GetStats() const override;
    void SetActive(bool active) override;
    bool IsActive() const override;
	void SetLeader(int id) override;
	int GetLeader() const override;
    User* GetMember(int id) const override;

};