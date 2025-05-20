#pragma once
#include <map>
#include <string>
#include "User.h"
#include "DLL.h"

class ORCHESTRATOR_API UserMng 
{
private:
    std::map<int, User*> users; 

public:
    UserMng() = default;
    ~UserMng();

    void addUser(User* user);
    void removeUser(int userId);
    User* getUser(int userId) const;
    std::map<int, User*> getAllUsers() const;
    void clear();
};
