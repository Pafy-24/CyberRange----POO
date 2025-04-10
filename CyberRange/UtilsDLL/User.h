#pragma once
#include <string>

class User {
public:
    virtual std::string getId() = 0;
    virtual std::string getUsername() = 0;
    virtual bool authenticate(std::string password) = 0;
    virtual int getAccessLevel() = 0;
    virtual void setAccessLevel(int level) = 0;
    virtual ~User() = default;
};
