#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Client.h"

class Client : public QMainWindow
{
    Q_OBJECT

public:
    Client(QWidget* parent = nullptr);
    ~Client();

private slots:
    void on_pushButton_clicked();

private:
    Ui::ClientClass ui;

    // Helper methods for tests
    void printMessage(const std::string& message);
    void testTCPClient(const std::string& serverAddress, int port, bool useTLS);
    void testUDPClient(const std::string& serverAddress, int port);
    void testDownload(const std::string& serverAddress, int port);
};