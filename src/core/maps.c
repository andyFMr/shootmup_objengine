#include "maps.h"
#include "entities.h"
#include "system.h"
#include <snes.h>

u8 score1[6] = {0, 0, 0, 0, 0, 0};
u8 score2[6] = {0, 0, 0, 0, 0, 0};

// WRAM Arrays (Shadow Buffers)
u16 buf_p1_score_top[6], buf_p1_score_bot[6];
u16 buf_p2_score_top[6], buf_p2_score_bot[6];
u16 buf_p1_lives_top[MAX_ICONS], buf_p1_lives_bot[MAX_ICONS];
u16 buf_p2_lives_top[MAX_ICONS], buf_p2_lives_bot[MAX_ICONS];
u16 buf_p1_bombs_top[MAX_ICONS], buf_p1_bombs_bot[MAX_ICONS];
u16 buf_p2_bombs_top[MAX_ICONS], buf_p2_bombs_bot[MAX_ICONS];

u8 upd_p1_score = 1, upd_p2_score = 1;
u8 upd_p1_lives = 1, upd_p2_lives = 1;
u8 upd_p1_bombs = 1, upd_p2_bombs = 1;

void addScore(u8 player, u8 pts)
{
  if (!pts)
    return;
  u8 *score_array = (player == 1) ? score1 : score2;
  score_array[0] += pts;

  u8 i = 0;
  while (score_array[i] > 9)
  {
    score_array[i] -= 10;
    i++;
    if (i > 5)
    {
      score_array[5] = 9;
      break;
    }
    score_array[i]++;
  }
}

void updateScoreBuffers()
{
  u8 i;
  if (upd_p1_score)
  {
    for (i = 0; i < 6; i++)
    {
      buf_p1_score_top[i] = TILE_ATTR(PAL_SCORE, TILE_TOP_0 + score1[5 - i]);
      buf_p1_score_bot[i] = TILE_ATTR(PAL_SCORE, TILE_BOT_0 + score1[5 - i]);
    }
  }
  if (upd_p2_score)
  {
    for (i = 0; i < 6; i++)
    {
      buf_p2_score_top[i] = TILE_ATTR(PAL_SCORE, TILE_TOP_0 + score2[5 - i]);
      buf_p2_score_bot[i] = TILE_ATTR(PAL_SCORE, TILE_BOT_0 + score2[5 - i]);
    }
  }
}

void updateIconBuffers()
{
  u8 i;
  if (upd_p1_lives)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p1_lives) ? TILE_TOP_LIVES : TILE_BLANK;
      u16 tile_bot = (i < p1_lives) ? TILE_BOT_LIVES : TILE_BLANK;
      buf_p1_lives_top[i] = TILE_ATTR(PAL_P1_LIVES, tile_top);
      buf_p1_lives_bot[i] = TILE_ATTR(PAL_P1_LIVES, tile_bot);
    }
  }
  if (upd_p2_lives)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p2_lives) ? TILE_TOP_LIVES : TILE_BLANK;
      u16 tile_bot = (i < p2_lives) ? TILE_BOT_LIVES : TILE_BLANK;
      buf_p2_lives_top[i] = TILE_ATTR(PAL_P2_LIVES, tile_top);
      buf_p2_lives_bot[i] = TILE_ATTR(PAL_P2_LIVES, tile_bot);
    }
  }
  if (upd_p1_bombs)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p1_bombs) ? TILE_TOP_BOMB : TILE_BLANK;
      u16 tile_bot = (i < p1_bombs) ? TILE_BOT_BOMB : TILE_BLANK;
      buf_p1_bombs_top[i] = TILE_ATTR(PAL_P1_BOMBS, tile_top);
      buf_p1_bombs_bot[i] = TILE_ATTR(PAL_P1_BOMBS, tile_bot);
    }
  }
  if (upd_p2_bombs)
  {
    for (i = 0; i < MAX_ICONS; i++)
    {
      u16 tile_top = (i < p2_bombs) ? TILE_TOP_BOMB : TILE_BLANK;
      u16 tile_bot = (i < p2_bombs) ? TILE_BOT_BOMB : TILE_BLANK;
      buf_p2_bombs_top[i] = TILE_ATTR(PAL_P2_BOMBS, tile_top);
      buf_p2_bombs_bot[i] = TILE_ATTR(PAL_P2_BOMBS, tile_bot);
    }
  }
}
