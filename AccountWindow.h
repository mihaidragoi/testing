#pragma once

#include <QMainWindow>
#include "ui_AccountWindow.h"
#include <memory>

class AccountWindow : public QMainWindow
{
	Q_OBJECT

public:
	AccountWindow(QWidget *parent = nullptr);
	~AccountWindow();

private slots:
	void On_pushButton_clicked();

private:
	Ui::AccountWindowClass m_ui_account_window;
};

