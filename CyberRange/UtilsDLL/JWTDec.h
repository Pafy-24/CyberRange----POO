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

    // Decode JWT token and return parsed JSON
    json decode(const std::string& token);

    // Extract raw payload without validating signature (for internal use)
    std::string decodePayload(const std::string& token);

    // Validate JWT token
    bool validate(const std::string& token);

    // Utility methods
    int getExpiryTime(const std::string& token);
    std::string getIssuer(const std::string& token);

    // Configuration
    void setSecretKey(std::string key);
    void setAlgorithm(std::string algo);
    void setVerification(bool verify);
};