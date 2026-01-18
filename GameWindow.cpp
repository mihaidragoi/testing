#include "GameWindow.h"
#include "NetworkClient.h"
#include "MainGameWindow.h"
#include <QMessageBox>
#include <vector>
#include <algorithm> 

GameWindow::GameWindow(std::string gameId, QWidget* parent)
    : QMainWindow(parent), m_currentGameId(gameId)
{
    m_game_window_ui.setupUi(this);
    connect(m_game_window_ui.m_endTurnButton, &QPushButton::clicked, this, &GameWindow::On_EndTurn_Clicked);
    connect(m_game_window_ui.m_startGameButton, &QPushButton::clicked, this, &GameWindow::On_StartGame_Clicked);

    std::vector<QPushButton*> potentialButtons = {
        m_game_window_ui.card1, m_game_window_ui.card2, m_game_window_ui.card3, m_game_window_ui.card4,
        m_game_window_ui.card5, m_game_window_ui.card6, m_game_window_ui.card7, m_game_window_ui.card8
    };

    m_handButtons.clear();
    for (auto* btn : potentialButtons) {
        if (btn) {
            m_handButtons.push_back(btn);
            connect(btn, &QPushButton::clicked, this, &GameWindow::On_HandCard_Clicked);
        }
    }

    if (m_game_window_ui.deckAscending1) {
        m_game_window_ui.deckAscending1->setProperty("stackIndex", 0);
        connect(m_game_window_ui.deckAscending1, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }
    if (m_game_window_ui.deckAscending2) {
        m_game_window_ui.deckAscending2->setProperty("stackIndex", 1);
        connect(m_game_window_ui.deckAscending2, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }
    if (m_game_window_ui.deckDescending1) {
        m_game_window_ui.deckDescending1->setProperty("stackIndex", 2);
        connect(m_game_window_ui.deckDescending1, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }
    if (m_game_window_ui.deckDescending2) {
        m_game_window_ui.deckDescending2->setProperty("stackIndex", 3);
        connect(m_game_window_ui.deckDescending2, &QPushButton::clicked, this, &GameWindow::On_Stack_Clicked);
    }

    m_chatUpdateTimer = new QTimer(this);
    connect(m_chatUpdateTimer, &QTimer::timeout, this, &GameWindow::UpdateGameState);
    m_chatUpdateTimer->start(1000);

    UpdateGameState();
    UpdateChatMessages();
}

GameWindow::~GameWindow()
{
}

void GameWindow::On_StartGame_Clicked()
{
    bool success = NetworkClient::StartGame(m_currentGameId);
    if (success) {
        if (m_game_window_ui.m_startGameButton) m_game_window_ui.m_startGameButton->hide();
        if (m_game_window_ui.m_endTurnButton) m_game_window_ui.m_endTurnButton->show();
        QMessageBox::information(this, "Info", "Game Started!");
        UpdateGameState();
    }
    else {
        QMessageBox::warning(this, "Error", "Failed to start game (Maybe already started?)");
    }
}

void GameWindow::On_EndTurn_Clicked()
{
    if (NetworkClient::EndTurn(m_currentGameId)) {
        UpdateGameState();
        m_selectedCardValue = -1;
    }
    else {
        QMessageBox::warning(this, "Action Failed", "Cannot end turn yet.");
    }
}

void GameWindow::UpdateChatMessages()
{
    if (!m_game_window_ui.messages) return;

    std::vector<std::string> messages = NetworkClient::GetMessages(m_currentGameId);
    if (messages.empty()) return;

    if (messages.size() > (size_t)m_game_window_ui.messages->count())
    {
        for (size_t i = m_game_window_ui.messages->count(); i < messages.size(); ++i)
        {
            m_game_window_ui.messages->addItem(QString::fromStdString(messages[i]));
        }
        m_game_window_ui.messages->scrollToBottom();
    }
}

void GameWindow::On_HandCard_Clicked()
{
    QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
    if (!clickedButton) return;
    if (clickedButton->text().isEmpty() && clickedButton->icon().isNull()) return;

    QVariant val = clickedButton->property("cardValue");
    if (val.isValid()) m_selectedCardValue = val.toInt();

    for (auto btn : m_handButtons) {
        if (btn) btn->setStyleSheet("");
    }
    clickedButton->setStyleSheet("border: 2px solid pink;");
}

void GameWindow::On_Stack_Clicked()
{
    if (m_selectedCardValue == -1) return;
    QPushButton* stackButton = qobject_cast<QPushButton*>(sender());
    if (!stackButton) return;

    int stackIndex = stackButton->property("stackIndex").toInt();
    bool isSucces = NetworkClient::PlayCard(m_currentGameId, m_selectedCardValue, stackIndex);
    if (isSucces)
    {
        m_selectedCardValue = -1;
        UpdateGameState();
        for (auto btn : m_handButtons) {
            if (btn) btn->setStyleSheet("");
        }
    }
}

void GameWindow::UpdateGameState()
{
    auto gameState = NetworkClient::GetGameState(m_currentGameId);

    if (gameState.m_status == "Won") {
        if (m_chatUpdateTimer) m_chatUpdateTimer->stop();
        QMessageBox::information(this, "Game Over", "You Won!");

        auto* mainGameWindow = new MainGameWindow(nullptr);
        mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainGameWindow->show();
        this->close();
        return;
    }
    if (gameState.m_status == "Lost") {
        if (m_chatUpdateTimer) m_chatUpdateTimer->stop();
        QMessageBox::information(this, "Game Over", "You Lost!");

        auto* mainGameWindow = new MainGameWindow(nullptr);
        mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
        mainGameWindow->show();
        this->close();
        return;
    }

    bool isRunning = (gameState.m_stacks.size() >= 4);

    if (isRunning) {
        if (m_game_window_ui.m_startGameButton) m_game_window_ui.m_startGameButton->hide();
        if (m_game_window_ui.m_endTurnButton) {
            m_game_window_ui.m_endTurnButton->show();
            m_game_window_ui.m_endTurnButton->setEnabled(gameState.m_isMyTurn);
        }

        if (gameState.m_isMyTurn) this->setWindowTitle("The Game - YOUR TURN");
        else this->setWindowTitle("The Game - Waiting for others...");

        if (m_game_window_ui.m_gameInfoLabel) {
            m_game_window_ui.m_gameInfoLabel->setText("Deck: " + QString::number(gameState.m_deckCount) + " cards");
        }

        if (m_game_window_ui.deckAscending1) {
            m_game_window_ui.deckAscending1->setText(QString::number(gameState.m_stacks[0]));
            SetCardImage(m_game_window_ui.deckAscending1, gameState.m_stacks[0]);
        }
        if (m_game_window_ui.deckAscending2) {
            m_game_window_ui.deckAscending2->setText(QString::number(gameState.m_stacks[1]));
            SetCardImage(m_game_window_ui.deckAscending2, gameState.m_stacks[1]);
        }
        if (m_game_window_ui.deckDescending1) {
            m_game_window_ui.deckDescending1->setText(QString::number(gameState.m_stacks[2]));
            SetCardImage(m_game_window_ui.deckDescending1, gameState.m_stacks[2]);
        }
        if (m_game_window_ui.deckDescending2) {
            m_game_window_ui.deckDescending2->setText(QString::number(gameState.m_stacks[3]));
            SetCardImage(m_game_window_ui.deckDescending2, gameState.m_stacks[3]);
        }

        UpdateHand(gameState.m_playerHand);
    }
    else {
        if (m_game_window_ui.m_startGameButton) m_game_window_ui.m_startGameButton->show();
        if (m_game_window_ui.m_endTurnButton) m_game_window_ui.m_endTurnButton->hide();
        this->setWindowTitle("The Game - Lobby");
        if (m_game_window_ui.m_gameInfoLabel) m_game_window_ui.m_gameInfoLabel->setText("Waiting to start...");
    }

    UpdateChatMessages();
}

void GameWindow::UpdateHand(const std::vector<int>& cardsReceived)
{
    size_t safeLimit = (std::min)(static_cast<size_t>(cardsReceived.size()), static_cast<size_t>(m_handButtons.size()));

    for (size_t i = 0; i < safeLimit; ++i)
    {
        QPushButton* btn = m_handButtons[i];
        if (!btn) continue;

        int val = cardsReceived[i];
        QString imagePath = QString(":/cards/GameImages/GameCards/ResizedCards/GameCard%1.jpg").arg(val);
        btn->setIcon(QIcon(imagePath));
        btn->setIconSize(QSize(80, 120));
        btn->setText("");
        btn->setProperty("cardValue", val);
        btn->setEnabled(true);
        btn->show();
    }

    for (size_t i = safeLimit; i < m_handButtons.size(); ++i)
    {
        QPushButton* btn = m_handButtons[i];
        if (!btn) continue;

        btn->setText("");
        btn->setProperty("cardValue", -1);
        btn->setEnabled(false);
        btn->hide();
    }
}

void GameWindow::SetCardImage(QPushButton* button, int cardValue)
{
    if (!button || cardValue < 0) return;

    QString imagePath = QString(":/cards/GameImages/GameCards/ResizedCards/GameCard%1.jpg").arg(cardValue);
    QIcon cardIcon(imagePath);
    if (!cardIcon.isNull())
    {
        button->setIcon(cardIcon);
        button->setIconSize(QSize(80, 120));
        button->setText("");
        button->setStyleSheet("border: none; background : transparent;");
    }
    else
    {
        button->setText(QString::number(cardValue));
    }
}

void GameWindow::On_leaveGameButton_clicked()
{
	auto confirm = QMessageBox::question(this, "Leave Game", "Are you sure you want to leave the game?");
    if (confirm == QMessageBox::Yes)
    {
        bool success = NetworkClient::LeaveGame(m_currentGameId);
        if (success)
        {
            if (m_chatUpdateTimer) m_chatUpdateTimer->stop();
            auto* mainGameWindow = new MainGameWindow(nullptr);
            mainGameWindow->setAttribute(Qt::WA_DeleteOnClose);
            mainGameWindow->show();
            this->close();
        }
        else
        {
            QMessageBox::warning(this, "Error", "Failed to leave the game.");
        }
	}
}

void GameWindow::On_sendButton_clicked()
{
    if (!m_game_window_ui.messageCurrent) return;
    QString message = m_game_window_ui.messageCurrent->text();
    if (!message.isEmpty())
    {
        if (NetworkClient::SendMessage(m_currentGameId, message.toStdString()))
        {
            m_game_window_ui.messageCurrent->clear();
            UpdateChatMessages();
        }
    }
}