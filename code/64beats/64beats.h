
#ifndef GAMEJAM2024_MINIGAME_H
#define GAMEJAM2024_MINIGAME_H 

    void minigame_init();
    void minigame_fixedloop(float deltatime);
    void minigame_loop(float deltatime);
    void minigame_cleanup();
    
#endif
void drawUI();
void drawUIForPlayer(int posx, int posy);

typedef struct {
    int x;
    int y;
} HUDPoints;

typedef struct {
    HUDPoints points[4]; // Array of 4 Point structs
} HUDPositions;

HUDPositions hudPos;