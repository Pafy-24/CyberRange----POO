#pragma once
#include <string>
#include "User.h"

class UTILS_API CUser : public User {
protected:
    int id;
    std::string username;
    std::string passwordHash;
    int accessLevel; // 0 = user simplu, 1 = common, 5 = writer, 10 = admin 
    std::string email;
    bool active;

public:
    CUser(std::string username, std::string email, int id = 0);
    CUser(std::string username, std::string email, int id, std::string role);
    int GetId() override;
    std::string GetUsername() override;
    std::string GetEmail() override;
    std::string GetPassword() override;
    void SetUsername(std::string username) override;
    void SetEmail(std::string email) override;
    void SetId(int id) override;
    int GetAccessLevel() override;
    void SetAccessLevel(int level) override;
    void SetPassword(const std::string& password) override;
    void SetActive(bool active);
    bool IsActive() const;
};