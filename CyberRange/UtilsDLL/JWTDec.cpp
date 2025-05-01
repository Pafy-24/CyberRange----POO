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
    : secretKey(key), algorithm(algo), verification(true)
{
}

json JWTDec::decode(const std::string& token)
{
    // Check if valid first when verification is enabled
    if (verification && !validate(token)) {
        return json::object(); // Return empty object if token is invalid
    }

    // Get the payload without verification
    std::string decodedPayload = decodePayload(token);

    // Parse the JSON payload
    try {
        return json::parse(decodedPayload);
    }
    catch (const json::parse_error&) {
        return json::object(); // Return empty object on parse error
    }
}

std::string JWTDec::decodePayload(const std::string& token)
{
    // Split the token into parts
    std::vector<std::string> parts;
    std::istringstream tokenStream(token);
    std::string part;

    while (std::getline(tokenStream, part, '.')) {
        parts.push_back(part);
    }

    // We need at least 3 parts for a valid JWT
    if (parts.size() < 3) {
        return "{}"; // Return empty JSON object if token doesn't have enough parts
    }

    // Decode the payload part (second part)
    return base64Decode(parts[1]);
}

bool JWTDec::validate(const std::string& token)
{
    // Split the token into parts
    std::vector<std::string> parts;
    std::istringstream tokenStream(token);
    std::string part;

    while (std::getline(tokenStream, part, '.')) {
        parts.push_back(part);
    }

    // We need exactly 3 parts for a valid JWT
    if (parts.size() != 3) {
        return false;
    }

    // Extract the signature
    std::string providedSignature = parts[2];

    // Create the signature for comparison
    std::string dataToSign = parts[0] + "." + parts[1];
    std::string calculatedSignature = createSignature(dataToSign);

    // Check if the signatures match
    if (providedSignature != calculatedSignature) {
        return false;
    }

    // Check if the token has expired
    bool origVerification = verification;
    verification = false; // Temporarily disable verification
    int expiry = getExpiryTime(token);
    verification = origVerification; // Restore verification setting

    if (expiry > 0) {
        std::time_t now = std::time(nullptr);
        if (now > expiry) {
            return false; // Token has expired
        }
    }

    return true;
}

int JWTDec::getExpiryTime(const std::string& token)
{
    json payload = decode(token);

    // Find the expiry time claim
    if (payload.contains("exp")) {
        try {
            return payload["exp"].get<int>();
        }
        catch (const std::exception&) {
            return 0; // Invalid expiry time
        }
    }

    return 0; // No expiry time found
}

std::string JWTDec::getIssuer(const std::string& token)
{
    json payload = decode(token);

    // Find the issuer claim
    if (payload.contains("iss")) {
        try {
            return payload["iss"].get<std::string>();
        }
        catch (const std::exception&) {
            return ""; // Invalid issuer
        }
    }

    return ""; // No issuer found
}

void JWTDec::setSecretKey(std::string key)
{
    secretKey = key;
}

void JWTDec::setAlgorithm(std::string algo)
{
    algorithm = algo;
}

void JWTDec::setVerification(bool verify)
{
    verification = verify;
}

// Base64 decode utility function
std::string JWTDec::base64Decode(const std::string& input)
{
    BIO* bio, * b64;

    // Create a buffer large enough for the decoded data
    std::vector<unsigned char> buffer(input.length());

    // Create a BIO base64 filter
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL); // No newlines in JWT base64

    // Create a memory BIO for the input
    bio = BIO_new_mem_buf(input.c_str(), input.length());
    bio = BIO_push(b64, bio);

    // Read the decoded data
    int decodedLength = BIO_read(bio, buffer.data(), input.length());
    BIO_free_all(bio);

    if (decodedLength <= 0) {
        return "{}"; // Return empty JSON object on decode error
    }

    return std::string(reinterpret_cast<char*>(buffer.data()), decodedLength);
}

// Create signature function for verification
std::string JWTDec::createSignature(const std::string& data)
{
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
        reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), result, &len);

    // Create a BIO for base64 encoding
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    // Write the data to be encoded
    BIO_write(bio, result, len);
    BIO_flush(bio);

    // Get the encoded data
    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encodedData;
}