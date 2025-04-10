#pragma once
#include <string>
#include <map>
#include "CController.h"
#include "User.h"
#include "JWTEnc.h"
#include "JWTDec.h"

class UserController : public CController {
private:
    std::map<std::string, User*> users;
    JWTEnc encoder;
    JWTDec decoder;

public:
    UserController();
    bool authenticate(std::string username, std::string password);
    std::string generateToken(User* user);
    bool validateToken(std::string token);
    User* getUser(std::string userId);
    void createUser(std::string userData);
    void updateUser(std::string userId, std::string userData);
    void deleteUser(std::string userId);
};
