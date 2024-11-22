
#define UI_SCALE 1.0
#define SCREEN_MARGIN_TOP 24
#define MAX_ARROWS 50
uint32_t songTime;
typedef struct
{
    float x;
    float y;
    float scale_factor_x;
    float scale_factor_y;
} arrow;

typedef enum {
    ARR_UP,
    ARR_DOWN,
    ARR_LEFT,
    ARR_RIGHT,
} ArrowDirection;

typedef struct {
    int time;
    ArrowDirection direction;
    uint8_t difficulty;
} arrowOnTrack;

typedef struct {
    arrowOnTrack arrows[MAX_ARROWS];
} track;

track myTrack;


#ifndef GAMEJAM2024_MINIGAME_H
#define GAMEJAM2024_MINIGAME_H 

    void minigame_init();
    void minigame_fixedloop(float deltatime);
    void minigame_loop(float deltatime);
    void minigame_cleanup();
    
#endif
int calculateXForArrow(uint8_t playerNum, uint8_t dir);
int calculateYForArrow(int time);
long currentTime;
void updateArrows();
void checkInputs();
void drawUI();
void drawUIForPlayer(uint8_t playerNum, uint8_t dir);
int countValidEntries();
void updateArrowList();

void drawArrows();
void drawArrowForPlayer(uint8_t playerNum, int time, uint8_t dir);

