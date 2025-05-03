#pragma once
#include <string>
#include <vector>
#include <map>
#include "CController.h"
#include "DBConn.h"

class DBController : public CController {
private:
	static DBController* instance;
	DBController() = default;
    DBController(std::string connString);

	DBController(const DBController&) = delete;
	DBController& operator=(const DBController&) = delete;
	DBController(DBController&&) = delete;
	DBController& operator=(DBController&&) = delete;
public:
    static DBController* getInstance(std::string connString);
    ~DBController();

    bool connect();
    bool disconnect();
    std::vector<std::map<std::string, std::string>> executeQuery(const std::string& query, const std::vector<std::string>& params = {});
    bool executeUpdate(const std::string& query, const std::vector<std::string>& params = {});
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    bool testConnection();
private:
    DBConn* dbConnection;
    std::string connectionString;
    bool isConnected;

    bool sanitizeInput(const std::string& input);
};