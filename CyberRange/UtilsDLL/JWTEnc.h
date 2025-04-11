#pragma once
#include "DLL.h"
#include <string>
#include <map>
#include "JSONEnc.h"


class UTILS_API JWTEnc: public JSONEnc {
private:
    std::string secretKey;
    int expiryTime;
    std::string algorithm;

public:
    JWTEnc(std::string key, std::string algo);
    std::string encode(std::string json);
    void setExpiryTime(int seconds);
    void setSecretKey(std::string key);
    void setAlgorithm(std::string algo);
    std::string base64Encode(const std::string& input);
    std::string createSignature(const std::string& data);
};
