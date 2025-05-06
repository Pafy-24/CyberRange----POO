#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <variant>

using JSONValue = std::variant<std::string, std::vector<std::string>>;

class JSONDec {
private:
    bool strictMode;
    bool allowComments;

    std::vector<std::string> parseArray(const std::string& json, size_t& pos);

public:
    JSONDec(bool strict = true);
    std::map<std::string, JSONValue> decode(std::string json);
    void setStrictMode(bool strict);
    void setAllowComments(bool allow);
    bool validate(std::string json);
};