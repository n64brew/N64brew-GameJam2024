#include "functions.h"
#include "../../core.h"
#include <math.h>

// UPDATE FUNCTIONS
void initPlayer(struct player* p, struct weapon* weapon){
    p->direction = 1;
    p->isAlive = true;
    p->verticalVelocity = 0.0;
    p->horizontalVelocity = 0.0;
    p->horizVelocityDir = 0;
    p->dropdownCounter = 0;
    p->onFloor = false;
    p->floorDroppable = false;
    p->moveLeft = false;
    p->moveRight = false;
    p->attackDirection = 1;
    p->attackTimer = 0;
    p->weapon = *weapon;
    p->weapon.xPos = p->xPos;
    p->weapon.yPos = p->yPos;
    p->slideCooldown = 0;
    p->onTopOfEnemy = false;
    p->ai_target = rand()%MAXPLAYERS;
    p->ai_reactionspeed = (2-core_get_aidifficulty())*5 + rand()%((3-core_get_aidifficulty())*3);
}

void updatePlayerBoundingBox(struct player *player) {
    // Define corner points
    player->leftTop.x = player->xPos;
    player->leftTop.y = player->yPos;
    player->leftBot.x = player->xPos;
    player->leftBot.y = player->yPos + player->height;
    player->rightTop.x = player->xPos + player->width;
    player->rightTop.y = player->yPos;
    player->rightBot.x = player->xPos + player->width;
    player->rightBot.y = player->yPos + player->height;

    // Define collision lines
    player->colTop.p1 = player->leftTop;
    player->colTop.p2 = player->rightTop;
    player->colRight.p1 = player->rightTop;
    player->colRight.p2 = player->rightBot;
    player->colBot.p1 = player->leftBot;
    player->colBot.p2 = player->rightBot;
    player->colLeft.p1 = player->leftTop;
    player->colLeft.p2 = player->leftBot;
}

void defineFloor(struct floorPiece *floor) {
    // Define corner points
    floor->leftTop.x = floor->xPos;
    floor->leftTop.y = floor->yPos;
    floor->leftBot.x = floor->xPos;
    floor->leftBot.y = floor->yPos + floor->height;
    floor->rightTop.x = floor->xPos + floor->width;
    floor->rightTop.y = floor->yPos;
    floor->rightBot.x = floor->xPos + floor->width;
    floor->rightBot.y = floor->yPos + floor->height;

    // Define collision lines
    floor->colTop.p1 = floor->leftTop;
    floor->colTop.p2 = floor->rightTop;
    floor->colRight.p1 = floor->rightTop;
    floor->colRight.p2 = floor->rightBot;
    floor->colBot.p1 = floor->leftBot;
    floor->colBot.p2 = floor->rightBot;
    floor->colLeft.p1 = floor->leftTop;
    floor->colLeft.p2 = floor->leftBot;
}

void updateWeaponHitbox(struct weapon *weapon) {
    // Define corner points
    weapon->leftTop.x = weapon->xPos;
    weapon->leftTop.y = weapon->yPos;
    weapon->leftBot.x = weapon->xPos;
    weapon->leftBot.y = weapon->yPos + weapon->height;
    weapon->rightTop.x = weapon->xPos + weapon->width;
    weapon->rightTop.y = weapon->yPos;
    weapon->rightBot.x = weapon->xPos + weapon->width;
    weapon->rightBot.y = weapon->yPos + weapon->height;

    // Define collision lines
    weapon->colTop.p1 = weapon->leftTop;
    weapon->colTop.p2 = weapon->rightTop;
    weapon->colRight.p1 = weapon->rightTop;
    weapon->colRight.p2 = weapon->rightBot;
    weapon->colBot.p1 = weapon->leftBot;
    weapon->colBot.p2 = weapon->rightBot;
    weapon->colLeft.p1 = weapon->leftTop;
    weapon->colLeft.p2 = weapon->leftBot;
}

