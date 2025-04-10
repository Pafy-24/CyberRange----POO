#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class Writer : public CUser {
private:
    std::vector<std::string> articles;
    std::string signature;

public:
    Writer(std::string username, std::string email);
    void addArticle(std::string articleId);
    std::vector<std::string> getArticles();
    void setSignature(std::string sig);
    std::string getSignature();
};
