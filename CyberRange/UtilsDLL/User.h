#pragma once
#include <string>
#include "DLL.h"

class UTILS_API User {
public:
    virtual int GetId() = 0;
    virtual std::string GetUsername() = 0;
    virtual std::string GetEmail() = 0;
    virtual void SetUsername(std::string username) = 0;
    virtual void SetEmail(std::string email) = 0;
    virtual void SetId(int id) = 0;

    virtual int GetAccessLevel() = 0;
    virtual void SetAccessLevel(int level) = 0;
    virtual ~User() = default;

    virtual void SetPassword(const std::string& hash) = 0;
    virtual std::string GetPassword() = 0;
    virtual void SetActive(bool active) = 0;
    virtual bool IsActive() const = 0;
};