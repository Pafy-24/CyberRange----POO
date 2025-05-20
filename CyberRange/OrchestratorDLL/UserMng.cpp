#include "pch.h"
#include "UserMng.h"

UserMng::~UserMng() 
{
    clear();
}

void UserMng::addUser(User* user) 
{
    if (user) 
    {
        users[user->GetId()] = user;
    }
}

void UserMng::removeUser(int userId) 
{
    auto it = users.find(userId);
    if (it != users.end()) 
    {
        delete it->second;
        users.erase(it);
    }
}

User* UserMng::getUser(int userId) const 
{
    auto it = users.find(userId);
    if (it != users.end()) 
    {
        return it->second;
    }
    return nullptr;
}

std::map<int, User*> UserMng::getAllUsers() const 
{
    return users;
}

void UserMng::clear() {
    for (auto& pair : users) {
        delete pair.second;
    }
    users.clear();
}
