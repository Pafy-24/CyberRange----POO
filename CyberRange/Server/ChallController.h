#pragma once
#include <string>
#include "CController.h"
#include "ChallMng.h"
#include "Chall.h"

class ChallController : public CController {
private:
    ChallMng* challMng;

public:
    ChallController(ChallMng& mng);
    void createChallenge(std::string params);
    void updateChallenge(std::string id, std::string params);
    void deleteChallenge(std::string id);
    void listChallenges(std::string contestId);
    Chall* getChallenge(std::string id);
};
