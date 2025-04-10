#pragma once
#include <string>
#include <vector>
#include <map>
#include "ChallMng.h"
#include "Contest.h"

class CChallMng : public ChallMng {
private:
    std::map<std::string, Contest*> contests;
    std::vector<Chall*> challs;
    std::string currentContestId;

public:
    CChallMng();
    void addChall(Chall* chall) override;
    void removeChall(std::string challId) override;
    Chall* getChall(std::string challId) override;
    std::vector<Chall*> getAllChalls() override;
    std::vector<Chall*> getChallsByType(ChallTypes type) override;
    void addContest(Contest* contest);
    Contest* getContest(std::string contestId);
    void setCurrentContest(std::string contestId);
    std::string getCurrentContestId();
};
