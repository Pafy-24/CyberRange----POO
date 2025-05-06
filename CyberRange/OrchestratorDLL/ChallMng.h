#pragma once
#include <string>
#include <vector>
#include <map>
#include "Chall.h"
#include "Contest.h"
#include "Tab.h"

class ORCHESTRATOR_API ChallMng {
private:
    std::map<int, Contest*> contests;
    std::map<int, Tab*> tabs;
	std::map<int, int, Chall*> challenges; 
public:
	ChallMng() = default;
    ~ChallMng();

    void addContest(Contest* contest);
    void addChallenge(int contestId, int challId, Chall* challenge);
	void removeContest(int contestId);
    Contest* getContest(int contestId) const;
	std::map<int, Contest*> getAllContests() const;

    void addTab(Tab* tab);
    void removeTab(int tabId);
    Tab* getTab(int tabId) const;
	std::map<int, Tab*> getAllTabs() const;

};
