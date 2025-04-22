#pragma once
#include <string>
#include <map>
#include "JSONDec.h"
class JWTDec : public JSONDec{
private:
    std::string secretKey;
    std::string algorithm;
    bool Verification = 1;

public:
    JWTDec(std::string key, std::string algo);
    std::map<std::string, std::string> decode(std::string token);
    bool validate(std::string token);
    int getExpiryTime(std::string token);
    std::string getIssuer(std::string token);
    void setSecretKey(std::string key);
    std::string base64Decode(const std::string& input);
    std::string createSignature(const std::string& data);
};
