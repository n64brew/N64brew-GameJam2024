/***************************************************************
                           minigame.c
                               
The file contains the minigame manager
***************************************************************/

#include <libdragon.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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
    minigame_sort
    Alphabetically sorts minigames based on their names
    @param  The first minigame
    @param  The second minigame
    @return -1 if a is less than b, 1 if a is greater than b, and 0 if they are equal
==============================*/

static int minigame_sort(const void *a, const void *b)
{
    const Minigame *ma = (Minigame *) a;
    const Minigame *mb = (Minigame *) b;
    return strcasecmp(ma->definition.gamename, mb->definition.gamename);
}


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
        if (strstr(minigamesdir.d_name, ".desc"))
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
        char buffer[512] = {0};
        Minigame* newdef = &global_minigame_list[gamecount];
        char* filename = minigamesdir.d_name;

        // Ignore the symbol and the dso file
        if (!strstr(filename, ".desc"))
            continue;

        // Get the filepath and open the description file
        char fullpath[global_minigamepath_len + strlen(filename) + 1];
        sprintf(fullpath, "%s%s", global_minigamepath, filename);
        FILE* file = fopen(fullpath, "r");

        // Get the lines of the minigame definition. 
        // These lines will be streamed directly to the minigame list
        assertf(file, "Unable to find minigame description in %s\n", filename);
        buffer[0] = 0;
        fgets(buffer, sizeof(buffer), file);  newdef->definition.gamename = strdup(buffer);
        fgets(buffer, sizeof(buffer), file);  newdef->definition.developername = strdup(buffer);
        fgets(buffer, sizeof(buffer), file);  buffer[strlen(buffer)-1] = 0;   // remove trailing \n
        newdef->definition.description = strdup(buffer);
        fgets(buffer, sizeof(buffer), file);  newdef->definition.instructions = strdup(buffer);

        // Set the internal name as the filename without the extension
        strrchr(filename, '.')[0] = '\0';
        newdef->internalname = strdup(filename);

        // Cleanup
        fclose(file);
        gamecount++;
    }
    while (dir_findnext("rom:/minigames/", &minigamesdir) == 0);

    qsort(global_minigame_list, gamecount, sizeof(Minigame), minigame_sort);
}


/*==============================
    minigame_loadnext
    Loads a minigame
    @param  The internal filename of the minigame to load
==============================*/

void minigame_loadnext(char* name)
{
    //debugf("Loading minigame: %s\n", name);

    // Find the minigame with that name
    global_minigame_current = NULL;
    for (size_t i=0; i<global_minigame_count; i++)
    {
        if (!strcmp(global_minigame_list[i].internalname, name))
        {
            //debugf("Success!\n");
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
    if (core_get_nextround() != NR_FREEPLAY)
        core_level_changeto(LEVEL_RESULTS);
    else
        core_level_changeto(LEVEL_MINIGAMESELECT);
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
    minigame_get_index
    TODO
==============================*/

int minigame_get_index()
{
    return (global_minigame_current - global_minigame_list);
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
