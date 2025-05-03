#pragma once
#include <string>
#include <vector>
#include "User.h"

class UTILS_API Team {
public:
    virtual int GetId() = 0;
    virtual int GetContestId() = 0;
    virtual std::string GetName() = 0;
    virtual std::vector<User*> GetMembers() = 0;
    virtual void SetLogo(std::string logoPath) = 0;
    virtual std::string GetLogo() = 0;
    virtual void SetId(int id) = 0;
    virtual void SetContestId(int contestId) = 0;
    virtual void SetName(std::string name) = 0;
    virtual void AddMember(User* user) = 0;
    virtual void RemoveMember(int userId) = 0;
    virtual int GetScore() = 0;
    virtual ~Team() = default;
};