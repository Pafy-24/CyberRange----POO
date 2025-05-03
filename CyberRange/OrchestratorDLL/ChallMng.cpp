#include"pch.h"
#include "ChallMng.h"
#include <algorithm>
#include "ChallFactory.h"

ChallMng::~ChallMng() 
{
    for (Chall* chall : challenges) 
        delete chall;
    for (auto& pair : contests) 
        delete pair.second;
    for (Tab* tab : tabs) 
        delete tab;
}

void ChallMng::addChallenge(Chall* chall)
{
    if (chall && std::find(challenges.begin(), challenges.end(), chall) == challenges.end()) 
    {
        challenges.push_back(chall);
    }
}

void ChallMng::removeChallenge(int challId) 
{
    auto it = std::remove_if(challenges.begin(), challenges.end(),
        [=](Chall* c) { return c->getId() == challId; });
    if (it != challenges.end()) 
    {
        delete* it;
        challenges.erase(it, challenges.end());
    }
}

Chall* ChallMng::getChallenge(int challId) const 
{
    auto it = std::find_if(challenges.begin(), challenges.end(),
        [=](const Chall* c) { return c->getId() == challId; });
    return it != challenges.end() ? *it : nullptr;
}

std::vector<Chall*> ChallMng::getAllChallenges() const 
{
    return challenges;
}

std::vector<Chall*> ChallMng::getChallengesByType(ChallTypes type) const 
{
    std::vector<Chall*> result;
    for (Chall* c : challenges) 
    {
        const auto& types = c->getTypes();
        if (std::find(types.begin(), types.end(), type) != types.end()) 
        {
            result.push_back(c);
        }
    }
    return result;
}

void ChallMng::loadChallengesFromData(const std::vector<std::map<std::string, std::string>>& rows) 
{
    auto loaded = ChallFactory::CreateFromList(rows);
    for (Chall* c : loaded) 
    {
        addChallenge(c);
    }
}

void ChallMng::addContest(Contest* contest) 
{
    if (contest) 
    {
        contests[contest->getId()] = contest;
    }
}

Contest* ChallMng::getContest(int contestId) const 
{
    auto it = contests.find(contestId);
    return it != contests.end() ? it->second : nullptr;
}

void ChallMng::setCurrentContest(int contestId) 
{
    if (contests.count(contestId))
    {
        currentContestId = std::to_string(contestId);
    }
}

std::string ChallMng::getCurrentContestId() const 
{
    return currentContestId;
}

void ChallMng::addTab(Tab* tab)
{
    if (tab && std::find(tabs.begin(), tabs.end(), tab) == tabs.end()) 
    {
        tabs.push_back(tab);
    }
}

void ChallMng::removeTab(int tabId) 
{
    auto it = std::remove_if(tabs.begin(), tabs.end(),
        [=](Tab* t) { return t->getId() == tabId; });
    if (it != tabs.end()) 
    {
        delete* it;
        tabs.erase(it, tabs.end());
    }
}

Tab* ChallMng::getTab(int tabId) const
{
    auto it = std::find_if(tabs.begin(), tabs.end(),
        [=](const Tab* t) { return t->getId() == tabId; });
    return it != tabs.end() ? *it : nullptr;
}

std::vector<Tab*> ChallMng::getAllTabs() const 
{
    return tabs;
}