


#define UI_SCALE 1.0
#define SCREEN_MARGIN_TOP 24
#define MAX_ARROWS 125 * 4 / 2
#define SPEED_MULTI 1.0
#define ACCURACY 200
int32_t songTime = -10000;
int16_t songBPM = 125;
int points[4];
int multi[4];

xm64player_t music;

typedef enum {
    INTRO,
    RUNNING,
    OUTRO,
    ENDED
} gamestate;


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
    bool hit[4];
} arrowOnTrack;

typedef struct {
    arrowOnTrack arrows[MAX_ARROWS];
    int trackLength;
    int arrowNum;
    int bpm;
    int introLength;
    char *songPath;
    
} track;

#ifndef GAMEJAM2024_MINIGAME_H
#define GAMEJAM2024_MINIGAME_H 

    void minigame_init();
    void minigame_fixedloop(float deltatime);
    void minigame_loop(float deltatime);
    void minigame_cleanup();
    
#endif

int getMulti(uint8_t player);
int calculateXForArrow(uint8_t playerNum, uint8_t dir);
int calculateYForArrow(int time);
int calculateDeltaTime(int arrowIndex);
long currentTime;
void updateArrows();
void checkInputs();
void drawUI();
void drawUIForPlayer(uint8_t playerNum, uint8_t dir);
int countValidEntries();
void updateArrowList();
void loadSong();
void AIButtons(int songTime, float deltatime);
int findNextTimestamp(int songTime);

void renderOutro();
void drawArrows();
void drawArrowForPlayer(uint8_t playerNum, int yPos, uint8_t dir);

