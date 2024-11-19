/*================
ASSETS_H
Contains paths to the assets to be loaded
================*/
#ifndef ASSETS_H
#define ASSETS_H

// ================ MODELS ==============
#define MODEL_COUNT 7
extern const char *model_paths[MODEL_COUNT];
#define MODEL_BOX 0      //"rom:/old_gods/box.t3dm",        
#define MODEL_FOOD 1      //"rom:/old_gods/food.t3dm",       
#define MODEL_MAP 2      //"rom:/old_gods/map.t3dm",       
#define MODEL_SHADOW 3      //"rom:/old_gods/shadow.t3dm",  
#define MODEL_SNAKE 4      //"rom:/old_gods/snake.t3dm",     
#define MODEL_SPIDER 5      //"rom:/old_gods/spider.t3dm", 
#define MODEL_FROG 6


// ================ TEXTURES ==============
#define TEXTURE_COUNT 10
extern const char *texture_path[TEXTURE_COUNT];

// Define for texture IDs
#define TEXTURE_ID_0 0      //"rom:/green.sprite",        // 0 player 1
#define TEXTURE_ID_1 1      //"rom:/green.sprite",        // 0 player 1
#define TEXTURE_ID_2 2      //"rom:/orange.sprite",       // 2 player 3
#define TEXTURE_ID_3 3      //"rom:/purple.sprite",       // 3 player 4
#define TEXTURE_ID_4 4      //"rom:/grey.sprite",         // 4 god
#define TEXTURE_ID_5 5      //"rom:/diamond0.sprite",     // 5 bucket
#define TEXTURE_ID_6 6      //"rom:/triangle0.sprite",    // 6 villages
#define TEXTURE_ID_7 7      //"rom:/checker.sprite",       // 7 level
#define TEXTURE_ID_8 8      //"rom:/dark.sprite",         // 8 level
#define TEXTURE_ID_9 9      //"rom:/god.sprite"           // 9 god

extern const char* animatedSpritePath;

// ================ AUDIO ==============
extern const char* cannonFXPath;
extern const char* laserFXPath;
extern const char* musicFXPath;

// ================ FONT ==============
extern const char* fontPath;
extern const char* fontPath2;
#define FONT1_ID 1
#define FONT2_ID 2



#endif  // ASSETS_H

