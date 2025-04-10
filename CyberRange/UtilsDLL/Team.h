#pragma once
#include <string>
#include <vector>
#include "User.h"

class Team {
public:
    virtual std::string getId() = 0;
    virtual std::string getName() = 0;
    virtual std::vector<User*> getMembers() = 0;
    virtual void addMember(User* user) = 0;
    virtual void removeMember(std::string userId) = 0;
    virtual int getScore() = 0;
    virtual ~Team() = default;
};
