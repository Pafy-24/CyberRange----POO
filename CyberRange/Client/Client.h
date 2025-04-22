#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Client.h"

class Client : public QMainWindow
{
    Q_OBJECT

public:
    void printMessage(const std::string& message);
    void testTCPClient(const std::string& serverAddress, int port);
    void testUDPClient(const std::string& serverAddress, int port);
    void testDBClient();
    void testDownload(const std::string& serverAddress, int port);
    Client(QWidget *parent = nullptr);
    ~Client();


private:
    Ui::ClientClass ui;
private slots:
    void on_pushButton_clicked();

};
