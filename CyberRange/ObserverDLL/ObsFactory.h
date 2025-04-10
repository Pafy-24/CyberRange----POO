#pragma once
#include <string>
#include "Observer.h"
#include "UserObs.h"
#include "SocketObs.h"
#include "ContestObs.h"
#include "ChallObs.h"
#include "TeamObs.h"
#include "LogObs.h"
#include "Logger.h"

class ObsFactory {
public:
    Observer* createObs(std::string type, Logger& logger);
    UserObs* createUserObs(Logger& logger);
    SocketObs* createSocketObs(Logger& logger);
    ContestObs* createContestObs(Logger& logger, std::string contestId);
    ChallObs* createChallObs(Logger& logger);
    TeamObs* createTeamObs(Logger& logger);
    LogObs* createLogObs(Logger& logger);
};
