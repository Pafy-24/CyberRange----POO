#pragma once
#include <string>
#include "ChallTypes.h"

class Chall {
public:
    virtual std::string getId() = 0;
    virtual std::string getName() = 0;
    virtual int getDifficulty() = 0;
    virtual ChallTypes getType() = 0;
    virtual std::string getText() = 0;
    virtual int getPoints() = 0;
    virtual ~Chall() = default;
};
