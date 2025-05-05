
#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include <QTimer>
#include <QShowEvent>
#include "MainMenu.h"
#include <QMouseEvent>
#include "ConnsFactory.h"
#include "ClientMng.h"
#include "AuthController.h"

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::LoginDialog), mousePressed(false), signalsConnected(false)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    ui->stackedWidget->setCurrentIndex(0);

    // Create fade-in effect for welcome label
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    ui->labelWelcome->setGraphicsEffect(effect);

    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(1500);
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // Create fade-in effect for the entire dialog
    this->setWindowOpacity(0.0);
    QPropertyAnimation* animation1 = new QPropertyAnimation(this, "windowOpacity");
    animation1->setDuration(500);
    animation1->setStartValue(0.0);
    animation1->setEndValue(1.0);
    animation1->start(QAbstractAnimation::DeleteWhenStopped);

    // Initialize client manager once with error handling
    try {
        ClientMng* clientMng = ClientMng::getInstance(1337, "127.0.0.1");
        if (!clientMng->isConnected()) {
            if (!clientMng->start()) {
                // Instead of immediately closing, show an error and allow retry
                QMessageBox::critical(this, "Connection Error",
                    "Could not connect to the server. Please check your network connection.");
                // Add a retry button to the login screen instead of closing immediately
                ui->loginButton->setText("Retry Connection");
                ui->loginButton->disconnect();
                connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::retryConnection);
                ui->lineEditUsername->setEnabled(false);
                ui->lineEditPassword->setEnabled(false);
                ui->registerButton->setEnabled(false);
                return;
            }
        }
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Connection error: %1").arg(e.what()));
        // Same approach as above - allow retry instead of immediate close
        ui->loginButton->setText("Retry Connection");
        ui->loginButton->disconnect();
        connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::retryConnection);
        ui->lineEditUsername->setEnabled(false);
        ui->lineEditPassword->setEnabled(false);
        ui->registerButton->setEnabled(false);
    }
}

LoginDialog::~LoginDialog()
{
    // Safely disconnect all signals to this object to prevent callbacks after destruction
    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (authCtrl) {
        disconnect(authCtrl, nullptr, this, nullptr);
    }

    delete ui;
}

void LoginDialog::retryConnection()
{
    // Attempt to reconnect to the server
    try {
        ui->loginButton->setEnabled(false);
        ui->loginButton->setText("Connecting...");

        ClientMng* clientMng = ClientMng::getInstance(1337, "127.0.0.1");
        if (clientMng->start()) {
            // Connection successful - restore normal UI state
            ui->loginButton->setText("Login");
            ui->loginButton->disconnect();
            connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::on_loginButton_clicked);
            ui->lineEditUsername->setEnabled(true);
            ui->lineEditPassword->setEnabled(true);
            ui->registerButton->setEnabled(true);
            QMessageBox::information(this, "Connection Restored", "Successfully connected to the server.");
        }
        else {
            ui->loginButton->setEnabled(true);
            ui->loginButton->setText("Retry Connection");
            QMessageBox::warning(this, "Connection Failed",
                "Could not connect to the server. Please check your network connection and try again.");
        }
    }
    catch (const std::exception& e) {
        ui->loginButton->setEnabled(true);
        ui->loginButton->setText("Retry Connection");
        QMessageBox::critical(this, "Error", QString("Connection error: %1").arg(e.what()));
    }
}

void LoginDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);

    // Connect signals only once when the dialog is shown
    if (!signalsConnected) {
        auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
        if (authCtrl) {
            // Disconnect any existing connections first to avoid duplicates
            disconnect(authCtrl, &AuthController::loginSucceeded, this, &LoginDialog::handleLoginSuccess);
            disconnect(authCtrl, &AuthController::loginFailed, this, &LoginDialog::handleLoginFailure);
            disconnect(authCtrl, &AuthController::registerSucceeded, this, &LoginDialog::handleRegisterSuccess);
            disconnect(authCtrl, &AuthController::registerFailed, this, &LoginDialog::handleRegisterFailure);

            // Now connect the signals using queued connections for thread safety
            connect(authCtrl, &AuthController::loginSucceeded, this, &LoginDialog::handleLoginSuccess, Qt::QueuedConnection);
            connect(authCtrl, &AuthController::loginFailed, this, &LoginDialog::handleLoginFailure, Qt::QueuedConnection);
            connect(authCtrl, &AuthController::registerSucceeded, this, &LoginDialog::handleRegisterSuccess, Qt::QueuedConnection);
            connect(authCtrl, &AuthController::registerFailed, this, &LoginDialog::handleRegisterFailure, Qt::QueuedConnection);

            signalsConnected = true;
        }
    }

    // Emit the shown signal
    emit shown();
}

