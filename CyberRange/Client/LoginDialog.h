#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ui_LoginDialog.h"

namespace Ui {
    class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

signals:
    void loginSuccess();

private slots:
    void on_pushloginButton_clicked();
    void on_pushregisterButton_clicked();

private:
    Ui::LoginDialog* ui;
};

#endif // LOGINDIALOG_H