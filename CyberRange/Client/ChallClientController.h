#pragma once
#include <string>
#include "CController.h"

class ChallClientController : public CController {
public:
    ChallClientController();
    void requestChallengeList();
    void requestChallengeDetails(std::string challId);
    void submitFlag(std::string challId, std::string flag);
};
