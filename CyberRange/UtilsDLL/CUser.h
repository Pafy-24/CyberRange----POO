#pragma once
#include <string>
#include "User.h"

class UTILS_API CUser : public User {
protected:
    std::string id;
    std::string username;
    std::string passwordHash;
	int accessLevel; // 0 = user simplu, 1 = common, 5 = writer, 1 = admin 
    std::string email;
    bool active;
    std::string hashPassword(const std::string& password) const;

public:
    CUser(std::string username, std::string email);
    std::string getId() override;
    std::string getUsername() override;
    bool authenticate(std::string password) override;
    int getAccessLevel() override;
    void setAccessLevel(int level) override;

    void setPassword(std::string password);
    std::string getEmail() const;
    void setActive(bool active);
    bool isActive() const;
};