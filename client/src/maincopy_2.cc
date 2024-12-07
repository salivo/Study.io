

/*******************************************************************************************
*
*   raylib [shapes] example - top down lights
*
*   Example originally created with raylib 4.2, last time updated with raylib 4.2
*
*   Example contributed by Vlad Adrian (@demizdor) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Jeffery Myers (@JeffM2501)
*
********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <cstdint>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>


// Custom Blend Modes
#define RLGL_SRC_ALPHA 0x0302
#define RLGL_MIN 0x8007
#define RLGL_MAX 0x8008

#define MAX_BOXES     20
#define MAX_SHADOWS   MAX_BOXES*3         // MAX_BOXES *3. Each box can cast up to two shadow volumes for the edges it is away from, and one for the box itself
#define MAX_LIGHTS    16

// Shadow geometry type
typedef struct ShadowGeometry {
    Vector2 vertices[4];
} ShadowGeometry;

// Light info type
typedef struct LightInfo {
    bool active;                // Is this light slot active?
    bool dirty;                 // Does this light need to be updated?
    bool valid;                 // Is this light in a valid position?

    Vector2 position;           // Light position
    RenderTexture mask;         // Alpha mask for the light
    float outerRadius;          // The distance the light touches
    Rectangle bounds;           // A cached rectangle of the light bounds to help with culling

    ShadowGeometry shadows[MAX_SHADOWS];
    int shadowCount;
} LightInfo;


LightInfo lights[MAX_LIGHTS] = { 0 };

// Move a light and mark it as dirty so that we update it's mask next frame
void MoveLight(int slot, float x, float y)
{
    lights[slot].dirty = true;
    lights[slot].position.x = x;
    lights[slot].position.y = y;

    // update the cached bounds
    lights[slot].bounds.x = x - lights[slot].outerRadius;
    lights[slot].bounds.y = y - lights[slot].outerRadius;
}

// Compute a shadow volume for the edge
// It takes the edge and projects it back by the light radius and turns it into a quad
void ComputeShadowVolumeForEdge(int slot, Vector2 sp, Vector2 ep)
{
    if (lights[slot].shadowCount >= MAX_SHADOWS) return;

    float extension = lights[slot].outerRadius*2;

    Vector2 spVector = Vector2Normalize(Vector2Subtract(sp, lights[slot].position));
    Vector2 spProjection = Vector2Add(sp, Vector2Scale(spVector, extension));

    Vector2 epVector = Vector2Normalize(Vector2Subtract(ep, lights[slot].position));
    Vector2 epProjection = Vector2Add(ep, Vector2Scale(epVector, extension));

    lights[slot].shadows[lights[slot].shadowCount].vertices[0] = sp;
    lights[slot].shadows[lights[slot].shadowCount].vertices[1] = ep;
    lights[slot].shadows[lights[slot].shadowCount].vertices[2] = epProjection;
    lights[slot].shadows[lights[slot].shadowCount].vertices[3] = spProjection;

    lights[slot].shadowCount++;
}

// Draw the light and shadows to the mask for a light
void DrawLightMask(int slot)
{
    // Use the light mask
    BeginTextureMode(lights[slot].mask);

        ClearBackground(WHITE);

        // Force the blend mode to only set the alpha of the destination
        rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MIN);
        rlSetBlendMode(BLEND_CUSTOM);

        // If we are valid, then draw the light radius to the alpha mask
        if (lights[slot].valid) DrawCircleGradient((int)lights[slot].position.x, (int)lights[slot].position.y, lights[slot].outerRadius, ColorAlpha(WHITE, 0), WHITE);

        rlDrawRenderBatchActive();

        // Cut out the shadows from the light radius by forcing the alpha to maximum
        rlSetBlendMode(BLEND_ALPHA);
        rlSetBlendFactors(RLGL_SRC_ALPHA, RLGL_SRC_ALPHA, RLGL_MAX);
        rlSetBlendMode(BLEND_CUSTOM);

        // Draw the shadows to the alpha mask
        for (int i = 0; i < lights[slot].shadowCount; i++)
        {
            DrawTriangleFan(lights[slot].shadows[i].vertices, 4, WHITE);
        }

        rlDrawRenderBatchActive();

        // Go back to normal blend mode
        rlSetBlendMode(BLEND_ALPHA);

    EndTextureMode();
}

// Setup a light
void SetupLight(int slot, float x, float y, float radius)
{
    lights[slot].active = true;
    lights[slot].valid = false;  // The light must prove it is valid
    lights[slot].mask = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
    lights[slot].outerRadius = radius;

    lights[slot].bounds.width = radius * 2;
    lights[slot].bounds.height = radius * 2;

    MoveLight(slot, x, y);

    // Force the render texture to have something in it
    DrawLightMask(slot);
}

// See if a light needs to update it's mask
bool UpdateLight(int slot, Rectangle* boxes, int count)
{
    if (!lights[slot].active || !lights[slot].dirty) return false;

    lights[slot].dirty = false;
    lights[slot].shadowCount = 0;
    lights[slot].valid = false;

    for (int i = 0; i < count; i++)
    {
        // Are we in a box? if so we are not valid
        if (CheckCollisionPointRec(lights[slot].position, boxes[i])) return false;

        // If this box is outside our bounds, we can skip it
        if (!CheckCollisionRecs(lights[slot].bounds, boxes[i])) continue;

        // Check the edges that are on the same side we are, and cast shadow volumes out from them

        // Top
        Vector2 sp = (Vector2){ boxes[i].x, boxes[i].y };
        Vector2 ep = (Vector2){ boxes[i].x + boxes[i].width, boxes[i].y };

        if (lights[slot].position.y > ep.y) ComputeShadowVolumeForEdge(slot, sp, ep);

        // Right
        sp = ep;
        ep.y += boxes[i].height;
        if (lights[slot].position.x < ep.x) ComputeShadowVolumeForEdge(slot, sp, ep);

        // Bottom
        sp = ep;
        ep.x -= boxes[i].width;
        if (lights[slot].position.y < ep.y) ComputeShadowVolumeForEdge(slot, sp, ep);

        // Left
        sp = ep;
        ep.y -= boxes[i].height;
        if (lights[slot].position.x > ep.x) ComputeShadowVolumeForEdge(slot, sp, ep);

        // The box itself
        lights[slot].shadows[lights[slot].shadowCount].vertices[0] = (Vector2){ boxes[i].x, boxes[i].y };
        lights[slot].shadows[lights[slot].shadowCount].vertices[1] = (Vector2){ boxes[i].x, boxes[i].y + boxes[i].height };
        lights[slot].shadows[lights[slot].shadowCount].vertices[2] = (Vector2){ boxes[i].x + boxes[i].width, boxes[i].y + boxes[i].height };
        lights[slot].shadows[lights[slot].shadowCount].vertices[3] = (Vector2){ boxes[i].x + boxes[i].width, boxes[i].y };
        lights[slot].shadowCount++;
    }

    lights[slot].valid = true;

    DrawLightMask(slot);

    return true;
}

// Set up some boxes
void SetupBoxes(Rectangle *boxes, int *count)
{
    boxes[0] = (Rectangle){ 150,80, 40, 40 };
    boxes[1] = (Rectangle){ 1200, 700, 40, 40 };
    boxes[2] = (Rectangle){ 200, 600, 40, 40 };
    boxes[3] = (Rectangle){ 1000, 50, 40, 40 };
    boxes[4] = (Rectangle){ 500, 350, 40, 40 };

    for (int i = 5; i < MAX_BOXES; i++)
    {
        boxes[i] = (Rectangle){(float)GetRandomValue(0,GetScreenWidth()), (float)GetRandomValue(0,GetScreenHeight()), (float)GetRandomValue(10,100), (float)GetRandomValue(10,100) };
    }

    *count = MAX_BOXES;
}


#define MAX_BUILDINGS   100

Texture2D Background;
Texture2D Player;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    const int sqw = 6;
    const int sqh = 140;
    Texture2D sprite;
    sprite = LoadTexture("client/assets/Sprite-0003");
    int rotation = 0;


    InitWindow(screenWidth, screenHeight, "raylib [core] example - 2d camera");

    Rectangle player = { screenWidth / 2, screenHeight / 2, sqw, sqh };
    Rectangle buildings[MAX_BUILDINGS] = { 0 };
    Color buildColors[MAX_BUILDINGS] = { 0 };

    int spacing = 0;

    for (int i = 0; i < MAX_BUILDINGS; i++)
    {
        buildings[i].width = (float)GetRandomValue(50, 200);
        buildings[i].height = (float)GetRandomValue(50, 100);
        buildings[i].y = screenHeight - 130.0f - buildings[i].height;
        buildings[i].x = -6000.0f + spacing;

        spacing += (int)buildings[i].width;

        buildColors[i] = (Color){ static_cast<unsigned char>(GetRandomValue(200, 240)), static_cast<unsigned char>(GetRandomValue(200, 240)), static_cast<unsigned char>(GetRandomValue(200, 250)), 255 };
    }

    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.x + ((float)sqw / 2), player.y + ((float)sqh / 2)};
    camera.offset = (Vector2){ screenWidth/2.0f, screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    uint8_t keys_tap_saved = 0b00000000;
    bool key_down = false;
    bool key_up = false;
    bool key_left = false;
    bool key_right = false;
    bool key_atack = false;
    // Main game loop


    Rectangle sourceRec = { 0.0f, 0.0f, (float)sqw, (float)sqh };

    // Destination rectangle (screen rectangle where drawing part of texture)
    Rectangle destRec = { screenWidth/2.0f, screenHeight/2.0f, sqw*2.0f, sqh*2.0f };

    Vector2 origin = { (float)sqw, (float)sqh };



    //
    //
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {

        uint8_t keys_tap_input = keys_tap_saved;

        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
        {
            key_right = true;
            player.x += 2;

        }
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
        {
            key_left = true;
            player.x -= 2;

        }
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
        {
            key_up = true;
            player.y -= 2;

        }
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
        {
            key_down = true;
            player.y += 2;
        }
        else if (IsKeyUp(KEY_RIGHT) || IsKeyUp(KEY_D))
        {
            key_right = false;
        }
        else if (IsKeyUp(KEY_LEFT) || IsKeyUp(KEY_A))
        {
            key_left = false;
        }
        else if (IsKeyUp(KEY_UP) || IsKeyUp(KEY_W))
        {
            key_up = false;
        }
        else if (IsKeyUp(KEY_DOWN) || IsKeyUp(KEY_S))
        {
            key_down = false;
        }
        if(IsKeyDown(KEY_Q))
        {
            rotation++;
        }
        else if(IsKeyDown(KEY_E))
        {
            rotation--;
        }



        keys_tap_input = key_up * 0b10000 + key_down * 0b1000 + key_left * 0b100 + key_right * 0b10 + key_atack * 0b1;


        if(keys_tap_input != keys_tap_saved)
        {


            keys_tap_saved = keys_tap_input;
        }



        // Camera target follows player
        camera.target = (Vector2){ player.x + ((float)sqw / 2) , player.y + ((float)sqh / 2)};


        // Camera zoom controls
        camera.zoom += ((float)GetMouseWheelMove()*0.05f);

        if (camera.zoom > 3.0f) camera.zoom = 3.0f;
        else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

        // Camera reset (zoom and rotation)
        if (IsKeyPressed(KEY_R))
        {
            camera.zoom = 1.0f;
            camera.rotation = 0.0f;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            DrawTexturePro(sprite, sourceRec, destRec, origin, (float)rotation, BLUE);

            BeginMode2D(camera);

                // DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY);

                for (int i = 0; i < MAX_BUILDINGS; i++) DrawRectangleRec(buildings[i], buildColors[i]);


                //Rectangle source= (Rectangle){0,0,50,50};
                //Rectangle dest = (Rectangle){25,25,50,50};
                //DrawTexturePro(sprite, sourceRec, destRec, origin, (float)rotation, BLUE);
                //DrawTexturePro(Background, , (Vector2){player.x, player.y}, 0, WHITE);

                DrawText(TextFormat("Player: %.1f  %.1f", player.x + 20, player.y + 20),  player.x + 20, player.y, 20, DARKGRAY);
                DrawRectangleRec(player, RED);

                DrawLine((int)camera.target.x, -screenHeight*10, (int)camera.target.x, screenHeight*10, GREEN);
                DrawLine(-screenWidth*10, (int)camera.target.y, screenWidth*10, (int)camera.target.y, GREEN);

            EndMode2D();

            // DrawRectangle( 10, 10, 250, 113, Fade(SKYBLUE, 0.5f));
            // DrawRectangleLines( 10, 10, 250, 113, BLUE);



        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
