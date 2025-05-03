#pragma once
#include <string>
#include <vector>
#include "CUser.h"

class UTILS_API Writer : public CUser {
private:
    std::vector<std::string> articles;
    std::string signature;
    bool articleExists(const std::string& articleId) const;

public:
    Writer(std::string username, std::string email, int id = 0);
    void AddArticle(const std::string& articleId);
    const std::vector<std::string>& GetArticles() const;
    void SetSignature(const std::string& sig);
    std::string GetSignature() const;
};