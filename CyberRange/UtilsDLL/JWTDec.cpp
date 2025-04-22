#include "pch.h"
#include "JWTDec.h"
#include <vector>
#include <sstream>
#include <ctime>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>

JWTDec::JWTDec(std::string key, std::string algo)
    : secretKey(key), algorithm(algo)
{
}

std::map<std::string, std::string> JWTDec::decode(std::string token)
{
    std::map<std::string, std::string> payload;

    // Check if valid first
    if(Verification)
        if (!validate(token))
        {
            return payload; // Return empty map if token is invalid
        }

    // Split the token into parts
    std::vector<std::string> parts;
    std::istringstream tokenStream(token);
    std::string part;

    while (std::getline(tokenStream, part, '.'))
    {
        parts.push_back(part);
    }

    // We need at least 3 parts for a valid JWT
    if (parts.size() < 3)
    {
        return payload; // Return empty map if token doesn't have enough parts
    }

    // Decode the payload part (second part)
    std::string decodedPayload = base64Decode(parts[1]);

    // Parse the JSON payload
    payload = JSONDec::decode(decodedPayload);

    return payload;
}

bool JWTDec::validate(std::string token)
{
    // Split the token into parts
    std::vector<std::string> parts;
    std::istringstream tokenStream(token);
    std::string part;

    while (std::getline(tokenStream, part, '.'))
    {
        parts.push_back(part);
    }

    // We need exactly 3 parts for a valid JWT
    if (parts.size() != 3)
    {
        return false;
    }

    // Extract the signature
    std::string providedSignature = parts[2];

    // Create the signature for comparison
    std::string dataToSign = parts[0] + "." + parts[1];
    std::string calculatedSignature = createSignature(dataToSign);

    // Check if the signatures match
    if (providedSignature != calculatedSignature)
    {
        return false;
    }

    // Check if the token has expired
    Verification = 0;
    int expiry = getExpiryTime(token);
    Verification = 1;
    std::time_t now = std::time(nullptr);

    if (expiry > 0 && now > expiry)
    {
        return false; // Token has expired
    }

    return true;
}

int JWTDec::getExpiryTime(std::string token)
{
    std::map<std::string, std::string> payload = decode(token);

    // Find the expiry time claim
    auto it = payload.find("exp");
    if (it != payload.end())
    {
        try
        {
            return std::stoi(it->second);
        }
        catch (const std::exception&)
        {
            return 0; // Invalid expiry time
        }
    }

    return 0; // No expiry time found
}

std::string JWTDec::getIssuer(std::string token)
{
    std::map<std::string, std::string> payload = decode(token);

    // Find the issuer claim
    auto it = payload.find("iss");
    if (it != payload.end())
    {
        return it->second;
    }

    return ""; // No issuer found
}

void JWTDec::setSecretKey(std::string key)
{
    secretKey = key;
}

// Base64 decode utility function
std::string JWTDec::base64Decode(const std::string& input)
{
    BIO* bio, * b64;
    int decodeLen = input.length();
    std::vector<char> buffer(decodeLen);

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input.c_str(), input.length());
    bio = BIO_push(b64, bio);

    // Tell OpenSSL not to expect newlines
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    int len = BIO_read(bio, buffer.data(), decodeLen);
    BIO_free_all(bio);

    return std::string(buffer.data(), len);
}

// Create signature function - similar to JWTEnc but for verification
std::string JWTDec::createSignature(const std::string& data)
{
    unsigned char* result;
    unsigned int len = EVP_MAX_MD_SIZE;

    result = HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
        reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), nullptr, &len);

    // Base64 encode the signature
    BIO* bio, * b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, result, len);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string encodedData(bufferPtr->data, bufferPtr->length - 1);
    BIO_free_all(bio);

    return encodedData;
}