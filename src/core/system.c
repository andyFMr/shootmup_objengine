#include <snes.h>
#include "system.h"
#include "maps.h"

int gFrames = 0;

const u16 BIT_MASK[16] = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000};

void myconsoleVblank()
{

  // UI Micro-DMA
  if (upd_p1_score)
  {
    dmaCopyVram((u8 *)buf_p1_score_top, UI_ADDR(P1_SCORE_X, P1_SCORE_Y), 12);
    dmaCopyVram((u8 *)buf_p1_score_bot, UI_ADDR(P1_SCORE_X, P1_SCORE_Y + 1), 12);
    upd_p1_score = 0;
  }
  if (upd_p2_score)
  {
    dmaCopyVram((u8 *)buf_p2_score_top, UI_ADDR(P2_SCORE_X, P2_SCORE_Y), 12);
    dmaCopyVram((u8 *)buf_p2_score_bot, UI_ADDR(P2_SCORE_X, P2_SCORE_Y + 1), 12);
    upd_p2_score = 0;
  }
  if (upd_p1_lives)
  {
    dmaCopyVram((u8 *)buf_p1_lives_top, UI_ADDR(P1_LIVES_X, P1_LIVES_Y), MAX_ICONS * 2);
    dmaCopyVram((u8 *)buf_p1_lives_bot, UI_ADDR(P1_LIVES_X, P1_LIVES_Y + 1), MAX_ICONS * 2);
    upd_p1_lives = 0;
  }
  if (upd_p2_lives)
  {
    dmaCopyVram((u8 *)buf_p2_lives_top, UI_ADDR(P2_LIVES_X, P2_LIVES_Y), MAX_ICONS * 2);
    dmaCopyVram((u8 *)buf_p2_lives_bot, UI_ADDR(P2_LIVES_X, P2_LIVES_Y + 1), MAX_ICONS * 2);
    upd_p2_lives = 0;
  }
  if (upd_p1_bombs)
  {
    dmaCopyVram((u8 *)buf_p1_bombs_top, UI_ADDR(P1_BOMBS_X, P1_BOMBS_Y), MAX_ICONS * 2);
    dmaCopyVram((u8 *)buf_p1_bombs_bot, UI_ADDR(P1_BOMBS_X, P1_BOMBS_Y + 1), MAX_ICONS * 2);
    upd_p1_bombs = 0;
  }
  if (upd_p2_bombs)
  {
    dmaCopyVram((u8 *)buf_p2_bombs_top, UI_ADDR(P2_BOMBS_X, P2_BOMBS_Y), MAX_ICONS * 2);
    dmaCopyVram((u8 *)buf_p2_bombs_bot, UI_ADDR(P2_BOMBS_X, P2_BOMBS_Y + 1), MAX_ICONS * 2);
    upd_p2_bombs = 0;
  }
}
