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

u8 frame_score1 = 0;
u8 frame_score2 = 0;

u8 p1_obj_idx = 255;
u8 p2_obj_idx = 255;

// Enemy bitmask cache
u16 en_active_mask = 0;
s16 en_x[16];
s16 en_y[16];

t_objs *snesobj;
u16 sprnum;

typedef struct
{
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
    // Players (2)
    64, 1764, TYPE_PLAYER1, 0, 0, 192, 1764, TYPE_PLAYER2, 0, 0,

    // Enemies (16)
    30, 200, TYPE_ENEMY, 0, 0, 100, 300, TYPE_ENEMY, 0, 0, 170, 400, TYPE_ENEMY,
    0, 0, 210, 500, TYPE_ENEMY, 0, 0, 50, 600, TYPE_ENEMY, 0, 0, 150, 700,
    TYPE_ENEMY, 0, 0, 80, 800, TYPE_ENEMY, 0, 0, 190, 900, TYPE_ENEMY, 0, 0, 20,
    1000, TYPE_ENEMY, 0, 0, 120, 1100, TYPE_ENEMY, 0, 0, 200, 1200, TYPE_ENEMY,
    0, 0, 40, 1300, TYPE_ENEMY, 0, 0, 160, 1400, TYPE_ENEMY, 0, 0, 90, 1500,
    TYPE_ENEMY, 0, 0, 130, 1550, TYPE_ENEMY, 0, 0, 180, 1600, TYPE_ENEMY, 0, 0,

    // Items (4)
    40, 400, TYPE_ITEM, 0, 0, 72, 800, TYPE_ITEM, 0, 0, 104, 1200, TYPE_ITEM, 0,
    0, 136, 1600, TYPE_ITEM, 0, 0,

    0xFFFF};

// ==========================================
// INITIALIZATION
// ==========================================
void initEntities()
{
  objInitEngine();

  objInitFunctions(TYPE_PLAYER1, &p1_init, &p1_update, NULL);
  objInitFunctions(TYPE_PLAYER2, &p2_init, &p2_update, NULL);
  objInitFunctions(TYPE_ENEMY, &enemy_init, &enemy_update, NULL);
  objInitFunctions(TYPE_ITEM, &item_init, &item_update, NULL);

  // Load objects from table
  objLoadObjects((char *)&tabobjects);

  // Pre-allocate bullet objects
  u8 i;
  for (i = 0; i < 6; i++)
  {
    bullet_p1_init(0, 100, TYPE_BULLET_P1, 0, 0);
    bullet_p2_init(0, 100, TYPE_BULLET_P2, 0, 0);
  }
}

// ==========================================
// PLAYER 1 (3 sprites: body, left wing, right wing)
// ==========================================
void p1_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
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

