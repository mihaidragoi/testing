#pragma once

#include <QMainWindow>
#include "ui_GameWindow.h"
#include <qtimer.h>
#include <QPushButton>
#include <QLabel>

class GameWindow : public QMainWindow
{
	Q_OBJECT

public:
	GameWindow(std::string gameId, QWidget* parent = nullptr);
	~GameWindow();

private:
	Ui::GameWindowClass m_game_window_ui;
	QTimer* m_chatUpdateTimer;
	std::string m_currentGameId;
	QList<QPushButton*> m_handButtons;
	int m_selectedCardValue = -1;

private slots:
	void On_sendButton_clicked();
	void UpdateChatMessages();
	void On_HandCard_Clicked();
	void On_Stack_Clicked();
	void On_EndTurn_Clicked();
	void On_StartGame_Clicked();
	void UpdateGameState();
	void UpdateHand(const std::vector<int>& cardsReceived);
	void SetCardImage(QPushButton* button, int cardValue);
	void On_leaveGameButton_clicked();
};