// PLAYER FIXED LOOP FUNCTIONS 
// ############################################################################################################################
bool isPlayerOnFloor(struct player *p, struct floorPiece *f) {
    // only check if they are on a floor if they are moving downward or if they are not moving vertically 
    if(p->verticalVelocity >= 0) {
        if (abs(p->colBot.p1.y - f->colTop.p1.y) <= FLOOR_TOLERANCE) {
            // Check if the x-ranges of the player's bottom and floor's top lines overlap
            if ((p->colBot.p1.x < f->colTop.p2.x && p->colBot.p2.x > f->colTop.p1.x) ||
                (f->colTop.p1.x < p->colBot.p2.x && f->colTop.p2.x > p->colBot.p1.x)) {
                p->yPos = f->colTop.p1.y - p->height; // Align the player with the floor
                p->verticalVelocity = 0.0; // Stop downward motion
                p->floorDroppable = f->floorDroppable;
                return true;
            }
        }
    }
    return false;
}

void handleMovementAndGravity(struct player* p, struct player** players) {
    // Handle gravity
    if (!p->onFloor) {
        if(!p->onTopOfEnemy){
            if (p->verticalVelocity + GRAVITY <= MAX_VERT_VELOCITY) {
                p->verticalVelocity += GRAVITY;
            }
        }
    }

    // Apply horizontal resistance
    p->horizontalVelocity = (p->horizontalVelocity > HORIZ_RESISTANCE) ? 
                            (p->horizontalVelocity - HORIZ_RESISTANCE) : 0.0;

    // Slide cooldown reduction
    if (p->slideCooldown > 0) {
        p->slideCooldown -= 1;
    }

    // Vertical position update with collision check
    if(!detectPlayerCollision(p, players, p->xPos, p->yPos + (int)p->verticalVelocity)){
        p->yPos += (int)p->verticalVelocity;
    } else {
        p->verticalVelocity = 0.0;
    }

    // Horizontal movement
    if (p->horizontalVelocity > 0) {
        float newXPos = p->xPos + ((int)p->horizontalVelocity * p->horizVelocityDir);
        if(validateAndMovePlayer(p, players, (int)newXPos, p->yPos, LEFT_EDGE, RIGHT_EDGE)){
            p->xPos = (int)newXPos;
        } else {
            p->horizontalVelocity = 0.0;
        }
    }

    // Left movement
    if (p->moveLeft) {
        int newXPos = p->xPos - PLAYER_HORIZ_MOVE_SPEED;
        p->xPos = validateAndMovePlayer(p, players, newXPos, p->yPos, LEFT_EDGE, RIGHT_EDGE) ? 
                  newXPos : p->xPos;
        p->moveLeft = false;
    }

    // Right movement
    if (p->moveRight) {
        int newXPos = p->xPos + PLAYER_HORIZ_MOVE_SPEED;
        p->xPos = validateAndMovePlayer(p, players, newXPos, p->yPos, LEFT_EDGE, RIGHT_EDGE) ? 
                  newXPos : p->xPos;
        p->moveRight = false;
    }

    // Update bounding box after position changes
    updatePlayerBoundingBox(p);
}

void updatePlayerPos(struct player* p, struct floorPiece** floors, int* numFloors, struct player** players) {
    p->onFloor = false;
    p->onTopOfEnemy = false;

    // Check floors and enemies below if not in dropdown state
    if (p->dropdownCounter > 0) {
        p->dropdownCounter -= 1;
    } else {
        for (int i = 0; i < *numFloors; i++) {
            if (isPlayerOnFloor(p, floors[i])) {
                p->onFloor = true;
                break;
            }
        }
    }

    for (int i = 0; i < MAXPLAYERS; i++) {
        if (p->id != players[i]->id && players[i]->isAlive && onTopOfEnemy(p, players[i])) {
            p->onTopOfEnemy = true;
            break;
        }
    }

    // Movement, gravity, and collision handling
    handleMovementAndGravity(p, players);

    // Update weapon position to player's new position
    p->weapon.xPos = p->xPos;
    p->weapon.yPos = p->yPos;

    // Handle attack cooldown and timing
    if (p->attackCooldown > 0) p->attackCooldown -= 1;
    if (p->attackTimer > 0) {
        p->attackTimer -= 1;
        if (p->attackTimer == 0) {
            p->attackCooldown = p->weapon.attackCooldown;
        }

        // Extend weapon hitbox in correct direction
        p->weapon.xPos = (p->attackDirection == 0) ? (p->xPos - p->weapon.width) : 
                          (p->xPos + p->width);
    }
}

