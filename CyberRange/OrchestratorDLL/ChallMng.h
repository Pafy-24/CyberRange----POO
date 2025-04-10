#pragma once
#include <string>
#include <vector>
#include "Chall.h"
#include "ChallTypes.h"

class ChallMng {
public:
    virtual void addChall(Chall* chall) = 0;
    virtual void removeChall(std::string challId) = 0;
    virtual Chall* getChall(std::string challId) = 0;
    virtual std::vector<Chall*> getAllChalls() = 0;
    virtual std::vector<Chall*> getChallsByType(ChallTypes type) = 0;
    virtual ~ChallMng() = default;
};
