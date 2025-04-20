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

    void AddPermission(const std::string& perm);
    void RemovePermission(const std::string& perm);
    bool HasPermission(const std::string& perm) const;

    void SetAdminKey(const std::string& key);
    bool ValidateAdminKey(const std::string& key) const;

    const std::vector<std::string>& GetPermissions() const;
};