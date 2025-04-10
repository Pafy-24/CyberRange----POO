#pragma once
#include <string>
#include "User.h"

class CUser : public User {
private:
    std::string id;
    std::string username;
    std::string passwordHash;
    int accessLevel;
    std::string email;
    bool active;

public:
    CUser(std::string username, std::string email);
    std::string getId() override;
    std::string getUsername() override;
    bool authenticate(std::string password) override;
    int getAccessLevel() override;
    void setAccessLevel(int level) override;
    void setPassword(std::string password);
    std::string getEmail();
    void setActive(bool active);
};
