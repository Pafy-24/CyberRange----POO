#include "MainMenu.h"
#include <QApplication>
#include "ui_MainMenu.h"
#include "ui_LoginDialog.h"
#include "LoginDialog.h"

MainMenu::MainMenu(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    ui->frameLeftMenu->setStyleSheet(
        "QFrame {"
        "background-color: rgba(13, 3, 25, 255);"
        "border-radius: 10px;"
        "}"
    );
	ui->centralwidget->setStyleSheet( // pentru separatori intre meniuri si pagina
        "background-color: rgba(13, 3, 25, 255);" // #b17ddf
        "border-radius: 10px;"
        "}"
    );
    ui->Content->setStyleSheet(
        "QFrame {"
        "background-color: #674983;" // #6a4c87
        "border-radius: 10px;"
        "}"
    );
    ui->frame_pages->setStyleSheet(
        "QFrame {"
        "background-color: rgba(13, 3, 25, 255);"
        "border-radius: 10px;"
        "}"
    );
    /*ui->stackedWidget->setStyleSheet(
        "QStackedWidget {"
        "background-image: url(:/background/fundal2.jpg);"
        "background-repeat: no-repeat;"
        "background-position: center;"
        "background-size: cover;"
        "}"
    );*/

    /*ui->HomeButton->setStyleSheet(
        " QPushButton {"
        "background-color: transparent;"
        "border-radius: 100px;"
        "border: 2px solid #b17ddf;"
        "color: white;"
        "}"

        "QPushButton:hover {"
        "background - color: transparent;"
        "border: 2px solid white;"
        "}"

        "QPushButton:pressed{"
        "background - color: transparent;"
        "border: 2px solid white;"
        "}"
    );*/
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