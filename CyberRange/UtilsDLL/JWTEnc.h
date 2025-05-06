#pragma once
#include "DLL.h"
#include <string>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

class UTILS_API JWTEnc {
private:
    std::string secretKey;
    int expiryTime;
    std::string algorithm;

public:
    JWTEnc(std::string key = "", std::string algo = "HS256");

    std::string encode(const std::string& jsonStr);

    std::string encode(const json& data);

    void setExpiryTime(int seconds);
    void setSecretKey(std::string key);
    void setAlgorithm(std::string algo);
    std::string base64Encode(const std::string& input);
    std::string createSignature(const std::string& data);
};