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
    void on_pushButton_clicked();

private:
    Ui::MainMenu* ui;
};

#endif // MAINMENU_H