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
    void createContest(const std::string& contestData, Connection* conn);
    void updateContest(const std::string& contestId, const std::string& contestData, Connection* client);
    void deleteContest(const std::string& contestId, Connection* client);
	void handleRequest(const std::string& data, Connection* client);
    void sendContestDetails(int contestId, Connection* client);
    void sendScoreboard(int contestId, Connection* client);
    void sendAllContests(Connection* client);
    std::map<std::string, Contest*>& getContests() { return contests; }
};
