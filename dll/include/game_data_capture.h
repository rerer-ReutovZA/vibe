#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include "game_types.h"
#include "memory_reader.h"
#include "websocket_client.h"

namespace CS16Capture {

/**
 * @brief Main class for capturing and sending CS 1.6 game data
 */
class GameDataCapture {
public:
    GameDataCapture();
    ~GameDataCapture();

    /**
     * @brief Initialize the capture system
     * @param wsHost WebSocket server host
     * @param wsPort WebSocket server port
     * @return true if initialization was successful
     */
    bool initialize(const std::string& wsHost, int wsPort);

    /**
     * @brief Start capturing and sending game data
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop capturing game data
     */
    void stop();

    /**
     * @brief Check if the capture is running
     * @return true if running
     */
    bool isRunning() const;

    /**
     * @brief Set the update interval in milliseconds
     * @param intervalMs Update interval in milliseconds
     */
    void setUpdateInterval(int intervalMs);

    /**
     * @brief Get the current game state
     * @return Current game state
     */
    GameState getCurrentGameState();

private:
    /**
     * @brief Capture thread function
     */
    void captureThreadFunc();

    /**
     * @brief Capture player data
     */
    bool capturePlayerData(GameState& state);

    /**
     * @brief Capture bomb data
     */
    bool captureBombData(GameState& state);

    /**
     * @brief Capture game events
     */
    bool captureGameEvents(GameState& state);

    /**
     * @brief Initialize memory offsets
     */
    bool initializeOffsets();

    /**
     * @brief Scan for memory patterns to find offsets
     */
    bool scanForOffsets();

    std::unique_ptr<MemoryReader> memoryReader_;
    std::unique_ptr<WebSocketClient> wsClient_;
    std::unique_ptr<std::thread> captureThread_;

    std::atomic<bool> isRunning_;
    std::atomic<bool> shouldStop_;
    
    int updateIntervalMs_;
    MemoryOffsets offsets_;
    
    // Cached data for change detection
    GameState lastState_;
    std::chrono::steady_clock::time_point lastUpdate_;
};

} // namespace CS16Capture
