#pragma once
#include <string>
#include <map>

class JSONDec {
private:
    bool strictMode;
    bool allowComments;

public:
    JSONDec(bool strict = true);
    std::map<std::string, std::string> decode(std::string json);
    void setStrictMode(bool strict);
    void setAllowComments(bool allow);
    bool validate(std::string json);
};
