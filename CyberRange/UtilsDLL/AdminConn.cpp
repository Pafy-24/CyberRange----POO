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
    // Hash the key for security
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, key.c_str(), key.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    adminKey = ss.str();

    // Set privilege level based on key length (just a simple example)
    // In a real system, this would validate against a database of authorized users
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

bool AdminConn::verifyPrivilege(int level) {
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