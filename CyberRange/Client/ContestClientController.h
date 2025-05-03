#pragma once
#include "CController.h"
#include <string>

class ContestClientController : public CController {
public:
    ContestClientController();

    void requestContestList();
    void requestScoreboard(const std::string& contestId);

    void handleServerResponse(const std::string& responseStr) override;
};