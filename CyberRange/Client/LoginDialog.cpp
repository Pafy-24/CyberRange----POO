#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include "MainMenu.h"
#include <QMouseEvent>
#include "ConnsFactory.h"
#include "ClientMng.h"

LoginDialog::LoginDialog(QWidget* parent) : QDialog(parent), ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

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
    std::string _username = ui->lineEditUsername->text().toStdString();
    std::string _password = ui->lineEditPassword->text().toStdString();
    std::string role;
	
    // Trimit comanda AUTH
    std::string authCommand = "AUTH " + _username + " " + _password + " CyberRangeDB";

	ClientMng* clientMng = ClientMng::getInstance();
	clientMng->sendRequest(authCommand);
    std::string response = clientMng->getConnection()->receive();

    if (response.find("OK") == 0) 
    {
        QMessageBox::information(this, "Succes", "Logged in succesfully!");

        // Fade out LoginDialog
        QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
        fadeOut->setDuration(500);
        fadeOut->setStartValue(1);
        fadeOut->setEndValue(0);

		std::string role = response.substr(3); // extragere rol din raspuns
        connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
            // dupa fade-out: open MainMenu
            MainMenu* menu = new MainMenu(); // dacă MainMenu primește conexiunea prin constructor
            menu->show();
            this->close();
            });
        fadeOut->start();
    }
    else 
    {
        QMessageBox::warning(this, "Error", "Login failed:\n" + QString::fromStdString(response));
    }

    //QString username = ui->lineEditUsername->text();
    //QString password = ui->lineEditPassword->text();

    //if (username == "admin" && password == "1234")
    //{ 
    //    // Fade out LoginDialog
    //    QPropertyAnimation* fadeOut = new QPropertyAnimation(this, "windowOpacity");
    //    fadeOut->setDuration(500);
    //    fadeOut->setStartValue(1);
    //    fadeOut->setEndValue(0);

    //    connect(fadeOut, &QPropertyAnimation::finished, this, [=]() {
    //        // dupa fade-out: open MainMenu
    //        MainMenu* menu = new MainMenu();
    //        menu->show();
    //        this->close(); 
    //        });
    //    fadeOut->start();
    //}
    //else 
    //{
    //    QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    //}
}

void LoginDialog::on_registerButton_clicked()
{
	std::string userRole = "admin"; // Default role for new users
    MainMenu* menu = new MainMenu();
    menu->configureUIForRole(userRole);
    menu->show();
    this->close();
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