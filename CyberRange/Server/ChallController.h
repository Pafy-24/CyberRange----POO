#pragma once
#include "CController.h"
#include "Chall.h"
#include "DBController.h"

class ChallController : public CController {
private:
    std::map<std::string, Chall*> challenges;

public:
    ChallController();
    void handleRequest(const std::string& data, Connection* client) override;
    void loadChallenge(const std::string& challId);
    void unloadChallenge(const std::string& challId);
};