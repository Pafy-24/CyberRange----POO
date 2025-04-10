#pragma once
#include <string>
#include <vector>
#include <map>
#include "CController.h"
#include "DBConn.h"

class DBController : public CController {
private:
    DBConn* dbConnection;
    std::string connectionString;

public:
    DBController(std::string connString);
    bool connect();
    bool disconnect();
    std::vector<std::map<std::string, std::string>> executeQuery(std::string query);
    bool executeUpdate(std::string query);
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool testConnection();
};
