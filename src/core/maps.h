#ifndef MAPS_H
#define MAPS_H

#include <snes.h>

#define BG3_VRAM_BASE 0x7800
#define UI_ADDR(x, y) (BG3_VRAM_BASE + ((y) * 32) + (x))

#define P1_SCORE_X 7
#define P1_SCORE_Y 6
#define P2_SCORE_X 24
#define P2_SCORE_Y 6
#define P1_LIVES_X 2
#define P1_LIVES_Y 8
#define P2_LIVES_X 27
#define P2_LIVES_Y 8
#define P1_BOMBS_X 2
#define P1_BOMBS_Y 0
#define P2_BOMBS_X 28
#define P2_BOMBS_Y 0

#define TILE_BLANK 0
#define TILE_TOP_0 1
#define TILE_BOT_0 14
#define TILE_TOP_LIVES 13
#define TILE_BOT_LIVES 26
#define TILE_TOP_BOMB 12
#define TILE_BOT_BOMB 25

#define PAL_SCORE 0
#define PAL_P1_LIVES 1
#define PAL_P2_LIVES 2
#define PAL_P1_BOMBS 3
#define PAL_P2_BOMBS 3

#define TILE_ATTR(pal, tile) (BG_TIL_PRIO | BG_TIL_PAL(pal) | BG_TIL_NUM(tile))

#define MAX_ICONS 5

// UI variables
extern u8 score1[6];
extern u8 score2[6];
extern u8 upd_p1_score, upd_p2_score;
extern u8 upd_p1_lives, upd_p2_lives;
extern u8 upd_p1_bombs, upd_p2_bombs;

extern u16 buf_p1_score_top[6], buf_p1_score_bot[6];
extern u16 buf_p2_score_top[6], buf_p2_score_bot[6];
extern u16 buf_p1_lives_top[MAX_ICONS], buf_p1_lives_bot[MAX_ICONS];
extern u16 buf_p2_lives_top[MAX_ICONS], buf_p2_lives_bot[MAX_ICONS];
extern u16 buf_p1_bombs_top[MAX_ICONS], buf_p1_bombs_bot[MAX_ICONS];
extern u16 buf_p2_bombs_top[MAX_ICONS], buf_p2_bombs_bot[MAX_ICONS];

// Functions
void addScore(u8 player, u8 pts);
void updateScoreBuffers(void);
void updateIconBuffers(void);

#endif
