#pragma once
#include "DLL.h"
#include <string>
#include <map>
#include <variant>
#include <vector>


class UTILS_API CustomSerial {
private:
    bool prettyPrint;
    std::string jwtKey;
    std::string jwtAlgo;

    static bool isJWTFormat(const std::string& data);

public:
    CustomSerial(bool jsonPretty = false, std::string jwtKey = "", std::string jwtAlgo = "HS256");
    std::string hash(const std::string& str) const;
    static std::string encode(const json& data, bool useJWT = false,
        const std::string& key = "", const std::string& algo = "HS256");

    static std::string encodeJSON(const json& data, bool prettyPrint = false);

    static std::string encodeJWT(const json& data,
        const std::string& key = "", const std::string& algo = "HS256");

    static json decode(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");

    static json decodeJSON(const std::string& jsonStr);

    static json decodeJWT(const std::string& token,
        const std::string& key = "", const std::string& algo = "HS256");

    static bool isValid(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");

    static std::string encode(std::map<std::string, std::string> data, bool useJWT = false,
        const std::string& key = "", const std::string& algo = "HS256");
    static std::map<std::string, std::string> decodeLegacy(const std::string& data,
        const std::string& key = "", const std::string& algo = "HS256");
};