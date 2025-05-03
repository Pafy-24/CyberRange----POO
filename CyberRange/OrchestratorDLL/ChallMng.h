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
	ChallMng() = default;
	~ChallMng();
    // Challenge management
    void addChallenge(Chall* chall);
    void removeChallenge(int challId);
    Chall* getChallenge(int challId) const;
    std::vector<Chall*> getAllChallenges() const;
    std::vector<Chall*> getChallengesByType(ChallTypes type) const;
    void loadChallengesFromData(const std::vector<std::map<std::string, std::string>>& rows);

    // Contest management
    void addContest(Contest* contest);
    Contest* getContest(int contestId) const;
    void setCurrentContest(int contestId);
    std::string getCurrentContestId() const;

    // Tab management
    void addTab(Tab* tab);
    void removeTab(int tabId);
    Tab* getTab(int tabId) const;
    std::vector<Tab*> getAllTabs() const;

};
