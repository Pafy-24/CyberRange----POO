#include "MainMenu.h"
#include <QApplication>
#include <QPropertyAnimation>
#include "ui_MainMenu.h"
#include "ui_LoginDialog.h"
#include "CustomSerial.h"
#include "LoginDialog.h"
#include "Client.h"
#include <qmessagebox.h>
#include "AuthController.h"
#include "ContestClientController.h"
#include "ClientMng.h"

#include <string>
#include <regex>

using jaon = nlohmann::json;

MainMenu::MainMenu(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainMenu)
{
    ui->setupUi(this);
    this->resize(900, 700);
    this->setFixedSize(900, 700);
    ui->stackedWidget->setCurrentIndex(3); // Home screen
	ui->stackedWidget_2->setCurrentIndex(0); 
	ui->stackedWidget_3->setCurrentIndex(0); 

    // Set up logo and alignments
    QPixmap pix(":/background/logo.png");
    ui->logoLabel->setPixmap(pix.scaled(300, 200, Qt::KeepAspectRatio));
    ui->titleLabel->setAlignment(Qt::AlignCenter);
    ui->quoteLabel->setAlignment(Qt::AlignCenter);
    ui->textBrowser->setAlignment(Qt::AlignCenter);

    connect(ui->contestWidget, &QTableWidget::cellClicked, this, &MainMenu::onContestCellClicked); 
    // Apply styling
    ui->frameLeftMenu->setStyleSheet(
        "QFrame {"
        "background-color: rgba(13, 3, 25, 255);"
        "border-radius: 10px;"
        "}"
    );
    ui->centralwidget->setStyleSheet(
        "background-color: rgba(13, 3, 25, 255);"
        "border-radius: 10px;"
    );
    ui->Content->setStyleSheet(
        "QFrame {"
        "background-color: #674983;"
        "border-radius: 10px;"
        "}"
    );
    ui->frame_pages->setStyleSheet(
        "QFrame {"
        "background-color: rgba(13, 3, 25, 255);"
        "border-radius: 10px;"
        "}"
    );

    ui->scoreboardButton->hide();
    ui->tasksButton->hide();
    ui->CreateTeamBtn->hide();

    // Fade-in animation effect
    setWindowOpacity(0.0);
    QPropertyAnimation* fadeIn = new QPropertyAnimation(this, "windowOpacity");
    fadeIn->setDuration(500);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

    // Set up signal connections for the AuthController
    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (authCtrl) {
        // Connect signals with queued connection for thread safety
        connect(authCtrl, &AuthController::logoutSucceeded, this, &MainMenu::handleLogoutSuccess, Qt::QueuedConnection);
        connect(authCtrl, &AuthController::logoutFailed, this, &MainMenu::handleLogoutFailure, Qt::QueuedConnection);
        connect(authCtrl, &AuthController::updateSucceeded, this, &MainMenu::handleUpdateSuccess, Qt::QueuedConnection);
        connect(authCtrl, &AuthController::updateFailed, this, &MainMenu::handleUpdateFailure, Qt::QueuedConnection);
        connect(authCtrl, &AuthController::deleteSucceeded, this, &MainMenu::handleDeleteSuccess, Qt::QueuedConnection);
        connect(authCtrl, &AuthController::deleteFailed, this, &MainMenu::handleDeleteFailure, Qt::QueuedConnection);
    }
    else {
        QMessageBox::warning(this, "Error", "AuthController is not available. Some functions may not work properly.");
    }
}

MainMenu::~MainMenu()
{
    // Disconnect all signals to prevent callbacks after destruction
    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (authCtrl) {
        disconnect(authCtrl, nullptr, this, nullptr);
    }

    delete ui;
}

void MainMenu::handleLogoutSuccess()
{
    // Create a new login dialog before starting the animation
    LoginDialog* login = new LoginDialog();

    QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
    fadeOut->setDuration(500);
    fadeOut->setStartValue(1);
    fadeOut->setEndValue(0);

    connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
        // Show the login dialog
        login->show();

        // Clean up properly
        fadeOut->deleteLater();
        this->hide();  // Hide first
        this->deleteLater();  // Then schedule for deletion
        });

    fadeOut->start();
}

void MainMenu::handleLogoutFailure(const QString& message)
{
    QMessageBox::warning(this, "Error", "Logout failed:\n" + message);
    ui->pushButtonLogout->setEnabled(true);
}

