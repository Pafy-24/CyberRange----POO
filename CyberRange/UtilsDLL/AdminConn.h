#pragma once
#include <string>
#include "DBConn.h"
class AdminConn : public DBConn {
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
