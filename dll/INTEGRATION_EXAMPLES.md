# Примеры интеграции CS 1.6 Data Capture DLL

Этот документ содержит примеры кода для интеграции DLL в различные приложения.

## Содержание

1. [C++ приложение](#c-приложение)
2. [Node.js WebSocket сервер](#nodejs-websocket-сервер)
3. [Python обработчик данных](#python-обработчик-данных)
4. [C# Unity интеграция](#c-unity-интеграция)
5. [Web Dashboard (React)](#web-dashboard-react)

---

## C++ приложение

### Простой загрузчик DLL

```cpp
#include <windows.h>
#include <iostream>
#include <string>

class CS16DataCaptureManager {
private:
    HMODULE hDll;
    
    // Function pointers
    typedef bool (*StartCaptureFunc)();
    typedef void (*StopCaptureFunc)();
    typedef bool (*IsCapturingFunc)();
    typedef void (*SetUpdateIntervalFunc)(int);
    
    StartCaptureFunc StartCapture;
    StopCaptureFunc StopCapture;
    IsCapturingFunc IsCapturing;
    SetUpdateIntervalFunc SetUpdateInterval;

public:
    CS16DataCaptureManager() : hDll(nullptr) {}
    
    ~CS16DataCaptureManager() {
        unload();
    }
    
    bool load(const std::string& dllPath = "cs16_datacapture.dll") {
        hDll = LoadLibraryA(dllPath.c_str());
        if (!hDll) {
            std::cerr << "Failed to load DLL: " << GetLastError() << std::endl;
            return false;
        }
        
        // Get function pointers
        StartCapture = (StartCaptureFunc)GetProcAddress(hDll, "StartCapture");
        StopCapture = (StopCaptureFunc)GetProcAddress(hDll, "StopCapture");
        IsCapturing = (IsCapturingFunc)GetProcAddress(hDll, "IsCapturing");
        SetUpdateInterval = (SetUpdateIntervalFunc)GetProcAddress(hDll, "SetUpdateInterval");
        
        if (!StartCapture || !StopCapture || !IsCapturing || !SetUpdateInterval) {
            std::cerr << "Failed to get DLL functions" << std::endl;
            unload();
            return false;
        }
        
        return true;
    }
    
    void unload() {
        if (hDll) {
            FreeLibrary(hDll);
            hDll = nullptr;
        }
    }
    
    bool start() {
        return StartCapture ? StartCapture() : false;
    }
    
    void stop() {
        if (StopCapture) StopCapture();
    }
    
    bool isActive() {
        return IsCapturing ? IsCapturing() : false;
    }
    
    void setInterval(int ms) {
        if (SetUpdateInterval) SetUpdateInterval(ms);
    }
};

// Использование
int main() {
    CS16DataCaptureManager manager;
    
    if (manager.load()) {
        std::cout << "DLL loaded successfully" << std::endl;
        
        if (manager.start()) {
            std::cout << "Capture started" << std::endl;
            manager.setInterval(100); // 100ms updates
            
            // Do work...
            Sleep(30000); // Run for 30 seconds
            
            manager.stop();
        }
    }
    
    return 0;
}
```

---

## Node.js WebSocket сервер

### Продвинутый WebSocket сервер с базой данных

```javascript
const WebSocket = require('ws');
const express = require('express');
const sqlite3 = require('sqlite3').verbose();

class CS16DataServer {
    constructor(port = 8080) {
        this.port = port;
        this.wss = null;
        this.db = null;
        this.clients = new Set();
        this.gameState = null;
        
        this.initDatabase();
        this.initWebSocket();
        this.initHTTPServer();
    }
    
    initDatabase() {
        this.db = new sqlite3.Database('cs16_data.db');
        
        this.db.run(`
            CREATE TABLE IF NOT EXISTS game_sessions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
                data TEXT
            )
        `);
        
        this.db.run(`
            CREATE TABLE IF NOT EXISTS player_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                session_id INTEGER,
                player_name TEXT,
                kills INTEGER,
                deaths INTEGER,
                assists INTEGER,
                money INTEGER,
                FOREIGN KEY(session_id) REFERENCES game_sessions(id)
            )
        `);
    }
    
    initWebSocket() {
        this.wss = new WebSocket.Server({ port: this.port });
        
        this.wss.on('connection', (ws) => {
            console.log('DLL client connected');
            
            ws.on('message', (data) => {
                try {
                    const gameState = JSON.parse(data);
                    this.handleGameState(gameState);
                    
                    // Broadcast to all web clients
                    this.broadcast(gameState);
                } catch (e) {
                    console.error('Parse error:', e);
                }
            });
            
            ws.on('close', () => {
                console.log('DLL client disconnected');
            });
        });
        
        console.log(`WebSocket server listening on port ${this.port}`);
    }
    
    initHTTPServer() {
        const app = express();
        const httpPort = 3001;
        
        app.use(express.static('public'));
        
        // API endpoint for current game state
        app.get('/api/game-state', (req, res) => {
            res.json(this.gameState || {});
        });
        
        // API endpoint for historical data
        app.get('/api/sessions', (req, res) => {
            this.db.all('SELECT * FROM game_sessions ORDER BY timestamp DESC LIMIT 10', 
                (err, rows) => {
                    if (err) {
                        res.status(500).json({ error: err.message });
                    } else {
                        res.json(rows);
                    }
                }
            );
        });
        
        app.listen(httpPort, () => {
            console.log(`HTTP server listening on port ${httpPort}`);
        });
    }
    
    handleGameState(gameState) {
        this.gameState = gameState;
        
        // Save to database
        this.db.run(
            'INSERT INTO game_sessions (data) VALUES (?)',
            [JSON.stringify(gameState)],
            function(err) {
                if (err) {
                    console.error('Database error:', err);
                }
            }
        );
        
        // Check for events
        if (gameState.events && gameState.events.length > 0) {
            gameState.events.forEach(event => {
                console.log(`[EVENT] ${event}`);
            });
        }
    }
    
    broadcast(data) {
        const message = JSON.stringify(data);
        this.clients.forEach(client => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(message);
            }
        });
    }
}

// Start server
const server = new CS16DataServer(8080);
```

---

## Python обработчик данных

### Анализ и визуализация данных

```python
import asyncio
import websockets
import json
import matplotlib.pyplot as plt
from datetime import datetime
from collections import defaultdict

class CS16DataAnalyzer:
    def __init__(self):
        self.player_stats = defaultdict(lambda: {
            'kills': [],
            'deaths': [],
            'money': []
        })
        
    async def handle_connection(self, websocket, path):
        print(f"Client connected: {path}")
        
        try:
            async for message in websocket:
                data = json.loads(message)
                self.process_game_state(data)
                
        except websockets.exceptions.ConnectionClosed:
            print("Client disconnected")
    
    def process_game_state(self, state):
        timestamp = datetime.now()
        
        # Process players
        for player in state.get('players', []):
            name = player['name']
            self.player_stats[name]['kills'].append(
                (timestamp, player['kills'])
            )
            self.player_stats[name]['deaths'].append(
                (timestamp, player['deaths'])
            )
            self.player_stats[name]['money'].append(
                (timestamp, player['money'])
            )
        
        # Process bomb
        bomb = state.get('bomb', {})
        if bomb.get('planted'):
            print(f"💣 BOMB: {bomb.get('timeRemaining', 0):.1f}s remaining")
        
        # Process events
        for event in state.get('events', []):
            print(f"[{timestamp.strftime('%H:%M:%S')}] EVENT: {event}")
    
    def plot_player_stats(self, player_name):
        if player_name not in self.player_stats:
            print(f"No data for player: {player_name}")
            return
        
        stats = self.player_stats[player_name]
        
        fig, axes = plt.subplots(3, 1, figsize=(12, 8))
        
        # Plot kills
        times, kills = zip(*stats['kills'])
        axes[0].plot(times, kills, 'r-', label='Kills')
        axes[0].set_ylabel('Kills')
        axes[0].legend()
        axes[0].grid(True)
        
        # Plot deaths
        times, deaths = zip(*stats['deaths'])
        axes[1].plot(times, deaths, 'b-', label='Deaths')
        axes[1].set_ylabel('Deaths')
        axes[1].legend()
        axes[1].grid(True)
        
        # Plot money
        times, money = zip(*stats['money'])
        axes[2].plot(times, money, 'g-', label='Money')
        axes[2].set_ylabel('Money ($)')
        axes[2].set_xlabel('Time')
        axes[2].legend()
        axes[2].grid(True)
        
        plt.tight_layout()
        plt.savefig(f'{player_name}_stats.png')
        plt.show()
    
    async def start_server(self, host='localhost', port=8080):
        server = await websockets.serve(self.handle_connection, host, port)
        print(f"WebSocket server started on {host}:{port}")
        await server.wait_closed()

# Использование
if __name__ == '__main__':
    analyzer = CS16DataAnalyzer()
    asyncio.run(analyzer.start_server())
```

---

## C# Unity интеграция

### Визуализация игровых данных в Unity

```csharp
using UnityEngine;
using System;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using Newtonsoft.Json;

public class CS16DataReceiver : MonoBehaviour
{
    [Serializable]
    public class PlayerData
    {
        public string name;
        public int kills;
        public int deaths;
        public int assists;
        public int money;
        public int team;
        public bool isAlive;
    }
    
    [Serializable]
    public class BombData
    {
        public bool planted;
        public float timeRemaining;
        public bool defused;
    }
    
    [Serializable]
    public class GameState
    {
        public PlayerData[] players;
        public BombData bomb;
        public string[] events;
        public int roundNumber;
        public float roundTime;
    }
    
    private TcpClient client;
    private Thread receiveThread;
    private GameState currentState;
    private bool isRunning = false;
    
    public string serverHost = "localhost";
    public int serverPort = 8080;
    
    void Start()
    {
        ConnectToServer();
    }
    
    void ConnectToServer()
    {
        try
        {
            client = new TcpClient(serverHost, serverPort);
            isRunning = true;
            
            receiveThread = new Thread(ReceiveData);
            receiveThread.IsBackground = true;
            receiveThread.Start();
            
            Debug.Log("Connected to CS 1.6 data server");
        }
        catch (Exception e)
        {
            Debug.LogError($"Connection failed: {e.Message}");
        }
    }
    
    void ReceiveData()
    {
        NetworkStream stream = client.GetStream();
        byte[] buffer = new byte[4096];
        
        while (isRunning)
        {
            try
            {
                int bytesRead = stream.Read(buffer, 0, buffer.Length);
                if (bytesRead > 0)
                {
                    string json = Encoding.UTF8.GetString(buffer, 0, bytesRead);
                    currentState = JsonConvert.DeserializeObject<GameState>(json);
                }
            }
            catch (Exception e)
            {
                Debug.LogError($"Receive error: {e.Message}");
                break;
            }
        }
    }
    
    void OnApplicationQuit()
    {
        isRunning = false;
        if (receiveThread != null && receiveThread.IsAlive)
        {
            receiveThread.Join(1000);
        }
        if (client != null)
        {
            client.Close();
        }
    }
    
    void Update()
    {
        if (currentState != null)
        {
            // Визуализация данных
            UpdatePlayerUI();
            UpdateBombUI();
        }
    }
    
    void UpdatePlayerUI()
    {
        // Обновите UI элементы с данными игроков
        foreach (var player in currentState.players)
        {
            Debug.Log($"{player.name}: K:{player.kills} D:{player.deaths} ${player.money}");
        }
    }
    
    void UpdateBombUI()
    {
        if (currentState.bomb.planted)
        {
            Debug.Log($"BOMB: {currentState.bomb.timeRemaining:F1}s");
        }
    }
}
```

---

## Web Dashboard (React)

### React компонент для отображения данных

```jsx
import React, { useState, useEffect } from 'react';
import './CS16Dashboard.css';

const CS16Dashboard = () => {
  const [gameState, setGameState] = useState(null);
  const [connected, setConnected] = useState(false);
  const [ws, setWs] = useState(null);

  useEffect(() => {
    // Connect to WebSocket server
    const websocket = new WebSocket('ws://localhost:8080');

    websocket.onopen = () => {
      setConnected(true);
      console.log('Connected to CS 1.6 data server');
    };

    websocket.onmessage = (event) => {
      try {
        const data = JSON.parse(event.data);
        setGameState(data);
      } catch (e) {
        console.error('Parse error:', e);
      }
    };

    websocket.onclose = () => {
      setConnected(false);
      console.log('Disconnected from server');
    };

    websocket.onerror = (error) => {
      console.error('WebSocket error:', error);
    };

    setWs(websocket);

    return () => {
      websocket.close();
    };
  }, []);

  if (!connected) {
    return (
      <div className="dashboard">
        <h1>CS 1.6 Data Dashboard</h1>
        <p className="status disconnected">Disconnected from server...</p>
      </div>
    );
  }

  if (!gameState) {
    return (
      <div className="dashboard">
        <h1>CS 1.6 Data Dashboard</h1>
        <p className="status waiting">Waiting for game data...</p>
      </div>
    );
  }

  return (
    <div className="dashboard">
      <h1>CS 1.6 Data Dashboard</h1>
      <p className="status connected">✓ Connected</p>

      <div className="round-info">
        <h2>Round {gameState.roundNumber}</h2>
        <p>Time: {gameState.roundTime.toFixed(1)}s</p>
      </div>

      <div className="bomb-status">
        <h2>Bomb Status</h2>
        {gameState.bomb.planted ? (
          <div className="bomb-planted">
            <p className="bomb-emoji">💣</p>
            <p className="bomb-timer">{gameState.bomb.timeRemaining.toFixed(1)}s</p>
          </div>
        ) : (
          <p>Not Planted</p>
        )}
      </div>

      <div className="players">
        <h2>Players</h2>
        <table>
          <thead>
            <tr>
              <th>Name</th>
              <th>Team</th>
              <th>K/D/A</th>
              <th>Money</th>
              <th>Status</th>
            </tr>
          </thead>
          <tbody>
            {gameState.players.map((player, index) => (
              <tr key={index} className={player.isAlive ? 'alive' : 'dead'}>
                <td>{player.name}</td>
                <td className={player.team === 1 ? 'team-t' : 'team-ct'}>
                  {player.team === 1 ? 'T' : 'CT'}
                </td>
                <td>{player.kills}/{player.deaths}/{player.assists}</td>
                <td>${player.money}</td>
                <td>{player.isAlive ? '✓' : '✗'}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>

      {gameState.events.length > 0 && (
        <div className="events">
          <h2>Recent Events</h2>
          <ul>
            {gameState.events.map((event, index) => (
              <li key={index}>{event}</li>
            ))}
          </ul>
        </div>
      )}
    </div>
  );
};

export default CS16Dashboard;
```

CSS для dashboard:

```css
.dashboard {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
  font-family: Arial, sans-serif;
}

.status {
  padding: 10px;
  border-radius: 5px;
  margin: 10px 0;
}

.status.connected {
  background-color: #4caf50;
  color: white;
}

.status.disconnected {
  background-color: #f44336;
  color: white;
}

.status.waiting {
  background-color: #ff9800;
  color: white;
}

.round-info {
  background-color: #2196F3;
  color: white;
  padding: 15px;
  border-radius: 5px;
  margin: 15px 0;
}

.bomb-status {
  background-color: #333;
  color: white;
  padding: 15px;
  border-radius: 5px;
  margin: 15px 0;
}

.bomb-planted {
  text-align: center;
}

.bomb-emoji {
  font-size: 48px;
  margin: 0;
}

.bomb-timer {
  font-size: 36px;
  color: #ff5252;
  font-weight: bold;
}

.players table {
  width: 100%;
  border-collapse: collapse;
  margin: 15px 0;
}

.players th,
.players td {
  padding: 10px;
  text-align: left;
  border-bottom: 1px solid #ddd;
}

.players th {
  background-color: #4CAF50;
  color: white;
}

.players tr.dead {
  opacity: 0.5;
}

.team-t {
  color: #ff5252;
  font-weight: bold;
}

.team-ct {
  color: #2196F3;
  font-weight: bold;
}

.events {
  background-color: #f5f5f5;
  padding: 15px;
  border-radius: 5px;
  margin: 15px 0;
}

.events ul {
  list-style-type: none;
  padding: 0;
}

.events li {
  padding: 5px 0;
  border-bottom: 1px solid #ddd;
}
```

---

## Заключение

Эти примеры показывают различные способы интеграции DLL с разными технологиями и платформами. Вы можете адаптировать их под свои нужды и расширять функционал.

Для получения дополнительной информации обратитесь к основной документации в `README.md`.
