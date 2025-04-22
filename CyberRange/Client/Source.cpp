#include <iostream>
#include "Admin.h"
#include "Writer.h"
#include "Common.h"

int main() {
    // Admin test
    Admin admin("adminUser", "admin@cyberrange.com");
    admin.SetAdminKey("supersecretkey123");
    admin.AddPermission("can_create_users");
    admin.AddPermission("can_reset_contest");

    std::cout << "[ADMIN]\n";
    std::cout << "Username: " << admin.GetUsername() << "\n";
    std::cout << "Has permission 'can_reset_contest'? " << std::boolalpha << admin.HasPermission("can_reset_contest") << "\n";
    std::cout << "Validate key: " << admin.ValidateAdminKey("supersecretkey123") << "\n\n";

    // Writer test
    Writer writer("writerUser", "writer@cyberrange.com");
    writer.AddArticle("Article001");
    writer.AddArticle("Article002");
    writer.SetSignature("WriterX");

    std::cout << "[WRITER]\n";
    std::cout << "Username: " << writer.GetUsername() << "\n";
    std::cout << "Signature: " << writer.GetSignature() << "\n";
    std::cout << "Articles: ";
    for (const auto& a : writer.GetArticles())
        std::cout << a << " ";
    std::cout << "\n\n";

    // Common user test
    Common common("commonUser", "common@cyberrange.com");
    common.AddSolvedChallenge("ChallA");
    common.AddSolvedChallenge("ChallB");
    common.SetTeam("TeamAlpha");
    common.AddScore(100);

    std::cout << "[COMMON USER]\n";
    std::cout << "Username: " << common.GetUsername() << "\n";
    std::cout << "Team: " << common.GetTeam() << "\n";
    std::cout << "Score: " << common.GetScore() << "\n";
    std::cout << "Challenges solved: ";
    for (const auto& c : common.GetSolvedChallenges())
        std::cout << c << " ";
    std::cout << "\n";

    return 0;
}