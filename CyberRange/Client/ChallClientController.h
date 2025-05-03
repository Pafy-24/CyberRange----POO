#pragma once
#include "CController.h"
#include <string>

class ChallClientController : public CController {
public:
    ChallClientController();

    void requestChallengeList();
    void requestChallengeDetails(const std::string& challId);
    void submitFlag(const std::string& challId, const std::string& flag);
    void handleServerResponse(const std::string& response) override;
};