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

//void MainMenu::on_logoutButton_clicked()
//{
//    this->close();
//    LoginDialog* login = new LoginDialog();
//    login->show();
//}

void MainMenu::on_ChallengesButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(3);
}

void MainMenu::on_CompeteButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(7);
}

void MainMenu::on_DashboardButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}
void MainMenu::on_HomeButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(1);
}
void MainMenu::on_LeaderboardButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(2);
}
void MainMenu::on_ProfileButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(4);
}
void MainMenu::on_SettingsButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(5);
}
void MainMenu::on_TrainingButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(6);
}