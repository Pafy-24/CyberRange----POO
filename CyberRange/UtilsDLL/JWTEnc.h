#pragma once
#include <string>
#include <map>

class JWTEnc {
private:
    std::string secretKey;
    int expiryTime;
    std::string algorithm;

public:
    JWTEnc(std::string key, std::string algo);
    std::string encode(std::map<std::string, std::string> payload);
    void setExpiryTime(int seconds);
    void setSecretKey(std::string key);
    void setAlgorithm(std::string algo);
};
