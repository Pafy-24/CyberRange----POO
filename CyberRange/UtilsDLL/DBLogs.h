#pragma once
#include <string>
#include <vector>
#include "DBConn.h"
#include "CLogger.h"

class DBLogs: public CLogger {
private:
    DBConn& dbConn;
    std::string tableName;

public:
    DBLogs(DBConn& conn, std::string table);
    void storeLog(std::string message, int level);
    std::vector<std::string> retrieveLogs(int count);
    void clearLogs();
    int getLogCount();
};
