#pragma once
#include <string>
#include <map>
#include <vector>
#include "CController.h"
#include "Contest.h"
#include "ChallMng.h"
#include "TeamController.h"
#include "Scoreboard.h"

class ContestController : public CController {
private:
    std::map<std::string, Contest*> contests;
	DBController* dbController;

public:
    ContestController(DBController* dbCtrl);
    void createContest(std::string contestData);
	void handleRequest(const std::string& data, Connection* client);
    void updateContest(std::string contestId, std::string contestData);
    void deleteContest(std::string contestId);
    Contest* getContest(std::string contestId);
    Scoreboard* getScoreboard(std::string contestId);
    void loadContest(const std::string& contestId);
    std::map<std::string, Contest*>& getContests() { return contests; }
    DBController* getDB();
};
