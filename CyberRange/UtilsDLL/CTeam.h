#pragma once
#include <string>
#include <vector>
#include <map>
#include "Team.h"
#include "CUser.h"

class CTeam : public Team {
private:
    std::string id;
    std::string name;
    std::vector<CUser*> users;
    std::map<std::string, int> stats;
    std::string logo;
    bool active;

public:
    CTeam(std::string name);
    std::string getId() override;
    std::string getName() override;
    std::vector<User*> getMembers() override;
    void addMember(User* user) override;
    void removeMember(std::string userId) override;
    int getScore() override;
    void addStats(std::string key, int value);
    std::map<std::string, int> getStats();
    void setLogo(std::string logoPath);
    std::string getLogo();
    void setActive(bool active);
};
