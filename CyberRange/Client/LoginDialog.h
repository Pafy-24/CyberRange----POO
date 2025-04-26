#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "ui_LoginDialog.h"
#include <QMouseEvent>

namespace Ui {
    class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    void loginSuccess();

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_exitButton_clicked();
private:
    Ui::LoginDialog* ui;
    QPoint mouseClickPosition;
    bool mousePressed;
};

#endif // LOGINDIALOG_H