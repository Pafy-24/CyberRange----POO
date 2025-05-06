#pragma once
#include <string>
#include <vector>
#include <map>
#include "Chall.h"
#include "ChallTypes.h"

class ORCHESTRATOR_API ChallFactory {
public:
    static Chall* CreateFromRow(const std::map<std::string, std::string>& row);
    static std::vector<Chall*> CreateFromList(const std::vector<std::map<std::string, std::string>>& rows);
 //   static Chall* CreateChall(const std::string& name,const std::string& desc, const std::string& types, const std::string& folder);

};