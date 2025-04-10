#pragma once
#include "CChallMng.h"
#include "Contest.h"
#include "Chall.h"
#include "Tab.h"

class ChallMngBuilder {
private:
    CChallMng challMng;

public:
    ChallMngBuilder();
    ChallMngBuilder& withContest(Contest* contest);
    ChallMngBuilder& withChall(Chall* chall);
    ChallMngBuilder& withTab(Tab* tab);
    CChallMng build();
};
