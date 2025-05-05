#pragma once
#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMainWindow>
#include "ui_MainMenu.h"
#include <QApplication>
#include "ConnsFactory.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenu; }
QT_END_NAMESPACE

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenu( QWidget* parent = nullptr);
	void configureUIForRole(int role);
    ~MainMenu();

private slots:
    void on_logoutButton_clicked();
    void on_AddChallengeButton_clicked();
    void on_CreateContestButton_clicked();
	void on_DashboardAdminButton_clicked();
	void on_DashboardCommonButton_clicked();
	void on_DashboardWriterButton_clicked();
	void on_HomeButton_clicked();
	void on_ContestsButton_clicked();
	void on_ManageUsersButton_clicked();
	void on_ProfileButton_clicked();
	void on_ReviewFlagsButton_clicked();
	void on_SettingsButton_clicked();
	void on_SolveChallengeButton_clicked();
	void on_TrainingButton_clicked();

	/// dasboard admin
	void on_pushButtonAddCh_clicked();
	void on_pushButtonQuickAddC_clicked();
	void on_pushButtonViewLC_clicked();


    void on_ConnButton_clicked();

private:
    Ui::MainMenu* ui;
    std::unique_ptr<Connection> tcpClient;
};

#endif // MAINMENU_H