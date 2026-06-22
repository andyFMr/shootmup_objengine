ifeq ($(strip $(PVSNESLIB_HOME)),)
$(error "Please create an environment variable PVSNESLIB_HOME by following this guide: https://github.com/alekmaul/pvsneslib/wiki/Installation")
endif

HIROM = 1
FASTROM = 1

include ${PVSNESLIB_HOME}/devkitsnes/snes_rules

.PHONY: bitmaps all

#---------------------------------------------------------------------------------
# ROMNAME is used in snes_rules file
export ROMNAME := shmup

assets: bitmaps

all: $(ROMNAME).sfc

cleanAll: cleanBuildRes cleanRom cleanGfx cleanImages

clean: cleanBuildRes cleanRom

cleanImages:
	@echo "Cleaning generated image files..."
	-bash -O globstar -c "rm -f res/gfx/**/*.{pic,chr,nmt,pal,map,clm,inc,as} res/gfx/**/*.*16 >/dev/null 2>&1"

fx.pic: res/gfx/entities/spr_fx.png
	@echo convert meta sprites as 16px  ... $(notdir $@)
	$(GFXCONV) -s 32 -o 16 -u 16 -p -i $<

airships.pic: res/gfx/entities/
	@echo convert meta sprites as 16px  ... $(notdir $@)
	$(GFXCONV) -s 32 -o 16 -u 16 -p -i $<spr_fighters1.png
	$(GFXCONV) -s 32 -o 16 -u 16 -p -i $<spr_fighters2.png

enemyships.pic: res/gfx/entities/
	@echo convert meta sprites as 16px  ... $(notdir $@)
	$(GFXCONV) -s 32 -o 16 -u 16 -p -i $<spr_enemies1.png
	$(GFXCONV) -s 32 -o 16 -u 16 -p -i $<spr_enemies2.png

tiles.pic: res/gfx/stages/tiles.png
	@echo convert map tileset... $(notdir $@)
	$(GFXCONV) -s 8 -o 80 -u 16 -p -m -i $<

stage_bg.m16: res/gfx/stages/stage_bg.tmj tiles.pic
	@echo convert map tiled ... $(notdir $@)
	$(TMXCONV) $< res/gfx/stages/tiles.map

stage_bg1.pic: res/gfx/stages/stage_bg1.png
	@echo convert map tileset ... $(notdir $@)
	$(GFXCONV) -y -s 8 -o 64 -u 16 -e 2 -F -p -m -t png -i $<

stage_bg3.pic: res/gfx/stages/stage_bg3.png
	@echo convert map tileset ... $(notdir $@)
	$(GFXCONV) -s 8 -o 32 -u 4 -e 0 -p -g -t png -m -i $<

bitmaps : 	fx.pic \
			airships.pic \
			enemyships.pic \
			stage_bg.m16 stage_bg1.pic stage_bg3.pic \
