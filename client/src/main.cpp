#include <emscripten.h>
#include <emscripten/val.h>
#include <string>
#include <iostream>
#include <sstream> // For parsing the raw string
#include "raylib.h"

using namespace emscripten;

// Variables to store the received rectangle position
float rectX = 0.0f;
float rectY = 0.0f;

// Function exposed to JS to update rect position
extern "C" {
    void update_position(float x, float y) {
        rectX = x;
        rectY = y;
        std::cout << "Updated rectX: " << rectX << ", rectY: " << rectY << std::endl;
    }
}
bool key_up = false;
bool key_down = false;
bool key_left = false;
bool key_right = false;
bool last_key_up = false;
bool last_key_down = false;
bool last_key_left = false;
bool last_key_right = false;
// Log messages from JS
EM_JS(void, js_send_control, (bool up, bool down, bool left, bool right), {
    console.log("la", up, down, left, right);
    if (window.ws && window.ws.readyState === WebSocket.OPEN) {
        const message = JSON.stringify({ up: up, down: down, left: left, right: right });
        window.ws.send(message);
        console.log("Sent message to server: ", message);
    } else {
        console.log("WebSocket not ready.");
    }
});


// WebSocket open function
EM_JS(void, js_open_ws, (), {
    // Create a global WebSocket connection

    window.ws = new WebSocket("ws://192.168.71.78:8765");

    ws.onopen = function () {
        console.log("Connected to WebSocket server");
    };

    ws.onmessage = function (event) {
        console.log("Message received from server: ", event.data);

        const jsonData = JSON.parse(event.data);
        if (jsonData.x !== undefined && jsonData.y !== undefined) {
            Module._update_position(jsonData.x, jsonData.y);
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

    // Open WebSocket connection
    js_open_ws();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            key_up = true;
        }
        else
        {
            key_up = false;
        }
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            key_down = true;
        }
        else
        {
            key_down = false;
        }
        if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            key_left = true;
        }
        else
        {
            key_left = false;
        }
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            key_right = true;
        }
        else
        {
            key_right = false;
        }
        if (key_up != last_key_up || key_down != last_key_down || key_left != last_key_left || key_right != last_key_right){
            js_send_control(key_up, key_down, key_left, key_right);



            last_key_up = key_up;
            last_key_down = key_down;
            last_key_left = key_left;
            last_key_right = key_right;


        }

        // Draw the rectangle based on received position
        DrawRectangle(static_cast<int>(rectX), static_cast<int>(rectY), 50, 50, BLUE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
