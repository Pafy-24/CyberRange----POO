#pragma once
#include <QDialog>
#include <QMouseEvent>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QDebug>
#include "ConnsFactory.h"

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
    void showEvent(QShowEvent* event) override;

signals:
    void loginSuccess();
    void registrationSuccess();
    void shown();  // New signal to indicate the dialog is shown

private slots:
    void on_loginButton_clicked();
    void on_registerButton_clicked();
    void on_exitButton_clicked();
    void on_pushButtonRegisterNow_clicked();
    void on_pushButtonBackToLogin_clicked();

    // Handler slots for AuthController signals
    void handleLoginSuccess();
    void handleLoginFailure(const QString& message);
    void handleRegisterSuccess();
    void handleRegisterFailure(const QString& message);

public slots:
    void retryConnection();

private:
    Ui::LoginDialog* ui;
    QPoint mouseClickPosition;
    bool mousePressed;

    // Track whether connections have been established
    bool signalsConnected;
};