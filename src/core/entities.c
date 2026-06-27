// clang-format off

#include "entities.h"
#include "maps.h"
#include "system.h"
#include <snes.h>

#define GAME_OB_MAX 32

extern u16 nbobjects;

u8 p1_lives = 3, p1_bombs = 3;
u8 p2_lives = 3, p2_bombs = 3;

u8 p1_cooldown = 0;
u8 p2_cooldown = 0;

u8 p1_weapon_level = 1;
u8 p2_weapon_level = 1;

u8 frame_score1 = 0;
u8 frame_score2 = 0;

u8 p1_obj_idx = 255;
u8 p2_obj_idx = 255;

// Enemy bitmask cache
u16 current_spawn_idx = 0;
u8 enemy_obj_indices[16];
u8 item_obj_indices[4];
u8 item_active_mask = 0;

u16 en_active_mask = 0;
s16 en_x[16];
s16 en_y[16];
u8 en_hp[16];
u8 en_flash[16];

u8 p1_death_timer = 0;
u8 p1_invuln_timer = 0;
u8 p2_death_timer = 0;
u8 p2_invuln_timer = 0;

// Explosion system (1 active at a time)
u8 explosion_timer = 0;
s16 explosion_x, explosion_y;
u16 explosion_spr_base;

t_objs *snesobj;
u16 sprnum;

typedef struct {
  u8 active;  // 0 = dead, 1 = alive
  u16 sprnum; // OAM index
  s16 x;
  s16 y;
} Bullet;

Bullet p1_bullets[6];
Bullet p2_bullets[6];

extern char spr_fighters1_til;
extern char spr_fighters2_til;
extern char spr_enemies1_til;
extern char spr_fx_til;

// ==========================================
// OBJECT TABLE
// ==========================================
const u16 tabobjects[] = {
     60,   1504, TYPE_ENEMY,  0, 0,
     92,   1504, TYPE_ENEMY,  0, 0,
     76,   1503, TYPE_ENEMY,  0, 0,
     38,   1440, TYPE_ENEMY,  0, 0,
     70,   1440, TYPE_ENEMY,  0, 0,
     54,   1439, TYPE_ENEMY,  0, 0,
    102,   1376, TYPE_ENEMY,  0, 0,
    134,   1376, TYPE_ENEMY,  0, 0,
    118,   1375, TYPE_ENEMY,  0, 0,
     94,   1312, TYPE_ENEMY,  0, 0,
    126,   1312, TYPE_ENEMY,  0, 0,
    110,   1311, TYPE_ENEMY,  0, 0,
     89,   1248, TYPE_ENEMY,  0, 0,
    121,   1248, TYPE_ENEMY,  0, 0,
    105,   1247, TYPE_ENEMY,  0, 0,
     67,   1184, TYPE_ENEMY,  0, 0,
     99,   1184, TYPE_ENEMY,  0, 0,
     83,   1183, TYPE_ENEMY,  0, 0,
     58,   1120, TYPE_ENEMY,  0, 0,
     90,   1120, TYPE_ENEMY,  0, 0,
     74,   1119, TYPE_ENEMY,  0, 0,
     54,   1056, TYPE_ENEMY,  0, 0,
     86,   1056, TYPE_ENEMY,  0, 0,
     70,   1055, TYPE_ENEMY,  0, 0,
    140,    992, TYPE_ENEMY,  0, 0,
    172,    992, TYPE_ENEMY,  0, 0,
    156,    991, TYPE_ENEMY,  0, 0,
     40,    928, TYPE_ENEMY,  0, 0,
     72,    928, TYPE_ENEMY,  0, 0,
     56,    927, TYPE_ENEMY,  0, 0,
     39,    864, TYPE_ENEMY,  0, 0,
     71,    864, TYPE_ENEMY,  0, 0,
     55,    863, TYPE_ENEMY,  0, 0,
     55,    800, TYPE_ENEMY,  0, 0,
     87,    800, TYPE_ENEMY,  0, 0,
     71,    799, TYPE_ENEMY,  0, 0,
     87,    736, TYPE_ENEMY,  0, 0,
    119,    736, TYPE_ENEMY,  0, 0,
    103,    735, TYPE_ENEMY,  0, 0,
     91,    672, TYPE_ENEMY,  0, 0,
    123,    672, TYPE_ENEMY,  0, 0,
    107,    671, TYPE_ENEMY,  0, 0,
    161,    608, TYPE_ENEMY,  0, 0,
    193,    608, TYPE_ENEMY,  0, 0,
    177,    607, TYPE_ENEMY,  0, 0,
     38,    544, TYPE_ENEMY,  0, 0,
     70,    544, TYPE_ENEMY,  0, 0,
     54,    543, TYPE_ENEMY,  0, 0,
     82,    480, TYPE_ENEMY,  0, 0,
    114,    480, TYPE_ENEMY,  0, 0,
     98,    479, TYPE_ENEMY,  0, 0,
    139,    416, TYPE_ENEMY,  0, 0,
    171,    416, TYPE_ENEMY,  0, 0,
    155,    415, TYPE_ENEMY,  0, 0,
     88,    352, TYPE_ENEMY,  0, 0,
    120,    352, TYPE_ENEMY,  0, 0,
    104,    351, TYPE_ENEMY,  0, 0,
    146,    288, TYPE_ENEMY,  0, 0,
    178,    288, TYPE_ENEMY,  0, 0,
    162,    287, TYPE_ENEMY,  0, 0,
    103,    224, TYPE_ENEMY,  0, 0,
    135,    224, TYPE_ENEMY,  0, 0,
    119,    223, TYPE_ENEMY,  0, 0,
     33,    160, TYPE_ENEMY,  0, 0,
     65,    160, TYPE_ENEMY,  0, 0,
     49,    159, TYPE_ENEMY,  0, 0,
     72,     96, TYPE_ENEMY,  0, 0,
    104,     96, TYPE_ENEMY,  0, 0,
     88,     95, TYPE_ENEMY,  0, 0,
    140,     32, TYPE_ENEMY,  0, 0,
    172,     32, TYPE_ENEMY,  0, 0,
    156,     31, TYPE_ENEMY,  0, 0,
    0xFFFF};

