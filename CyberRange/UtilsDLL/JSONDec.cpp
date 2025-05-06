#include "pch.h"
#include "JSONDec.h"
#include <sstream>
#include <cctype>
#include <stdexcept>

JSONDec::JSONDec(bool strict) : strictMode(strict), allowComments(false)
{
}

std::map<std::string, JSONValue> JSONDec::decode(std::string json)
{
    std::map<std::string, JSONValue> result;

    size_t pos = 0;
    while (pos < json.length() && std::isspace(json[pos])) pos++;

    if (pos >= json.length() || json[pos] != '{')
    {
        if (strictMode)
            throw std::runtime_error("Invalid JSON: missing opening brace");
        return result;
    }

    pos++;

    bool expectingComma = false;
    while (pos < json.length())
    {
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        if (allowComments && pos + 1 < json.length() && json[pos] == '/' && json[pos + 1] == '/')
        {
            while (pos < json.length() && json[pos] != '\n') pos++;
            continue;
        }

        if (pos < json.length() && json[pos] == '}')
            break;

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

        while (pos < json.length() && std::isspace(json[pos])) pos++;

        if (pos >= json.length() || json[pos] != '"')
        {
            if (strictMode)
                throw std::runtime_error("Invalid JSON: expected key");
            break;
        }

        std::string key;
        pos++; 
        while (pos < json.length() && json[pos] != '"')
        {
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

        pos++; 

        while (pos < json.length() && std::isspace(json[pos])) pos++;

        if (pos >= json.length() || json[pos] != ':')
        {
            if (strictMode)
                throw std::runtime_error("Invalid JSON: expected colon");
            break;
        }

        pos++;

        while (pos < json.length() && std::isspace(json[pos])) pos++;

        if (pos < json.length() && json[pos] == '[')
        {
            std::vector<std::string> array = parseArray(json, pos);
            result[key] = array;
        }
        else if (pos < json.length() && json[pos] == '"')
        {
            std::string value;
            pos++; 
            while (pos < json.length() && json[pos] != '"')
            {
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

            pos++; 
            result[key] = value;
        }
        else
        {
            std::string value;
            while (pos < json.length() && json[pos] != ',' && json[pos] != '}' && !std::isspace(json[pos]))
            {
                value += json[pos];
                pos++;
            }
            result[key] = value;
        }

        while (pos < json.length() && std::isspace(json[pos])) pos++;

        expectingComma = true;
    }

    return result;
}

std::vector<std::string> JSONDec::parseArray(const std::string& json, size_t& pos)
{
    std::vector<std::string> result;

    pos++; 

    bool expectingComma = false;
    while (pos < json.length())
    {
        while (pos < json.length() && std::isspace(json[pos])) pos++;

        if (pos < json.length() && json[pos] == ']')
        {
            pos++; 
            return result;
        }

        if (expectingComma)
        {
            if (pos < json.length() && json[pos] == ',')
            {
                pos++;
                expectingComma = false;
            }
            else if (strictMode)
            {
                throw std::runtime_error("Invalid JSON: expected comma in array");
            }
        }

        while (pos < json.length() && std::isspace(json[pos])) pos++;

        if (pos < json.length() && json[pos] == '"')
        {
            std::string value;
            pos++; 

            while (pos < json.length() && json[pos] != '"')
            {
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
                    throw std::runtime_error("Invalid JSON: unterminated string in array");
                break;
            }

            pos++; 
            result.push_back(value);
        }
        else
        {
            std::string value;
            while (pos < json.length() && json[pos] != ',' && json[pos] != ']' && !std::isspace(json[pos]))
            {
                value += json[pos];
                pos++;
            }

            if (!value.empty())
            {
                result.push_back(value);
            }
            else if (strictMode)
            {
                throw std::runtime_error("Invalid JSON: empty array element");
            }
        }

        while (pos < json.length() && std::isspace(json[pos])) pos++;

        expectingComma = true;
    }

    if (strictMode)
        throw std::runtime_error("Invalid JSON: unterminated array");

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