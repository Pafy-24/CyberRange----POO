#pragma once
#include <string>
#include <map>
#include <vector>
#include "CController.h"
#include "Team.h"
#include "UserController.h"

class TeamController : public CController {
private:
    std::map<std::string, Team*> teams;
    UserController& userCtrl;

public:
    TeamController(UserController& userController);
    void createTeam(std::string teamData);
    void updateTeam(std::string teamId, std::string teamData);
    void deleteTeam(std::string teamId);
    Team* getTeam(std::string teamId);
    bool addUserToTeam(std::string userId, std::string teamId);
    bool removeUserFromTeam(std::string userId, std::string teamId);
    std::vector<Team*> listTeams();
};
