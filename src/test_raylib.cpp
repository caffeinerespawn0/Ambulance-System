#include "raylib.h"

int main(){
    InitWindow(600,400,"raylib test");
    SetTargetFPS(60);

    while(!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawCircle(300,200,40,RED);
        DrawText("it works",240,300,30,DARKGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}