#include "../include/data_capture.h"
#include "../include/logger.h"
#include <thread>

namespace CS16Capture {

DataCapture::DataCapture()
    : isRunning_(false)
    , shouldStop_(false)
    , updateIntervalMs_(100)
{
}

DataCapture::~DataCapture() {
    stop();
}

DataCapture* DataCapture::getInstance() {
    static DataCapture instance;
    return &instance;
}

bool DataCapture::initialize(const std::string& wsHost, int wsPort) {
    LOG_INFO("Initializing DataCapture...");

    // Initialize memory reader
    memoryReader_ = std::make_unique<MemoryReader>();
    if (!memoryReader_->initialize()) {
        LOG_ERROR("Failed to initialize MemoryReader");
        return false;
    }

    // Initialize WebSocket client
    wsClient_ = std::make_unique<WebSocketClient>();
    if (!wsClient_->connect(wsHost, wsPort)) {
        LOG_ERROR("Failed to connect to WebSocket server");
        return false;
    }

    // Initialize memory offsets
    if (!initializeOffsets()) {
        LOG_WARNING("Failed to initialize memory offsets - will attempt pattern scanning");
        if (!scanForOffsets()) {
            LOG_ERROR("Failed to find memory offsets via pattern scanning");
            return false;
        }
    }

    LOG_INFO("DataCapture initialized successfully");
    return true;
}

bool DataCapture::start() {
    if (isRunning_) {
        LOG_WARNING("DataCapture is already running");
        return true;
    }

    shouldStop_ = false;
    isRunning_ = true;

    captureThread_ = std::make_unique<std::thread>(&DataCapture::captureThreadFunc, this);

    LOG_INFO("DataCapture started");
    return true;
}

void DataCapture::stop() {
    if (!isRunning_) {
        return;
    }

    shouldStop_ = true;
    isRunning_ = false;

    if (captureThread_ && captureThread_->joinable()) {
        captureThread_->join();
    }

    if (wsClient_) {
        wsClient_->disconnect();
    }

    LOG_INFO("DataCapture stopped");
}

bool DataCapture::isRunning() const {
    return isRunning_;
}

void DataCapture::setUpdateInterval(int intervalMs) {
    updateIntervalMs_ = intervalMs;
    LOG_INFO("Update interval set to " + std::to_string(intervalMs) + "ms");
}

GameState DataCapture::getCurrentGameState() {
    return lastState_;
}

void DataCapture::captureThreadFunc() {
    LOG_INFO("Capture thread started");

    while (!shouldStop_) {
        auto startTime = std::chrono::steady_clock::now();

        GameState currentState;

        // Capture all game data
        bool success = true;
        success &= capturePlayerData(currentState);
        success &= captureBombData(currentState);
        success &= captureGameEvents(currentState);

        if (success) {
            // Send data via WebSocket
            if (wsClient_ && wsClient_->isConnected()) {
                if (wsClient_->sendGameState(currentState)) {
                    LOG_DEBUG("Game state sent successfully");
                } else {
                    LOG_WARNING("Failed to send game state");
                }
            }

            lastState_ = currentState;
        } else {
            LOG_DEBUG("Failed to capture complete game state");
        }

        // Sleep for remaining time to maintain update interval
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        auto remaining = std::chrono::milliseconds(updateIntervalMs_) - elapsed;

        if (remaining.count() > 0) {
            std::this_thread::sleep_for(remaining);
        }
    }

    LOG_INFO("Capture thread stopped");
}

bool DataCapture::capturePlayerData(GameState& state) {
    if (!memoryReader_) {
        return false;
    }

    // This is a placeholder implementation
    // In a real implementation, you would read from actual game memory
    // using the memory offsets specific to CS 1.6

    // Example: Read player list base address
    uintptr_t playerListBase = offsets_.playerListBase;
    if (playerListBase == 0) {
        LOG_DEBUG("Player list base address not set");
        return false;
    }

    // For demonstration, create mock player data
    // In real implementation, iterate through player list and read each player's data
    const int maxPlayers = 32;
    for (int i = 0; i < maxPlayers; ++i) {
        uintptr_t playerAddr = playerListBase + (i * 0x1000); // Example offset

        if (!memoryReader_->isValidAddress(playerAddr)) {
            continue;
        }

        PlayerData player;
        
        // Read player name
        player.name = memoryReader_->readString(playerAddr + offsets_.playerNameOffset);
        if (player.name.empty()) {
            continue;
        }

        // Read player stats
        memoryReader_->readMemory(playerAddr + offsets_.playerKillsOffset, player.kills);
        memoryReader_->readMemory(playerAddr + offsets_.playerDeathsOffset, player.deaths);
        memoryReader_->readMemory(playerAddr + offsets_.playerAssistsOffset, player.assists);
        memoryReader_->readMemory(playerAddr + offsets_.playerMoneyOffset, player.money);
        memoryReader_->readMemory(playerAddr + offsets_.playerTeamOffset, player.team);
        memoryReader_->readMemory(playerAddr + offsets_.playerAliveOffset, player.isAlive);

        state.players.push_back(player);
    }

    LOG_DEBUG("Captured " + std::to_string(state.players.size()) + " players");
    return true;
}

bool DataCapture::captureBombData(GameState& state) {
    if (!memoryReader_) {
        return false;
    }

    uintptr_t bombBase = offsets_.bombBase;
    if (bombBase == 0) {
        LOG_DEBUG("Bomb base address not set");
        return false;
    }

    // Read bomb status
    memoryReader_->readMemory(bombBase + offsets_.bombPlantedOffset, state.bomb.planted);
    memoryReader_->readMemory(bombBase + offsets_.bombTimerOffset, state.bomb.timeRemaining);
    memoryReader_->readMemory(bombBase + offsets_.bombDefusedOffset, state.bomb.defused);

    LOG_DEBUG("Bomb status - Planted: " + std::to_string(state.bomb.planted) + 
              ", Time: " + std::to_string(state.bomb.timeRemaining));
    
    return true;
}

bool DataCapture::captureGameEvents(GameState& state) {
    // Check for state changes to detect events
    
    // Bomb planted event
    if (state.bomb.planted && !lastState_.bomb.planted) {
        state.events.push_back(GameEvent::BOMB_PLANTED);
        LOG_INFO("Event: Bomb Planted");
    }

    // Bomb defused event
    if (state.bomb.defused && !lastState_.bomb.defused) {
        state.events.push_back(GameEvent::BOMB_DEFUSED);
        LOG_INFO("Event: Bomb Defused");
    }

    // Round start detection (simplified)
    if (state.roundNumber != lastState_.roundNumber) {
        state.events.push_back(GameEvent::ROUND_START);
        LOG_INFO("Event: Round Start");
    }

    return true;
}

bool DataCapture::initializeOffsets() {
    LOG_INFO("Initializing memory offsets...");

    // Get base address of the game module
    uintptr_t baseAddr = memoryReader_->getModuleBase("hl.exe");
    if (baseAddr == 0) {
        // Try alternative module name
        baseAddr = memoryReader_->getModuleBase("hw.dll");
        if (baseAddr == 0) {
            LOG_ERROR("Failed to get game module base address");
            return false;
        }
    }

    LOG_INFO("Game module base address: 0x" + 
             std::to_string(baseAddr));

    // These offsets are placeholders and need to be determined for the specific
    // CS 1.6 Steam version through reverse engineering
    // You would typically use tools like Cheat Engine or IDA Pro to find these

    offsets_.playerListBase = baseAddr + 0x00000000;  // TODO: Find actual offset
    offsets_.bombBase = baseAddr + 0x00000000;        // TODO: Find actual offset
    offsets_.gameStateBase = baseAddr + 0x00000000;   // TODO: Find actual offset

    offsets_.playerNameOffset = 0x00;
    offsets_.playerKillsOffset = 0x00;
    offsets_.playerDeathsOffset = 0x00;
    offsets_.playerAssistsOffset = 0x00;
    offsets_.playerMoneyOffset = 0x00;
    offsets_.playerTeamOffset = 0x00;
    offsets_.playerAliveOffset = 0x00;

    offsets_.bombPlantedOffset = 0x00;
    offsets_.bombTimerOffset = 0x00;
    offsets_.bombDefusedOffset = 0x00;

    LOG_WARNING("Using placeholder offsets - these need to be determined for your CS 1.6 version");
    return false; // Return false to trigger pattern scanning
}

bool DataCapture::scanForOffsets() {
    LOG_INFO("Scanning for memory patterns...");

    uintptr_t baseAddr = memoryReader_->getModuleBase("hl.exe");
    if (baseAddr == 0) {
        baseAddr = memoryReader_->getModuleBase("hw.dll");
        if (baseAddr == 0) {
            return false;
        }
    }

    // Example pattern for player list (these are placeholders)
    // Real patterns need to be found through reverse engineering
    std::vector<uint8_t> playerListPattern = {0x55, 0x8B, 0xEC, 0x83, 0xEC};
    std::string playerListMask = "xxxxx";

    uintptr_t playerListAddr = memoryReader_->findPattern(
        playerListPattern, 
        playerListMask, 
        baseAddr, 
        0x100000  // Search in first 1MB
    );

    if (playerListAddr != 0) {
        offsets_.playerListBase = playerListAddr;
        LOG_INFO("Found player list pattern at: 0x" + std::to_string(playerListAddr));
    }

    // Add more pattern searches for other offsets here

    return playerListAddr != 0;
}

} // namespace CS16Capture
