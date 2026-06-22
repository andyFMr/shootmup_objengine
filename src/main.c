/*---------------------------------------------------------------------------------
    Shmup Demo
---------------------------------------------------------------------------------*/
#include <snes.h>

#include "../res/gfx/entities/spr_enemies1.inc"
#include "../res/gfx/entities/spr_fighters1.inc"
#include "../res/gfx/entities/spr_fighters2.inc"
#include "../res/gfx/entities/spr_fx.inc"
#include "../res/gfx/stages/stage_bg1.inc"
#include "../res/gfx/stages/stage_bg3.inc"
#include "../res/gfx/stages/tiles.inc"
#include "../res/gfx/stages/title.inc"

#include "core/entities.h"
#include "core/maps.h"
#include "core/system.h"
#include "snes/background.h"
#include "snes/input.h"
#include "snes/sprite.h"

extern char stages_bg, stages_obj, stages_att, stages_def;

u16 nbobjects;

// ==============================================================================
// ENTRY POINT
// ==============================================================================
int main(void) {
  // --- Init Title Screen ---

  bgInitTileSet(0, &title_til, &title_pal, 2, (&title_tilend - &title_til),
                (&title_palend - &title_pal), BG_16COLORS, 0x2000);
  bgInitMapSet(0, &title_map, (&title_mapend - &title_map) * 2, SC_64x32,
               0x4000);

  setMode(BG_MODE1, 0);

  bgSetDisable(1);
  bgSetDisable(2);

  oamInitGfxAttr(0x0000, OBJ_SIZE8_L16);

  dmaCopyVram((u8 *)&spr_fx_til, 0x0000, 0x800);
  dmaCopyCGram(&spr_fx_pal, 128 + 0 * 16, 16 * 2);
  oamSetEx(0, OBJ_LARGE, OBJ_SHOW);
  setPaletteColor(0, 0);
  setFadeEffect(FADE_IN);

  u8 cursor_pos = 0;
  u8 pad_delay = 0;
  while (1) {
    if (game_state == STATE_TITLE) {
      u16 pad0 = padsDown(0);

      if (pad0 & KEY_UP) {
        if (cursor_pos > 0)
          cursor_pos--;
        pad_delay = 10;
      } else if (pad0 & KEY_DOWN) {
        if (cursor_pos < 2)
          cursor_pos++;
        pad_delay = 10;
      }

      if (pad0 & KEY_A) {
        gFrames = 0;
        setFadeEffect(FADE_OUT);

        if (cursor_pos == 0)
          game_mode = MODE_1P;
        else if (cursor_pos == 1)
          game_mode = MODE_2P;
        else if (cursor_pos == 2)
          game_mode = MODE_PERFORMANCE;

        game_state = STATE_GAME;
      }

      u16 cursor_y = 136 + (cursor_pos * 16);
      oamSet(0, 80, cursor_y, 3, 0, 0, 12, 0);

      WaitForVBlank();

    } else if (game_state == STATE_GAME) {
      // --- Init Backgrounds ---
      bgInitTileSet(0, &stage_bg1_til, &stage_bg1_pal, 2,
                    (&stage_bg1_tilend - &stage_bg1_til),
                    (&stage_bg1_palend - &stage_bg1_pal), BG_16COLORS, 0x2000);
      bgSetMapPtr(0, 0x2800, SC_32x64);

      bgInitTileSet(1, &tiles_til, &tiles_pal, 3, (&tiles_tilend - &tiles_til),
                    80 * 2, BG_16COLORS, 0x3000);
      bgSetMapPtr(1, 0x6800, SC_64x32);

      bgInitTileSet(2, &stage_bg3_til, &stage_bg3_pal, 0,
                    (&stage_bg3_tilend - &stage_bg3_til), 16 * 2 * 2,
                    BG_16COLORS, 0x7000);
      bgSetMapPtr(2, 0x7800, SC_32x32);

      WaitForVBlank();
      dmaCopyVram(&stage_bg1_map, 0x2800, 4096);
      dmaCopyVram(&stage_bg3_map, 0x7800, 2048);
      setMode(BG_MODE1, BG3_MODE1_PRIORITY_HIGH);
      bgSetScroll(0, 0, 256);
      bgSetScroll(2, 0, 48);

      // --- Init OAM & Graphics ---
      oamInitGfxAttr(0x0000, OBJ_SIZE8_L16);

      dmaCopyVram((u8 *)&spr_fighters1_til, 0x0000, 0x800);
      dmaCopyVram((u8 *)&spr_fighters2_til, 0x0400, 0x800);
      dmaCopyVram((u8 *)&spr_enemies1_til, 0x0C20, 0x800);
      dmaCopyVram((u8 *)&spr_fx_til, 0x0800, 0x400);
      dmaCopyVram((u8 *)&spr_fx_til + 0x1000, 0x0C00, 0x040);
      dmaCopyVram((u8 *)&spr_fx_til + 0x1200, 0x0D00, 0x040);

      WaitForVBlank();
      dmaCopyCGram(&spr_fighters1_pal, 128 + 1 * 16, 16 * 2);
      dmaCopyCGram(&spr_fighters2_pal, 128 + 2 * 16, 16 * 2);
      dmaCopyCGram(&spr_enemies1_pal, 128 + 3 * 16, 16 * 2);
      dmaCopyCGram(&spr_fx_pal, 128 + 0 * 16, 16 * 2);
      setPaletteColor(0, 0);

      updateScoreBuffers();
      updateIconBuffers();
      nmiSet(myconsoleVblank);

      // Load map in memory and set initial camera
      mapLoad((u8 *)&stages_bg, (u8 *)&stages_def, (u8 *)&stages_att);
      mapSetMapOptions(MAP_OPT_BG2);

      // PVSnesLib mapLoad defaults to 0,0. We force the camera to the bottom of
      // the map:
      y_pos = 1792 - 224;
      mapUpdate();

      // --- Init Entities ---
      nbobjects = 0;
      initEntities();

      // ==============================================================================
      // MAIN GAME LOOP
      // ==============================================================================
      while (game_state == STATE_GAME) {
        gFrames++;
        if (gFrames == 60)
          setFadeEffect(FADE_IN);
        // Autoscroll the map
        if (y_pos > 0 && (gFrames & 3) == 3) {
          y_pos -= 1;
        }
        mapUpdate();

        frame_score1 = 0;
        frame_score2 = 0;

        objUpdateAll();

        bullets_update();

        // ------------------------------------------------------------------
        // SCORE & STATE UPDATES
        // ------------------------------------------------------------------
        if (frame_score1) {
          addScore(1, frame_score1);
          upd_p1_score = 1;
        }
        if (frame_score2) {
          addScore(2, frame_score2);
          upd_p2_score = 1;
        }

        if (upd_p1_score || upd_p2_score)
          updateScoreBuffers();
        if (upd_p1_lives || upd_p2_lives || upd_p1_bombs || upd_p2_bombs)
          updateIconBuffers();

        // ------------------------------------------------------------------
        // VBLANK WAIT & SCROLL
        // ------------------------------------------------------------------
        WaitForVBlank();
        mapVblank();
      }
    }
  }
  return 0;
}