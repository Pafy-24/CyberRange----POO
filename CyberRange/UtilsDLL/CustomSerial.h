#pragma once
#include "DLL.h"
#include "JWTDec.h"
#include "JWTEnc.h"
#include <string>
#include <map>

class UTILS_API CustomSerial {
private:
    static bool isJWTFormat(const std::string& data);

public:
    // Universal encode function
    static std::string encode(std::map<std::string, std::string> data, bool useJWT = false,
        const std::string& key = "", const std::string& algo = "HS256");

    // JSON-specific encode
    static std::string encodeJSON(std::map<std::string, std::string> data);

    // JWT-specific encode
    static std::string encodeJWT(std::map<std::string, std::string> data,
        const std::string& key = "", const std::string& algo = "HS256");

    // Universal decode function - auto-detects format
    static std::map<std::string, std::string> decode(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");

    // JSON-specific decode
    static std::map<std::string, std::string> decodeJSON(const std::string& json);

    // JWT-specific decode
    static std::map<std::string, std::string> decodeJWT(const std::string& token,
        const std::string& key = "", const std::string& algo = "HS256");

    // Check if data is valid JSON or JWT
    static bool isValid(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");
};