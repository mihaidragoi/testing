#include "ClientGame.h"
#include <iostream>
#include <exception>

ClientGame::ClientGame()
    : m_deckCount(0)
    , m_currentPlayerIndex(-1)
    , m_myPlayerIndex(-1)
    , m_cardsPlayedThisTurn(0)
{
}

void ClientGame::UpdateState(const nlohmann::json& j)
{
    try {

        m_gameStatus = j.at("status").get<std::string>();
        m_currentPlayerIndex = j.at("currentPlayerIndex").get<int>();
        m_cardsPlayedThisTurn = j.at("cardsPlayedThisTurn").get<int>();
        m_myPlayerIndex = j.at("yourIndex").get<int>();
        m_deckCount = j.at("deckCount").get<int>();

        m_localStacks.clear();
        const auto& stacksArray = j.at("stacks");

        m_localStacks.reserve(stacksArray.size());

        for (const auto& sJson : stacksArray) {
            std::string typeStr = sJson.at("type").get<std::string>();
            int topVal = sJson.at("topValue").get<int>();

            StackType type = (typeStr == "Ascending") ? StackType::Ascending : StackType::Descending;
            int baseVal = (type == StackType::Ascending) ? 1 : 100;

            Stack tempStack(type, baseVal);

            if (topVal != baseVal) {
                tempStack.PlaceCard(Card(topVal));
            }
            m_localStacks.push_back(std::move(tempStack));
        }

        m_myHand.clear();

        if (j.contains("players")) {
            const auto& playersArray = j.at("players");

            if (m_myPlayerIndex >= 0 && m_myPlayerIndex < static_cast<int>(playersArray.size())) {
                const auto& myData = playersArray[m_myPlayerIndex];

                if (myData.contains("hand")) {
                    const auto& handArray = myData.at("hand");
                    m_myHand.reserve(handArray.size());

                    for (const auto& cJson : handArray) {
                        int val = cJson.at("value").get<int>();
                        m_myHand.emplace_back(val);
                    }
                }
            }
        }

    }
    catch (const nlohmann::json::exception& e) {
        std::cerr << "[ClientGame Error] JSON Parsing failed: " << e.what() << '\n';
    }
    catch (const std::exception& e) {

        std::cerr << "[ClientGame Error] Standard exception: " << e.what() << '\n';
    }
    catch (...) {
        std::cerr << "[ClientGame Error] Unknown error occurred during UpdateState.\n";
    }
}

bool ClientGame::IsMyTurn() const {
    return m_currentPlayerIndex == m_myPlayerIndex && m_gameStatus == "Running";
}

bool ClientGame::CanPlayCardLocally(int handIndex, int stackIndex) const {
    if (!IsMyTurn()) return false;

    if (handIndex < 0 || handIndex >= static_cast<int>(m_myHand.size())) return false;
    if (stackIndex < 0 || stackIndex >= static_cast<int>(m_localStacks.size())) return false;

    return m_localStacks[stackIndex].CanPlay(m_myHand[handIndex]);
}

const std::vector<Card>& ClientGame::GetMyHand() const { return m_myHand; }
const std::vector<Stack>& ClientGame::GetStacks() const { return m_localStacks; }
int ClientGame::GetDeckCount() const { return m_deckCount; }
int ClientGame::GetCurrentPlayerIndex() const { return m_currentPlayerIndex; }
int ClientGame::GetMyPlayerIndex() const { return m_myPlayerIndex; }
std::string ClientGame::GetGameStatus() const { return m_gameStatus; }
int ClientGame::GetCardsPlayedThisTurn() const { return m_cardsPlayedThisTurn; }