#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class Admin : public CUser {
private:
    std::vector<std::string> permissions;
    std::string adminKey;

public:
    Admin(std::string username, std::string email);
    void addPermission(std::string perm);
    void removePermission(std::string perm);
    bool hasPermission(std::string perm);
    void setAdminKey(std::string key);
    bool validateAdminKey(std::string key);
};
