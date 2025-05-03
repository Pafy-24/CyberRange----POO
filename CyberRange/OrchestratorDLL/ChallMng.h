#pragma once
#include <string>
#include <vector>
#include <map>
#include "Chall.h"
#include "Contest.h"
#include "Tab.h"

class ORCHESTRATOR_API ChallMng {
private:
    std::vector<Chall*> challenges;
    std::map<int, Contest*> contests;
    std::vector<Tab*> tabs;
    std::string currentContestId;

public:
    ChallMng();
    ~ChallMng();

    void addChallenge(Chall* chall);
    void removeChallenge(int challId);
    Chall* getChallenge(int challId);
    std::vector<Chall*> getAllChallenges();
    std::vector<Chall*> getChallengesByType(ChallTypes type);

    void addContest(Contest* contest);
    Contest* getContest(int contestId);
    void setCurrentContest(int contestId);
    std::string getCurrentContestId() const;

    void addTab(Tab* tab);
    void removeTab(int tabId);
    Tab* getTab(int tabId);
    std::vector<Tab*> getAllTabs();
};