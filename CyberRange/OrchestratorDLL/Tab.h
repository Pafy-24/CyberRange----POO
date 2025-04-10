#pragma once
#include <string>
#include <vector>
#include "CChallMng.h"

class Tab: public CChallMng {
private:
    std::string id;
    std::string name;
    std::vector<std::string> challIds;
    bool visible;

public:
    Tab(std::string name);
    std::string getId();
    std::string getName();
    void addChallenge(std::string challId);
    void removeChallenge(std::string challId);
    std::vector<std::string> getChallenges();
    void setVisible(bool visible);
    bool isVisible();
};
