#include "MainMenu.h"
#include <QApplication>
#include "ui_MainMenu.h"
#include "ui_LoginDialog.h"
#include "LoginDialog.h"

MainMenu::MainMenu(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainMenu)
{
    ui->setupUi(this);
}

MainMenu::~MainMenu()
{
    delete ui;
}

void MainMenu::on_pushButton_clicked()
{
    this->close();
    LoginDialog* login = new LoginDialog();
    login->show();
}