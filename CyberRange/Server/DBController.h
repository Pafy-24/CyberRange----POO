#pragma once
#include "CController.h"
#include "Observable.h"
#include "DBConn.h"
#include <string>
#include <vector>
#include <map>


class DBController : public CController, public Observable {
private:
    std::string connectionString;
    bool isConnected;
    DBConn* dbConnection;

    bool sanitizeInput(const std::string& input);

public:
    DBController(std::string connString);
    ~DBController();

    bool connect();
    bool disconnect();
    bool testConnection();

    std::vector<std::map<std::string, std::string>> executeQuery(
        const std::string& query,
        const std::vector<std::string>& params = {}
    );

    bool executeUpdate(
        const std::string& query,
        const std::vector<std::string>& params = {}
    );

    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
};