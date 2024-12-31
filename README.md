# N64brew Game Jam 2024

For 2024, N64brew decided to put together a minigame jam. This repository originally contained the template project, and now it contains the final ROM that was put together at the end of the jam. You can obtain the final ROM from the [releases page](../../releases). You can also download the old template we provided, in case you wish to make a minigame of your own.

Below is the original description of this repository's README before the final ROM was put together.

---

Welcome to this year's N64brew game jam. This year, we are creating a large collective minigame ROM, and that requires every project to use the same base. Therefore, we are providing this base template for all projects to fork from. Unfortunately, due to time constraints, we were only able to provide a single template for [Libdragon](https://github.com/DragonMinded/libdragon).

Please ensure that you are using the **latest** `preview` branch of Libdragon, that you have [Tiny3D](https://github.com/HailToDodongo/tiny3d) installed, and that you are able to compile this ROM without issues. This template has both an OpenGL and [Tiny3D](https://github.com/HailToDodongo/tiny3d) example game. We also recommend that you fork this repository so that you can then perform a pull request for your final project (although you will lose the ability to make your repository private).


### Starting a new minigame

To make things simple, every single minigame will be compiled into its own DSO file and loaded at runtime. In order to ensure that happens, each minigame should be placed in its own folder inside the `code`. Folder names should be alphanumeric. Inside this folder, you can create as many subfolders and code files as you need, be it C, C++, or header files. The makefile will automatically handle compiling of files for you.

In order for your minigame to work with the template's minigame manager, the following is required:
* You must have the following functions defined:
```c
/*==============================
    minigame_init
    The minigame initialization function
==============================*/
void minigame_init();

/*==============================
    minigame_fixedloop
    Code that is called every loop, at a fixed delta time.
    Use this function for stuff where a fixed delta time is 
    important, like physics.
    @param  The fixed delta time for this tick
==============================*/
void minigame_fixedloop(float deltatime);

/*==============================
    minigame_loop
    Code that is called every loop.
    @param  The delta time for this tick
==============================*/
void minigame_loop(float deltatime);

/*==============================
    minigame_cleanup
    Clean up any memory used by your game just before it ends.
==============================*/
void minigame_cleanup();
```
* You must have the following global struct defined:
```c
const MinigameDef minigame_def = {
    .gamename = "Example Game",
    .developername = "Your Name",
    .description = "This is an example game.",
    .instructions = "Press A to win."
};
```

We have provided a blank minigame template in `assets/blank/blank_template.c` that includes everything you need to get started with a new game. Just move this folder over to the `code` folder, and rename the `blank` folder and `blank_template.c` file to whatever you want (ideally something that matches your game).

Please be careful with cleaning up the memory used by your project, use the `sys_get_heap_stats` function provided by Libdragon to compare the heap allocations during your minigame initialization and after everything has been cleaned up. Libdragon does use `malloc` internally for handling some things, so if you notice that your cleanup function doesn't account for all bytes, try running your minigame two or three more times. The memory usage should stabilize after the first run of the minigame.

Both the `core.h` and `minigame.h` headers include some public functions which you should be using in your project. Most importantly, you should be using `core_get_playercontroller` to get a specific player's controller port, as there is no guarantee that player 1's controller is plugged into port 1 on the console.

If you are working on multiple minigames, **do not** cross reference files between them. For instance, if you create a function `myfunc` inside of a minigame, do not try to access `myfunc` in a separate minigame's codebase. You can duplicate the function for your other minigame without any problems as the minigames are loaded at runtime and thus will not interfere with one another.

Regarding assets, to avoid name conflicts with other projects in the final ROM, you should create a folder for your specific minigame in the `assets` folder. You can then create an `mk` file to list out any assets which you need for your project (as well as allow you to configure things like fonts). Check the `snake3d` or `polyquiz` game for an example of how to add external assets.

When in doubt, refer to how the example games are done.


### Configuration during testing

When you boot the ROM, a small menu appears to let you configure the testing environment. Alternatively, you can modify the provided `config.h` file to automatically set a specific configuration (and thus skip the menu). **This is the only core file which you should be making any modifications to**, you should avoid making **any changes** to the template itself. If you encounter a bug in the template, feel free to open an issue or create a pull request with a fix **so that said fix can be made available to all users**.


### Minigame QOL recommendations

Here's some suggestions of QOL things you should do for minigames, they are **not** requirements:
* Have a 3 second countdown at the start. In the examples, we are using `assets/core/Countdown.wav` for the countdown, and `assets/core/Start.wav` when the minigame starts.
* In the examples, we are also using `assets/core/Stop.wav` when the minigame ends, and then `assets/core/Winner.wav` when the winner is announced.
* Allow the minigame to be paused by pressing START, and possibly exit as well
* It's recommended to keep player colors consistent between games. We set some definitions in `core.h` which you should use. 
* We have button icons available in `assets/core`
* Try to keep your (compressed) assets under 2 MiB, since everyone needs to share the ROM space. Not a big deal if you MUST go over.

### Building with libdragon-cli

Clone this repository with `--recurse-submodules` or if you haven't run:

```bash
git submodule update --init
```

Init container:

```bash
libdragon init
```

Build:

```bash
libdragon make -C tiny3d install
libdragon make -C tiny3d/tools/gltf_importer install
libdragon make
```