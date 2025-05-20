#include "pch.h"
#include "CUser.h"
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>



CUser::CUser(std::string username, std::string email, int id)
    : id(id), username(username), email(email), accessLevel(0), active(true), passwordHash("")
{
}

CUser::CUser(std::string username, std::string email, int id, std::string role)
	: id(id), username(username), email(email), accessLevel(0), active(true), passwordHash("")
{
	if (role == "common") {
		accessLevel = 1;
	}
	else if (role == "writer") {
		accessLevel = 5;
	}
	else if (role == "admin") {
		accessLevel = 10;
	}
}

int CUser::GetId()
{
    return id;
}

std::string CUser::GetUsername()
{
    return username;
}

std::string CUser::GetEmail()
{
    return email;
}

std::string CUser::GetPassword()
{
    return passwordHash;
}

void CUser::SetUsername(std::string username)
{
    this->username = username;
}

void CUser::SetEmail(std::string email)
{
    this->email = email;
}

void CUser::SetId(int id)
{
    this->id = id;
}



int CUser::GetAccessLevel()
{
    return accessLevel;
}

void CUser::SetAccessLevel(int level)
{
    accessLevel = level;
}

void CUser::SetPassword(const std::string& password)
{
    passwordHash = password;
}

void CUser::SetActive(bool active)
{
    this->active = active;
}

bool CUser::IsActive() const
{
    return active;
}