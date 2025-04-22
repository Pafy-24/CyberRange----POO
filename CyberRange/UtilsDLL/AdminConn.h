#pragma once
#include <string>
#include "DBConn.h"

class UTILS_API AdminConn : public DBConn {
private:
    std::string adminKey;
    int privilege;
    bool secure;

protected:
    bool sanitizeQuery(std::string& query) override; // Override to bypass sanitization

public:
    AdminConn(std::string connStr);
    virtual ~AdminConn();

    void setAdminKey(std::string key);
    bool verifyPrivilege(int level) const;
    void enableSecureMode();
    void disableSecureMode();

    // Metode suplimentare pentru administrare
    bool isSecureModeEnabled() const { return secure; }
    int getPrivilegeLevel() const { return privilege; }
};