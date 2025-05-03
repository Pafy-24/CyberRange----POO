#include "pch.h"
#include "Writer.h"

Writer::Writer(std::string username, std::string email, int id)
    : CUser(username, email, id)
{
    SetAccessLevel(5);
    articles.clear();
    signature = "";
}

bool Writer::articleExists(const std::string& articleId) const
{
    return std::find(articles.begin(), articles.end(), articleId) != articles.end();
}

void Writer::AddArticle(const std::string& articleId)
{
    if (!articleExists(articleId))
    {
        articles.push_back(articleId);
    }
}

const std::vector<std::string>& Writer::GetArticles() const
{
    return articles;
}

void Writer::SetSignature(const std::string& sig)
{
    signature = sig;
}

std::string Writer::GetSignature() const
{
    return signature;
}