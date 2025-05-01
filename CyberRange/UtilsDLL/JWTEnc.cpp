#include "pch.h"
#include "JWTEnc.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/bio.h>

JWTEnc::JWTEnc(std::string key, std::string algo)
    : secretKey(key), algorithm(algo), expiryTime(0)
{
}

std::string JWTEnc::encode(const std::string& jsonStr)
{
    std::string header = R"({"alg":")" + algorithm + R"(","typ":"JWT"})";

    // Base64 encode the header
    std::string encodedHeader = base64Encode(header);

    // Parse the JSON string to add expiry time if needed
    json payload;
    try {
        payload = json::parse(jsonStr);
    }
    catch (const json::parse_error&) {
        // If parsing fails, use empty JSON object
        payload = json::object();
    }

    // Add expiry time to payload if set
    if (expiryTime > 0) {
        std::time_t now = std::time(nullptr);
        std::time_t exp = now + expiryTime;
        payload["exp"] = exp;
    }

    // Base64 encode the payload
    std::string encodedPayload = base64Encode(payload.dump());

    // Create the signature
    std::string dataToSign = encodedHeader + "." + encodedPayload;
    std::string signature = createSignature(dataToSign);

    // Return the final JWT
    return encodedHeader + "." + encodedPayload + "." + signature;
}

// Method with nlohmann::json input
std::string JWTEnc::encode(const json& data)
{
    return encode(data.dump());
}

void JWTEnc::setExpiryTime(int seconds)
{
    expiryTime = seconds;
}

void JWTEnc::setSecretKey(std::string key)
{
    secretKey = key;
}

void JWTEnc::setAlgorithm(std::string algo)
{
    algorithm = algo;
}

std::string JWTEnc::base64Encode(const std::string& input)
{
    BIO* bio, * b64;
    BUF_MEM* bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    // Disable newlines in base64 output
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(bio, input.c_str(), input.length());
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encodedData;
}

std::string JWTEnc::createSignature(const std::string& data)
{
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
        reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), result, &len);

    return base64Encode(std::string(reinterpret_cast<char*>(result), len));
}