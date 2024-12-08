#include <emscripten.h>
#include <emscripten/val.h>
#include <string>
#include <iostream>
#include <unordered_map>
#include "include/player.hpp"
#include "raylib.h"

using namespace emscripten;

bool key_up = false;
bool key_down = false;
bool key_left = false;
bool key_right = false;
bool last_key_up = false;
bool last_key_down = false;
bool last_key_left = false;
bool last_key_right = false;

std::unordered_map<int, Player> players;

int myuuid = 0;


extern "C" {
    void update_position(int playerId, float x, float y) {
        if (players.find(playerId) == players.end()) {
            // Create a new player if not found
            players[playerId] = Player(x, y);
            std::cout << "Connected player with ID: " << playerId << std::endl;
        } else {
            // Update the existing player's position
            players[playerId].setPosition(x, y);
        }
    }
}

extern "C" {
    void set_my_uuid(int playerId) {
        myuuid = playerId;
    }
}

extern "C" {
    void delete_player(int playerId) {
        if (players.find(playerId) != players.end()) {
            players.erase(playerId);
            std::cout << "Deleted player with ID: " << playerId << std::endl;
        }
    }
}

// Log messages from JS
EM_JS(void, js_send_control, (bool up, bool down, bool left, bool right), {
    if (window.ws && window.ws.readyState === WebSocket.OPEN) {
        const message = JSON.stringify({ up: up, down: down, left: left, right: right });
        window.ws.send(message);
    } else {
        console.log("WebSocket not ready.");
    }
});

// WebSocket open function
EM_JS(void, js_open_ws, (), {
    window.ws = new WebSocket("ws://192.168.71.78:8765");

    ws.onopen = function () {
        console.log("Connected to WebSocket server");
    };

    ws.onmessage = function (event) {
        const jsonData = JSON.parse(event.data);

        if (jsonData.type === "positions") {
            const players = jsonData.players;

            for (const id in players) {
                if (players.hasOwnProperty(id)) {
                    const idnum = parseInt(id, 10);
                    Module.ccall(
                        'update_position',
                        null,
                        ['number', 'number', 'number'],
                        [idnum, players[id].x, players[id].y]
                    );
                }
            }
        } else if (jsonData.type === "disconnect") {
            const playerId = jsonData.player_id;
            Module.ccall(
                'delete_player',
                null,
                ['number'],
                [playerId]
            );
            console.log(`Player ${playerId} disconnected.`);
        } else if (jsonData.type === "welcome") {
            const MyUuid = jsonData.player_id;
            Module.ccall(
                'set_my_uuid',
                null,
                ['number'],
                [MyUuid]
            );
            console.log(`Player ${playerId} disconnected.`);
            console.log(`My ID ${playerId}.`);
        }
    };

    ws.onclose = function () {
        console.log("WebSocket closed");
    };

    ws.onerror = function (error) {
        console.log("WebSocket error: ", error);
    };
});

// Main entry point for Raylib
int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Raylib WebSocket Example");
    SetTargetFPS(60);
    Camera2D camera = { 0 };
    camera.target = {0,0};
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    // Open WebSocket connection
    js_open_ws();

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(RAYWHITE);

                // Handle input
                key_up = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W);
                key_down = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S);
                key_left = IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A);
                key_right = IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D);

                if (key_up != last_key_up || key_down != last_key_down || key_left != last_key_left || key_right != last_key_right) {
                    js_send_control(key_up, key_down, key_left, key_right);
                    last_key_up = key_up;
                    last_key_down = key_down;
                    last_key_left = key_left;
                    last_key_right = key_right;
                }
                camera.target = players[myuuid].getCenter();
                BeginMode2D(camera);
                    for (const auto& [id, player] : players) {
                        player.draw();
                    }
                BeginMode2D(camera);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
