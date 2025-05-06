#pragma once
#include <string>
#include <map>
#include "json.hpp"

using json = nlohmann::json;

class JWTDec {
private:
    std::string secretKey;
    std::string algorithm;
    bool verification;

    std::string base64Decode(const std::string& input);
    std::string createSignature(const std::string& data);

public:
    JWTDec(std::string key = "", std::string algo = "HS256");

    json decode(const std::string& token);

    std::string decodePayload(const std::string& token);

    bool validate(const std::string& token);

    int getExpiryTime(const std::string& token);
    std::string getIssuer(const std::string& token);

    void setSecretKey(std::string key);
    void setAlgorithm(std::string algo);
    void setVerification(bool verify);
};