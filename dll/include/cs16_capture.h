#ifndef CS16_CAPTURE_H
#define CS16_CAPTURE_H

#include <string>
#include <vector>
#include <memory>

struct PlayerData {
    std::string name;
    int kills = 0;
    int deaths = 0;
    int assists = 0;
    int money = 0;
};

struct BombData {
    bool planted = false;
    float timeRemaining = 0.0f;
};

struct GameData {
    std::vector<PlayerData> players;
    BombData bomb;
    std::vector<std::string> events;
};

class CS16Capture {
public:
    static CS16Capture& getInstance();
    
    bool initialize();
    void shutdown();
    bool captureGameData(GameData& outData);
    
    CS16Capture(const CS16Capture&) = delete;
    CS16Capture& operator=(const CS16Capture&) = delete;

private:
    CS16Capture();
    ~CS16Capture();
    
    bool initialized_;
};

#endif // CS16_CAPTURE_H
