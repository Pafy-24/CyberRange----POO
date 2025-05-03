#pragma once
#include "CController.h"
#include "Team.h"
#include "DBController.h"

class TeamController : public CController {
private:
    DBController* dbController;
    std::map<std::string, Team*> teams;

public:
    TeamController(DBController* dbCtrl);
    void handleRequest(const std::string& data, Connection* client) override;
    void loadTeam(const std::string& teamId);
    void unloadTeam(const std::string& teamId);
	DBController* getDB() const { return dbController; }
};