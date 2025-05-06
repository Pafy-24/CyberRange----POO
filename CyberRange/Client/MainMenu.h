#pragma once
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
    explicit MainMenu(QWidget* parent = nullptr);
    void configureUIForRole(int role);
    ~MainMenu();

private slots:
    // Slots for logout handling
    void handleLogoutSuccess();
    void handleLogoutFailure(const QString& message);


    void handleUpdateSuccess();
    void handleUpdateFailure(const QString& message);
    void handleDeleteSuccess();
    void handleDeleteFailure(const QString& message);

    void handleLoadContests();
	void handleContestDetailsLoad();

    void on_pushButtonLogout_clicked();
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
	
    // Dashboard admin
    void on_pushButtonAddCh_clicked();
    void on_pushButtonQuickAddC_clicked();
    void on_pushButtonViewLC_clicked();

    // Profile  
    void on_pushButtonSubmit_clicked();
    void on_pushButtonDeleteAcc_clicked();

    void on_ConnButton_clicked();

    // Contests
    void onContestCellClicked(int row, int column);
	void on_ActiveButton_clicked();
	void on_ExpiredButton_clicked();
	void on_backButton_clicked();
	void on_descriptionButton_clicked();
	void on_scoreboardButton_clicked();
	void on_tasksButton_clicked(); 
	void on_JoinButton_clicked();

    // Tabs
    void onTabSelected(int row);
    void on_TabButton_clicked();

	// Add challenge

private:
    Ui::MainMenu* ui;
    std::unique_ptr<Connection> tcpClient;
};