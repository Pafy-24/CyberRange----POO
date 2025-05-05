#pragma once
#include <string>
#include <memory>
#include "CUser.h"
#include "Common.h"
#include "Writer.h"
#include "Admin.h"
#include "CTeam.h"


class UTILS_API UsersFactory {
private:

public:

    static std::unique_ptr<User> CreateUser(const std::string& username, const std::string& email, int id);
    static std::unique_ptr<User> CreateAdmin(const std::string& username, const std::string& email, int id);
    static std::unique_ptr<User> CreateWriter(const std::string& username, const std::string& email, int id);
    static std::unique_ptr<Team> CreateTeam(const std::string& name, int contestId = 0, int id = 0);

};