bool detectPlayerCollision(struct player* p, struct player** players, int newX, int newY) {
    for (int i = 0; i < MAXPLAYERS; i++) {
        if (p->id != players[i]->id && players[i]->isAlive) {
            struct player* jerk = players[i];
            if (checkBoundingBoxOverlap(newX, newY, p->width, p->height,
                                        jerk->xPos, jerk->yPos, jerk->width, jerk->height)) {
                return true;
            }
        }
    }
    return false;
}

bool validateAndMovePlayer(struct player* p, struct player** players, int newX, int newY, int leftEdge, int rightEdge) {
    if (newX >= leftEdge && (newX + p->width) <= rightEdge) {
        return !detectPlayerCollision(p, players, newX, newY);
    }
    return false;
}

// check if player2 is beneath player1
bool onTopOfEnemy(struct player *player1, struct player *player2){
    if (player2->yPos < player1->yPos - TOLERANCE) {
        return false;
    }

    return checkBoundingBoxOverlap(
            player1->xPos, player1->yPos+TOLERANCE, player1->width, player1->height,
            player2->xPos, player2->yPos, player2->width, player2->height);
}

// check if two bounding boxes overlap - GENERAL TOLERANCE
bool checkBoundingBoxOverlap(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2) {
    return (x1 < x2 + width2 - TOLERANCE) && (x1 + width1 - TOLERANCE > x2) &&
           (y1 < y2 + height2 - TOLERANCE) && (y1 + height1 - TOLERANCE > y2);
}

// check collision between players and opposing weapons
void checkPlayerWeaponCollision(struct player* player1, struct player* player2) {
    // player 1's weapon vs player 2
    if (checkBoundingBoxOverlap(
            player2->xPos, player2->yPos, player2->width, player2->height,
            player1->weapon.xPos, player1->weapon.yPos, player1->weapon.width, player1->weapon.height)) {
        // player 1's weapon hit player 2
        player2->isAlive = false; // player 2 is dead
    }
}

void initPlayerSlide(struct player* p){
    p->horizontalVelocity = SLIDE_HORIZ_STRENGTH;
    if(p->direction == 0){
        p->horizVelocityDir = -1;
    } else if(p->direction == 1){
        p->horizVelocityDir = 1;
    }
    p->verticalVelocity = -SLIDE_VERT_STRENGTH;
    p->slideCooldown = SLIDE_COOLDOWN;
    p->onFloor = false;
}

// AI FUNCTIONS
// AI function to calculate distance between two players
float calculateDistance(struct player* player1, struct player* player2) {
    float deltaX = player2->xPos - player1->xPos;
    float deltaY = (player2->yPos + player2->height) - player1->yPos;
    return sqrt(pow(deltaX, 2) + pow(deltaY, 2));
}

// Returns true if the target player is above the AI player
bool targetIsAbove(struct player* ai, struct player* target) {
    return target->yPos < ai->yPos;
}

// Returns true if the target player is more than 40px above the AI player
bool shouldCompJump(struct player* ai, struct player* target) {
    return (ai->yPos - target->yPos) > 30;
}

// Returns 0 if the target is to the left, 1 if the target is to the right
int targetDirection(struct player* ai, struct player* target) {
    return target->xPos > ai->xPos ? 1 : 0;
}

