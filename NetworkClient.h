#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <vector>

struct GameState {
    std::vector<int> m_stacks;
    std::vector<int> m_playerHand;
    bool m_isMyTurn;
    std::string m_status;
    int m_deckCount = 0;
};

class NetworkClient {
public:
    static std::string m_username;

    static bool RegisterUser(const std::string& username, const std::string& password);
    static bool LoginUser(const std::string& username, const std::string& password);

    static int CreateGame(const std::string& pin);
    static int JoinGame(const std::string& pin);
    static int JoinRandomGame();

    static std::vector<std::string> GetMessages(const std::string& gameId);
    static bool SendMessage(const std::string& gameId, const std::string& message);
    static bool PlayCard(const std::string& gameId, int cardValue, int stackIndex);
    static bool EndTurn(const std::string& gameId);
    static bool StartGame(const std::string& gameId);

    static GameState GetGameState(const std::string& gameId);
	static bool LeaveGame(const std::string& gameId);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
};