// ==========================================
// INITIALIZATION
// ==========================================
void initEntities() {
  objInitEngine();

  objInitFunctions(TYPE_PLAYER1, &p1_init, &p1_update, NULL);
  objInitFunctions(TYPE_PLAYER2, &p2_init, &p2_update, NULL);
  objInitFunctions(TYPE_ENEMY, &enemy_init, &enemy_update, NULL);
  objInitFunctions(TYPE_ITEM, &item_init, &item_update, NULL);

  // Players
  p1_init(64, 1764, TYPE_PLAYER1, 0, 0);
  p2_init(192, 1764, TYPE_PLAYER2, 0, 0);

  // Pre-allocate 16 enemies
  u8 i;
  for (i = 0; i < 16; i++) {
    enemy_init(0, 240, TYPE_ENEMY, 0, 0);
  }

  if (game_mode == MODE_BENCHMARK) {
    for (i = 0; i < 16; i++) {
      u8 idx = enemy_obj_indices[i];
      *((s16 *)&(objbuffers[idx].xpos[1])) = rand() % 220;
      *((s16 *)&(objbuffers[idx].ypos[1])) = 1568 + (rand() % 220);
      objbuffers[idx].xvel = (rand() & 1) ? 1 : -1;
      objbuffers[idx].yvel = (rand() & 1) ? 1 : -1;
    }
  } else {
    en_active_mask = 0; // All dormant
    current_spawn_idx = 0;
  }

  for (i = 0; i < 16; i++) {
    en_hp[i] = 3;
    en_flash[i] = 0;
  }
  p1_death_timer = 0;
  p1_invuln_timer = 0;
  p2_death_timer = 0;
  p2_invuln_timer = 0;

  // Pre-allocate 4 items
  for (i = 0; i < 4; i++) {
    item_init(0, 240, TYPE_ITEM, 0, 0);
  }
  item_active_mask = 0; // All dormant

  // Pre-allocate bullet objects
  for (i = 0; i < 6; i++) {
    bullet_p1_init(0, 100, TYPE_BULLET_P1, 0, 0);
    bullet_p2_init(0, 100, TYPE_BULLET_P2, 0, 0);
  }

  // Pre-allocate explosion sprites (9 total: 1 frame1 + 4 frame2 + 4 frame3)
  explosion_spr_base = nbobjects;

  // Frame 1: 1 sprite (frame 160)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 40;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Frame 2: 4 sprites (frames 162, 164, 164 hflip+vflip, 162 hflip+vflip)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 41;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 42;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 42;
  oambuffer[nbobjects].oamattribute = 0xF0 | (0 << 1); // hflip+vflip
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 41;
  oambuffer[nbobjects].oamattribute = 0xF0 | (0 << 1); // hflip+vflip
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Frame 3: 4 sprites (frames 166, 168, 168 hflip+vflip, 166 hflip+vflip)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 43;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 44;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 44;
  oambuffer[nbobjects].oamattribute = 0xF0 | (0 << 1); // hflip+vflip
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 43;
  oambuffer[nbobjects].oamattribute = 0xF0 | (0 << 1); // hflip+vflip
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;
}

