#pragma once
#include <string>
#include "DLL.h"
#include <iostream>

class UTILS_API User {
public:
    virtual std::string GetId() = 0;
    virtual std::string GetUsername() = 0;
    virtual std::string GetEmail() = 0;
    virtual bool Authenticate(std::string password) = 0;
    virtual int GetAccessLevel() = 0;
    virtual void SetAccessLevel(int level) = 0;
    virtual ~User() = default;
};
