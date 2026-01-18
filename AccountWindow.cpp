#include "AccountWindow.h"
#include <MainGameWindow.h>

AccountWindow::AccountWindow(QWidget *parent)
	: QMainWindow(parent)
{
	m_ui_account_window.setupUi(this);
	connect(m_ui_account_window.pushButton, &QPushButton::clicked, this, &AccountWindow::On_pushButton_clicked);
}

AccountWindow::~AccountWindow()
{}

void AccountWindow::On_pushButton_clicked()
{
	auto* mainGameWindow = new MainGameWindow(nullptr);
	mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
	mainGameWindow->show();
	this->close();
}

