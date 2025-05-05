#include "pch.h"
#include "UsersFactory.h"

std::unique_ptr<User> UsersFactory::CreateUser(const std::string& username, const std::string& email, int id)
{
	return std::make_unique<Common>(username, email, id);
}
std::unique_ptr<User> UsersFactory::CreateAdmin(const std::string& username, const std::string& email,int id)
{
	return std::make_unique<Admin>(username, email,id);
}
std::unique_ptr<User> UsersFactory::CreateWriter(const std::string& username, const std::string& email, int id)
{
	return std::make_unique<Writer>(username, email, id);
}


std::unique_ptr<Team> UsersFactory::CreateTeam(const std::string& name, int contestId, int id)
{
    return std::make_unique<CTeam>(name, id, contestId);
}