// ==========================================
// PLAYER 1 (3 sprites: body, left wing, right wing)
// ==========================================
void p1_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx) {
  if (objNew(type, xp, yp) == 0)
    return;
  objGetPointer(objgetid);
  snesobj = &objbuffers[objptr - 1];
  snesobj->width = 16;
  snesobj->height = 16;
  snesobj->sprnum = nbobjects;

  // Sprite 0: top
  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp;
  oambuffer[nbobjects].oamframeid = 2;
  oambuffer[nbobjects].oamattribute = 0x30 | (1 << 1); // Priority 3, Palette 1
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Sprite 1: left wing
  oambuffer[nbobjects].oamx = xp - 8;
  oambuffer[nbobjects].oamy = yp + 16;
  oambuffer[nbobjects].oamframeid = 10;
  oambuffer[nbobjects].oamattribute = 0x30 | (1 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Sprite 2: right wing
  oambuffer[nbobjects].oamx = xp + 8;
  oambuffer[nbobjects].oamy = yp + 16;
  oambuffer[nbobjects].oamframeid = 11;
  oambuffer[nbobjects].oamattribute = 0x30 | (1 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  p1_obj_idx = objgetid;
}

void p1_update(u8 idx) {
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
  sprnum = snesobj->sprnum;

  // Death timer: player is hidden
  if (p1_death_timer > 0) {
    p1_death_timer--;
    oambuffer[sprnum].oamy = 240;
    oambuffer[sprnum + 1].oamy = 240;
    oambuffer[sprnum + 2].oamy = 240;
    oamFix16Draw(sprnum);
    oamFix16Draw(sprnum + 1);
    oamFix16Draw(sprnum + 2);
    // Autoscroll even while dead
    if (y_pos > 0 && (gFrames & 3) == 3)
      *oy = *oy - 1;
    if (p1_death_timer == 0) {
      p1_invuln_timer = 180; // 3 seconds invulnerability
      p1_lives--;
      upd_p1_lives = 1;
      p1_weapon_level = 1;
    }
    return;
  }

  s16 px = *ox;
  s16 py = *oy;

  u16 pad1 = padsCurrent(0);

  if (pad1 & KEY_UP)
    py -= 2;
  if (pad1 & KEY_DOWN)
    py += 2;
  if (pad1 & KEY_LEFT)
    px -= 2;
  if (pad1 & KEY_RIGHT)
    px += 2;

  // Autoscroll the player with the map so they stay on screen
  if (y_pos > 0 && (gFrames & 3) == 3)
    py -= 1;

  if (px < 8)
    px = 8;
  else if (px > 232)
    px = 232;
  if (py < (s16)y_pos + 8)
    py = (s16)y_pos + 8;
  else if (py > (s16)y_pos + 208)
    py = (s16)y_pos + 208;

  *ox = px;
  *oy = py;

  if ((pad1 & KEY_X) && p1_bombs > 0 && p1_cooldown == 0) {
    p1_bombs--;
    upd_p1_bombs = 1;
    p1_cooldown = 30;
  }

  if (p1_cooldown > 0)
    p1_cooldown--;

  if (p1_invuln_timer > 0)
    p1_invuln_timer--;

  if ((pad1 & (KEY_A | KEY_B | KEY_Y)) && p1_cooldown == 0) {
    u8 i;
    for (i = 0; i < 6; i++) {
      if (p1_bullets[i].active == 0) {
        // Level 1=center(2), Level 2=center+right(6), Level 3=all(7)
        p1_bullets[i].active = (p1_weapon_level == 1) ? 2 : (p1_weapon_level == 2) ? 6 : 7;
        p1_bullets[i].x = px;
        p1_bullets[i].y = py - 16;
        break;
      }
    }
    p1_cooldown = 6;
  }

  // Calculate screen coordinates
  s16 screen_x = px;
  s16 screen_y = py - (s16)y_pos;

  if (screen_y > 240 || screen_y < -16) {
    oambuffer[sprnum].oamy = 240;
    oambuffer[sprnum + 1].oamy = 240;
    oambuffer[sprnum + 2].oamy = 240;
  } else {
    // Blink during invulnerability
    if (p1_invuln_timer > 0 && (p1_invuln_timer & 4)) {
      oambuffer[sprnum].oamy = 240;
      oambuffer[sprnum + 1].oamy = 240;
      oambuffer[sprnum + 2].oamy = 240;
      oamFix16Draw(sprnum);
      oamFix16Draw(sprnum + 1);
      oamFix16Draw(sprnum + 2);
    } else {
      // Draw top
      oambuffer[sprnum].oamx = screen_x;
      oambuffer[sprnum].oamy = screen_y;
      oamFix16Draw(sprnum);

      // Draw left wing
      oambuffer[sprnum + 1].oamx = screen_x - 8;
      oambuffer[sprnum + 1].oamy = screen_y + 16;
      oamFix16Draw(sprnum + 1);

      // Draw right wing
      oambuffer[sprnum + 2].oamx = screen_x + 8;
      oambuffer[sprnum + 2].oamy = screen_y + 16;
      oamFix16Draw(sprnum + 2);
    }
  }
}

// ==========================================
// PLAYER 2 (3 sprites: body, left wing, right wing)
// ==========================================
void p2_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx) {
  if (game_mode != MODE_2P && game_mode != MODE_BENCHMARK)
    return;

  if (objNew(type, xp, yp) == 0)
    return;
  objGetPointer(objgetid);
  snesobj = &objbuffers[objptr - 1];
  snesobj->width = 16;
  snesobj->height = 16;
  snesobj->sprnum = nbobjects;

  // Sprite 0: top
  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp;
  oambuffer[nbobjects].oamframeid = 18;
  oambuffer[nbobjects].oamattribute = 0x30 | (2 << 1); // Priority 3, Palette 2
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Sprite 1: left wing
  oambuffer[nbobjects].oamx = xp - 8;
  oambuffer[nbobjects].oamy = yp + 16;
  oambuffer[nbobjects].oamframeid = 26;
  oambuffer[nbobjects].oamattribute = 0x30 | (2 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Sprite 2: right wing
  oambuffer[nbobjects].oamx = xp + 8;
  oambuffer[nbobjects].oamy = yp + 16;
  oambuffer[nbobjects].oamframeid = 27;
  oambuffer[nbobjects].oamattribute = 0x30 | (2 << 1);
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  p2_obj_idx = objgetid;
}

void p2_update(u8 idx) {
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
  sprnum = snesobj->sprnum;

  // Death timer: player is hidden
  if (p2_death_timer > 0) {
    p2_death_timer--;
    oambuffer[sprnum].oamy = 240;
    oambuffer[sprnum + 1].oamy = 240;
    oambuffer[sprnum + 2].oamy = 240;
    oamFix16Draw(sprnum);
    oamFix16Draw(sprnum + 1);
    oamFix16Draw(sprnum + 2);
    if (y_pos > 0 && (gFrames & 3) == 3)
      *oy = *oy - 1;
    if (p2_death_timer == 0) {
      p2_invuln_timer = 180;
      p2_lives--;
      upd_p2_lives = 1;
      p2_weapon_level = 1;
    }
    return;
  }

  s16 px = *ox;
  s16 py = *oy;

  u16 pad2 = padsCurrent(1);

  if (pad2 & KEY_UP)
    py -= 2;
  if (pad2 & KEY_DOWN)
    py += 2;
  if (pad2 & KEY_LEFT)
    px -= 2;
  if (pad2 & KEY_RIGHT)
    px += 2;

  // Autoscroll the player with the map so they stay on screen
  if (y_pos > 0 && (gFrames & 3) == 3)
    py -= 1;

  if (px < 8)
    px = 8;
  else if (px > 232)
    px = 232;
  if (py < y_pos + 8)
    py = y_pos + 8;
  else if (py > y_pos + 208)
    py = y_pos + 208;

  *ox = px;
  *oy = py;

  if ((pad2 & KEY_X) && p2_bombs > 0 && p2_cooldown == 0) {
    p2_bombs--;
    upd_p2_bombs = 1;
    p2_cooldown = 30;
  }

  if (p2_cooldown > 0)
    p2_cooldown--;

  if (p2_invuln_timer > 0)
    p2_invuln_timer--;

  if ((pad2 & (KEY_A | KEY_B | KEY_Y)) && p2_cooldown == 0) {
    u8 i;
    for (i = 0; i < 6; i++) {
      if (p2_bullets[i].active == 0) {
        p2_bullets[i].active = (p2_weapon_level == 1) ? 2 : (p2_weapon_level == 2) ? 6 : 7;
        p2_bullets[i].x = px;
        p2_bullets[i].y = py - 16;
        break;
      }
    }
    p2_cooldown = 6;
  }

  // Calculate screen coordinates
  s16 screen_x = px;
  s16 screen_y = py - (s16)y_pos;

  if (screen_y > 240 || screen_y < -16) {
    oambuffer[sprnum].oamy = 240;
    oambuffer[sprnum + 1].oamy = 240;
    oambuffer[sprnum + 2].oamy = 240;
  } else {
    if (p2_invuln_timer > 0 && (p2_invuln_timer & 4)) {
      oambuffer[sprnum].oamy = 240;
      oambuffer[sprnum + 1].oamy = 240;
      oambuffer[sprnum + 2].oamy = 240;
      oamFix16Draw(sprnum);
      oamFix16Draw(sprnum + 1);
      oamFix16Draw(sprnum + 2);
    } else {
      // Draw top
      oambuffer[sprnum].oamx = screen_x;
      oambuffer[sprnum].oamy = screen_y;
      oamFix16Draw(sprnum);

      // Draw left wing
      oambuffer[sprnum + 1].oamx = screen_x - 8;
      oambuffer[sprnum + 1].oamy = screen_y + 16;
      oamFix16Draw(sprnum + 1);

      // Draw right wing
      oambuffer[sprnum + 2].oamx = screen_x + 8;
      oambuffer[sprnum + 2].oamy = screen_y + 16;
      oamFix16Draw(sprnum + 2);
    }
  }
}

// ==========================================
// ENEMY
// ==========================================
void enemy_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx) {
  if (objNew(type, xp, yp) == 0)
    return;
  objGetPointer(objgetid);
  snesobj = &objbuffers[objptr - 1];
  snesobj->width = 16;
  snesobj->height = 16;
  snesobj->sprnum = nbobjects;

  // Find free bit for collision
  u8 e;
  for (e = 0; e < 16; e++) {
    if (!(en_active_mask & BIT_MASK[e])) {
      en_active_mask |= BIT_MASK[e];
      snesobj->xmin = e;
      break;
    }
  }

  // Sprite 0: top
  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp - 16;
  oambuffer[nbobjects].oamframeid = 49;
  oambuffer[nbobjects].oamattribute = 0x30 | (3 << 1); // Priority 3, Palette 3
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Sprite 1: bottom
  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp;
  oambuffer[nbobjects].oamframeid = 57;
  oambuffer[nbobjects].oamattribute = 0x30 | (3 << 1); // Priority 3, Palette 3
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  enemy_obj_indices[e] = objptr - 1;

  snesobj->xvel = 0;
  snesobj->yvel = 0;
}

void enemy_update(u8 idx) {
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
  sprnum = snesobj->sprnum;

  u8 e = snesobj->xmin;
  u16 bit_e = BIT_MASK[e];

  if (en_active_mask & bit_e) {
    s16 px = *ox + snesobj->xvel;
    s16 py = *oy + snesobj->yvel;

    if (game_mode == MODE_BENCHMARK) {
      if (px <= 0 && snesobj->xvel < 0)
        snesobj->xvel = 1;
      else if (px >= 224 && snesobj->xvel > 0)
        snesobj->xvel = -1;

      // Bounce relative to camera view
      if (py <= (s16)y_pos && snesobj->yvel < 0)
        snesobj->yvel = 1;
      else if (py >= (s16)y_pos + 208 && snesobj->yvel > 0)
        snesobj->yvel = -2;
    } else {
      // Normal mode: just move, and destroy if off bottom
      if (py > (s16)y_pos + 240) {
        en_active_mask &= ~bit_e;
        oambuffer[sprnum].oamy = 240;
        oambuffer[sprnum + 1].oamy = 240;
        return;
      }
    }

    *ox = px;
    *oy = py;

    en_x[e] = px;
    en_y[e] = py;

    s16 screen_x = px;
    s16 screen_y = py - (s16)y_pos;

    if (screen_y > 240 || screen_y < -32) {
      oambuffer[sprnum].oamy = 240;
      oambuffer[sprnum + 1].oamy = 240;
    } else {
      // Flash effect: use palette 0 when hit
      if (en_flash[e] > 0) {
        en_flash[e]--;
        oambuffer[sprnum].oamattribute = 0x30 | (0 << 1);
        oambuffer[sprnum + 1].oamattribute = 0x30 | (0 << 1);
      } else {
        oambuffer[sprnum].oamattribute = 0x30 | (3 << 1);
        oambuffer[sprnum + 1].oamattribute = 0x30 | (3 << 1);
      }

      oambuffer[sprnum].oamx = screen_x;
      oambuffer[sprnum].oamy = screen_y - 16;
      oambuffer[sprnum + 1].oamx = screen_x;
      oambuffer[sprnum + 1].oamy = screen_y;
      oamFix16Draw(sprnum);
      oamFix16Draw(sprnum + 1);

      // Player-enemy collision (not in benchmark)
      if (game_mode != MODE_BENCHMARK) {
          if (p1_obj_idx != 255 && p1_death_timer == 0 && p1_invuln_timer == 0) {
          s16 p1x = *((s16 *)&(objbuffers[p1_obj_idx].xpos[1]));
          s16 p1y = *((s16 *)&(objbuffers[p1_obj_idx].ypos[1]));
          if ((u16)(p1y - py + 16) < 32 && (u16)(p1x - px + 12) < 24) {
            p1_death_timer = 60;
            spawn_explosion(p1x, p1y);
          }
        }
          if (p2_obj_idx != 255 && p2_death_timer == 0 && p2_invuln_timer == 0) {
          s16 p2x = *((s16 *)&(objbuffers[p2_obj_idx].xpos[1]));
          s16 p2y = *((s16 *)&(objbuffers[p2_obj_idx].ypos[1]));
          if ((u16)(p2y - py + 16) < 32 && (u16)(p2x - px + 12) < 24) {
            p2_death_timer = 60;
            spawn_explosion(p2x, p2y);
          }
        }
      }
    }
  } else {
    if (game_mode == MODE_BENCHMARK) {
      // Respawn logic
      en_active_mask |= bit_e;
      *ox = rand() % 220;
      *oy = (s16)y_pos - 32;
      snesobj->xvel = (rand() % 2 == 0) ? 1 : -1;
      snesobj->yvel = 1;
    }

    // Hide it exactly once
    if (oambuffer[sprnum].oamy != 240) {
      oambuffer[sprnum].oamy = 240;
      oambuffer[sprnum + 1].oamy = 240;
      oamFix16Draw(sprnum);
      oamFix16Draw(sprnum + 1);
    }
  }
}

// ==========================================
// ITEM
// ==========================================
void item_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx) {
  if (objNew(type, xp, yp) == 0)
    return;
  objGetPointer(objgetid);
  snesobj = &objbuffers[objptr - 1];
  snesobj->width = 16;
  snesobj->height = 16;
  snesobj->sprnum = nbobjects;

  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp;
  oambuffer[nbobjects].oamframeid = 38;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Find free slot
  {
    u8 i;
    for (i = 0; i < 4; i++) {
      if (!(item_active_mask & (1 << i))) {
        item_active_mask |= (1 << i);
        snesobj->xmin = i;
        item_obj_indices[i] = objptr - 1;
        break;
      }
    }
  }

  snesobj->xvel = 0;
  snesobj->yvel = 1;
}

