#include "../include/websocket_client.h"
#include "../include/logger.h"
#include <sstream>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace CS16Capture {

WebSocketClient::WebSocketClient()
    : connected_(false)
    , autoReconnect_(true)
    , shouldStop_(false)
    , port_(0)
#ifdef _WIN32
    , socket_(nullptr)
#else
    , socket_(-1)
#endif
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOG_ERROR("WSAStartup failed");
    }
#endif
}

WebSocketClient::~WebSocketClient() {
    disconnect();
    
#ifdef _WIN32
    WSACleanup();
#endif
}

bool WebSocketClient::connect(const std::string& host, int port) {
    if (connected_) {
        LOG_WARNING("Already connected to WebSocket server");
        return true;
    }

    host_ = host;
    port_ = port;

#ifdef _WIN32
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (::connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        LOG_ERROR("Failed to connect to " + host + ":" + std::to_string(port));
        closesocket(sock);
        return false;
    }

    socket_ = reinterpret_cast<void*>(sock);
#else
    // Linux implementation
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (::connect(sock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        LOG_ERROR("Failed to connect to " + host + ":" + std::to_string(port));
        close(sock);
        return false;
    }

    socket_ = sock;
#endif

    connected_ = true;
    shouldStop_ = false;

    // Start send thread
    sendThread_ = std::make_unique<std::thread>(&WebSocketClient::sendThreadFunc, this);

    LOG_INFO("Connected to WebSocket server at " + host + ":" + std::to_string(port));
    return true;
}

void WebSocketClient::disconnect() {
    if (!connected_) {
        return;
    }

    shouldStop_ = true;
    connected_ = false;

    // Wait for send thread to finish
    if (sendThread_ && sendThread_->joinable()) {
        sendThread_->join();
    }

#ifdef _WIN32
    if (socket_ != nullptr) {
        closesocket(reinterpret_cast<SOCKET>(socket_));
        socket_ = nullptr;
    }
#else
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
#endif

    LOG_INFO("Disconnected from WebSocket server");
}

bool WebSocketClient::isConnected() const {
    return connected_;
}

bool WebSocketClient::sendGameState(const GameState& state) {
    std::string json = gameStateToJson(state);
    return sendMessage(json);
}

bool WebSocketClient::sendMessage(const std::string& jsonMessage) {
    if (!connected_) {
        if (autoReconnect_) {
            tryReconnect();
            if (!connected_) {
                return false;
            }
        } else {
            return false;
        }
    }

    std::lock_guard<std::mutex> lock(queueMutex_);
    messageQueue_.push(jsonMessage);
    
    return true;
}

void WebSocketClient::setAutoReconnect(bool enable) {
    autoReconnect_ = enable;
}

size_t WebSocketClient::getPendingMessageCount() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(queueMutex_));
    return messageQueue_.size();
}

std::string WebSocketClient::gameStateToJson(const GameState& state) {
    std::ostringstream json;
    json << "{\n";
    
    // Players array
    json << "  \"players\": [\n";
    for (size_t i = 0; i < state.players.size(); ++i) {
        const auto& player = state.players[i];
        json << "    {\n";
        json << "      \"name\": \"" << player.name << "\",\n";
        json << "      \"kills\": " << player.kills << ",\n";
        json << "      \"deaths\": " << player.deaths << ",\n";
        json << "      \"assists\": " << player.assists << ",\n";
        json << "      \"money\": " << player.money << ",\n";
        json << "      \"team\": " << player.team << ",\n";
        json << "      \"isAlive\": " << (player.isAlive ? "true" : "false") << "\n";
        json << "    }";
        if (i < state.players.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    
    // Bomb data
    json << "  \"bomb\": {\n";
    json << "    \"planted\": " << (state.bomb.planted ? "true" : "false") << ",\n";
    json << "    \"timeRemaining\": " << state.bomb.timeRemaining << ",\n";
    json << "    \"defused\": " << (state.bomb.defused ? "true" : "false") << "\n";
    json << "  },\n";
    
    // Events array
    json << "  \"events\": [\n";
    for (size_t i = 0; i < state.events.size(); ++i) {
        json << "    \"" << gameEventToString(state.events[i]) << "\"";
        if (i < state.events.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    
    // Round info
    json << "  \"roundNumber\": " << state.roundNumber << ",\n";
    json << "  \"roundTime\": " << state.roundTime << "\n";
    
    json << "}";
    
    return json.str();
}

std::string WebSocketClient::gameEventToString(GameEvent event) {
    switch (event) {
        case GameEvent::ROUND_START:    return "Round Start";
        case GameEvent::ROUND_END:      return "Round End";
        case GameEvent::BOMB_PLANTED:   return "Bomb Planted";
        case GameEvent::BOMB_DEFUSED:   return "Bomb Defused";
        case GameEvent::BOMB_EXPLODED:  return "Bomb Exploded";
        case GameEvent::PLAYER_KILLED:  return "Player Killed";
        default:                        return "Unknown";
    }
}

void WebSocketClient::sendThreadFunc() {
    LOG_INFO("WebSocket send thread started");

    while (!shouldStop_) {
        std::string message;
        
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            if (!messageQueue_.empty()) {
                message = messageQueue_.front();
                messageQueue_.pop();
            }
        }

        if (!message.empty() && connected_) {
#ifdef _WIN32
            SOCKET sock = reinterpret_cast<SOCKET>(socket_);
            int result = send(sock, message.c_str(), static_cast<int>(message.length()), 0);
            if (result == SOCKET_ERROR) {
                LOG_ERROR("Failed to send message: " + std::to_string(WSAGetLastError()));
                connected_ = false;
            } else {
                LOG_DEBUG("Sent message: " + message.substr(0, 100) + "...");
            }
#else
            ssize_t result = send(socket_, message.c_str(), message.length(), 0);
            if (result < 0) {
                LOG_ERROR("Failed to send message");
                connected_ = false;
            } else {
                LOG_DEBUG("Sent message: " + message.substr(0, 100) + "...");
            }
#endif
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_INFO("WebSocket send thread stopped");
}

bool WebSocketClient::tryReconnect() {
    LOG_INFO("Attempting to reconnect to WebSocket server...");
    
    disconnect();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return connect(host_, port_);
}

} // namespace CS16Capture
