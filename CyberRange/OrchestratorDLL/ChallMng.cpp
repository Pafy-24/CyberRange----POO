#include "ChallMng.h"

ChallMng::~ChallMng() {
    for (auto& pair : contests) {
        delete pair.second;
    }
    contests.clear();

    for (auto& pair : tabs) {
        delete pair.second;
    }
    tabs.clear();
}

void ChallMng::addContest(Contest* contest) {
    if (contest) {
        contests[contest->getId()] = contest;
    }
}

void ChallMng::removeContest(int contestId) {
    auto it = contests.find(contestId);
    if (it != contests.end()) {
        delete it->second;
        contests.erase(it);
    }
}

Contest* ChallMng::getContest(int contestId) const {
    auto it = contests.find(contestId);
    if (it != contests.end()) {
        return it->second;
    }
    return nullptr;
}

std::map<int, Contest*> ChallMng::getAllContests() const {
    return contests;
}

void ChallMng::addTab(Tab* tab) {
    if (tab) {
        tabs[tab->getId()] = tab;
    }
}

void ChallMng::removeTab(int tabId) {
    auto it = tabs.find(tabId);
    if (it != tabs.end()) {
        delete it->second;
        tabs.erase(it);
    }
}

Tab* ChallMng::getTab(int tabId) const {
    auto it = tabs.find(tabId);
    if (it != tabs.end()) {
        return it->second;
    }
    return nullptr;
}

std::map<int, Tab*> ChallMng::getAllTabs() const {
    return tabs;
}