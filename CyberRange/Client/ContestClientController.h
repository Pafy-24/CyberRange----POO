#pragma once
#include <string>
#include "CController.h"

class ContestClientController : public CController {
public:
    ContestClientController();
    void listContests();
    void registerForContest(std::string contestId);
    void getScoreboard(std::string contestId);
};