void generateCompInputs(struct player* ai, struct player* target, struct floorPiece** floors, int* numFloors){
    if (ai->id != target->id && target->isAlive) { // Check for a valid target
        // Move towards the direction of the target
        float dist = calculateDistance(ai, target);

        // random numbers used for various movement options
        int rand50 = rand() % 2;
        int rand25 = rand() % 4;

        // Check if the target is above the AI
        bool isAbove = targetIsAbove(ai, target);
        
        // Determine if the target is to the left or right
        int direction = targetDirection(ai, target);

        int aiDiff = core_get_aidifficulty();

        // attack if close, and the reaction time has elapsed
        if (dist < 45) {
            if(ai->attackTimer <= 0){
                if (ai->ai_reactionspeed <= 0) {
                    ai->attackDirection = ai->direction;
                    ai->attackTimer = ai->weapon.attackTimer;
                    ai->ai_reactionspeed = (2-aiDiff)*5 + rand()%((3-aiDiff)*3);
                } else {
                    ai->ai_reactionspeed--;
                }
            }

            // randomly slide away from enemy: 25% chance
            if(ai->onFloor || ai->onTopOfEnemy){
                if(ai->slideCooldown == 0){
                    if(rand25 == 1){
                        if(direction == 1){
                            initAiSlide(ai, 0);
                        } else {
                            initAiSlide(ai, 1);
                        }
                    }
                }
            }
        } else if (dist > 80) {
            // randomly slide in direction of the enemy to chase
            // EASY: 0% CHANCE
            if(aiDiff > 0){
                if(ai->onFloor || ai->onTopOfEnemy){
                    if(ai->slideCooldown == 0){
                        // MED: 25% CHANCE
                        if(aiDiff == 1){
                            if(rand25 == 1){
                                initAiSlide(ai, direction);
                            }
                        // HARD: 50% CHANCE
                        } else {
                            if(rand50 == 1){
                                initAiSlide(ai, direction);
                            }
                        }
                    }
                }
            }
        }

        // jump or slide based on circumstances
        if(ai->onTopOfEnemy){
            if(rand50 == 0){
                if(ai->slideCooldown == 0){
                    if(direction == 0){
                        initAiSlide(ai, 1);
                    } else {
                        initAiSlide(ai, 0);
                    }
                }
            } else {
                ai->verticalVelocity = -JUMP_STRENGTH;
                ai->onTopOfEnemy = false; 
            }
        }
        
        if(ai->onFloor){
            if (isAbove) {
                // 25% chance to slide instead of jumping
                if(rand25 == 1){
                    if(ai->slideCooldown == 0){
                        if(direction == 0){
                            initAiSlide(ai, 1);
                        } else {
                            initAiSlide(ai, 0);
                        }
                    }
                } else {
                    if(shouldCompJump(ai, target)){
                        ai->verticalVelocity = -JUMP_STRENGTH;
                        ai->onFloor = false;
                    }
                }
            } else {  
                if(ai->floorDroppable){
                    ai->verticalVelocity = DROPDOWN_STRENGTH;
                    ai->onFloor = false;
                    ai->dropdownCounter = 5; // ignore floor detection for 2 frames so you don't get pushed back out
                }
            }
        }

        if (direction == 0) {
            ai->moveLeft = true;
            ai->direction = 0;
        } else {
            ai->moveRight = true;
            ai->direction = 1;
        }
    } else {
        ai->ai_target = rand()%MAXPLAYERS; // (Attempt) to aquire a new target this frame
    }
}

void initAiSlide(struct player* ai, int dir){
    ai->horizontalVelocity = SLIDE_HORIZ_STRENGTH;
    if(dir == 0){
        ai->horizVelocityDir = -1;
    } else if(dir == 1){
        ai->horizVelocityDir = 1;
    }
    ai->verticalVelocity = -SLIDE_VERT_STRENGTH;
    ai->slideCooldown = SLIDE_COOLDOWN;
    ai->onFloor = false;
}

// PLAYER LOOP FUNCTIONS 
// ############################################################################################################################
// UPDATE PLAYER STATE BASED ON CONTROLLER INPUT
void pollPlayerInput(struct player *p, joypad_buttons_t *joypad_held){
    if(p->onFloor || p->onTopOfEnemy){
        // drop down if on droppable floor
        if(joypad_held->a && joypad_held->d_down){
            if(p->floorDroppable && !p->onTopOfEnemy){
                p->verticalVelocity = DROPDOWN_STRENGTH;
                p->onFloor = false;
                p->dropdownCounter = 5; // ignore floor detection for 2 frames so you don't get pushed back out
            }       
        // initiate slide    
        } else if (joypad_held->l){
            if(p->slideCooldown == 0){
                initPlayerSlide(p);
            }
        // initiate jump
        } else if (joypad_held->a) {
            p->verticalVelocity = -JUMP_STRENGTH;
            p->onFloor = false;
            p->onTopOfEnemy = false;
        }
    }

    // MOVE LEFT
    if(joypad_held->d_left){
        if (!p->moveLeft){
            p->moveLeft = true;
            p->direction = 0;
        }
    }
    // MOVE RIGHT
    if(joypad_held->d_right){
        if (!p->moveRight){
            p->moveRight = true;
            p->direction = 1;
        }
    }
}

