#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>

LoginDialog::LoginDialog(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_pushButtonLogin_clicked()
{
    QString user = ui->lineEditUsername->text();
    QString pass = ui->lineEditPassword->text();

    if (user == "admin" && pass == "1234") {
        emit loginSuccess(); // dacă vrei să conectezi asta extern
        accept(); // închide dialogul cu succes
    }
    else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password.");
    }
}

void LoginDialog::on_pushButtonCancel_clicked()
{
    reject(); // închide cu eșec
}