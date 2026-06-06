#include "raylib.h"
#include "game.h"

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Boxing Evolution - PIF");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);
    SetRandomSeed((unsigned int)GetTime());

    Jogo *jogo = (Jogo *)MemAlloc(sizeof(Jogo));
    if (!jogo) { CloseWindow(); return 1; }

    JogoInit(jogo);

    while (!WindowShouldClose()) {
        JogoUpdate(jogo);
        JogoDraw(jogo);
    }

    MemFree(jogo);
    CloseWindow();
    return 0;
}
