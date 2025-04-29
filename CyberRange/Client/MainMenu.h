#pragma once
#ifndef MAINMENU_H
#define MAINMENU_H

#include <QMainWindow>
#include "ui_MainMenu.h"
#include <QApplication>

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenu; }
QT_END_NAMESPACE

class MainMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget* parent = nullptr);
    ~MainMenu();

private slots:
    void on_logoutButton_clicked();
    void on_ChallengesButton_clicked();
    void on_CompeteButton_clicked();
	void on_DashboardButton_clicked();
	void on_HomeButton_clicked();
	void on_LeaderboardButton_clicked();
	void on_ProfileButton_clicked();
	void on_SettingsButton_clicked();
	void on_TrainingButton_clicked();

    void on_ConnButton_clicked();

private:
    Ui::MainMenu* ui;
};

#endif // MAINMENU_H