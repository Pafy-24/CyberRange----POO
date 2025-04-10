#pragma once
#include <string>

class AdminConn {
private:
    std::string adminKey;
    int privilege;
    bool secure;

public:
    AdminConn(std::string connStr);
    void setAdminKey(std::string key);
    bool verifyPrivilege(int level);
    void enableSecureMode();
    void disableSecureMode();
};
