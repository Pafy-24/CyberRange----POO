#include "MainMenu.h"
#include <QApplication>
#include "ui_MainMenu.h"
#include "ui_LoginDialog.h"
#include "LoginDialog.h"
#include "Client.h"

MainMenu::MainMenu( QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(3);
    QPixmap pix(":/background/logo.png");
    ui->logoLabel->setPixmap(pix.scaled(300, 200, Qt::KeepAspectRatio));
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->quoteLabel->setAlignment(Qt::AlignCenter);
    ui->textBrowser->setAlignment(Qt::AlignCenter);

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

void MainMenu::on_logoutButton_clicked()
{
    this->close();
    LoginDialog* login = new LoginDialog();
    login->show();
}

void MainMenu::on_AddChallengeButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(8);
}

void MainMenu::on_CreateContestButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(4);
}

void MainMenu::on_DashboardAdminButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void MainMenu::on_DashboardCommonButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void MainMenu::on_DashboardWriterButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}

void MainMenu::on_HomeButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(3);
}

void MainMenu::on_JoinContestButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(5);
}

void MainMenu::on_ManageUsersButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(7);
}

void MainMenu::on_ProfileButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(9);
}

void MainMenu::on_ReviewFlagsButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(6);
}

void MainMenu::on_SettingsButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(10);
}

void MainMenu::on_SolveChallengeButton_clicked()
{
	ui->stackedWidget->setCurrentIndex(12);
}

void MainMenu::on_TrainingButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(11);
}

void MainMenu::configureUIForRole(std::string role)
{
    // Ascundem totul la început
    ui->AddChallengeButton->hide();
    ui->CreateContestButton->hide();
    ui->DashboardAdminButton->hide();
    ui->DashboardWriterButton->hide();
    ui->DashboardCommonButton->hide();
    ui->HomeButton->hide();
    ui->JoinContestButton->hide();
    ui->ManageUsersButton->hide();
    ui->ProfileButton->hide();
    ui->ReviewFlagsButton->hide();
    ui->SettingsButton->hide();
    ui->SolveChallengeButton->hide();
    ui->TrainingButton->hide();

    // Elemente comune
    ui->HomeButton->show();
    ui->SettingsButton->show();
    ui->ProfileButton->show();
    

    if (role == "admin") 
    {
        ui->DashboardAdminButton->show();
        ui->ManageUsersButton->show();
        ui->AddChallengeButton->show();
        ui->CreateContestButton->show();
    }
    else if (role == "writer") 
    {
        ui->DashboardWriterButton->show();
        ui->AddChallengeButton->show();
        ui->ReviewFlagsButton->show();
    }
    else if (role == "common") 
    {
        ui->DashboardCommonButton->show();
        ui->JoinContestButton->show();
        ui->TrainingButton->show();
        ui->SolveChallengeButton->show();
    }
}

void MainMenu::on_ConnButton_clicked()
{
	Client* client = new Client();
    client->show();
	this->close();
}