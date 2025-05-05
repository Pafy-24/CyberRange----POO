#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include <QTimer>
#include "MainMenu.h"
#include <QMouseEvent>
#include "ConnsFactory.h"
#include "ClientMng.h"
#include "AuthController.h"

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ClientMng::getInstance()->start();
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    ui->lineEditEmail->hide();
    ui->lineEditEmail2->hide();
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(this);
    ui->labelWelcome->setGraphicsEffect(effect);

    QPropertyAnimation* animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(1500); // 1.5 secunde
    animation->setStartValue(0);
    animation->setEndValue(1);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    this->setWindowOpacity(0.0);
    QPropertyAnimation* animation1 = new QPropertyAnimation(this, "windowOpacity");
    animation1->setDuration(500); // 500 ms
    animation1->setStartValue(0.0);
    animation1->setEndValue(1.0);
    animation1->start();

    ClientMng* clientMng = ClientMng::getInstance(1337, "127.0.0.1");
    clientMng->start();
    if (!clientMng->isConnected()) 
    {
        QMessageBox::critical(this, "Error", "Connection to server failed.");
        this->close();
        return;
    }
    else
    {
		ui->label->setText("Succesfully connected to server!");
    }
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginButton_clicked()
{
    const std::string username = ui->lineEditUsername->text().toStdString();
    const std::string password = ui->lineEditPassword->text().toStdString();

    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("AuthController"));
    if (!authCtrl) {
        QMessageBox::warning(this, "Eroare", "AuthController indisponibil.");
        return;
    }
    authCtrl->requestLogin(username, password);


    // conectăm semnalele o singură dată (sau într-un constructor)
    connect(authCtrl, &AuthController::loginSucceeded, this, [=]() {
        QMessageBox::information(this, "Succes", "Autentificare reușită!");

        std::string userRole = authCtrl->getRole();
        QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
        fadeOut->setDuration(500);
        fadeOut->setStartValue(1);
        fadeOut->setEndValue(0);

        connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
            MainMenu* menu = new MainMenu();
            menu->configureUIForRole(userRole);
            menu->show();
            this->close();
            });

        fadeOut->start();
        });

    connect(authCtrl, &AuthController::loginFailed, this, [=](const QString& message) {
        QMessageBox::warning(this, "Eroare", "Autentificare eșuată:\n" + message);
        });
}

void LoginDialog::on_registerButton_clicked()
{
	std::string userRole = "admin"; // Default role for new users
    MainMenu* menu = new MainMenu();
    menu->configureUIForRole(userRole);
    menu->show();
    this->close();
}

//void LoginDialog::on_registerButton_clicked()
//{
//    const std::string username = ui->lineEditUsername->text().toStdString();
//    const std::string password = ui->lineEditPassword->text().toStdString();
//    const std::string email = ui->lineEditEmail->text().toStdString();
//
//    auto* authCtrl = dynamic_cast<AuthController*>(ClientMng::getInstance()->getController("Auth"));
//    if (!authCtrl) {
//        QMessageBox::warning(this, "Eroare", "AuthController indisponibil.");
//        return;
//    }
//
//    connect(authCtrl, &AuthController::loginSucceeded, this, [=]() {
//        QMessageBox::information(this, "Succes", "Înregistrare reușită!");
//
//        std::string userRole = authCtrl->getRole();  // presupui că e „common”
//        QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
//        fadeOut->setDuration(500);
//        fadeOut->setStartValue(1);
//        fadeOut->setEndValue(0);
//
//        connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
//            MainMenu* menu = new MainMenu();
//            menu->configureUIForRole(userRole);
//            menu->show();
//            this->close();
//            });
//
//        fadeOut->start();
//        });
//
//    connect(authCtrl, &AuthController::loginFailed, this, [=](const QString& message) {
//        QMessageBox::warning(this, "Eroare", "Înregistrare eșuată:\n" + message);
//        });
//
//    authCtrl->requestRegister(username, password, email);
//}

void LoginDialog::on_exitButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Exit", "Are you sure you want to exit?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QApplication::quit();
    }
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