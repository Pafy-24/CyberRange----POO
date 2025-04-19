#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class UTILS_API Admin : public CUser {
private:
    std::vector<std::string> permissions;
    std::string adminKeyHash;

    std::string hashKey(const std::string& key) const;

public:
    Admin(std::string username, std::string email);

    void addPermission(const std::string& perm);
    void removePermission(const std::string& perm);
    bool hasPermission(const std::string& perm) const;

    void setAdminKey(const std::string& key);
    bool validateAdminKey(const std::string& key) const;

    const std::vector<std::string>& getPermissions() const;
};