#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include "game_types.h"

namespace CS16Capture {

/**
 * @brief WebSocket client for sending game data
 */
class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();

    /**
     * @brief Connect to the WebSocket server
     * @param host Server host (e.g., "localhost")
     * @param port Server port (e.g., 8080)
     * @return true if connection was successful
     */
    bool connect(const std::string& host, int port);

    /**
     * @brief Disconnect from the WebSocket server
     */
    void disconnect();

    /**
     * @brief Check if connected to the server
     * @return true if connected
     */
    bool isConnected() const;

    /**
     * @brief Send game state to the server
     * @param state Game state to send
     * @return true if send was successful
     */
    bool sendGameState(const GameState& state);

    /**
     * @brief Send a raw JSON message
     * @param jsonMessage JSON message to send
     * @return true if send was successful
     */
    bool sendMessage(const std::string& jsonMessage);

    /**
     * @brief Set auto-reconnect on disconnect
     * @param enable Enable/disable auto-reconnect
     */
    void setAutoReconnect(bool enable);

    /**
     * @brief Get the number of pending messages
     * @return Number of messages in the send queue
     */
    size_t getPendingMessageCount() const;

private:
    /**
     * @brief Convert game state to JSON string
     */
    std::string gameStateToJson(const GameState& state);

    /**
     * @brief Convert game event to string
     */
    std::string gameEventToString(GameEvent event);

    /**
     * @brief Background thread for sending messages
     */
    void sendThreadFunc();

    /**
     * @brief Attempt to reconnect
     */
    bool tryReconnect();

    std::atomic<bool> connected_;
    std::atomic<bool> autoReconnect_;
    std::atomic<bool> shouldStop_;
    
    std::string host_;
    int port_;

    // Message queue for async sending
    std::queue<std::string> messageQueue_;
    std::mutex queueMutex_;
    
    // Send thread
    std::unique_ptr<std::thread> sendThread_;

#ifdef _WIN32
    // Windows socket handle
    void* socket_;  // SOCKET type
#else
    int socket_;
#endif
};

} // namespace CS16Capture