void MainMenu::on_pushButtonLogout_clicked()
{
    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (!authCtrl) {
        QMessageBox::warning(this, "Error", "AuthController unavailable.");
        return;
    }

    // Disable the logout button to prevent multiple clicks
    ui->pushButtonLogout->setEnabled(false);
    ui->pushButtonLogout->setText("Logging out...");
    QApplication::processEvents();

    // Show a confirmation dialog
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Logout", "Are you sure you want to logout?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        authCtrl->requestLogout();
    }
    else {
        // If user cancels, re-enable the button
        ui->pushButtonLogout->setEnabled(true);
        ui->pushButtonLogout->setText("Logout");
    }
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
    // Example: set 3 rows and 4 columns
    int rowCount = 3;
    ui->contestWidget->setRowCount(rowCount);
    ui->contestWidget->setColumnCount(3);

    // Set headers
    // Set column headers
    QStringList headers = {"Name", "Start Date", "End Date" };
    ui->contestWidget->setHorizontalHeaderLabels(headers);

    // Hide row numbers (vertical header)
    ui->contestWidget->verticalHeader()->setVisible(false);

    // adaugare contests

    // redimensionare coloane
    ui->contestWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainMenu::on_ManageUsersButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(7);
}

std::string parse_string(std::string& S) {
    //Remove double escapation with \ on string
	std::string result;
	std::regex re(R"(\\)");
	std::string replacement = "";
	result = std::regex_replace(S, re, replacement);
	result = result.substr(1, result.size() - 2); 
	return result;
}

void MainMenu::on_ProfileButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(9);
	auto userData = CustomSerial::decodeJWT(ClientMng::getInstance()->getAuthToken());
	auto userstring = userData.dump();
    userstring = parse_string(userstring);
	userData = json::parse(userstring);

	auto username = userData["username"].get<std::string>();
	auto email = userData["email"].get<std::string>();
	ui->label_9->setText("User: "+QString::fromStdString(username));
	ui->label_10->setText("Email: "+QString::fromStdString(email));
}

void MainMenu::on_ReviewFlagsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(6);
}

void MainMenu::on_SettingsButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(10);
}

void MainMenu::on_TabButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(12);
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
    // Disable the submit button to prevent multiple clicks
    ui->pushButtonSubmit->setEnabled(false);

    const std::string username = ui->lineEditNewUsername->text().toStdString();
    const std::string old = ui->lineEditOldPasswd->text().toStdString();
    const std::string password = ui->lineEditNewPasswd->text().toStdString();
    const std::string email = ui->lineEditNewEmail->text().toStdString();

    // Confirm changes
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Update",
        "Are you sure you want to update your profile? You will be logged out after updating.",
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        ui->pushButtonSubmit->setEnabled(true);
        return;
    }

    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (!authCtrl) {
        QMessageBox::warning(this, "Error", "AuthController unavailable.");
        ui->pushButtonSubmit->setEnabled(true);
        return;
    }

    ui->pushButtonSubmit->setText("Updating...");
    QApplication::processEvents();

    authCtrl->requestUpdate(username, old, password, email);
}

void MainMenu::on_pushButtonDeleteAcc_clicked()
{
    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (!authCtrl) {
        QMessageBox::warning(this, "Error", "AuthController unavailable.");
        return;
    }

    // Disable the button to prevent multiple clicks
    ui->pushButtonDeleteAcc->setEnabled(false);

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Deletion",
        "Are you sure you want to delete your account? This action cannot be undone!",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Double confirm for critical action
        reply = QMessageBox::warning(this, "Final Confirmation",
            "ALL YOUR DATA WILL BE PERMANENTLY DELETED. Continue?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            ui->pushButtonDeleteAcc->setText("Deleting...");
            QApplication::processEvents();
            authCtrl->requestDelete();
            // The response will be handled by signal handlers
        }
        else {
            ui->pushButtonDeleteAcc->setEnabled(true);
        }
    }
    else {
        ui->pushButtonDeleteAcc->setEnabled(true);
    }
}

