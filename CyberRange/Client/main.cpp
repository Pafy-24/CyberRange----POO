#include <QApplication>
#include "LoginDialog.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    LoginDialog login;
    login.show(); // sau login.exec() 
    return a.exec();
}