void item_update(u8 idx) {
  u8 i;
  snesobj = &objbuffers[idx];
  i = snesobj->xmin;

  if (!(item_active_mask & (1 << i))) {
    if (oambuffer[snesobj->sprnum].oamy != 240) {
      oambuffer[snesobj->sprnum].oamy = 240;
      oamFix16Draw(snesobj->sprnum);
    }
    return;
  }

  s16 *ox = (s16 *)&(snesobj->xpos[1]);
  s16 *oy = (s16 *)&(snesobj->ypos[1]);
  sprnum = snesobj->sprnum;

  s16 px = *ox + snesobj->xvel;
  s16 py = *oy + snesobj->yvel;

  if (px <= 0 && snesobj->xvel < 0)
    snesobj->xvel = 2;
  else if (px >= 232 && snesobj->xvel > 0)
    snesobj->xvel = -2;

  if (py <= (s16)y_pos && snesobj->yvel < 0)
    snesobj->yvel = 2;
  else if (py >= (s16)y_pos + 208 && snesobj->yvel > 0)
    snesobj->yvel = -3;

  *ox = px;
  *oy = py;

  // Loopless box collision check with players
  if (p1_obj_idx != 255 && objbuffers[p1_obj_idx].type == TYPE_PLAYER1) {
    s16 p1x = *((s16 *)&(objbuffers[p1_obj_idx].xpos[1]));
    s16 p1y = *((s16 *)&(objbuffers[p1_obj_idx].ypos[1]));
    if ((u16)(p1y - py + 12) < 24) {
      if ((u16)(p1x - px + 12) < 24) {
        addScore(1, 10);
        upd_p1_score = 1;
        u16 frame = oambuffer[sprnum].oamframeid;
        if (frame == 38) {
          if (p1_weapon_level < 3) p1_weapon_level++;
        } else if (frame == 46) {
          if (p1_lives < 9) p1_lives++;
          upd_p1_lives = 1;
        } else if (frame == 37) {
          if (p1_bombs < 9) p1_bombs++;
          upd_p1_bombs = 1;
        } else if (frame == 39) {
          p1_weapon_level = 3;
        }
        oambuffer[sprnum].oamy = 240;
        oamFix16Draw(sprnum);
        item_active_mask &= ~(1 << i); // Free slot
        return;
      }
    }
  }

  if (p2_obj_idx != 255 && objbuffers[p2_obj_idx].type == TYPE_PLAYER2) {
    s16 p2x = *((s16 *)&(objbuffers[p2_obj_idx].xpos[1]));
    s16 p2y = *((s16 *)&(objbuffers[p2_obj_idx].ypos[1]));
    if ((u16)(p2y - py + 12) < 24) {
      if ((u16)(p2x - px + 12) < 24) {
        addScore(2, 10);
        upd_p2_score = 1;
        u16 frame = oambuffer[sprnum].oamframeid;
        if (frame == 38) {
          if (p2_weapon_level < 3) p2_weapon_level++;
        } else if (frame == 46) {
          if (p2_lives < 9) p2_lives++;
          upd_p2_lives = 1;
        } else if (frame == 37) {
          if (p2_bombs < 9) p2_bombs++;
          upd_p2_bombs = 1;
        } else if (frame == 39) {
          p2_weapon_level = 3;
        }
        oambuffer[sprnum].oamy = 240;
        oamFix16Draw(sprnum);
        item_active_mask &= ~(1 << i); // Free slot
        return;
      }
    }
  }

  s16 screen_x = px;
  s16 screen_y = py - (s16)y_pos;

  if (screen_y > 240 || screen_y < -16) {
    oambuffer[sprnum].oamy = 240;
  } else {
    oambuffer[sprnum].oamx = screen_x;
    oambuffer[sprnum].oamy = screen_y;
    oamFix16Draw(sprnum);
  }
}

