#pragma once
#include <string>
#include <vector>
#include <map>
#include "Chall.h"
#include "ChallTypes.h"

class ORCHESTRATOR_API ChallFactory {
public:
    // creeaza un challenge dintr-un rand primit de la server
    static Chall* CreateFromRow(const std::map<std::string, std::string>& row);

    // creeaza o lista de challenge-uri dintr-un vector de randuri
    static std::vector<Chall*> CreateFromList(const std::vector<std::map<std::string, std::string>>& rows);
};