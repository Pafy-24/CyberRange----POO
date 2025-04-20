#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class UTILS_API Team {
public:
    virtual std::string GetId() = 0;
    virtual std::string GetName() = 0;
    virtual std::vector<CUser*> GetMembers() = 0;
    virtual void AddMember(CUser* user) = 0;
    virtual void RemoveMember(std::string userId) = 0;
    virtual int GetScore() = 0;
    virtual ~Team() = default;
};