// ==========================================
// EXPLOSION ANIMATION
// ==========================================
void spawn_explosion(s16 x, s16 y) {
  explosion_timer = 24;
  explosion_x = x;
  explosion_y = y;
}

void explosion_update(void) {
  u16 s = explosion_spr_base;
  u8 i;

  if (explosion_timer == 0) {
    // Hide all 9 explosion sprites
    for (i = 0; i < 9; i++) {
      oambuffer[s + i].oamy = 240;
      oamFix16Draw(s + i);
    }
    return;
  }
  explosion_timer--;

  s16 sx = explosion_x;
  s16 sy = explosion_y - (s16)y_pos;

  // Hide all first
  for (i = 0; i < 9; i++) {
    oambuffer[s + i].oamy = 240;
  }

  if (sy < -16 || sy > 240) {
    for (i = 0; i < 9; i++) oamFix16Draw(s + i);
    return;
  }

  if (explosion_timer >= 16) {
    // Frame 1: 1
    oambuffer[s].oamx = sx - 8;
    oambuffer[s].oamy = sy - 8;
    oamFix16Draw(s);
    for (i = 1; i < 9; i++) oamFix16Draw(s + i);
  } else if (explosion_timer >= 8) {
    // Frame 2: 4 sprites (s+1 to s+4)
    oambuffer[s + 1].oamx = sx - 8;
    oambuffer[s + 1].oamy = sy - 16;
    oambuffer[s + 2].oamx = sx - 8;
    oambuffer[s + 2].oamy = sy;
    oambuffer[s + 3].oamx = sx + 8;
    oambuffer[s + 3].oamy = sy - 16;
    oambuffer[s + 4].oamx = sx + 8;
    oambuffer[s + 4].oamy = sy;
    for (i = 0; i < 9; i++) oamFix16Draw(s + i);
  } else {
    // Frame 3: 4 sprites (s+5 to s+8)
    oambuffer[s + 5].oamx = sx - 8;
    oambuffer[s + 5].oamy = sy - 16;
    oambuffer[s + 6].oamx = sx + 8;
    oambuffer[s + 6].oamy = sy - 16;
    oambuffer[s + 7].oamx = sx - 8;
    oambuffer[s + 7].oamy = sy;
    oambuffer[s + 8].oamx = sx + 8;
    oambuffer[s + 8].oamy = sy;
    for (i = 0; i < 9; i++) oamFix16Draw(s + i);
  }
}

