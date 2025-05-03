#include "pch.h"
#include "CUser.h"
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

std::string CUser::hashPassword(const std::string& password) const
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), password.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}

CUser::CUser(std::string username, std::string email, int id)
    : id(id), username(username), email(email), accessLevel(0), active(true), passwordHash("")
{
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

bool CUser::Authenticate(std::string password)
{
    return passwordHash == hashPassword(password);
}

int CUser::GetAccessLevel()
{
    return accessLevel;
}

void CUser::SetAccessLevel(int level)
{
    accessLevel = level;
}

void CUser::SetPassword(std::string password)
{
    passwordHash = hashPassword(password);
}

void CUser::SetActive(bool active)
{
    this->active = active;
}

bool CUser::IsActive() const
{
    return active;
}