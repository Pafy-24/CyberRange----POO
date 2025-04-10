#pragma once
#include <string>
#include "CController.h"

class TeamClientController : public CController {
public:
    TeamClientController();
    void createTeam(std::string teamData);
    void joinTeam(std::string teamId);
    void leaveTeam();
};
