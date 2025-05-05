#include "pch.h"
#include "CustomSerial.h"
#include "JWTDec.h"
#include "JWTEnc.h"
#include <openssl/sha.h>

// Constructor initializing members
CustomSerial::CustomSerial(bool jsonPretty, std::string jwtKey, std::string jwtAlgo)
    : prettyPrint(jsonPretty), jwtKey(jwtKey), jwtAlgo(jwtAlgo)
{
}
std::string CustomSerial::hash(const std::string& str) const
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}


// Universal encode function with json support
std::string CustomSerial::encode(const json& data, bool useJWT,
    const std::string& key, const std::string& algo)
{
    if (useJWT) {
        return encodeJWT(data, key, algo);
    }
    else {
        return encodeJSON(data);
    }
}

// JSON-specific encode
std::string CustomSerial::encodeJSON(const json& data, bool prettyPrint)
{
    return prettyPrint ? data.dump(4) : data.dump();
}

// JWT-specific encode
std::string CustomSerial::encodeJWT(const json& data,
    const std::string& key, const std::string& algo)
{

    JWTEnc encoder(key, algo);
    return encoder.encode(data);
}

// Universal decode function - auto-detects format
json CustomSerial::decode(const std::string& data,
    const std::string& key, const std::string& algo)
{
    if (isJWTFormat(data)) {
        return decodeJWT(data, key, algo);
    }
    else {
        return decodeJSON(data);
    }
}

// JSON-specific decode
json CustomSerial::decodeJSON(const std::string& jsonStr)
{
    try {
        return json::parse(jsonStr);
    }
    catch (const json::parse_error& e) {
        // Return empty JSON object on error
        return json::object();
    }
}

// JWT-specific decode
json CustomSerial::decodeJWT(const std::string& token,
    const std::string& key, const std::string& algo)
{
    // Decode the JWT using JWT decoder
    JWTDec decoder(key, algo);
    std::string payload = decoder.decodePayload(token);

    // Parse the payload as JSON
    return decodeJSON(payload);
}

// Check if data is valid based on auto-detection
bool CustomSerial::isValid(const std::string& data,
    const std::string& key, const std::string& algo)
{
    if (isJWTFormat(data)) {
        JWTDec decoder(key, algo);
        return decoder.validate(data);
    }
    else {
        try {
            json::parse(data);
            return true;
        }
        catch (const json::parse_error&) {
            return false;
        }
    }
}

// Helper method to identify if string is in JWT format
bool CustomSerial::isJWTFormat(const std::string& data)
{
    // JWT tokens have 3 parts separated by dots
    size_t firstDot = data.find('.');
    if (firstDot == std::string::npos) {
        return false;
    }

    size_t secondDot = data.find('.', firstDot + 1);
    if (secondDot == std::string::npos) {
        return false;
    }

    // Check if there's text after the second dot
    if (secondDot + 1 >= data.length()) {
        return false;
    }

    // Additional check: JWT typically starts with "{"
    // but after base64 encoding, it usually starts with "ey"
    if (data.substr(0, 2) != "ey") {
        return false;
    }

    return true;
}

// Legacy methods for backward compatibility
std::string CustomSerial::encode(std::map<std::string, std::string> data, bool useJWT,
    const std::string& key, const std::string& algo)
{
    // Convert old string map to json
    json jsonData = json::object();
    for (const auto& pair : data) {
        jsonData[pair.first] = pair.second;
    }

    return encode(jsonData, useJWT, key, algo);
}

std::map<std::string, std::string> CustomSerial::decodeLegacy(const std::string& data,
    const std::string& key, const std::string& algo)
{
    // Use the new decode method but convert the result back to string-only map
    json jsonResult = decode(data, key, algo);
    std::map<std::string, std::string> stringResult;

    for (auto& [key, value] : jsonResult.items()) {
        if (value.is_string()) {
            stringResult[key] = value.get<std::string>();
        }
        else if (value.is_array()) {
            // For arrays, convert to a comma-separated string
            std::string joinedArray;
            for (size_t i = 0; i < value.size(); ++i) {
                if (value[i].is_string()) {
                    joinedArray += value[i].get<std::string>();
                    if (i < value.size() - 1) {
                        joinedArray += ",";
                    }
                }
            }
            stringResult[key] = joinedArray;
        }
        else if (value.is_number()) {
            // Convert numbers to strings
            stringResult[key] = std::to_string(value.get<double>());
        }
        else if (value.is_boolean()) {
            // Convert booleans to strings
            stringResult[key] = value.get<bool>() ? "true" : "false";
        }
        else {
            // For other types, use empty string
            stringResult[key] = "";
        }
    }

    return stringResult;
}