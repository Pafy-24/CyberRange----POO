#include "pch.h"
#include "CustomSerial.h"

// Universal encode function
std::string CustomSerial::encode(std::map<std::string, std::string> data, bool useJWT,
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
std::string CustomSerial::encodeJSON(std::map<std::string, std::string> data)
{
    JSONEnc encoder;
    return encoder.encode(data);
}

// JWT-specific encode
std::string CustomSerial::encodeJWT(std::map<std::string, std::string> data,
    const std::string& key, const std::string& algo)
{
    // Convert map to JSON first
    std::string json = encodeJSON(data);

    // Then encode as JWT
    JWTEnc encoder(key, algo);
    return encoder.encode(json);
}

// Universal decode function - auto-detects format
std::map<std::string, std::string> CustomSerial::decode(const std::string& data,
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
std::map<std::string, std::string> CustomSerial::decodeJSON(const std::string& json)
{
    JSONDec decoder;
    return decoder.decode(json);
}

// JWT-specific decode
std::map<std::string, std::string> CustomSerial::decodeJWT(const std::string& token,
    const std::string& key, const std::string& algo)
{
    JWTDec decoder(key, algo);
    return decoder.decode(token);
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
        JSONDec decoder;
        return decoder.validate(data);
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