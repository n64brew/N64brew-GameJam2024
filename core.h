#ifndef GAMEJAM2024_CORE_H
#define GAMEJAM2024_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

    /***************************************************************
                         Public Core Constants
    ***************************************************************/

    // Use this to standardize player colors
    #define PLAYERCOLOR_1  RGBA32(255, 0, 0, 255)
    #define PLAYERCOLOR_2  RGBA32(0, 255, 0, 255)
    #define PLAYERCOLOR_3  RGBA32(0, 0, 255, 255)
    #define PLAYERCOLOR_4  RGBA32(255, 255, 0, 255)

    // Player number definition
    typedef enum {
        PLAYER_1 = 0,
        PLAYER_2 = 1,
        PLAYER_3 = 2,
        PLAYER_4 = 3,
    } PlyNum;

    // AI difficulty definition
    typedef enum {
        DIFF_EASY = 0,
        DIFF_MEDIUM = 1,
        DIFF_HARD = 2,
    } AiDiff;


    /***************************************************************
                         Public Core Functions
    ***************************************************************/

    /*==============================
        core_get_playercount
        Get the number of human players
        @return The number of players
    ==============================*/
    uint32_t core_get_playercount();

    /*==============================
        core_get_playercontroller
        Get the controller port of this player.
        Because player 1's controller might not be plugged 
        into port number 1.
        @param  The player we want
        @return The controller port
    ==============================*/
    joypad_port_t core_get_playercontroller(PlyNum ply);

    /*==============================
        core_get_aidifficulty
        Gets the current AI difficulty
        @return The AI difficulty
    ==============================*/
    AiDiff core_get_aidifficulty();

    /*==============================
        core_get_subtick
        Gets the current subtick. Use this to help smooth
        movements in your draw loop. 
        @return The current subtick, as a 
                percentage (0.0f to 1.0f)
    ==============================*/
    double core_get_subtick();

    /*==============================
        core_set_winner
        Set the winner of the minigame. You can call this
        multiple times to set multiple winners.
        @param  The winning player
    ==============================*/
    void core_set_winner(PlyNum ply);

    
    /***************************************************************
                        Internal Core Functions
                  Do not use anything below this line
    ***************************************************************/

    #define TICKRATE   30
    #define DELTATIME  (1.0f/(double)TICKRATE)

    #define MAXPLAYERS  4

    void core_set_playercount(uint32_t playercount);
    void core_set_aidifficulty(AiDiff difficulty);
    void core_set_subtick(double subtick);
    void core_reset_winners();

#ifdef __cplusplus
}
#endif

#endif