void pollAttackInput(struct player *p, joypad_buttons_t *joypad_held){
    // can only attack if prev attack has finished
    if(p->attackTimer == 0 && p->attackCooldown == 0){
        // ATTACK FORWARD
        if(joypad_held->b){
            // set timer + direction
            p->attackDirection = p->direction;
            p->attackTimer = p->weapon.attackTimer;

            //extend weapon hitbox in correct direction
            if(p->direction == 0){
                p->weapon.xPos -= p->weapon.width;
            } else if(p->direction == 1){
                p->weapon.xPos += p->width;
            }
        }
    }
}

void rdpq_draw_one_rectangle(int *x, int *y, int *w, int *h, color_t color){
    // set rdp to primitive mode
    rdpq_set_mode_fill(color);

    // draw rectangle
    rdpq_fill_rectangle(*x, *y, *x + *w, *y + *h);
}

void rdpq_draw_one_floor_piece(int *x, int *y, int *w, int *h, color_t color){
    // set rdp to primitive mode
    rdpq_set_mode_fill(color);

    // draw rectangle
    rdpq_fill_rectangle(*x, *y, *x + *w, *y + *h);
}

void draw_players_and_level(struct player** players, sprite_t** player_sprites, sprite_t** player_left_attack_anim, 
                            sprite_t** player_right_attack_anim, struct floorPiece** floors, int* numFloors, color_t WHITE){
    // draw floors
    for(int i = 0; i < *numFloors; i++){
        rdpq_draw_one_floor_piece(&floors[i]->xPos, &floors[i]->yPos, &floors[i]->width, &floors[i]->height, WHITE);
    }

    // DRAW PLAYER SPRITES
    sprite_t* fighter_left_neutral = player_sprites[0];
    sprite_t* fighter_right_neutral = player_sprites[1];
    sprite_t* fighter_left_jump = player_sprites[2];
    sprite_t* fighter_right_jump = player_sprites[3];
    sprite_t* fighter_left_slide = player_sprites[4];
    sprite_t* fighter_right_slide = player_sprites[5];
    for(int i = 0; i < 4; i++){
        // draw player if alive
        if(players[i]->isAlive){
            rdpq_sync_pipe(); // Hardware crashes otherwise
            rdpq_sync_tile(); // Hardware crashes otherwise
            rdpq_set_mode_standard();
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            if(players[i]->attackTimer > 0){
                // pull attack animation frame based on attack timer value
                int animIndex = (players[i]->attackTimer)-1;
                if(players[i]->attackDirection == 0 && player_left_attack_anim[animIndex]){
                    rdpq_sprite_blit(player_left_attack_anim[animIndex], players[i]->xPos, players[i]->yPos, NULL);
                } else if(players[i]->attackDirection == 1 && player_left_attack_anim[animIndex]){
                    rdpq_sprite_blit(player_right_attack_anim[animIndex], players[i]->xPos, players[i]->yPos, NULL);
                }
            } else {
                if(players[i]->direction == 0){
                    // NEUTRAL
                    if(players[i]->onFloor || players[i]->onTopOfEnemy){
                        rdpq_sprite_blit(fighter_left_neutral, players[i]->xPos, players[i]->yPos, NULL);
                    } else {
                        // SLIDING
                        if(players[i]->slideCooldown > 0){
                            rdpq_sprite_blit(fighter_left_slide, players[i]->xPos, players[i]->yPos, NULL);
                        // FREE FALL
                        } else {
                            rdpq_sprite_blit(fighter_left_jump, players[i]->xPos, players[i]->yPos, NULL);
                        }
                    }
                } else if(players[i]->direction == 1){
                    // NEUTRAL
                    if(players[i]->onFloor || players[i]->onTopOfEnemy){
                        rdpq_sprite_blit(fighter_right_neutral, players[i]->xPos, players[i]->yPos, NULL);
                    } else {
                        // SLIDING
                        if(players[i]->slideCooldown > 0){
                            rdpq_sprite_blit(fighter_right_slide, players[i]->xPos, players[i]->yPos, NULL);
                        // FREE FALL
                        } else {
                            rdpq_sprite_blit(fighter_right_jump, players[i]->xPos, players[i]->yPos, NULL);
                        }
                    }
                }
            }
        }
    }
}