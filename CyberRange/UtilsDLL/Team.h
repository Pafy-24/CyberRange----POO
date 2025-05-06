#pragma once
#include <string>
#include <vector>
#include "User.h"

class UTILS_API Team {
public:
    virtual int GetId() const = 0;
    virtual int GetContestId() const = 0;
    virtual std::string GetName() const = 0;
    virtual std::vector<User*> GetMembers() const = 0;
    virtual void SetLogo(std::string logoPath) = 0;
    virtual std::string GetLogo() const = 0;
    virtual void SetId(int id) = 0;
    virtual void SetContestId(int contestId) = 0;
    virtual void SetName(std::string name) = 0;
    virtual void AddMember(User* user) = 0;
    virtual void RemoveMember(int userId) = 0;
    virtual int GetScore() const = 0;
    virtual void AddStats(std::string key, int value) = 0;
    virtual std::map<std::string, int> GetStats() const = 0;
    virtual void SetActive(bool active) = 0;
    virtual bool IsActive() const = 0;
    virtual void SetLeader(int id) = 0;
    virtual int GetLeader() const = 0;
    virtual User* GetMember(int id) const = 0;
    virtual ~Team() = default;
};