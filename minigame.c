/***************************************************************
                           minigame.c
                               
The file contains the minigame manager
***************************************************************/

#include <libdragon.h>
#include <string.h>
#include "core.h"
#include "minigame.h"


/*********************************
             Globals
*********************************/

// Minigame info
static bool      global_minigame_ending = false;
static Minigame* global_minigame_current = NULL;
Minigame* global_minigame_list;
size_t    global_minigame_count;

// Helper consts
static const char*  global_minigamepath = "rom:/minigames/";
static const size_t global_minigamepath_len = 15;


/*==============================
    minigame_loadall
    Loads all the minigames from the filesystem
==============================*/

void minigame_loadall()
{
    size_t gamecount = 0;
    dir_t minigamesdir;

    // First, go through the minigames path and count the number of minigames
    dir_findfirst(global_minigamepath, &minigamesdir);
    do
    {
        if (!strstr(minigamesdir.d_name, ".sym"))
            gamecount++;
    }
    while (dir_findnext(global_minigamepath, &minigamesdir) == 0);
    global_minigame_count = gamecount;

    // Allocate the list of minigames
    global_minigame_list = (Minigame*)malloc(sizeof(Minigame) * gamecount);

    // Look through the minigames path and register all the known minigames
    gamecount = 0;
    dir_findfirst(global_minigamepath, &minigamesdir);
    do
    {
        void* handle;
        MinigameDef* loadeddef;
        Minigame* newdef = &global_minigame_list[gamecount];
        char* filename = minigamesdir.d_name;

        // Ignore the symbol file
        if (strstr(filename, ".sym"))
            continue;

        // Get the filepath and open the dso
        char fullpath[global_minigamepath_len + strlen(filename) + 1];
        sprintf(fullpath, "%s%s", global_minigamepath, filename);
        handle = dlopen(fullpath, RTLD_LOCAL);

        // Get the symbols of the minigame definition. 
        // Since these symbols will only be temporarily stored in memory, we must make a deep copy
        loadeddef = dlsym(handle, "minigame_def");
        assertf(loadeddef, "Unable to find symbol minigame_def in %s\n", filename);
        newdef->definition.gamename      = strdup(loadeddef->gamename);
        newdef->definition.developername = strdup(loadeddef->developername);
        newdef->definition.description   = strdup(loadeddef->description);
        newdef->definition.instructions  = strdup(loadeddef->instructions);

        // Set the internal name as the filename without the extension
        strrchr(filename, '.')[0] = '\0';
        newdef->internalname = strdup(filename);

        // Cleanup
        dlclose(handle);
        gamecount++;
    }
    while (dir_findnext("rom:/minigames/", &minigamesdir) == 0);
}


/*==============================
    minigame_play
    Executes a minigame
    @param  The internal filename of the minigame to play
==============================*/

void minigame_play(char* name)
{
    debugf("Loading minigame: %s\n", name);

    // Find the minigame with that name
    global_minigame_current = NULL;
    for (size_t i=0; i<global_minigame_count; i++)
    {
        if (!strcmp(global_minigame_list[i].internalname, name))
        {
            debugf("Success!\n");
            global_minigame_current = &global_minigame_list[i];
            break;
        }
    }
    assertf(global_minigame_current != NULL, "Unable to find minigame with internal name '%s'", name);

    // Load the dso and assign the internal functions
    char fullpath[global_minigamepath_len + strlen(name) + 4 + 1];
    sprintf(fullpath, "%s%s.dso", global_minigamepath, name);
    global_minigame_current->handle = dlopen(fullpath, RTLD_LOCAL);

    global_minigame_current->funcPointer_init      = dlsym(global_minigame_current->handle, "minigame_init");
    global_minigame_current->funcPointer_loop      = dlsym(global_minigame_current->handle, "minigame_loop");
    global_minigame_current->funcPointer_fixedloop = dlsym(global_minigame_current->handle, "minigame_fixedloop");
    global_minigame_current->funcPointer_cleanup   = dlsym(global_minigame_current->handle, "minigame_cleanup");
}


/*==============================
    minigame_end
    Ends the current minigame
==============================*/

void minigame_end()
{
    global_minigame_ending = true;
}


/*==============================
    minigame_get_game
    Gets the currently executing minigame
    @return The currently executing minigame
==============================*/

Minigame* minigame_get_game()
{
    return global_minigame_current;
}


/*==============================
    minigame_get_ended
    Checks whether the current minigame is ending
    @return Whether the current minigame is ending
==============================*/

bool minigame_get_ended()
{
    return global_minigame_ending;
}


/*==============================
    minigame_cleanup
    Cleans up minigame settings and memory used by the manager
==============================*/

void minigame_cleanup()
{
    global_minigame_ending = false;
    dlclose(global_minigame_current->handle);
    global_minigame_current->handle = NULL;
}