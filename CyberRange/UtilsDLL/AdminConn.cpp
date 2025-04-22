#include "pch.h"
#include "AdminConn.h"
#include <iostream>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

AdminConn::AdminConn(std::string connStr)
    : DBConn(connStr), privilege(0), secure(true) {
    // Admin connections should always use TLS
    enableTLS();
}

AdminConn::~AdminConn() {
    // Clear sensitive data
    adminKey.clear();
}

void AdminConn::setAdminKey(std::string key) {
    // Hash the key for security using EVP API
    unsigned char hash[SHA256_DIGEST_LENGTH];
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) {
        std::cerr << "Failed to create EVP_MD_CTX" << std::endl;
        return;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1 ||
        EVP_DigestUpdate(ctx, key.c_str(), key.size()) != 1 ||
        EVP_DigestFinal_ex(ctx, hash, nullptr) != 1) {
        std::cerr << "Failed to compute SHA256 hash using EVP API" << std::endl;
        EVP_MD_CTX_free(ctx);
        return;
    }

    EVP_MD_CTX_free(ctx);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    adminKey = ss.str();

    // Set privilege level based on key length (just a simple example)
    if (key.length() >= 16) {
        privilege = 2; // Full admin
    }
    else if (key.length() >= 8) {
        privilege = 1; // Limited admin
    }
    else {
        privilege = 0; // No privileges
    }

    std::cout << "Admin key set with privilege level " << privilege << std::endl;
}


bool AdminConn::verifyPrivilege(int level) const {
    if (privilege >= level) {
        return true;
    }

    std::cerr << "Insufficient privileges. Required level: " << level
        << ", Current level: " << privilege << std::endl;
    return false;
}

void AdminConn::enableSecureMode() {
    secure = true;
    std::cout << "Secure mode enabled" << std::endl;
}

void AdminConn::disableSecureMode() {
    if (verifyPrivilege(2)) { // Only highest privilege can disable secure mode
        secure = false;
        std::cout << "Secure mode disabled" << std::endl;
    }
    else {
        std::cerr << "Insufficient privileges to disable secure mode" << std::endl;
    }
}

bool AdminConn::sanitizeQuery(std::string& query) {
    // Admin connections bypass SQL injection checks if secure mode is disabled
    if (!secure) {
        std::cout << "Warning: Executing unsanitized query in admin mode" << std::endl;
        return true;
    }

    // If secure mode is enabled, use the parent class's sanitization
    return DBConn::sanitizeQuery(query);
}