// ==========================================
// ITEM DROP FROM ENEMIES
// ==========================================
void try_spawn_item(s16 ex, s16 ey) {
  if (game_mode == MODE_BENCHMARK) return;

  if ((rand() & 11) != 0) return;
  u8 e;
  for (e = 0; e < 4; e++) {
    if (!(item_active_mask & (1 << e))) {
      item_active_mask |= (1 << e);
      u8 idx = item_obj_indices[e];
      *((s16 *)&(objbuffers[idx].xpos[1])) = ex;
      *((s16 *)&(objbuffers[idx].ypos[1])) = ey;
      objbuffers[idx].xvel = (rand() & 1) ? 2 : -2;
      objbuffers[idx].yvel = 1;

      u16 sprnum = objbuffers[idx].sprnum;
      u16 frame = 38;
      u8 rand_val = rand() & 15; // 1 in 16 chance for each rare item
      if (rand_val == 0) frame = 46; // life
      else if (rand_val == 1) frame = 37; // bomb
      else if (rand_val == 2) frame = 39; // full bullet

      oambuffer[sprnum].oamframeid = frame;
      return;
    }
  }
}

// ==========================================
// BULLETS
// ==========================================
void bullets_update(void) {
  u8 i;
  u16 mask, bit_e;
  s16 *pey, *pex;

  // Player 1 Bullets
  for (i = 0; i < 6; i++) {
    u8 st = p1_bullets[i].active;
    u16 snum = p1_bullets[i].sprnum;
    if (!st) {
      if (oambuffer[snum].oamy != 240 || oambuffer[snum + 1].oamy != 240 ||
          oambuffer[snum + 2].oamy != 240) {
        oambuffer[snum].oamy = 240;
        oambuffer[snum + 1].oamy = 240;
        oambuffer[snum + 2].oamy = 240;
        oamFix16Draw(snum);
        oamFix16Draw(snum + 1);
        oamFix16Draw(snum + 2);
      }
      continue;
    }

    s16 px = p1_bullets[i].x;
    s16 py = p1_bullets[i].y - 16;
    p1_bullets[i].y = py;

    // Destroy if off-screen (above camera)
    if (py < (s16)y_pos - 16) {
      p1_bullets[i].active = 0;
      continue;
    }

    // Collision math
    if ((snes_vblank_count & 1) == 0) {
      u8 j;

      for (j = 0; j < 16; j++) {
        s16 ey = en_y[j];
        if (ey <= (s16)y_pos - 16 || ey >= (s16)y_pos + 240)
          continue;

        if ((u16)(py - ey + 16) < 48) {
          s16 ex = en_x[j];
          s16 dx = px - ex;
          if ((u16)(dx + 32) < 64) {
            if ((st & 1) && (u16)(dx + 10) < 48) {
              st &= ~1;
              if (game_mode != MODE_BENCHMARK && en_hp[j] > 1) {
                en_hp[j]--;
                en_flash[j] = 8;
              } else {
                en_hp[j] = 0;
                en_active_mask &= ~BIT_MASK[j];
                try_spawn_item(ex, ey);
                spawn_explosion(ex, ey);
                en_y[j] = y_pos - 32;
                oambuffer[nbobjects].oamy = 240;
              }
              frame_score1++;
            }
            if ((st & 2) && (u16)(dx + 24) < 48) {
              st &= ~2;
              if (game_mode != MODE_BENCHMARK && en_hp[j] > 1) {
                en_hp[j]--;
                en_flash[j] = 8;
              } else {
                en_hp[j] = 0;
                en_active_mask &= ~BIT_MASK[j];
                try_spawn_item(ex, ey);
                spawn_explosion(ex, ey);
                en_y[j] = y_pos - 32;
              }
              frame_score1++;
            }
            if ((st & 4) && (u16)(dx + 40) < 48) {
              st &= ~4;
              if (game_mode != MODE_BENCHMARK && en_hp[j] > 1) {
                en_hp[j]--;
                en_flash[j] = 8;
              } else {
                en_hp[j] = 0;
                en_active_mask &= ~BIT_MASK[j];
                try_spawn_item(ex, ey);
                spawn_explosion(ex, ey);
                en_y[j] = y_pos - 32;
              }
              frame_score1++;
            }
            if (!st) {
              p1_bullets[i].active = 0;
              break;
            }
          }
        }
      }
    }

    p1_bullets[i].active = st;

    s16 screen_x = px;
    s16 screen_y = py - (s16)y_pos;

    oambuffer[snum].oamx = screen_x - 14;
    oambuffer[snum].oamy = (st & 1) ? screen_y : 240;
    oamFix16Draw(snum);

    oambuffer[snum + 1].oamx = screen_x;
    oambuffer[snum + 1].oamy = (st & 2) ? screen_y : 240;
    oamFix16Draw(snum + 1);

    oambuffer[snum + 2].oamx = screen_x + 16;
    oambuffer[snum + 2].oamy = (st & 4) ? screen_y : 240;
    oamFix16Draw(snum + 2);
  }

  // Player 2 Bullets
  for (i = 0; i < 6; i++) {
    u8 st = p2_bullets[i].active;
    u16 snum = p2_bullets[i].sprnum;
    if (!st) {
      if (oambuffer[snum].oamy != 240 || oambuffer[snum + 1].oamy != 240 ||
          oambuffer[snum + 2].oamy != 240) {
        oambuffer[snum].oamy = 240;
        oambuffer[snum + 1].oamy = 240;
        oambuffer[snum + 2].oamy = 240;
        oamFix16Draw(snum);
        oamFix16Draw(snum + 1);
        oamFix16Draw(snum + 2);
      }
      continue;
    }

    s16 px = p2_bullets[i].x;
    s16 py = p2_bullets[i].y - 16;
    p2_bullets[i].y = py;

    if (py < (s16)y_pos - 16) {
      p2_bullets[i].active = 0;
      continue;
    }

    // Collision math
    if ((snes_vblank_count & 1) == 0) {
      u8 j;
      for (j = 0; j < 16; j++) {
        s16 ey = en_y[j];
        if (ey <= (s16)y_pos - 16 || ey >= (s16)y_pos + 240)
          continue;

        if ((u16)(py - ey + 16) < 48) {
          s16 ex = en_x[j];
          s16 dx = px - ex;
          if ((u16)(dx + 32) < 64) {
            if ((st & 1) && (u16)(dx + 8) < 48) {
              st &= ~1;
              if (game_mode != MODE_BENCHMARK && en_hp[j] > 1) {
                en_hp[j]--;
                en_flash[j] = 8;
              } else {
                en_hp[j] = 0;
                en_active_mask &= ~BIT_MASK[j];
                try_spawn_item(ex, ey);
                spawn_explosion(ex, ey);
                en_y[j] = y_pos - 32;
              }
              frame_score2++;
            }
            if ((st & 2) && (u16)(dx + 24) < 48) {
              st &= ~2;
              if (game_mode != MODE_BENCHMARK && en_hp[j] > 1) {
                en_hp[j]--;
                en_flash[j] = 8;
              } else {
                en_hp[j] = 0;
                en_active_mask &= ~BIT_MASK[j];
                try_spawn_item(ex, ey);
                spawn_explosion(ex, ey);
                en_y[j] = y_pos - 32;
              }
              frame_score2++;
            }
            if ((st & 4) && (u16)(dx + 40) < 48) {
              st &= ~4;
              if (game_mode != MODE_BENCHMARK && en_hp[j] > 1) {
                en_hp[j]--;
                en_flash[j] = 8;
              } else {
                en_hp[j] = 0;
                en_active_mask &= ~BIT_MASK[j];
                try_spawn_item(ex, ey);
                spawn_explosion(ex, ey);
                en_y[j] = y_pos - 32;
              }
              frame_score2++;
            }
            if (!st) {
              p2_bullets[i].active = 0;
              break;
            }
          }
        }
      }
    }

    p2_bullets[i].active = st;

    s16 screen_x = px;
    s16 screen_y = py - (s16)y_pos;

    oambuffer[snum].oamx = screen_x - 16;
    oambuffer[snum].oamy = (st & 1) ? screen_y : 240;
    oamFix16Draw(snum);

    oambuffer[snum + 1].oamx = screen_x;
    oambuffer[snum + 1].oamy = (st & 2) ? screen_y : 240;
    oamFix16Draw(snum + 1);

    oambuffer[snum + 2].oamx = screen_x + 16;
    oambuffer[snum + 2].oamy = (st & 4) ? screen_y : 240;
    oamFix16Draw(snum + 2);
  }
}