void LoginDialog::handleLoginSuccess()
{
    QMessageBox::information(this, "Success", "Login successful!");

    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (!authCtrl) {
        QMessageBox::warning(this, "Error", "AuthController unavailable.");
        return;
    }

    int userRole = authCtrl->getRole();

    // Create the main menu before starting the animation
    MainMenu* menu = new MainMenu();
    menu->configureUIForRole(userRole);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
    fadeOut->setDuration(500);
    fadeOut->setStartValue(1);
    fadeOut->setEndValue(0);

    connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
        // Check if dialog is still visible before proceeding
        if (!this->isVisible()) {
            fadeOut->deleteLater();
            return;
        }

        // Show the main menu and schedule this dialog for deletion
        menu->show();
        fadeOut->deleteLater();
        this->hide();  // Hide first, then schedule deletion
        this->close();  // Use deleteLater for safer cleanup
        });

    fadeOut->start();
}

void LoginDialog::handleLoginFailure(const QString& message)
{
    QMessageBox::warning(this, "Error", "Login failed:\n" + message);
}

void LoginDialog::handleRegisterSuccess()
{
    ui->stackedWidget->setCurrentIndex(0);
    QMessageBox::information(this, "Success", "Registration successful! You can now log in.");
}

void LoginDialog::handleRegisterFailure(const QString& message)
{
    QMessageBox::warning(this, "Error", "Registration failed:\n" + message);
}

void LoginDialog::on_loginButton_clicked()
{
    // Disable the button to prevent double clicks
    ui->loginButton->setEnabled(false);

    const std::string username = ui->lineEditUsername->text().toStdString();
    const std::string password = ui->lineEditPassword->text().toStdString();

    if (username.empty() || password.empty()) {
        QMessageBox::warning(this, "Error", "Username and password are required.");
        ui->loginButton->setEnabled(true);
        return;
    }

    try {
        auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
        if (!authCtrl) {
            QMessageBox::warning(this, "Error", "AuthController unavailable.");
            ui->loginButton->setEnabled(true);
            return;
        }

        // Show loading indicator
        ui->loginButton->setText("Please wait...");
        QApplication::processEvents();

        authCtrl->requestLogin(username, password);

        // Re-enable the button after a short delay
        QTimer::singleShot(1000, [this]() {
            if (this && isVisible()) {
                ui->loginButton->setEnabled(true);
                ui->loginButton->setText("Login");
            }
            });
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Login request error: %1").arg(e.what()));
        ui->loginButton->setEnabled(true);
        ui->loginButton->setText("Login");
    }
}

void LoginDialog::on_registerButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void LoginDialog::on_exitButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Exit", "Are you sure you want to exit?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void LoginDialog::on_pushButtonRegisterNow_clicked()
{
    // Disable the button to prevent double clicks
    ui->pushButtonRegisterNow->setEnabled(false);

    const std::string username = ui->lineEditUsernameReg->text().toStdString();
    const std::string password = ui->lineEditPasswdReg->text().toStdString();
    const std::string email = ui->lineEditEmailReg->text().toStdString();

    // Validate input
    if (username.empty() || password.empty() || email.empty()) {
        QMessageBox::warning(this, "Error", "All fields are required.");
        ui->pushButtonRegisterNow->setEnabled(true);
        return;
    }

    try {
        auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
        if (!authCtrl) {
            QMessageBox::warning(this, "Error", "AuthController unavailable.");
            ui->pushButtonRegisterNow->setEnabled(true);
            return;
        }

        // Show loading indicator
        ui->pushButtonRegisterNow->setText("Please wait...");
        QApplication::processEvents();

        authCtrl->requestRegister(username, password, email);

        // Re-enable the button after a short delay
        QTimer::singleShot(1000, [this]() {
            if (this && isVisible()) {
                ui->pushButtonRegisterNow->setEnabled(true);
                ui->pushButtonRegisterNow->setText("Register Now");
            }
            });
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Registration request error: %1").arg(e.what()));
        ui->pushButtonRegisterNow->setEnabled(true);
        ui->pushButtonRegisterNow->setText("Register Now");
    }
}

void LoginDialog::on_pushButtonBackToLogin_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void LoginDialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        mousePressed = true;
        mouseClickPosition = event->globalPosition().toPoint() - this->frameGeometry().topLeft();
    }
}

void LoginDialog::mouseMoveEvent(QMouseEvent* event)
{
    if (mousePressed) {
        this->move(event->globalPosition().toPoint() - mouseClickPosition);
    }
}

void LoginDialog::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        mousePressed = false;
    }
}