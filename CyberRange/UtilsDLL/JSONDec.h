#pragma once
#include <string>
#include <map>
#include <vector>
#include <variant>

// Define JSON value type that can be either a string or an array of strings
using JSONValue = std::variant<std::string, std::vector<std::string>>;

class JSONDec {
private:
    bool strictMode;
    bool allowComments;

    // Helper method to parse JSON array
    std::vector<std::string> parseArray(const std::string& json, size_t& pos);

public:
    JSONDec(bool strict = true);
    std::map<std::string, JSONValue> decode(std::string json);
    void setStrictMode(bool strict);
    void setAllowComments(bool allow);
    bool validate(std::string json);
};