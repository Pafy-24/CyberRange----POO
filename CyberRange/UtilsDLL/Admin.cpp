#include "pch.h"
#include "Admin.h"

std::string Admin::hashKey(const std::string& key) const
{
    return std::string();
}

Admin::Admin(std::string username, std::string email) : CUser(username, email)
{
	id = "ADM_" + username;
	setAccessLevel(10);
}

void Admin::addPermission(const std::string& perm)
{
	permissions.push_back(perm);
	// Optionally, you can also check for duplicates before adding
	auto it = std::find(permissions.begin(), permissions.end(), perm);
	if (it == permissions.end()) 
	{
		permissions.push_back(perm);
	}
	else 
	{
		// Handle duplicate permission case if needed
	}
}

void Admin::removePermission(const std::string& perm)
{
}

bool Admin::hasPermission(const std::string& perm) const
{
	return false;
}

void Admin::setAdminKey(const std::string& key)
{
}

bool Admin::validateAdminKey(const std::string& key) const
{
	return false;
}

const std::vector<std::string>& Admin::getPermissions() const
{
	return permissions;
}
