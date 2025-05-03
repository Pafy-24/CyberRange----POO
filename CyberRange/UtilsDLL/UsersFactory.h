#pragma once
#include <string>
#include <memory>
#include "CUser.h"
#include "Common.h"
#include "Writer.h"
#include "Admin.h"
#include "CTeam.h"

enum class UserType {
    BASIC,
    COMMON,
    WRITER,
    ADMIN
};

class UTILS_API UsersFactory {
private:

public:

    static std::unique_ptr<User> CreateUser(UserType type, const std::string& username, const std::string& email, int id = 0);
    static std::unique_ptr<Team> CreateTeam(const std::string& name, int contestId = 0, int id = 0);

};