void MainMenu::configureUIForRole(int role)
{
    // Hide everything first
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

    // Common elements for all users
    ui->HomeButton->show();
    ui->SettingsButton->show();
    ui->ProfileButton->show();

    // Set window title based on role
    QString roleText;

    switch (role)
    {
    case 1: // Regular user
        ui->DashboardCommonButton->show();
        ui->ContestsButton->show();
        ui->TabButton->show();
        roleText = "User Dashboard";
        break;
    case 5: // Writer
        ui->DashboardWriterButton->show();
        ui->AddChallengeButton->show();
        ui->ReviewFlagsButton->show();
        roleText = "Writer Dashboard";
        break;
    case 10: // Admin
        ui->ContestsButton->show();
        ui->DashboardAdminButton->show();
        ui->ManageUsersButton->show();
        ui->AddChallengeButton->show();
        ui->CreateContestButton->show();
        roleText = "Admin Dashboard";
        break;
    default:
        roleText = "Dashboard";
        break;
    }

    this->setWindowTitle(roleText);

    // Show welcome message with role
    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (authCtrl) {
        QString username = QString::fromStdString(authCtrl->getCurrentUser());
        ui->titleLabel->setText("Welcome, " + username + "!");
        ui->quoteLabel->setText("You are logged in as: " + roleText);
    }

    // Default to home screen
    ui->stackedWidget->setCurrentIndex(3);
}


void MainMenu::on_ConnButton_clicked()
{
    Client* client = new Client();
    client->show();
    this->close();
    this->deleteLater();
}

void MainMenu::onContestCellClicked(int row, int column)
{
    if (column == 0) 
    {
        QString contestName = ui->contestWidget->item(row, column)->text();
        // Exemplu: poți deschide o pagină dedicată concursului
        qDebug() << "Clicked contest:" << contestName;
        ///

        ui->stackedWidget->setCurrentIndex(13); // pagina cu detalii
    }
}

void MainMenu::on_ActiveButton_clicked()
{
	ui->stackedWidget_2->setCurrentIndex(0);
}

void MainMenu::on_ExpiredButton_clicked()
{
	ui->stackedWidget_2->setCurrentIndex(1);
}

void MainMenu::on_backButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(5);
}

void MainMenu::on_descriptionButton_clicked()
{
	ui->stackedWidget_3->setCurrentIndex(0);
    // de pus descriere in label
}

void MainMenu::on_scoreboardButton_clicked()
{
	ui->stackedWidget_3->setCurrentIndex(1);
    int rowCount = 3;
    ui->scoreboardWidget->setRowCount(rowCount);
    ui->scoreboardWidget->setColumnCount(3);

    // Set headers
    // Set column headers
    QStringList headers = { "#", "Team/User Name", "Score" };
    ui->scoreboardWidget->setHorizontalHeaderLabels(headers);

    // Hide row numbers (vertical header)
    ui->scoreboardWidget->verticalHeader()->setVisible(false);
    QVector<QPair<QString, int>> teams = {
        { "CyberWarriors", 750 },
        { "HackMasters", 620 },
        { "CTFAddicts", 590 },
        { "RedTeamElite", 400 }
    };

    // Exemplu de date statice (le poți înlocui cu cele din baza de date mai târziu)
    for (int i = 0; i < rowCount; ++i) {
        ui->scoreboardWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));               // #
        ui->scoreboardWidget->setItem(i, 1, new QTableWidgetItem(teams[i].first));                      // Team Name
        ui->scoreboardWidget->setItem(i, 2, new QTableWidgetItem(QString::number(teams[i].second)));    // Score
    }

    // redimensionare coloane
    ui->scoreboardWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainMenu::on_tasksButton_clicked()
{
	ui->stackedWidget_3->setCurrentIndex(2);
}

void MainMenu::handleUpdateSuccess()
{
    QMessageBox::information(this, "Success", "Profile updated successfully!");
    // The logout will be handled by the AuthController
}

void MainMenu::handleUpdateFailure(const QString& message)
{
    QMessageBox::warning(this, "Error", "Update failed:\n" + message);
    // Re-enable the submit button
    ui->pushButtonSubmit->setEnabled(true);
}

void MainMenu::handleDeleteSuccess()
{
    QMessageBox::information(this, "Success", "Account deleted successfully!");
    // The logout will be handled by the AuthController
}

void MainMenu::handleDeleteFailure(const QString& message)
{
    QMessageBox::warning(this, "Error", "Account deletion failed:\n" + message);
    // Re-enable the delete button
    ui->pushButtonDeleteAcc->setEnabled(true);
}