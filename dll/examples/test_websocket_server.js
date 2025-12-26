/* eslint-disable @typescript-eslint/no-require-imports */
/* eslint-disable @typescript-eslint/no-unused-vars */
/**
 * Simple WebSocket test server for CS 1.6 Data Capture DLL
 * 
 * Usage:
 * 1. npm install ws
 * 2. node test_websocket_server.js
 */

const net = require('net');

const PORT = 8080;

console.log('===========================================');
console.log('CS 1.6 Data Capture - Test Server');
console.log('===========================================');
console.log(`Listening on port ${PORT}...`);
console.log('Waiting for DLL connection...\n');

const server = net.createServer((socket) => {
    console.log(`[${new Date().toISOString()}] Client connected from ${socket.remoteAddress}:${socket.remotePort}`);
    
    let buffer = '';

    socket.on('data', (data) => {
        buffer += data.toString();
        
        // Try to parse JSON messages
        const messages = buffer.split('\n');
        buffer = messages.pop() || ''; // Keep incomplete message in buffer

        messages.forEach((message) => {
            if (message.trim()) {
                try {
                    const gameData = JSON.parse(message);
                    console.log('\n--- Game State Received ---');
                    console.log(`Time: ${new Date().toLocaleTimeString()}`);
                    console.log(`Round: ${gameData.roundNumber} | Time: ${gameData.roundTime.toFixed(1)}s`);
                    
                    // Display player info
                    console.log(`\nPlayers (${gameData.players.length}):`);
                    gameData.players.forEach((player, index) => {
                        const alive = player.isAlive ? '✓' : '✗';
                        const team = player.team === 1 ? 'T' : 'CT';
                        console.log(`  ${index + 1}. [${team}] ${alive} ${player.name} - K:${player.kills} D:${player.deaths} A:${player.assists} $${player.money}`);
                    });
                    
                    // Display bomb info
                    if (gameData.bomb.planted) {
                        console.log(`\n💣 BOMB: Planted | Time: ${gameData.bomb.timeRemaining.toFixed(1)}s`);
                    } else {
                        console.log('\n💣 BOMB: Not planted');
                    }
                    
                    // Display events
                    if (gameData.events.length > 0) {
                        console.log('\n🎮 Events:');
                        gameData.events.forEach(event => {
                            console.log(`  - ${event}`);
                        });
                    }
                    
                    console.log('---------------------------');
                } catch (e) {
                    // Not JSON or incomplete, might be raw TCP data
                    console.log(`[${new Date().toISOString()}] Raw data: ${message.substring(0, 100)}...`);
                }
            }
        });
    });

    socket.on('error', (err) => {
        console.error(`[${new Date().toISOString()}] Socket error:`, err.message);
    });

    socket.on('close', () => {
        console.log(`[${new Date().toISOString()}] Client disconnected`);
    });
});

server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
        console.error(`Error: Port ${PORT} is already in use`);
        process.exit(1);
    } else {
        console.error('Server error:', err);
    }
});

server.listen(PORT, () => {
    console.log(`Server is ready and listening on port ${PORT}`);
    console.log('\nWaiting for game data...\n');
});

// Handle Ctrl+C gracefully
process.on('SIGINT', () => {
    console.log('\n\nShutting down server...');
    server.close(() => {
        console.log('Server closed');
        process.exit(0);
    });
});
