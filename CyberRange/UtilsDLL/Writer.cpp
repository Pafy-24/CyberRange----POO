#include "pch.h"
#include "Writer.h"

Writer::Writer(std::string username, std::string email) : CUser(username, email)
{
    SetAccessLevel(5); // utilizator writer
	id = "WRT_" + username;
    articles.clear();
    signature = "";
}

bool Writer::articleExists(const std::string& articleId) const 
{
    for (const auto& art : articles) 
    {
        if (art == articleId) 
        {
            return true;
        }
    }
    return false;
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