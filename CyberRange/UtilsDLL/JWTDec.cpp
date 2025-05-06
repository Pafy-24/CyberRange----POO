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
    if (verification && !validate(token)) {
        return json::object();
    }

    std::string decodedPayload = decodePayload(token);

    try {
        return json::parse(decodedPayload);
    }
    catch (const json::parse_error&) {
        return json::object(); 
    }
}

std::string JWTDec::decodePayload(const std::string& token)
{
    std::vector<std::string> parts;
    std::istringstream tokenStream(token);
    std::string part;

    while (std::getline(tokenStream, part, '.')) {
        parts.push_back(part);
    }

    if (parts.size() < 3) {
        return "{}"; 
    }

    return base64Decode(parts[1]);
}

bool JWTDec::validate(const std::string& token)
{
    std::vector<std::string> parts;
    std::istringstream tokenStream(token);
    std::string part;

    while (std::getline(tokenStream, part, '.')) {
        parts.push_back(part);
    }

    if (parts.size() != 3) {
        return false;
    }

    std::string providedSignature = parts[2];

    std::string dataToSign = parts[0] + "." + parts[1];
    std::string calculatedSignature = createSignature(dataToSign);

    if (providedSignature != calculatedSignature) {
        return false;
    }

    bool origVerification = verification;
    verification = false;
    int expiry = getExpiryTime(token);
    verification = origVerification;

    if (expiry > 0) {
        std::time_t now = std::time(nullptr);
        if (now > expiry) {
            return false; 
        }
    }

    return true;
}

int JWTDec::getExpiryTime(const std::string& token)
{
    json payload = decode(token);

    if (payload.contains("exp")) {
        try {
            return payload["exp"].get<int>();
        }
        catch (const std::exception&) {
            return 0; 
        }
    }

    return 0; 
}

std::string JWTDec::getIssuer(const std::string& token)
{
    json payload = decode(token);

    if (payload.contains("iss")) {
        try {
            return payload["iss"].get<std::string>();
        }
        catch (const std::exception&) {
            return ""; 
        }
    }

    return "";
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

std::string JWTDec::base64Decode(const std::string& input)
{
    BIO* bio, * b64;

    std::vector<unsigned char> buffer(input.length());

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    bio = BIO_new_mem_buf(input.c_str(), input.length());
    bio = BIO_push(b64, bio);

    int decodedLength = BIO_read(bio, buffer.data(), input.length());
    BIO_free_all(bio);

    if (decodedLength <= 0) {
        return "{}";
    }

    return std::string(reinterpret_cast<char*>(buffer.data()), decodedLength);
}

std::string JWTDec::createSignature(const std::string& data)
{
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int len = 0;

    HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
        reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), result, &len);

    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);

    BIO_write(bio, result, len);
    BIO_flush(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string encodedData(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return encodedData;
}