void bullet_p1_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx) {
  static u8 p1_b_idx = 0;
  if (p1_b_idx >= 6)
    return;

  p1_bullets[p1_b_idx].active = 0;
  p1_bullets[p1_b_idx].sprnum = nbobjects;

  // Bullet 1 (Left)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 48;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Bullet 2 (Center)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 48;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Bullet 3 (Right)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 48;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  p1_b_idx++;
}

void bullet_p2_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx) {
  static u8 p2_b_idx = 0;
  if (p2_b_idx >= 6)
    return;

  p2_bullets[p2_b_idx].active = 0;
  p2_bullets[p2_b_idx].sprnum = nbobjects;

  // Bullet 1 (Left)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 48;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Bullet 2 (Center)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 48;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Bullet 3 (Right)
  oambuffer[nbobjects].oamx = 0;
  oambuffer[nbobjects].oamy = 240;
  oambuffer[nbobjects].oamframeid = 48;
  oambuffer[nbobjects].oamattribute = 0x30 | (0 << 1); // Priority 3, Palette 0
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  p2_b_idx++;
}

void spawn_entities(void) {
  if (game_mode == MODE_BENCHMARK)
    return;

  while (tabobjects[current_spawn_idx * 5] != 0xFFFF) {
    s16 y = (s16)tabobjects[current_spawn_idx * 5 + 1];
    if (y < (s16)y_pos)
      break;

    u16 type = tabobjects[current_spawn_idx * 5 + 2];
    s16 x = (s16)tabobjects[current_spawn_idx * 5];
    if (type == TYPE_ENEMY) {
      u8 e;
      u8 spawned = 0;
      for (e = 0; e < 16; e++) {
        if (!(en_active_mask & BIT_MASK[e])) {
          en_active_mask |= BIT_MASK[e];
          u8 idx = enemy_obj_indices[e];
          *((s16 *)&(objbuffers[idx].xpos[1])) = x;
          *((s16 *)&(objbuffers[idx].ypos[1])) = (s16)y_pos - 16;
          objbuffers[idx].xvel = 0;
          objbuffers[idx].yvel = 3;
          en_hp[e] = 3;
          en_flash[e] = 0;
          spawned = 1;
          break;
        }
      }
      if (!spawned)
        break;
    }

    current_spawn_idx++;
  }
}
