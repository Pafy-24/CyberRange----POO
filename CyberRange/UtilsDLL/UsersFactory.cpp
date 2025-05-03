#include "pch.h"
#include "UsersFactory.h"

std::unique_ptr<User> UsersFactory::CreateUser(UserType type, const std::string& username, const std::string& email, int id)
{

    switch (type) {
    case UserType::BASIC:
        return std::make_unique<CUser>(username, email, id);
    case UserType::COMMON:
        return std::make_unique<Common>(username, email, id);
    case UserType::WRITER:
        return std::make_unique<Writer>(username, email, id);
    case UserType::ADMIN:
        return std::make_unique<Admin>(username, email, id);
    default:
        return nullptr;
    }
}

std::unique_ptr<Team> UsersFactory::CreateTeam(const std::string& name, int contestId, int id)
{
    return std::make_unique<CTeam>(name, id, contestId);
}
