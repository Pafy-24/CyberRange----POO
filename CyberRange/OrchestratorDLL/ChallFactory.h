#pragma once
#include <string>
#include "Chall.h"
#include "ChallTypes.h"

class ChallFactory {
public:
    Chall* createCryptoChall(std::string name, int difficulty);
    Chall* createPwnChall(std::string name, int difficulty);
    Chall* createWebChall(std::string name, int difficulty);
    Chall* createForensicsChall(std::string name, int difficulty);
    Chall* createMiscChall(std::string name, int difficulty);
};
