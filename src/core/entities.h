#ifndef ENTITIES_H
#define ENTITIES_H

#include <snes.h>

#define TYPE_PLAYER1 0
#define TYPE_PLAYER2 1
#define TYPE_ENEMY 2
#define TYPE_ITEM 3
#define TYPE_BULLET_P1 4
#define TYPE_BULLET_P2 5

extern u8 p1_lives, p1_bombs;
extern u8 p2_lives, p2_bombs;

extern u8 p1_cooldown;
extern u8 p2_cooldown;

extern u8 frame_score1, frame_score2;

// Init Engine
void initEntities(void);
void spawn_entities(void);

// Player 1
void p1_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx);
void p1_update(u8 idx);

// Player 2
void p2_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx);
void p2_update(u8 idx);

// Enemy
void enemy_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx);
void enemy_update(u8 idx);

// Item
void item_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx);
void item_update(u8 idx);

// Projectiles
void bullet_p1_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx);

void bullet_p2_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx);

// Projectiles
void bullets_update(void);

// Explosion
void explosion_update(void);
void spawn_explosion(s16 x, s16 y);

#endif
