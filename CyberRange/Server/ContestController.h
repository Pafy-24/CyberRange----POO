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
    ChallMng& challMng;
    TeamController& teamCtrl;

public:
    ContestController(ChallMng& challMng, TeamController& teamCtrl);
    void createContest(std::string contestData);
    void updateContest(std::string contestId, std::string contestData);
    void deleteContest(std::string contestId);
    Contest* getContest(std::string contestId);
    bool addTeamToContest(std::string teamId, std::string contestId);
    bool removeTeamFromContest(std::string teamId, std::string contestId);
    std::vector<Contest*> listContests();
    Scoreboard* getScoreboard(std::string contestId);
};
