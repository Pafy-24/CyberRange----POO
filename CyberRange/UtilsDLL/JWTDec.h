#pragma once
#include <string>
#include <map>
#include "JSONDec.h"
class JWTDec : public JSONDec{
private:
    std::string secretKey;
    std::string algorithm;

public:
    JWTDec(std::string key, std::string algo);
    std::map<std::string, std::string> decode(std::string token);
    bool validate(std::string token);
    int getExpiryTime(std::string token);
    std::string getIssuer(std::string token);
    void setSecretKey(std::string key);
};
