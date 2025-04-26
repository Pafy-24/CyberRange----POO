#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "MainMenu.h"
#include <QMouseEvent>

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
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_loginButton_clicked()
{
    QString username = ui->lineEditUsername->text();
    QString password = ui->lineEditPassword->text();

    if (username == "admin" && password == "1234") 
    {
        MainMenu* menu = new MainMenu();
        menu->show();
        this->close(); // inchide fereastra de login
    }
    else 
    {
        QMessageBox::warning(this, "Login Failed", "Wrong username or password.");
    }
}

void LoginDialog::on_registerButton_clicked()
{
    reject(); // închide cu eșec
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