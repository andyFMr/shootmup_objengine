#ifndef SYSTEM_H
#define SYSTEM_H

#include <snes.h>

extern int gFrames;
extern const u16 BIT_MASK[16];

enum GameState {
    STATE_TITLE,
    STATE_GAME
};

enum GameMode {
    MODE_1P = 0,
    MODE_2P = 1,
    MODE_PERFORMANCE = 2
};

extern u8 game_state;
extern u8 game_mode;

void myconsoleVblank(void);

#endif
