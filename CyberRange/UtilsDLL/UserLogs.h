#pragma once
#include <string>
#include <vector>
#include <map>

class UserLogs {
private:
    std::map<std::string, std::vector<std::string>> userActions;

public:
    UserLogs(std::string file);
    void logUserAction(std::string userId, std::string action);
    std::vector<std::string> getUserHistory(std::string userId);
    void clearUserHistory(std::string userId);
    std::map<std::string, int> getActivitySummary();
};
