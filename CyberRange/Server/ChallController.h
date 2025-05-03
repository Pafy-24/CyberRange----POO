#pragma once
#include "CController.h"
#include "Chall.h"
#include "DBController.h"

class ChallController : public CController {
private:
    DBController* dbController;
    std::map<std::string, Chall*> challenges;

public:
    ChallController(DBController* dbCtrl);
    void handleRequest(const std::string& data, Connection* client) override;
    void loadChallenge(const std::string& challId);
    void unloadChallenge(const std::string& challId);
};