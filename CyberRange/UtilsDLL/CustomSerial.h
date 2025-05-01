#pragma once
#include "DLL.h"
#include "json.hpp"
#include <string>
#include <map>
#include <variant>
#include <vector>

using json = nlohmann::json;

class UTILS_API CustomSerial {
private:
    bool prettyPrint;
    std::string jwtKey;
    std::string jwtAlgo;

    static bool isJWTFormat(const std::string& data);

public:
    // Constructor
    CustomSerial(bool jsonPretty = false, std::string jwtKey = "", std::string jwtAlgo = "HS256");

    // Universal encode function
    static std::string encode(const json& data, bool useJWT = false,
        const std::string& key = "", const std::string& algo = "HS256");

    // JSON-specific encode
    static std::string encodeJSON(const json& data, bool prettyPrint = false);

    // JWT-specific encode
    static std::string encodeJWT(const json& data,
        const std::string& key = "", const std::string& algo = "HS256");

    // Universal decode function - auto-detects format
    static json decode(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");

    // JSON-specific decode
    static json decodeJSON(const std::string& jsonStr);

    // JWT-specific decode
    static json decodeJWT(const std::string& token,
        const std::string& key = "", const std::string& algo = "HS256");

    // Check if data is valid JSON or JWT
    static bool isValid(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");

    // Legacy methods for backward compatibility
    static std::string encode(std::map<std::string, std::string> data, bool useJWT = false,
        const std::string& key = "", const std::string& algo = "HS256");
    static std::map<std::string, std::string> decodeLegacy(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");
};