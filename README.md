# SNES Shoot 'em Up Demo

A simple vertical Shoot 'em Up demo for the Super Nintendo (SNES), developed in C using the **PVSnesLib** library.

## Features

- **Vertical Autoscroll**: The map automatically scrolls from bottom to top using the PVSnesLib *Map Engine*.
- **Multiplayer (2 Players)**: Support for two simultaneous players featuring lives, shooting, bombs, and a scoring system.
- **Entity Management**: An integrated object system that correctly maps entity positions to absolute map coordinates, smoothly tracking the camera's movement.
- **Collisions**: Loopless box collision detection between bullets and enemies/items, properly synchronized with the background scrolling.
- **Graphical Interface (HUD)**: Score, lives, and bombs counters utilizing separate buffers which are pushed to VRAM (OAM and Background) via DMA during VBlank.

## Code Structure

- `src/main.c`: Main game loop, map loading, camera scrolling speed control, and rendering updates.
- `src/core/entities.c`: Initialization, movement logic, autoscroll handling, collisions, and drawing (OAM) for players, enemies, items, and projectiles.
- `src/core/maps.c / maps.h`: HUD management (score, bombs, and lives), graphical UI buffers, and `mapVblank` to update the background tiles in real time.
- `src/core/system.c`: Custom `VBlank` (NMI) routine that synchronizes the HUD and graphical updates during vertical retrace (preventing visual glitches).

## How to Compile

Ensure that **PVSnesLib** is properly installed in your development environment.

1. To clean previous builds:
   ```bash
   make clean
   ```

2. To completely wipe all generated asset files (`.pic`, `.pal`, `.map`, etc.) along with the build:
   ```bash
   make cleanAll
   ```

3. To force a rebuild of all graphical assets from the original PNGs and TMX map files:
   ```bash
   make assets
   ```

4. To compile the code and link the final ROM:
   ```bash
   make
   ```

5. The final `.sfc` ROM file will be generated in the root project folder and can be run on emulators like **Mesen** or **Snes9x**.

## Controls
- **D-Pad**: Move the ship.
- **Y / B / A**: Shoot.
- **X**: Use bomb.
