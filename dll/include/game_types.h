#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace CS16Capture {

/**
 * @brief Player data structure
 */
struct PlayerData {
    std::string name;
    int32_t kills;
    int32_t deaths;
    int32_t assists;
    int32_t money;
    int32_t team;  // 1 = Terrorist, 2 = Counter-Terrorist
    bool isAlive;

    PlayerData()
        : kills(0), deaths(0), assists(0), money(0), team(0), isAlive(false) {}
};

/**
 * @brief Bomb status data structure
 */
struct BombData {
    bool planted;
    float timeRemaining;  // in seconds
    bool defused;
    
    BombData()
        : planted(false), timeRemaining(0.0f), defused(false) {}
};

/**
 * @brief Game event types
 */
enum class GameEvent {
    ROUND_START,
    ROUND_END,
    BOMB_PLANTED,
    BOMB_DEFUSED,
    BOMB_EXPLODED,
    PLAYER_KILLED,
    UNKNOWN
};

/**
 * @brief Complete game state
 */
struct GameState {
    std::vector<PlayerData> players;
    BombData bomb;
    std::vector<GameEvent> events;
    int32_t roundNumber;
    float roundTime;
    
    GameState()
        : roundNumber(0), roundTime(0.0f) {}
};

/**
 * @brief Memory offsets for CS 1.6 (Steam version)
 * These offsets may need to be updated based on the game version
 */
struct MemoryOffsets {
    // Base addresses (relative to module base)
    uintptr_t playerListBase;
    uintptr_t bombBase;
    uintptr_t gameStateBase;
    
    // Player offsets
    size_t playerNameOffset;
    size_t playerKillsOffset;
    size_t playerDeathsOffset;
    size_t playerAssistsOffset;
    size_t playerMoneyOffset;
    size_t playerTeamOffset;
    size_t playerAliveOffset;
    
    // Bomb offsets
    size_t bombPlantedOffset;
    size_t bombTimerOffset;
    size_t bombDefusedOffset;
    
    MemoryOffsets()
        : playerListBase(0), bombBase(0), gameStateBase(0),
          playerNameOffset(0), playerKillsOffset(0), playerDeathsOffset(0),
          playerAssistsOffset(0), playerMoneyOffset(0), playerTeamOffset(0),
          playerAliveOffset(0), bombPlantedOffset(0), bombTimerOffset(0),
          bombDefusedOffset(0) {}
};

} // namespace CS16Capture
