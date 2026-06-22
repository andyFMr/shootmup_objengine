; Arquivo gerado automaticamente
.include "hdr.asm"

.section ".rodata1" superfree
; --- Folder data: stages ---
stages_bg:
.incbin "res/gfx/stages/BG1.m16"
stages_obj:
.incbin "res/gfx/stages/stage_bg.o16"
stages_att:
.incbin "res/gfx/stages/stage_bg.b16"
stages_def:
.incbin "res/gfx/stages/stage_bg.t16"

.include "res/gfx/entities/spr_enemies1_data.as" ; 4128 bytes
.include "res/gfx/entities/spr_enemies2_data.as" ; 4128 bytes
.include "res/gfx/entities/spr_fighters1_data.as" ; 2080 bytes
.include "res/gfx/entities/spr_fighters2_data.as" ; 2080 bytes
.include "res/gfx/entities/spr_fx_data.as" ; 8224 bytes
.include "res/gfx/stages/stage_bg1_data.as" ; 6176 bytes
.include "res/gfx/stages/stage_bg3_data.as" ; 2544 bytes
.ends

.section ".rodata2" superfree
.include "res/gfx/stages/tiles_data.as" ; 27456 bytes
.ends
