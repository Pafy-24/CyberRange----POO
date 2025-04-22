#include "pch.h"
#include "Admin.h"
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

std::string Admin::hashKey(const std::string& key) const
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256(reinterpret_cast<const unsigned char*>(key.c_str()), key.size(), hash);

	std::stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
		ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

	return ss.str();
}

Admin::Admin(std::string username, std::string email) : CUser(username, email)
{
	id = "ADM_" + username;
	SetAccessLevel(10);
}

void Admin::AddPermission(const std::string& perm)
{
	auto it = std::find(permissions.begin(), permissions.end(), perm);
	if (it == permissions.end()) 
	{
		permissions.push_back(perm);
	}
	else 
	{
		std::cout << "Permission already exists: " << perm << std::endl;
	}
}

void Admin::RemovePermission(const std::string& perm)
{
	permissions.erase(std::remove(permissions.begin(), permissions.end(), perm), permissions.end());
}

bool Admin::HasPermission(const std::string& perm) const
{
	return std::find(permissions.begin(), permissions.end(), perm) != permissions.end();
}

void Admin::SetAdminKey(const std::string& key)
{
	adminKeyHash = hashKey(key);
}

bool Admin::ValidateAdminKey(const std::string& key) const
{
	return adminKeyHash == hashKey(key);
}

const std::vector<std::string>& Admin::GetPermissions() const
{
	return permissions;
}
