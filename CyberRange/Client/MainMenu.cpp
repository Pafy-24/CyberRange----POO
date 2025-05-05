#include "MainMenu.h"
#include <QApplication>
#include "ui_MainMenu.h"
#include "ui_LoginDialog.h"
#include "LoginDialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include "AuthController.h"
#include "ClientMng.h"

MainMenu::MainMenu( QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    this->resize(900, 700); 
    this->setFixedSize(900, 700);
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

void MainMenu::on_ContestsButton_clicked()
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

void MainMenu::on_pushButtonAddCh_clicked()
{
	ui->stackedWidget->setCurrentIndex(8);
}

void MainMenu::on_pushButtonQuickAddC_clicked()
{
	ui->stackedWidget->setCurrentIndex(4);
}

void MainMenu::on_pushButtonViewLC_clicked()
{
	ui->stackedWidget->setCurrentIndex(5);
}

void MainMenu::on_pushButtonSubmit_clicked()
{
    const std::string username = ui->lineEditNewUsername->text().toStdString();
    const std::string password = ui->lineEditNewPasswd->text().toStdString();
    const std::string email = ui->lineEditNewEmail->text().toStdString();

    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (!authCtrl) {
        QMessageBox::warning(this, "Eroare", "AuthController indisponibil.");
        return;
    }

    authCtrl->requestUpdate(username, password, email);
    ui->stackedWidget->setCurrentIndex(0);
}

void MainMenu::configureUIForRole(int role)
{
    // Ascundem totul la început
    ui->AddChallengeButton->hide();
    ui->CreateContestButton->hide();
    ui->DashboardAdminButton->hide();
    ui->DashboardWriterButton->hide();
    ui->DashboardCommonButton->hide();
    ui->HomeButton->hide();
    ui->ContestsButton->hide();
    ui->ManageUsersButton->hide();
    ui->ProfileButton->hide();
    ui->ReviewFlagsButton->hide();
    ui->SettingsButton->hide();
    ui->TabButton->hide();
    ui->TrainingButton->hide();

    // Elemente comune
    ui->HomeButton->show();
    ui->SettingsButton->show();
    ui->ProfileButton->show();
    

    switch (role)
    {
    case 1:
        ui->DashboardCommonButton->show();
        ui->ContestsButton->show();
        ui->TrainingButton->show();
        ui->TabButton->show();
        break;
    case 5:
        ui->DashboardWriterButton->show();
        ui->AddChallengeButton->show();
        ui->ReviewFlagsButton->show();
        break;
    case 10:
        ui->DashboardAdminButton->show();
        ui->ManageUsersButton->show();
        ui->AddChallengeButton->show();
        ui->CreateContestButton->show();
        break;
    }
}

void MainMenu::on_ConnButton_clicked()
{
	Client* client = new Client();
    client->show();
	this->close();
}