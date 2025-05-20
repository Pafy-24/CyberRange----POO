#pragma once
#include "CController.h"
#include "Chall.h"
#include "DBController.h"
#include <map>
#include <string>

class ChallController : public CController {
private:

    Chall* getChallenge(int challId, int tabId, int contestId, Connection* client);
    bool updateLeaderboard(int challId, int tabId, int contestId, int userId, int teamId,
        const std::string& flag, int points, Connection* client);

public:
    ChallController();

    void handleRequest(const std::string& data, Connection* client) override;

};