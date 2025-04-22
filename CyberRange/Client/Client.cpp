#include "Client.h"

Client::Client(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
}

Client::~Client()
{}

//void Client::on_pushButton_clicked()
//{
//	QString ip = ui.lineEdit->text();
//	int port = ui.lineEdit_2->text().toInt();
//	QString message = ui.lineEdit_3->text();
//	if (ip.isEmpty() || port <= 0 || message.isEmpty()) {
//		QMessageBox::warning(this, "Input Error", "Please fill in all fields correctly.");
//		return;
//	}
//	// Assuming you have a method to send the message to the server
//	sendMessageToServer(ip, port, message);
//}