#include "pch.h"
#include "JSONDec.h"
#include <sstream>
#include <cctype>
#include <stdexcept>

JSONDec::JSONDec(bool strict) : strictMode(strict), allowComments(false)
{
}

std::map<std::string, std::string> JSONDec::decode(std::string json)
{
    std::map<std::string, std::string> result;

    // Skip whitespace and find the opening brace
    size_t pos = 0;
    while (pos < json.length() && std::isspace(json[pos])) pos++;

    if (pos >= json.length() || json[pos] != '{')
    {
        if (strictMode)
            throw std::runtime_error("Invalid JSON: missing opening brace");
        return result;
    }

    pos++; // Skip the opening brace

    // Parse the key-value pairs
    bool expectingComma = false;
    while (pos < json.length())
    {
        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        // Check for comments if allowed
        if (allowComments && pos + 1 < json.length() && json[pos] == '/' && json[pos + 1] == '/')
        {
            // Skip to the end of the line
            while (pos < json.length() && json[pos] != '\n') pos++;
            continue;
        }

        // Check for closing brace
        if (pos < json.length() && json[pos] == '}')
            break;

        // Check for comma
        if (expectingComma)
        {
            if (pos < json.length() && json[pos] == ',')
            {
                pos++;
                expectingComma = false;
            }
            else if (strictMode)
            {
                throw std::runtime_error("Invalid JSON: expected comma");
            }
        }

        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        // Parse key
        if (pos >= json.length() || json[pos] != '"')
        {
            if (strictMode)
                throw std::runtime_error("Invalid JSON: expected key");
            break;
        }

        // Extract key
        std::string key;
        pos++; // Skip the opening quote
        while (pos < json.length() && json[pos] != '"')
        {
            // Handle escape sequences
            if (json[pos] == '\\' && pos + 1 < json.length())
            {
                pos++;
                switch (json[pos])
                {
                case '"': key += '"'; break;
                case '\\': key += '\\'; break;
                case '/': key += '/'; break;
                case 'b': key += '\b'; break;
                case 'f': key += '\f'; break;
                case 'n': key += '\n'; break;
                case 'r': key += '\r'; break;
                case 't': key += '\t'; break;
                case 'u':
                    // Unicode escapes not fully implemented in this simple version
                    if (strictMode)
                        throw std::runtime_error("Unicode escapes not supported in this implementation");
                    key += "\\u";
                    break;
                default:
                    if (strictMode)
                        throw std::runtime_error("Invalid escape sequence");
                    key += json[pos];
                }
            }
            else
            {
                key += json[pos];
            }
            pos++;
        }

        if (pos >= json.length())
        {
            if (strictMode)
                throw std::runtime_error("Invalid JSON: unterminated string");
            break;
        }

        pos++; // Skip the closing quote

        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        // Check for colon
        if (pos >= json.length() || json[pos] != ':')
        {
            if (strictMode)
                throw std::runtime_error("Invalid JSON: expected colon");
            break;
        }

        pos++; // Skip the colon

        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        // Parse value
        std::string value;

        if (pos < json.length() && json[pos] == '"')
        {
            // String value
            pos++; // Skip the opening quote
            while (pos < json.length() && json[pos] != '"')
            {
                // Handle escape sequences
                if (json[pos] == '\\' && pos + 1 < json.length())
                {
                    pos++;
                    switch (json[pos])
                    {
                    case '"': value += '"'; break;
                    case '\\': value += '\\'; break;
                    case '/': value += '/'; break;
                    case 'b': value += '\b'; break;
                    case 'f': value += '\f'; break;
                    case 'n': value += '\n'; break;
                    case 'r': value += '\r'; break;
                    case 't': value += '\t'; break;
                    case 'u':
                        // Unicode escapes not fully implemented in this simple version
                        if (strictMode)
                            throw std::runtime_error("Unicode escapes not supported in this implementation");
                        value += "\\u";
                        break;
                    default:
                        if (strictMode)
                            throw std::runtime_error("Invalid escape sequence");
                        value += json[pos];
                    }
                }
                else
                {
                    value += json[pos];
                }
                pos++;
            }

            if (pos >= json.length())
            {
                if (strictMode)
                    throw std::runtime_error("Invalid JSON: unterminated string");
                break;
            }

            pos++; // Skip the closing quote
        }
        else
        {
            // Number, boolean, or null
            while (pos < json.length() && json[pos] != ',' && json[pos] != '}' && !std::isspace(json[pos]))
            {
                value += json[pos];
                pos++;
            }
        }

        // Add the key-value pair to the result
        result[key] = value;

        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        expectingComma = true;
    }

    return result;
}

void JSONDec::setStrictMode(bool strict)
{
    strictMode = strict;
}

void JSONDec::setAllowComments(bool allow)
{
    allowComments = allow;
}

bool JSONDec::validate(std::string json)
{
    try
    {
        decode(json);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}