#include"pch.h"
#include "ChallMng.h"
#include <algorithm>

ChallMng::ChallMng() : currentContestId("") {}

ChallMng::~ChallMng() {
    for (auto* chall : challenges) delete chall;
    for (auto& pair : contests) delete pair.second;
    for (auto* tab : tabs) delete tab;
}

void ChallMng::addChallenge(Chall* chall) {
    if (chall && std::find(challenges.begin(), challenges.end(), chall) == challenges.end()) {
        challenges.push_back(chall);
    }
}

void ChallMng::removeChallenge(int challId) {
    challenges.erase(
        std::remove_if(challenges.begin(), challenges.end(),
            [&challId](const Chall* chall) { return chall->getId() == challId; }),
        challenges.end());
}

Chall* ChallMng::getChallenge(int challId) {
    auto it = std::find_if(challenges.begin(), challenges.end(),
        [&challId](const Chall* chall) { return chall->getId() == challId; });
    return it != challenges.end() ? *it : nullptr;
}

std::vector<Chall*> ChallMng::getAllChallenges() {
    return challenges;
}

std::vector<Chall*> ChallMng::getChallengesByType(ChallTypes type) {
    std::vector<Chall*> result;
    for (auto* chall : challenges) {
        auto types = chall->getTypes();
        if (std::find(types.begin(), types.end(), type) != types.end()) {
            result.push_back(chall);
        }
    }
    return result;
}

void ChallMng::addContest(Contest* contest) {
    if (contest) {
        contests[contest->getId()] = contest;
    }
}

Contest* ChallMng::getContest(int contestId) {
    auto it = contests.find(contestId);
    return it != contests.end() ? it->second : nullptr;
}

void ChallMng::setCurrentContest(int contestId) {
    if (contests.find(contestId) != contests.end()) {
        currentContestId = contestId;
    }
}

std::string ChallMng::getCurrentContestId() const {
    return currentContestId;
}

void ChallMng::addTab(Tab* tab) {
    if (tab && std::find(tabs.begin(), tabs.end(), tab) == tabs.end()) {
        tabs.push_back(tab);
    }
}

void ChallMng::removeTab(int tabId) {
    tabs.erase(
        std::remove_if(tabs.begin(), tabs.end(),
            [&tabId](const Tab* tab) { return tab->getId() == tabId; }),
        tabs.end());
}

Tab* ChallMng::getTab(int tabId) {
    auto it = std::find_if(tabs.begin(), tabs.end(),
        [&tabId](const Tab* tab) { return tab->getId() == tabId; });
    return it != tabs.end() ? *it : nullptr;
}

std::vector<Tab*> ChallMng::getAllTabs() {
    return tabs;
}