void p1_update(u8 idx)
{
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
  sprnum = snesobj->sprnum;

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

  if ((pad1 & KEY_X) && p1_bombs > 0 && p1_cooldown == 0)
  {
    p1_bombs--;
    upd_p1_bombs = 1;
    p1_cooldown = 30;
  }

  if (p1_cooldown > 0)
    p1_cooldown--;

  if ((pad1 & (KEY_A | KEY_B | KEY_Y)) && p1_cooldown == 0)
  {
    u8 i;
    for (i = 0; i < 6; i++)
    {
      if (p1_bullets[i].active == 0)
      {
        p1_bullets[i].active = 7;
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

  if (screen_y > 240 || screen_y < -16)
  {
    oambuffer[sprnum].oamy = 240;
    oambuffer[sprnum + 1].oamy = 240;
    oambuffer[sprnum + 2].oamy = 240;
  }
  else
  {
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

// ==========================================
// PLAYER 2 (3 sprites: body, left wing, right wing)
// ==========================================
void p2_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
  if (game_mode != MODE_2P && game_mode != MODE_PERFORMANCE)
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

void p2_update(u8 idx)
{
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
  sprnum = snesobj->sprnum;

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

  if ((pad2 & KEY_X) && p2_bombs > 0 && p2_cooldown == 0)
  {
    p2_bombs--;
    upd_p2_bombs = 1;
    p2_cooldown = 30;
  }

  if (p2_cooldown > 0)
    p2_cooldown--;

  if ((pad2 & (KEY_A | KEY_B | KEY_Y)) && p2_cooldown == 0)
  {
    u8 i;
    for (i = 0; i < 6; i++)
    {
      if (p2_bullets[i].active == 0)
      {
        p2_bullets[i].active = 7;
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

  if (screen_y > 240 || screen_y < -16)
  {
    oambuffer[sprnum].oamy = 240;
    oambuffer[sprnum + 1].oamy = 240;
    oambuffer[sprnum + 2].oamy = 240;
  }
  else
  {
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

// ==========================================
// ENEMY
// ==========================================
void enemy_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
  if (objNew(type, xp, yp) == 0)
    return;
  objGetPointer(objgetid);
  snesobj = &objbuffers[objptr - 1];
  snesobj->width = 16;
  snesobj->height = 16;
  snesobj->sprnum = nbobjects;

  // Find free bit for collision
  u8 e;
  for (e = 0; e < 16; e++)
  {
    if (!(en_active_mask & BIT_MASK[e]))
    {
      en_active_mask |= BIT_MASK[e];
      snesobj->xmin = e;
      break;
    }
  }

  // Sprite 0: top
  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp;
  oambuffer[nbobjects].oamframeid = 49;
  oambuffer[nbobjects].oamattribute = 0x30 | (3 << 1); // Priority 3, Palette 3
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  // Sprite 1: bottom
  oambuffer[nbobjects].oamx = xp;
  oambuffer[nbobjects].oamy = yp + 16;
  oambuffer[nbobjects].oamframeid = 57;
  oambuffer[nbobjects].oamattribute = 0x30 | (3 << 1); // Priority 3, Palette 3
  oambuffer[nbobjects].oamrefresh = 1;
  nbobjects++;

  snesobj->xvel = (rand() & 1) ? 1 : -1;
  snesobj->yvel = (rand() & 1) ? 1 : -1;
}

void enemy_update(u8 idx)
{
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
  sprnum = snesobj->sprnum;

  u8 e = snesobj->xmin;
  u16 bit_e = BIT_MASK[e];

  if (en_active_mask & bit_e)
  {
    s16 px = *ox + snesobj->xvel;
    s16 py = *oy + snesobj->yvel;

    if (px <= 0 && snesobj->xvel < 0)
      snesobj->xvel = 1;
    else if (px >= 224 && snesobj->xvel > 0)
      snesobj->xvel = -1;

    // Bounce relative to camera view
    if (py <= (s16)y_pos && snesobj->yvel < 0)
      snesobj->yvel = 1;
    else if (py >= (s16)y_pos + 208 && snesobj->yvel > 0)
      snesobj->yvel = -2;

    *ox = px;
    *oy = py;

    en_x[e] = px;
    en_y[e] = py;

    s16 screen_x = px;
    s16 screen_y = py - (s16)y_pos;

    if (screen_y > 240 || screen_y < -32)
    {
      oambuffer[sprnum].oamy = 240;
      oambuffer[sprnum + 1].oamy = 240;
    }
    else
    {
      oambuffer[sprnum].oamx = screen_x;
      oambuffer[sprnum].oamy = screen_y;
      oambuffer[sprnum + 1].oamx = screen_x;
      oambuffer[sprnum + 1].oamy = screen_y + 16;
      oamFix16Draw(sprnum);
      oamFix16Draw(sprnum + 1);
    }
  }
  else
  {
    if (game_mode == MODE_PERFORMANCE)
    {
      // Respawn logic - spawn just above the current screen
      en_active_mask |= bit_e;
      *ox = rand() % 220;
      *oy = (s16)y_pos - 32;
      snesobj->xvel = (rand() % 2 == 0) ? 1 : -1;
      snesobj->yvel = 1;
    }

    // Hide it exactly once
    if (oambuffer[sprnum].oamy != 240)
    {
      oambuffer[sprnum].oamy = 240;
      oambuffer[sprnum + 1].oamy = 240;
      oamFix16Draw(sprnum);
      oamFix16Draw(sprnum + 1);
    }
    return;
  }
}

// ==========================================
// ITEM
// ==========================================
void item_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
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

  snesobj->xvel = (objptr & 1) ? 2 : -2;
  snesobj->yvel = (objptr & 2) ? 2 : -2;
}

void item_update(u8 idx)
{
  snesobj = &objbuffers[idx];
  s16 *ox = (s16 *)&(snesobj->xpos + 1);
  s16 *oy = (s16 *)&(snesobj->ypos + 1);
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
  if (p1_obj_idx != 255 && objbuffers[p1_obj_idx].type == TYPE_PLAYER1)
  {
    s16 p1x = *((s16 *)&(objbuffers[p1_obj_idx].xpos + 1));
    s16 p1y = *((s16 *)&(objbuffers[p1_obj_idx].ypos + 1));
    if ((u16)(p1y - py + 12) < 24)
    {
      if ((u16)(p1x - px + 12) < 24)
      {
        addScore(1, 10);
        upd_p1_score = 1;
        oambuffer[sprnum].oamy = 240;
        oamFix16Draw(sprnum);
        objtokill = 1; // Destroy item
        return;
      }
    }
  }

  if (p2_obj_idx != 255 && objbuffers[p2_obj_idx].type == TYPE_PLAYER2)
  {
    s16 p2x = *((s16 *)&(objbuffers[p2_obj_idx].xpos + 1));
    s16 p2y = *((s16 *)&(objbuffers[p2_obj_idx].ypos + 1));
    if ((u16)(p2y - py + 12) < 24)
    {
      if ((u16)(p2x - px + 12) < 24)
      {
        addScore(2, 10);
        upd_p2_score = 1;
        oambuffer[sprnum].oamy = 240;
        oamFix16Draw(sprnum);
        objtokill = 1; // Destroy item
        return;
      }
    }
  }

  s16 screen_x = px;
  s16 screen_y = py - (s16)y_pos;

  if (screen_y > 240 || screen_y < -16)
  {
    oambuffer[sprnum].oamy = 240;
  }
  else
  {
    oambuffer[sprnum].oamx = screen_x;
    oambuffer[sprnum].oamy = screen_y;
    oamFix16Draw(sprnum);
  }
}

// ==========================================
// BULLETS
// ==========================================
void bullets_update(void)
{
  u8 i;
  u16 mask, bit_e;
  s16 *pey, *pex;

  // Player 1 Bullets
  for (i = 0; i < 6; i++)
  {
    u8 st = p1_bullets[i].active;
    u16 snum = p1_bullets[i].sprnum;
    if (!st)
    {
      if (oambuffer[snum].oamy != 240 || oambuffer[snum + 1].oamy != 240 || oambuffer[snum + 2].oamy != 240)
      {
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
    if (py < (s16)y_pos - 16)
    {
      p1_bullets[i].active = 0;
      continue;
    }

    // Collision math
    if ((snes_vblank_count & 3) == 0)
    {
      u8 j;

      for (j = 0; j < 16; j++)
      {
        s16 ey = en_y[j];
        if (ey <= (s16)y_pos - 16 || ey >= (s16)y_pos + 240)
          continue;

        if ((u16)(py - ey + 16) < 48)
        {
          s16 ex = en_x[j];
          s16 dx = px - ex;
          if ((u16)(dx + 32) < 64)
          {
            if ((st & 1) && (u16)(dx + 10) < 48)
            {
              st &= ~1;
              en_active_mask &= ~BIT_MASK[j];
              en_y[j] = y_pos - 32;
              oambuffer[nbobjects].oamy = 240;
              frame_score1++;
            }
            if ((st & 2) && (u16)(dx + 24) < 48)
            {
              st &= ~2;
              en_active_mask &= ~BIT_MASK[j];
              en_y[j] = y_pos - 32;
              frame_score1++;
            }
            if ((st & 4) && (u16)(dx + 40) < 48)
            {
              st &= ~4;
              en_active_mask &= ~BIT_MASK[j];
              en_y[j] = y_pos - 32;
              frame_score1++;
            }
            if (!st)
            {
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
  for (i = 0; i < 6; i++)
  {
    u8 st = p2_bullets[i].active;
    u16 snum = p2_bullets[i].sprnum;
    if (!st)
    {
      if (oambuffer[snum].oamy != 240 || oambuffer[snum + 1].oamy != 240 || oambuffer[snum + 2].oamy != 240)
      {
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

    if (py < (s16)y_pos - 16)
    {
      p2_bullets[i].active = 0;
      continue;
    }

    // Collision math
    if ((snes_vblank_count & 3) == 0)
    {
      u8 j;
      for (j = 0; j < 16; j++)
      {
        s16 ey = en_y[j];
        if (ey <= (s16)y_pos - 16 || ey >= (s16)y_pos + 240)
          continue;

        if ((u16)(py - ey + 16) < 48)
        {
          s16 ex = en_x[j];
          s16 dx = px - ex;
          if ((u16)(dx + 32) < 64)
          {
            if ((st & 1) && (u16)(dx + 8) < 48)
            {
              st &= ~1;
              en_active_mask &= ~BIT_MASK[j];
              en_y[j] = y_pos - 32;
              frame_score2++;
            }
            if ((st & 2) && (u16)(dx + 24) < 48)
            {
              st &= ~2;
              en_active_mask &= ~BIT_MASK[j];
              en_y[j] = y_pos - 32;
              frame_score2++;
            }
            if ((st & 4) && (u16)(dx + 40) < 48)
            {
              st &= ~4;
              en_active_mask &= ~BIT_MASK[j];
              en_y[j] = y_pos - 32;
              frame_score2++;
            }
            if (!st)
            {
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

void bullet_p1_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
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

void bullet_p2_init(u16 xp, u16 yp, u16 type, u16 minx, u16 maxx)
{
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
