#include "def.h"
#include "maxmod.h"
#include "LinkConnection.h"
#include "meter1_tiles_bin.h"
#include "soundbank.h"
#include "sprite36tiles_bin.h"
#include "sprite5tiles_bin.h"
#include "sprites.h"
#include "tetrisEngine.h"
#include "tetromino.hpp"
#include "tonc.h"
#include "logging.h"
#include <string>
#include <tuple>
#include "text.h"

#include "rumble.h"

#include "classic_pal_bin.h"
#include "logging.h"
#include "posprintf.h"
#include "tonc_core.h"
#include "tonc_input.h"
#include "tonc_memdef.h"
#include "tonc_memmap.h"
#include "tonc_video.h"

using namespace Tetris;

void diagnose();
void clearGlow();
void addGlow(Drop);
void addPlaceEffect(Drop);
void countdown();
void drawFrame();
void drawGrid();
void progressBar();
void showBar(int, int, int, int);
void showBestMove();
bool checkDiagonal(int);
void showFinesse();
void showSpeedMeter(int);
void hideMinos();
void disappear();
INLINE int getBoard(int,int);
void zoneFlash();
void resetZonePalette();
void control();
void showTimer();
void update();
void rainbowPalette();
void frameSnow();
void showZoneText();
void showFullMeter();
Function getActionFromKey(int key);
void liftKeys();
void journeyFlash();
void setJourneyGraphics(Save * save, int level);

Game* game;
OBJ_ATTR* pawnSprite;
OBJ_ATTR* pawnShadow;
OBJ_ATTR* holdSprite;
OBJ_ATTR* holdFrameSprite;
OBJ_ATTR* queueFrameSprites[3];

OBJ_ATTR* queueSprites[5];

OBJ_ATTR* moveSprites[3];

bool onStates = false;

int clearTimer = 0;

int maxClearTimer = 20;

std::string clearTypeText = "";
#define maxClearTextTimer 100
#define clearTextHeight 16

s16 glow[20][10];

int push = 0;
int pushTimer = 0;

int restartTimer = 0;
#define maxRestartTimer 20

int attackFlashTimer = 0;
#define attackFlashMax  10

int rumbleTimer = 0;
#define rumbleMax 1

std::list<FloatText> floatingList;

std::list<Effect> effectList;

std::list<PlaceEffect> placeEffectList;

#define eventPauseTimerMax 60

int eventPauseTimer = 0;

static bool holdingSave = false;
static Settings previousSettings;

#define flashTimerMax 16
static int flashTimer = 0;

COLOR * previousPalette = nullptr;

static int rainbowTimer = 1;
static bool rainbowIncreasing = 0;
static u16 rainbow[5];

static bool creditRefresh = false;
static u16 fullMeterTimer = 0;
#define fullMeterTimerMax 5 * 60;
#define fullMeterAnimationLength 15
// static bool refreshSkin = false;
// Bot *testBot;

void GameScene::draw(){
    control();
    checkSounds();
    showPawn();
    showShadow();

    showHold();
    showQueue();

    drawGrid();
    screenShake();
    showClearText();
    showPlaceEffect();

    oam_copy(oam_mem, obj_buffer, 32);
    if(game->eventTimer)
        oam_copy(&oam_mem[64], &obj_buffer[64], 27);
    obj_aff_copy(obj_aff_mem, obj_aff_buffer, 32);
    if (game->refresh) {
        update();
        showBackground();
        game->resetRefresh();
    }else if (game->clearLock && !eventPauseTimer){
        showBackground();
        showTimer();
    }else{
        showTimer();
    }
}

void update() {
    // aprintClearArea(0, 0, 10, 6);
    // aprintClearArea(0, 13, 10, 7);

    if(!game->zoneTimer)
        showText();
    else{
        showZoneText();
    }

    showTimer();
    showClearText();

    if(proMode){
        showFinesseCombo();
        showPPS();
    }
}

void checkSounds() {
    if (game->sounds.hold)
        sfx(SFX_HOLD);
    if (game->sounds.shift)
        sfx(SFX_SHIFT2);
    if (game->sounds.place) {
        sfx(SFX_PLACE);
        shake = (shakeMax * (savefile->settings.shakeAmount)/ 4) /2;
        if(!game->sounds.finesse)
            for(int i = 0; i < 3; i++)
                obj_hide(moveSprites[i]);

        rumbleTimer = rumbleMax * savefile->settings.rumble;
    }
    if (game->sounds.invalid){
        sfx(SFX_INVALID);
        rumbleTimer = rumbleMax * savefile->settings.rumble;
    }
    if (game->sounds.rotate)
        sfx(SFX_ROTATE);
    if (game->sounds.finesse){
        if(game->trainingMode)
            showBestMove();

        sfx(SFX_MENUCANCEL);
    }

    if (game->sounds.clear) {

        if(game->previousClear.linesCleared <= 4){
            int speed = game->comboCounter - 1;
            if (speed > 10)
                speed = 10;

            mm_sound_effect clear = {
                {SFX_LEVELUP},
                (mm_hword)((1.0 + (float)speed / 10) * (1 << (10 - (game->zoneTimer != 0)))),
                0,
                (u8)(255 * (float) savefile->settings.sfxVolume / 10),
                128,
            };

            mmEffectEx(&clear);
        }else{
            sfx(SFX_MULTICLEAR);
        }

        int soundEffect = -1;

        clearTypeText = "";
        if(game->zoneTimer){

        } else if (game->previousClear.isPerfectClear == 1) {
            soundEffect = SFX_PERFECTCLEAR;
            clearTypeText = "perfect clear";
            effectList.push_back(Effect(0));
        } else if (game->previousClear.isTSpin == 2) {
            if (game->previousClear.isBackToBack == 1)
                soundEffect = SFX_BACKTOBACKTSPIN;
            else
                soundEffect = SFX_TSPIN;

            if (game->previousClear.linesCleared == 1) {
                clearTypeText = "t-spin single";
            } else if (game->previousClear.linesCleared == 2) {
                clearTypeText = "t-spin double";
            } else if (game->previousClear.linesCleared == 3) {
                clearTypeText = "t-spin triple";
            }
        } else if (game->previousClear.isTSpin == 1) {
            soundEffect = SFX_TSPINMINI;

            clearTypeText = "t-spin mini";
        } else if (game->previousClear.linesCleared > 4) {
            int n = game->previousClear.linesCleared;
            if(n < 8){
                clearTypeText = "quad";
                soundEffect = SFX_QUAD;
            }else if(n < 12){
                clearTypeText = "octo";
                soundEffect = SFX_OCTORIS;
            }else if(n < 16){
                clearTypeText = "dodeca";
                soundEffect = SFX_DODECATRIS;
            }else if(n < 18){
                clearTypeText = "decahexa";
                soundEffect = SFX_DECAHEXATRIS;
            }else if(n < 20){
                clearTypeText = "perfectus";
                soundEffect = SFX_PERFECTRIS;
            }else{
                clearTypeText = "ultimus";
                soundEffect = SFX_ULTIMATRIS;
            }
        } else if (game->previousClear.linesCleared == 4) {
            if (game->previousClear.isBackToBack == 1)
                soundEffect = SFX_BACKTOBACKQUAD;
            else
                soundEffect = SFX_QUAD;
            clearTypeText = "quad";
        } else if (game->previousClear.linesCleared == 3) {
            soundEffect = SFX_TRIPLE;
            clearTypeText = "triple";
        } else if (game->previousClear.linesCleared == 2) {
            soundEffect = SFX_DOUBLE;
            clearTypeText = "double";
        }

        if (savefile->settings.floatText)
            floatingList.push_back(FloatText(clearTypeText));

        if (savefile->settings.announcer && soundEffect != -1)
            sfx(soundEffect);
    }

    if (game->sounds.levelUp) {
        sfx(SFX_LEVELUPSOUND);

        if(savefile->settings.colors == 3){
            int n = getClassicPalette();

            for(int i = 0; i < 8; i++){
                memcpy16(&pal_bg_mem[i*16+1], &nesPalette[n][0],4);

                memcpy16(&pal_obj_mem[i*16+1], &nesPalette[n][0],4);
            }
        }

        if(game->gameMode == MASTER){
            maxClearTimer = game->maxClearDelay;
        }
		else if(game->gameMode == MARATHON){			
			journeyLevelUp = true;
        }
    }

    std::string sectionText = "";

    switch(game->sounds.section){
        case -1:
            sectionText = "regret!!";
            sfx(SFX_REGRET);
            break;
        case 1:
            sectionText = "cool!!";
            sfx(SFX_COOL);
            break;
        case 2: sfx(SFX_SECRET); break;
    }

    if (sectionText.size() && savefile->settings.floatText)
        floatingList.push_back(FloatText(sectionText));


    if(game->sounds.disappear){
        disappear();
    }

    if (game->sounds.zone == 1) {
        clearText();
        gradient(false);

        holdingSave = true;
        previousSettings = savefile->settings;

        savefile->settings.colors = 4;
        savefile->settings.clearEffect = 2;
        setPalette();
        setClearEffect();
        showBackground();

        flashTimer = flashTimerMax;
        zoneFlash();

        u16 * dest = (u16*) &se_mem[26];
        for(int i = 0; i < 20; i++){
            dest[i * 32 + 9] = 4;
            dest[i * 32 + 20] = 4 + 0x400;
        }

        if (savefile->settings.lightMode){
            for(int i = 0; i < 4; i++){
                COLOR c = RGB15(10-i, 10-i, 10-i);
                pal_bg_mem[(0+i) * 16 + 5] += c;
            }
        }

        // if(savefile->settings.lightMode)
        //     memset16(&pal_bg_mem[4 * 16 + 5], 0x4a52, 1);

        mmSetModuleTempo(512);
        mmSetModuleVolume(512 * ((float)savefile->settings.volume / 20));

        sfx(SFX_ZONESTART);
    } else if (game->sounds.zone == 2) {
        savefile->settings.lightMode = !previousSettings.lightMode;

        setPalette();
        if (savefile->settings.lightMode){
            for(int i = 0; i < 4; i++){
                COLOR c = RGB15(10-i, 10-i, 10-i);
                pal_bg_mem[(0+i) * 16 + 5] += c;
            }
        }

        // if(savefile->settings.lightMode)
        //     memset16(&pal_bg_mem[4 * 16 + 5], 0x4a52, 1);

    } else if (game->sounds.zone == -1) {
        aprintClearArea(10, 0, 10, 20);
        resetZonePalette();
        zoneFlash();
    }

    if(game->sounds.meter){
        fullMeterTimer = fullMeterAnimationLength;
    }

    if(game->gameMode == MARATHON && subMode){
        showFullMeter();
    }

    game->resetSounds();
}

void showBackground() {

    bool showEdges = savefile->settings.edges;

    u16* dest = (u16*)se_mem[25];

    std::list<int>::iterator l2c = game->linesToClear.begin();
    bool up, down, left, right;
    bool before = false, after = false;

    dest += 10;
    for (int i = 20; i < 40; i++) {
        if (game->linesToClear.size() > 0) {
            before = after = false;
            for(auto const& l : game->linesToClear){
                if(l == i - 1)
                    before = true;
                if(l == i + 1)
                    after = true;
                if(l > i + 1)
                    break;
            }
        }

        if(game->clearLock && i != *l2c && clearTimer != 1){
            dest+= 32;
            continue;
        }

        for (int j = 0; j < 10; j++) {
            if (!game->board[i][j] || (game->clearLock && i == *l2c && showEdges) || (game->disappearing && game->disappearTimers[i][j] == 1)) {
                if (!showEdges) {
                    *dest++ = 0;
                    continue;
                }

                if(!game->disappearing){
                    up = (game->board[i - 1][j] > 0 && !before);
                    left = (j - 1 >= 0 && game->board[i][j - 1] > 0 && !(i == *l2c && game->clearLock));
                    right = (j + 1 <= 9 && game->board[i][j + 1] > 0 && !(i == *l2c && game->clearLock));
                    down = (i + 1 <= 39 && game->board[i + 1][j] > 0 && !after);
                }else{
                    up = (getBoard(j,i-1) > 0 && !before);
                    left = (j - 1 >= 0 && getBoard(j-1, i) > 0 && !(i == *l2c && game->clearLock));
                    right = (j + 1 <= 9 && getBoard(j+1,i) > 0 && !(i == *l2c && game->clearLock));
                    down = (i + 1 <= 39 && getBoard(j,i+1) > 0 && !after);
                }

                int count = up + down + left + right;

                int n = 0;
                if (count == 0) {
                    n = 0;
                } else if (count == 4) {
                    // if(up && left && right && down)//full
                    n = 0x0019;
                } else if (count == 3) {
                    if (up && left && !right && down)//1 side missing
                        n = 0x0018;
                    else if (up && !left && right && down)
                        n = 0x0418;
                    else if (up && left && right && !down)
                        n = 0x0017;
                    else if (!up && left && right && down)
                        n = 0x0817;
                    else
                        n = 0x000a;
                } else if (count == 2) {
                    if (up && left && !right && !down)//2 sides missing
                        n = 0x0016;
                    else if (up && !left && right && !down)
                        n = 0x0416;
                    else if (!up && left && !right && down)
                        n = 0x0816;
                    else if (!up && !left && right && down)
                        n = 0x0c16;
                    else if (up && !left && !right && down)//
                        n = 0x0012;
                    else if (!up && left && right && !down)
                        n = 0x0011;
                    else
                        n = 0x000b;
                } else if (count == 1) {
                    if (!up && left && !right && !down)//4 sides missing
                        n = 0x0010;
                    else if (!up && !left && right && !down)
                        n = 0x0410;
                    else if (up && !left && !right && !down)
                        n = 0x000f;
                    else if (!up && !left && !right && down)
                        n = 0x080f;
                    else
                        n = 0;
                }
                *dest++ = n + (savefile->settings.lightMode * 0x1000);
            } else{
                int offset = 1;

                int n = (game->board[i][j] - 1) & 0xf;
                int r = (game->board[i][j]) >> 4;

                if(savefile->settings.skin == 7 || savefile->settings.skin == 8)
                    offset = 48 + (n);
                else if(savefile->settings.skin >= 11)
                    offset = 128 + GameInfo::connectedConversion[r];

                if(n != 8)
                    *dest++ = (offset + ((n) << 12) * !game->zoneTimer);
                else
                    *dest++ = 3;
            }

            if (game->clearLock && i == *l2c) {
                dest--;

                if (!showEdges)
                    *dest = 0;
                if (j < 5) {
                    if (clearTimer < maxClearTimer - 10 + j * 2)
                        *dest = 3 + savefile->settings.lightMode * 0x1000;
                } else {
                    if (clearTimer < maxClearTimer - 10 + (9 - j) * 2)
                        *dest = 3 + savefile->settings.lightMode * 0x1000;
                }
                dest++;

                if(clearTimer < 0){
                    aprint("ERROR",12,i);
                }

            }
        }
        if (i == *l2c)
            ++l2c;
        dest += 22;
    }
}

void showPawn() {
    pawnSprite = &obj_buffer[0];
    if (game->clearLock || game->pawn.current == -1) {
        obj_hide(pawnSprite);
        return;
    }

    obj_unhide(pawnSprite, 0);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int n = game->pawn.board[game->pawn.rotation][i][j];
            if (n > 0){
                if(savefile->settings.skin == 11)
                    memcpy16(&tile_mem[4][16 * 7 + i * 4 + j], &sprite38tiles_bin[GameInfo::connectedConversion[(n)>>4] * 32], sprite1tiles_bin_size / 2);
                else if(savefile->settings.skin == 12)
                    memcpy16(&tile_mem[4][16 * 7 + i * 4 + j], &sprite39tiles_bin[GameInfo::connectedConversion[(n)>>4] * 32], sprite1tiles_bin_size / 2);
                else if(savefile->settings.skin == 13)
                    memcpy16(&tile_mem[4][16 * 7 + i * 4 + j], &sprite40tiles_bin[GameInfo::connectedConversion[(n)>>4] * 32], sprite1tiles_bin_size / 2);
                else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                    memcpy16(&tile_mem[4][16 * 7 + i * 4 + j], blockSprite, sprite1tiles_bin_size / 2);
                else
                    memcpy16(&tile_mem[4][16 * 7 + i * 4 + j], classicTiles[savefile->settings.skin-7][game->pawn.current], sprite1tiles_bin_size / 2);
            }else
                memset16(&tile_mem[4][16 * 7 + i * 4 + j], 0, sprite1tiles_bin_size / 2);
        }
    }

    int n = game->pawn.current;

    int blend = 0;

    if(game->maxLockTimer > 1)
        blend += 16-(game->lockTimer * 16) / game->maxLockTimer;

    if(!game->zoneTimer){
        if (!savefile->settings.lightMode){
            if(savefile->settings.colors == 2)
                clr_fade((COLOR*)classic_pal_bin, 0x0000, &pal_obj_mem[11 * 16], 8, blend);
            else if(savefile->settings.colors == 3){
                clr_fade((COLOR*)&nesPalette[getClassicPalette()][0], 0x0000, &pal_obj_mem[11 * 16+1], 4, blend);
            }else if(savefile->settings.colors == 4){
                clr_fade((COLOR*)&monoPalette[0][0], 0x0000, &pal_obj_mem[11 * 16+1], 4, blend);
            }else if(savefile->settings.colors == 5){
                clr_fade((COLOR*)&arsPalette[0][n], 0x0000, &pal_obj_mem[11 * 16+1], 4, blend);
            }else if(savefile->settings.colors == 6){
                clr_fade((COLOR*)&arsPalette[1][n], 0x0000, &pal_obj_mem[11 * 16+1], 4, blend);
            } else
                clr_fade_fast((COLOR*)&palette[savefile->settings.colors][n * 16], 0x0000, &pal_obj_mem[11 * 16], 8, blend);
        }else{
            if(savefile->settings.colors == 2)
                clr_adj_brightness(&pal_obj_mem[11 * 16], (COLOR*)classic_pal_bin, 8, int2fx(blend) >> 5);
            else if(savefile->settings.colors == 3){
                clr_adj_brightness(&pal_obj_mem[11 * 16+1], (COLOR*)&nesPalette[getClassicPalette()][0], 8, int2fx(blend) >> 5);
            }else if(savefile->settings.colors == 4){
                clr_adj_brightness(&pal_obj_mem[11 * 16+1], (COLOR*)&monoPalette[1][0], 4, int2fx(blend) >> 5);
            }else if(savefile->settings.colors == 5){
                clr_adj_brightness(&pal_obj_mem[11 * 16+1], (COLOR*)&arsPalette[0][n], 4, int2fx(blend) >> 5);
            }else if(savefile->settings.colors == 6){
                clr_adj_brightness(&pal_obj_mem[11 * 16+1], (COLOR*)&arsPalette[1][n], 4, int2fx(blend) >> 5);
            }else
                clr_adj_brightness(&pal_obj_mem[11 * 16], (COLOR*)&palette[savefile->settings.colors][n * 16], 8, int2fx(blend) >> 5);
        }
    }

    if(!game->pawn.big){
        obj_set_attr(pawnSprite, ATTR0_SQUARE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_BUILD(16 * 7, 11, 2));
        obj_set_pos(pawnSprite, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->pawn.y - 20) * 8 + shake * savefile->settings.shake);
    }else{
        obj_set_attr(pawnSprite, ATTR0_SQUARE | ATTR0_AFF_DBL | ATTR0_MOSAIC, ATTR1_SIZE(2) | ATTR1_AFF_ID(31), ATTR2_BUILD(16 * 7, 11, 2));
        obj_aff_identity(&obj_aff_buffer[31]);
        obj_aff_scale(&obj_aff_buffer[31], 1<<7, 1<<7);
        obj_set_pos(pawnSprite, (10 + game->pawn.x*2) * 8 + push * savefile->settings.shake, (game->pawn.y*2 - 20) * 8 + shake * savefile->settings.shake);
    }
}

void showShadow() {
    pawnShadow = &obj_buffer[1];
    if (game->clearLock || game->pawn.current == -1 || game->gameMode == CLASSIC || (game->gameMode == MASTER && game->level >= 100) ) {
        obj_hide(pawnShadow);
        return;
    }

    u8* shadowTexture;

    bool bld = false;
    switch (savefile->settings.shadow) {
        case 0: shadowTexture = (u8*)sprite2tiles_bin; break;
        case 1: shadowTexture = (u8*)sprite15tiles_bin; break;
        case 2: shadowTexture = (u8*)sprite16tiles_bin; break;
        case 3:
            if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                shadowTexture = blockSprite;
            else
                shadowTexture = (u8*)classicTiles[savefile->settings.skin-7][game->pawn.current];
            bld = true;
            break;
        case 4: obj_hide(pawnShadow); return;
        default: shadowTexture = (u8*)sprite2tiles_bin; break;
    }

    obj_unhide(pawnShadow, 0);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(game->pawn.board[game->pawn.rotation][i][j] > 0){
                memcpy16(&tile_mem[4][16 * 8 + i * 4 + j], shadowTexture, sprite2tiles_bin_size / 2);
            } else
                memset16(&tile_mem[4][16 * 8 + i * 4 + j], 0, sprite2tiles_bin_size / 2);
        }
    }

    int n = game->pawn.current;

    if(!game->zoneTimer){
        if (!savefile->settings.lightMode){
            if(savefile->settings.colors == 2)
                clr_fade((COLOR*)classic_pal_bin, 0x0000, &pal_obj_mem[10 * 16], 8, (14) * bld);
            else if(savefile->settings.colors == 3){
                clr_fade((COLOR*)&nesPalette[getClassicPalette()][0], 0x0000, &pal_obj_mem[10 * 16+1], 4, (14) * bld);
            }else if(savefile->settings.colors == 4){
                clr_fade((COLOR*)&monoPalette[0][0], 0x0000, &pal_obj_mem[10 * 16+1], 4, 14);
            }else if(savefile->settings.colors == 5){
                clr_fade((COLOR*)&arsPalette[0][n], 0x0000, &pal_obj_mem[10 * 16+1], 4, (14) * bld);
            }else if(savefile->settings.colors == 6){
                clr_fade((COLOR*)&arsPalette[1][n], 0x0000, &pal_obj_mem[10 * 16+1], 4, (14) * bld);
            } else
                clr_fade_fast((COLOR*)&palette[savefile->settings.colors][n * 16], 0x0000, &pal_obj_mem[10 * 16], 8, (14) * bld);
        }else{
            if(savefile->settings.colors == 2)
                clr_adj_brightness(&pal_obj_mem[10 * 16], (COLOR*)classic_pal_bin, 8, float2fx(0.25));
            else if(savefile->settings.colors == 3){
                clr_adj_brightness(&pal_obj_mem[10 * 16+1], (COLOR*)&nesPalette[getClassicPalette()][0], 8, float2fx(0.25));
            }else if(savefile->settings.colors == 4){
                clr_adj_brightness(&pal_obj_mem[10 * 16+1], (COLOR*)&monoPalette[1][0], 4, float2fx(0.25));
            }else if(savefile->settings.colors == 5){
                clr_adj_brightness(&pal_obj_mem[10 * 16+1], (COLOR*)&arsPalette[0][n], 4, float2fx(0.25));
            }else if(savefile->settings.colors == 6){
                clr_adj_brightness(&pal_obj_mem[10 * 16+1], (COLOR*)&arsPalette[1][n], 4, float2fx(0.25));
            }else
                clr_adj_brightness(&pal_obj_mem[10 * 16], (COLOR*)&palette[savefile->settings.colors][n * 16], 8, float2fx(0.25));
        }
    }

    if(!game->pawn.big){
        obj_set_attr(pawnShadow, ATTR0_SQUARE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_BUILD(16 * (8 - (savefile->settings.shadow == 3)), 10, 2));
        obj_set_pos(pawnShadow, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->lowest() - 20) * 8 + shake * savefile->settings.shake);
    }else{
        obj_set_attr(pawnShadow, ATTR0_SQUARE | ATTR0_AFF_DBL | ATTR0_MOSAIC, ATTR1_SIZE(2) | ATTR1_AFF_ID(30), ATTR2_BUILD(16 * (8 - (savefile->settings.shadow == 3)), 10, 2));
        obj_aff_identity(&obj_aff_buffer[30]);
        obj_aff_scale(&obj_aff_buffer[30],1<<7,1<<7);
        obj_set_pos(pawnShadow, (10 + game->pawn.x*2) * 8 + push * savefile->settings.shake, (game->lowest()*2 - 20) * 8 + shake * savefile->settings.shake);
    }
}

void showHold() {
    holdSprite = &obj_buffer[2];
    holdFrameSprite = &obj_buffer[10];

    if(game->gameMode != CLASSIC && !game->zoneTimer){
        obj_unhide(holdFrameSprite, 0);
        obj_set_attr(holdFrameSprite, ATTR0_SQUARE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_BUILD(512, 8, 3));
        obj_set_pos(holdFrameSprite, 4 * 8 + 7 + (push < 0) * push, 9 * 8 - 2);
    }else{
        obj_hide(holdFrameSprite);
    }

    if (game->held == -1) {
        obj_hide(holdSprite);
        return;
    }

    int add = !(game->held == 0 || game->held == 3);
    int palette = (game->canHold)? game->held : 7;

    int yoffset = - (6 * (game->rotationSystem != SRS && savefile->settings.skin >= 7));

    if (savefile->settings.skin < 7) {
        obj_unhide(holdSprite, 0);
        obj_set_attr(holdSprite, ATTR0_WIDE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_PALBANK(palette));
        holdSprite->attr2 = ATTR2_BUILD(9 * 16 + 8 * game->held, palette * (game->zoneTimer == 0), 3);
        obj_set_pos(holdSprite, (5) * 8 + add * 3 + 3 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0 && game->rotationSystem != ARS));
    } else {
        obj_unhide(holdSprite, ATTR0_AFF);
        obj_set_attr(holdSprite, ATTR0_SQUARE | ATTR0_AFF | ATTR0_MOSAIC, ATTR1_SIZE(2) | ATTR1_AFF_ID(5), ATTR2_PALBANK(palette));
        holdSprite->attr2 = ATTR2_BUILD(16 * game->held, palette * (game->zoneTimer == 0), 3);
        FIXED size;
        if(savefile->settings.skin < 9)
            size = 357;//~1.4
        else
            size = 349;//~1.4

        obj_aff_scale(&obj_aff_buffer[5], size, size);
        obj_set_pos(holdSprite, (5) * 8 + add * 3 + 3 - 4 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0 && game->rotationSystem != ARS) - 3 + yoffset);
    }
}

void showQueue() {
    int maxQueue = savefile->settings.maxQueue;

    if(game->gameMode == CLASSIC)
        maxQueue = 1;
    else if(game->gameMode == MASTER)
        maxQueue = (maxQueue < 3)? maxQueue : 3;

    for (int i = 0; i < maxQueue; i++)
        queueSprites[i] = &obj_buffer[3 + i];

    for (int i = 0; i < 3; i++)
        queueFrameSprites[i] = &obj_buffer[11 + i];

    if(game->zoneTimer){
        obj_hide(queueFrameSprites[0]);
        obj_hide(queueFrameSprites[1]);
        obj_hide(queueFrameSprites[2]);
    }else if(maxQueue > 1){
        for (int i = 0; i < 3; i++) {
            obj_unhide(queueFrameSprites[i], 0);
            obj_set_attr(queueFrameSprites[i], ATTR0_SQUARE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_BUILD(512 + 16 + 16 * i, 8, 3));
            obj_set_pos(queueFrameSprites[i], 173 + (push > 0) * push, 12 + 32 * i - (i * 9 * (5-maxQueue)));
        }
    }else{
        obj_unhide(queueFrameSprites[0], 0);
        obj_set_attr(queueFrameSprites[0], ATTR0_SQUARE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_BUILD(512, 8, 3));
        obj_set_pos(queueFrameSprites[0], 173 + (push > 0) * push, 12);

        obj_hide(queueFrameSprites[1]);
        obj_hide(queueFrameSprites[2]);
    }

    int startX = 22 * 8 + 1;
    int yoffset = 4 * (maxQueue == 1) - (5 * (game->rotationSystem != SRS && savefile->settings.skin >= 7));

    std::list<int>::iterator q = game->queue.begin();
    for (int k = 0; k < 5; k++){

        if(k >= maxQueue){
            obj_hide(queueSprites[k]);
            continue;
        }

        int n = *q;

        int add = !(n == 0 || n == 3);
        if (savefile->settings.skin < 7) {
            obj_unhide(queueSprites[k], 0);
            obj_set_attr(queueSprites[k], ATTR0_WIDE | ATTR0_MOSAIC, ATTR1_SIZE(2), ATTR2_PALBANK(n));
            queueSprites[k]->attr2 = ATTR2_BUILD(16 * 9 + 8 * n, n * (game->zoneTimer == 0), 3);
            obj_set_pos(queueSprites[k], startX + add * 3 + (push > 0) * push, (3 + (k * 3)) * 6 - 3 * (n == 0) + yoffset + 8 * (game->rotationSystem == ARS && n == 0));
        } else {
            obj_unhide(queueSprites[k], ATTR0_AFF);
            obj_set_attr(queueSprites[k], ATTR0_SQUARE | ATTR0_AFF | ATTR0_MOSAIC, ATTR1_SIZE(2) | ATTR1_AFF_ID(k), ATTR2_PALBANK(n));
            queueSprites[k]->attr2 = ATTR2_BUILD(16 * n, n * (game->zoneTimer == 0), 3);
            obj_aff_identity(&obj_aff_buffer[k]);
            // FIXED size = 358 + sizeControl;//~1.4
            FIXED size;
            if(savefile->settings.skin < 9)
                size = 357;//~1.4
            else
                size = 349;//~1.4
            obj_aff_scale(&obj_aff_buffer[k], size, size);
            obj_set_pos(queueSprites[k], startX + add * 3 + (push > 0) * push - 4, (3 + (k * 3)) * 6 - 3 * (game->rotationSystem != ARS && n == 0) - 4 + yoffset);
        }

        if(q != game->queue.end())
            ++q;
        else
            break;
    }
}

void control() {
    if (pause)
        return;

    key_poll();

    Keys k = savefile->settings.keys;

    if (key_hit(KEY_START) && !multiplayer && !eventPauseTimer) {
        sfx(SFX_MENUCONFIRM);
        pause = true;
        mmPause();
        clearText();
        update();
    }

    if (key_hit(k.hold) && !checkDiagonal(k.hold)) {
        game->hold(1);
    }

    if (key_released(k.hold)){
        game->hold(0);
    }

    if (key_hit(k.moveLeft) && !checkDiagonal(k.moveLeft))
        game->keyLeft(1);
    if (key_hit(k.moveRight) && !checkDiagonal(k.moveRight))
        game->keyRight(1);

    if (key_hit(k.hardDrop) && !checkDiagonal(k.hardDrop))
        game->keyDrop(1);

    if (key_hit(k.softDrop) && !checkDiagonal(k.softDrop))
        game->keyDown(1);

    if (key_is_down(KEY_A) && key_is_down(KEY_B) && savefile->settings.abHold){
        game->hold(1);
    }else{
        if (key_hit(k.rotateCW) && !checkDiagonal(k.rotateCW))
            game->rotateCW(1);

        if (key_hit(k.rotateCCW) && !checkDiagonal(k.rotateCCW))
            game->rotateCCW(1);
    }

    if (key_hit(k.rotate180) && !checkDiagonal(k.rotate180)){
        game->rotateTwice(1);
    }

    if(savefile->settings.abHold && key_released(KEY_A) && key_released(KEY_B)){
        game->hold(0);
    }

    if (key_released(k.rotateCW))
        game->rotateCW(0);

    if (key_released(k.rotateCCW))
        game->rotateCCW(0);

    if (key_released(k.rotate180)){
        game->rotateTwice(0);
    }

    if (key_released(k.softDrop))
        game->keyDown(0);

    if (key_released(k.moveLeft)){
        game->keyLeft(0);
    }

    if (key_released(k.moveRight)){
        game->keyRight(0);
    }

    if (key_is_down(KEY_L) && key_is_down(KEY_R) && !(game->gameMode == BATTLE || game->gameMode == MASTER) && !eventPauseTimer) {
        if(restartTimer++ > maxRestartTimer || !savefile->settings.resetHold)
            playAgain = true;
    }else{
        restartTimer = 0;
    }

    if(key_is_down(k.zone) && game->gameMode == MARATHON && subMode && !game->zoneTimer){
        game->activateZone(1);
    }

    if(savefile->settings.diagonalType == 1 || game->rotationSystem == ARS){
        if(key_released(KEY_RIGHT) || key_released(KEY_LEFT)){
            if(key_is_down(KEY_UP)){
                Function f = getActionFromKey(KEY_UP);
                (game->*f.gameFunction)(1);
            }else if(key_is_down(KEY_DOWN)){
                Function f = getActionFromKey(KEY_DOWN);
                (game->*f.gameFunction)(1);
            }
        }
    }
}

void showTimer() {
    if (!(game->gameMode == TRAINING)) {

        std::string timer = timeToString(gameSeconds);
        aprintClearArea(0, 1, 10, 1);
        aprint(timer, 1 - (timer.size() > 8), 1);
    }

    if(game->trainingMode){
    //
        // aprints("Finesse:",0,7,2);
        // showFinesse();
        aprint("Finesse", 1, 14);
        aprintClearArea(0, 15, 10, 15);
        aprintf(game->finesseFaults, 4, 15);
    }

    if(proMode){
        // clearSmallText();
        showPPS();
    }
}

void showText() {
    if (game->gameMode == MARATHON || game->gameMode == ULTRA || game->gameMode == BLITZ || game->gameMode == CLASSIC) {

        aprint("Score", 3, 3);

        std::string score = std::to_string(game->score);

        int x = 8 - score.size();
        aprintClearArea(0, 5, 10, 1);
        aprint(score, (x > 0)? x : 0, 5);

        if (game->gameMode != ULTRA) {
            aprint("Level", 2, 14);

            aprintClearArea(0, 15, 10, 1);
            aprintf(game->level, 4, 15);
        }

    } else if (game->gameMode == TRAINING) {
        aprint("Training", 1, 1);
    } else if (game->gameMode == DIG && subMode) {
        aprint("Pieces", 2, 14);

        aprintClearArea(0, 15, 10, 1);
        aprintf(game->pieceCounter, 4, 15);
    } else if (game->gameMode == MASTER){
        aprint("Level", 3, 14);

        std::string str = std::to_string(game->level);

        aprint(str, 7-str.size(), 16);

        showSpeedMeter((int)game->speed);

        int n = ((game->level / 100)+1) * 100;

        if(n == 1000)
            n--;

        aprintf(n, 4, 18);

        aprint("Grade",3,3);

        str = GameInfo::masterGrades[game->grade + game->coolCount];

        aprintClearArea(0, 5, 10, 1);
        aprint(str,4 - (str.length() == 1),5);
    }

    if (game->gameMode != BATTLE && game->gameMode != BLITZ && !(game->gameMode == SPRINT && subMode == 1) && game->gameMode != MASTER){
        aprint("Lines", 2, 17);
        aprintClearArea(0, 18, 10, 1);
        if (game->gameMode == DIG)
            aprintf(game->garbageCleared, 4, 18);
        else
            aprintf(game->linesCleared, 4, 18);

    } else if(game->gameMode == BLITZ){
        aprint("Lines", 2, 17);
        std::string str;

        if(game->level > 1)
            str+= std::to_string(game->linesCleared-GameInfo::blitzLevels[game->level-2]);
        else
            str+= std::to_string(game->linesCleared);

        str+= " / ";

        if(game->level > 1)
            str+= std::to_string(GameInfo::blitzLevels[game->level-1] - GameInfo::blitzLevels[game->level-2]);
        else
            str+= "3";

        aprintClearArea(0, 18, 10, 1);
        aprint(str,2,18);

    } else if(game->gameMode == BATTLE || (game->gameMode == SPRINT && subMode == 1)){
        aprint("Attack", 2, 17);
        aprintClearArea(0, 18, 10, 1);
        aprintf(game->linesSent, 4, 18);
    }
}

void showPPS(){
    FIXED t = (gameSeconds + game->eventTimer) * float2fx(0.0167f);

    FIXED pps;

    if(t <= 0){
        pps = 0;
    }else{
        pps = fxdiv(int2fx(game->pieceCounter),(t));
    }

    std::string str = "";

    str += std::to_string(fx2int(pps)) + ".";

    int fractional = pps & 0xff;
    for(int i = 0; i < 2; i++){
        fractional *= 10;
        str += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

	memset32(&tile_mem[2][113],0,8*3);
    aprints(str,25,0,2);

    aprints("PPS:",0,0,2);
}

void showFinesse(){
    if(gameSeconds == 0)
        return;
    aprints(std::to_string(game->finesseFaults),4*9,7,2);
}

void showClearText() {

    if(game->gameMode != CLASSIC && !game->zoneTimer){
        if (game->comboCounter > 1) {
            aprint("Combo x", 21, clearTextHeight - 1);

            aprintf(game->comboCounter, 28, clearTextHeight - 1);
        } else {
            // aprint("          ", 20, clearTextHeight - 1);
            aprintClearArea(20, clearTextHeight - 1, 10, 1);
        }

        if (game->b2bCounter > 0) {
            aprint("Streak", 22, clearTextHeight + 1);

            aprint("x", 24, clearTextHeight + 2);
            aprintf(game->b2bCounter + 1, 25, clearTextHeight + 2);
        } else {
            // aprint("          ", 20, clearTextHeight + 1);
            // aprint("          ", 20, clearTextHeight + 2);
            aprintClearArea(20, clearTextHeight + 1, 10, 2);
        }
    }

    if(game->zoneTimer && !game->lost)
        return;

    std::list<FloatText>::iterator index = floatingList.begin();

    for (int i = 0; i < (int)floatingList.size(); i++) {
        std::string text = (*index).text;
        if (index->timer++ > maxClearTextTimer) {
            index = floatingList.erase(index);
            // aprint("            ", 9, 0);
            aprintClearArea(9, 0, 12, 1);
        } else {
            int height = 0;
            if (index->timer < 2 * maxClearTextTimer / 3)
                height = 5 * (float)index->timer / ((float)2 * maxClearTextTimer / 3);
            else
                height = (30 * (float)(index->timer) / maxClearTextTimer) - 15;
            if (text.size() <= 10) {
                aprint(text, 15 - text.size() / 2, 15 - height);
            } else {

                std::size_t pos = text.find(" ");

                if(pos != std::string::npos){
                    // aprint("            ", 9, 15 - height);
                    aprintClearArea(9, 15 - height, 12, 1);
                    std::string part1 = text.substr(0, pos);
                    std::string part2 = text.substr(pos + 1);

                    if (15 - height - 1 > 0)
                        aprint(part1, 15 - part1.size() / 2, 15 - height - 1);
                    aprint(part2, 15 - part2.size() / 2, 15 - height);
                }else{
                    aprint(text, 15 - text.size() / 2, 15 - height);
                }
            }
            // aprint("            ", 9, 15 - height + 1);
            aprintClearArea(9, 15 - height + 1, 12, 1);
            ++index;
        }
    }
}

void gameLoop(){
    GameScene* s = new GameScene();
    changeScene(s);

    setGradient(savefile->settings.backgroundGradient);
    gradient(true);

    VBlankIntrWait();
    setSkin();

    memset32(&tile_mem[2][110], 0, 400*8);
    setSmallTextArea(110, 3, 7, 9, 10);
    clearText();
    gameSeconds = 0;

    memcpy16(&tile_mem[4][256+3],meterTiles[0],meter1_tiles_bin_size/2);

    update();

    oam_init(obj_buffer, 128);
    drawFrame();

    mmStop();

    showHold();
    showQueue();

    if(game->gameMode == MASTER)
        showSpeedMeter((int)game->speed);

    showZoneMeter();

    oam_copy(oam_mem, obj_buffer, 128);

	if (!resumeJourney){
		countdown();
	}
	else{
		resumeJourney = false;
	}

    if (!(game->gameMode == TRAINING)) {
        playSongRandom(1);
    }

    if(game->gameMode == MASTER){
        maxClearTimer = game->maxClearDelay;
    }else if(proMode && game->rotationSystem == SRS){
        maxClearTimer = 1;
        game->maxClearDelay = 1;
    }else{
        maxClearTimer = 20;
        game->maxClearDelay = 20;
    }

    update();

    while (1) {
        diagnose();
        if (!game->lost && !pause && !game->eventLock) {
            // profile_start();
            game->update();
            // log(std::to_string(profile_stop()));
        }
        handleMultiplayer();

        progressBar();

        // if(ENABLE_BOT){
        //     profile_start();
        //     // testBot->run();
        //     botGame->update();
        //     handleBotGame();
        //     addToResults(profile_stop(),0);
        // }

        if(creditRefresh && !game->clearLock)
            refreshCredits();

        gameSeconds = game->timer;

        if (game->clearLock && !(game->eventLock && game->gameMode != MASTER)) {
            clearTimer++;

            if(clearTimer >= 100){
                game->removeClearLock();
                shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
                rumbleTimer = rumbleMax * 2 * savefile->settings.rumble;
                clearTimer = 0;
                update();
            }
        }

        if(game->eventLock){
            if(eventPauseTimer == 0){
                if(game->gameMode == MASTER){
                    showBackground();
                    eventPauseTimer = eventPauseTimerMax;
                    setupCredits();
                    flashTimer = flashTimerMax;
                    gradient(false);
                } else if(game->gameMode == MARATHON){
                    flashTimer = flashTimerMax;
                    eventPauseTimer = flashTimerMax + 2;
                }
            }

            eventPauseTimer--;

            if(eventPauseTimer == 0)
                game->removeEventLock();
        }

        Tetris::Drop latestDrop = game->getDrop();

        if (latestDrop.on){
            addGlow(latestDrop);
            addPlaceEffect(latestDrop);
        }

        canDraw = true;
        VBlankIntrWait();

        rumble_update();
        if (clearTimer >= maxClearTimer || maxClearTimer <= 0 || (game->gameMode == SURVIVAL && clearTimer)) {
            game->removeClearLock();
            shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
            rumbleTimer = rumbleMax * 2 * savefile->settings.rumble;
            clearTimer = 0;
            update();
        }

        if(rumbleTimer > 0){
            rumbleTimer--;

            rumble_set_state(rumble_start);
        }else{
            rumble_set_state(rumble_hard_stop);
        }

        if ((game->won || game->lost) && !(flashTimer || eventPauseTimer)){
            endScreen();
            if(!(playAgain && multiplayer))
                return;
            else
                playAgain = false;
        }

        if (pause){

            if(pauseMenu()){
                pause = false;
                return;
            }
            liftKeys();
        }

        if (playAgain) {
            return;
        }

        if(game->zoneTimer){
            if(gameSeconds % 4 == 0){
                frameSnow();
            }

            if(!flashTimer)
                rainbowPalette();

        }

        if(game->gameMode == MARATHON && game->subMode && gameSeconds % 2 == 0){
            if(rainbowIncreasing)
                rainbowTimer++;
            else
                rainbowTimer--;

            if(rainbowTimer >= (32 * 2) - 1 || rainbowTimer == 0)
                rainbowIncreasing = !rainbowIncreasing;

            showZoneMeter();
        }

        if(game->eventTimer){
            showCredits();
        }

        if(flashTimer)
            zoneFlash();

        sqran(qran() % frameCounter);
		
		if(!game->eventLock && journeyLevelUp){
			// flashTimer = flashTimerMax;
			// journeyFlash();
			
			setJourneyGraphics(savefile, game->level);

			delete journeySave;
			journeySave = new Game(*game);
			journeySaveExists = true;
			return;
		}
    }
}

void addGlow(Tetris::Drop location) {
    for (int i = 0; i < location.endY && i < 20; i++)
        for (int j = location.startX; j < location.endX; j++)
            glow[i][j] = glowDuration;

    if (game->comboCounter > 0 || game->zoneTimer) {
        int xCenter = (location.endX - location.startX) / 2 + location.startX;
        if (game->previousClear.isTSpin) {
            for (int i = 0; i < 20; i++)
                for (int j = 0; j < 10; j++)
                    glow[i][j] = glowDuration + abs(xCenter - j) + abs(location.endY - i);
        } else {
            for (int i = 0; i < 20; i++)
                for (int j = 0; j < 10; j++)
                    glow[i][j] = glowDuration + Sqrt(abs(xCenter - j) * abs(xCenter - j) + abs(location.endY - i) * abs(location.endY - i));
        }

        if (game->previousClear.isBackToBack == 1)
            effectList.push_back(Effect(1 + (game->previousClear.isTSpin != 0), xCenter, location.endY));
    }
}

void clearGlow() {
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            glow[i][j] = 0;
    drawGrid();
}

void countdown() {
    int timer = 0;
    int timerMax = 120;
    showQueue();
    showHold();
    oam_copy(oam_mem, obj_buffer, 128);

    while (timer++ < timerMax - 1) {
        VBlankIntrWait();

        if (timer < timerMax / 3) {
            aprint("READY?", 12, 10);
            if (timer == 1 && savefile->settings.announcer){
                sfx(SFX_READY);
            }
        } else if (timer < 2 * timerMax / 3){
            key_poll();
            aprint("READY?", 12, 10);
            if(key_is_down(KEY_L) && key_is_down(KEY_R)){
                // restart = true;
                playAgain = true;
                break;
            }
        }else {
            aprint("  GO  ", 12, 10);
            if (timer == 2 * timerMax / 3 && savefile->settings.announcer)
                sfx(SFX_GO);
        }
    }
    showBackground();
    clearText();

    if(key_is_down(KEY_LEFT)){
        game->keyLeft(1);
    }else if(key_is_down(KEY_RIGHT)){
        game->keyRight(1);
    }
}

void screenShake() {
    if (!savefile->settings.shake)
        return;

    REG_BG0VOFS = -shake;

    REG_BG1VOFS = -shake;

    if (shake) {
        if (shake > 0)
            shake--;
        else
            shake++;
        shake *= -1;
    }

    REG_BG0HOFS = -push;
    REG_BG1HOFS = -push;

    if (game->pushDir != 0) {
        if (abs(push) < pushMax * (savefile->settings.shakeAmount)/4)
            push += game->pushDir * (1 + (savefile->settings.shakeAmount > 2));
    } else {
        if (push > 0)
            push--;
        else if (push < 0)
            push++;
    }
}

void drawFrame() {
    u16* dest = (u16*)se_mem[26];

    dest += 9;

    int color = 8 * 0x1000;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 2; j++) {
            *dest++ = 0x0004 + j * 0x400 + color;
            dest += 10;
        }
        dest += 32 - 22;
    }

    progressBar();

    for (int i = 0; i < 3; i++) {
        obj_unhide(queueFrameSprites[i], 0);
        obj_set_attr(queueFrameSprites[i], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_BUILD(512 + 16 + 16 * i, color, 0));
        obj_set_pos(queueFrameSprites[i], 173, 12 + 32 * i);
    }

    drawGrid();
}

void drawGrid() {
    std::list<Effect>::iterator index = effectList.begin();

    for (int i = 0; i < (int)effectList.size() && (savefile->settings.effects); i++) {
        if (index->timer < index->duration) {
            switch (index->type) {
            case 0:
                if (index->timer % glowDuration == 0) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            glow[i][j] = glowDuration;
                }
                break;
            case 1:
                if (index->timer == index->duration - 1) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            if (glow[i][j] < glowDuration)
                                glow[i][j] = glowDuration + Sqrt(abs(index->x - j) * abs(index->x - j) + abs(index->y - i) * abs(index->y - i));
                }
                break;
            case 2:
                if (index->timer == index->duration - 1) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            if (glow[i][j] < glowDuration)
                                glow[i][j] = glowDuration + abs(index->x - j) + abs(index->y - i);
                }
                break;
            case 3:
                if (index->timer == 0) {
                    for (int i = 0; i < 20; i++)
                        for (int j = 0; j < 10; j++)
                            glow[i][j] = glowDuration + abs(index->x - j) + abs(index->y - i);
                }
                break;
            }

            index->timer++;
        } else {
            index = effectList.erase(index);
        }

        std::advance(index, 1);
    }

    u32 gridTile;
    int palOffset = 4;

    switch (savefile->settings.backgroundGrid) {
        case 0: gridTile = 0x0002; break;
        case 1: gridTile = 0x000c; break;
        case 2: gridTile = 0x001a; break;
        case 3: gridTile = 0x001e; break;
        case 4: gridTile = 0x001f; break;
        case 5: gridTile = 0x0020; break;
        default: gridTile = 0x0002; break;
    }

    u16* dest = (u16*)se_mem[26];

    memset16(&dest[31*32+10],gridTile,10);
    memset16(&dest[20*32+10],gridTile,10);

    dest += 10;

    if (savefile->settings.lightMode && !game->zoneTimer)
        gridTile += palOffset * 0x1000;

    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 10; j++) {
            if (glow[i][j] == 0 || !savefile->settings.effects) {
                *dest++ = gridTile;
            } else if (glow[i][j] > glowDuration) {
                glow[i][j]--;
                *dest++ = gridTile;
            } else if (glow[i][j] > 0) {
                glow[i][j]--;
                int color = 0;
                if (glow[i][j] >= glowDuration * 3 / 4)
                    color = 3;
                else if (glow[i][j] >= glowDuration * 2 / 4)
                    color = 2;
                else if (glow[i][j] >= glowDuration * 1 / 4)
                    color = 1;
                *dest++ = gridTile + color * 0x1000;
            }
        }
        dest += 22;
    }
}

void progressBar() {
    if (game->goal == 0 || game->zoneTimer)
        return;

    int current;
    int max = game->goal;

    if (game->gameMode == DIG)
        current = game->garbageCleared;
    else if (game->gameMode == ULTRA || game->gameMode == BLITZ)
        current = game->timer;
    else if (game->gameMode == SPRINT && game->subMode)
        current = game->linesSent;
    else if (game->gameMode != SURVIVAL)
        current = game->linesCleared;
    else
        return;

    if (game->gameMode != BATTLE) {
        showBar(current, max, 20, 8);
    }else{
        if (++attackFlashTimer > attackFlashMax)
            attackFlashTimer = 0;

        if (attackFlashTimer < attackFlashMax / 2) {
            memset32(&pal_bg_mem[8 * 16 + 5], 0x421f, 1);
        } else {
            memset32(&pal_bg_mem[8 * 16 + 5], 0x7fff, 1);
        }

        //attack bar
        showBar(game->getIncomingGarbage(), 20, 9, 8);
    }
}

void showBar(int current, int max, int x, int palette) {
    palette *= 0x1000;

    if (max > 10000) {
        current /= 10;
        max /= 10;
    }

    int pixels = fx2int(fxmul(fxdiv(int2fx(current), int2fx(max)), int2fx(158)));
    int segments = fx2int(fxdiv(int2fx(current), fxdiv(int2fx(max), int2fx(20))));
    int edge = pixels - segments * 8 + 1 * (segments != 0);

    u16* dest = (u16*)se_mem[26];

    dest += x;
    for (int i = 0; i < 20; i++) {

        if (i == 0) {
            if (segments == 19) {
                if (edge == 0)
                    *dest = 0x0006 + palette;
                else {
                    *dest = 0x000a + palette;
                    for (int j = 0; j < 7; j++) {
                        TILE* tile = &tile_mem[0][10];
                        if (j == 0) {
                            if (6 - j > edge)
                                tile->data[j + 1] = 0x13300332;
                            else
                                tile->data[j + 1] = 0x13344332;
                        } else {
                            if (6 - j > edge)
                                tile->data[j + 1] = 0x13000032;
                            else
                                tile->data[j + 1] = 0x13444432;
                        }
                    }
                }
            } else if (segments >= 20)
                *dest = 0x0008 + palette;
            else
                *dest = 0x0006 + palette;

        } else if (i == 19) {
            if (segments == 0) {
                if (edge == 0)
                    *dest = 0x0806 + palette;
                else {
                    *dest = 0x080a + palette;
                    for (int j = 0; j < 7; j++) {
                        TILE* tile = &tile_mem[0][10];
                        if (j == 0) {
                            if (j >= edge)
                                tile->data[j + 1] = 0x13300332;
                            else
                                tile->data[j + 1] = 0x13344332;
                        } else {
                            if (j >= edge)
                                tile->data[j + 1] = 0x13000032;
                            else
                                tile->data[j + 1] = 0x13444432;
                        }
                    }
                }
            } else
                *dest = 0x0808 + palette;
        } else {
            if (19 - i > segments) {
                *dest = 0x0007 + palette;
            } else if (19 - i == segments) {
                *dest = 0x000b + palette;
                for (int j = 0; j < 8; j++) {
                    TILE* tile = &tile_mem[0][11];
                    if (7 - j > edge) {
                        tile->data[j] = 0x13000032;
                    } else {
                        tile->data[j] = 0x13444432;
                    }
                }
            } else {
                *dest = 0x0009 + palette;
            }
        }

        dest += 32;
    }
}

void showBestMove(){
    std::list<int> moveList = game->previousBest;

    if((int) moveList.size() > 1)
        moveList.pop_back();

    for(int i = 0; i < 3; i++)
        moveSprites[i] = &obj_buffer[16+i];

    auto move = moveList.begin();

    for(int i = 0; i < 3; i++){
        if(i > (int) moveList.size() - 1){
            obj_hide(moveSprites[i]);
            continue;
        }
        obj_unhide(moveSprites[i],0);

        obj_set_attr(moveSprites[i], 0, 0, ATTR2_BUILD(512 + 128 + *move, 15, 2));

        obj_set_pos(moveSprites[i], 38 + 12 * i, 54);

        ++move;
    }
}

bool checkDiagonal(int key){
    if(!savefile->settings.diagonalType && game->rotationSystem != ARS)
        return false;
    return ((key == KEY_DOWN || key == KEY_UP) && (key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)));
}

void showPlaceEffect(){

    const int size = 384;

    for(int i = 0; i < 3; i++)
        obj_hide(&obj_buffer[19+i]);

    auto it = placeEffectList.begin();
    for(int i = 0; it != placeEffectList.end(); i++){
        if(it->timer == 0){
            obj_hide(it->sprite);
            placeEffectList.erase(it++);
            i--;
            continue;
        }

        bool flip = false;
        int xoffset = 0;
        int yoffset = 0;
        int r = it->rotation;

        int n = 2 - ((it->timer-1)/4);

        switch(it->piece){
        case 0:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n],size);
            if(game->rotationSystem == NRS){
                if(r % 2 == 1)
                    r = 1;
                else
                    r = 2;
            }else if(game->rotationSystem == ARS){
                if(r % 2 == 1)
                    r = 1;
                else
                    r = 0;
            }
            break;
        case 1:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+3],size);
            xoffset = yoffset = -4;

            if (game->rotationSystem == ARS && r == 2)
                yoffset += 8;

            if(game->rotationSystem == NRS || game->rotationSystem == ARS)
                r = (r+2) % 4;

            break;
        case 2:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+3],size);
            xoffset = yoffset = -4;
            flip = true;

            if (game->rotationSystem == ARS && r == 2)
                yoffset += 8;

            if(game->rotationSystem == NRS || game-> rotationSystem == ARS)
                r = (r+2) % 4;
            break;
        case 3:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+6],size);
            yoffset = -4 + (game->rotationSystem == NRS || game-> rotationSystem == ARS) * 8;
            r = 0;
            break;
        case 4:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+9],size);
            xoffset = -4;
            yoffset = -4;
            if(game->rotationSystem == NRS || game-> rotationSystem == ARS){
                if(r % 2 == 1)
                    r = 1 + 2 * (game->rotationSystem == ARS);
                else
                    r = 2;
            }
            break;
        case 5:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+12],size);
            xoffset = yoffset = -4;

            if (game->rotationSystem == ARS && r == 2)
                yoffset += 8;

            if(game->rotationSystem == NRS || game-> rotationSystem == ARS)
                r = (r+2) % 4;
            break;
        case 6:
            memcpy16(&tile_mem[5][138 + 32 * i],placeEffectTiles[n+9],size);
            xoffset = -4;
            yoffset = -4;
            flip = true;
            if(game->rotationSystem == NRS || game-> rotationSystem == ARS){
                if(r % 2 == 1)
                    r = 1;
                else
                    r = 2;
            }
            break;
        default:
            break;
        }

        if(!flip){
            if((r == 1 || r == 2))
                xoffset += -1;
            if(r > 1)
                yoffset += -1;
        }else{
            if((r == 1 || r == 2))
                yoffset += -1;
            if(r < 2)
                xoffset += -1;
        }

        FIXED spin = 0;
        int x,y;

        if(it->timer > 5 && it->piece != 3){
            spin = 0x4000 * (it->rotating) * ((it->timer-6));
            spin /= 12;

            if(it->dx || it->dy){
                int mix = 255 * (6-(it->timer/2));

                x = fx2int(lerp(int2fx(it->x - (it->dx*2)), int2fx(it->x), mix/6));
                y = fx2int(lerp(int2fx(it->y - (it->dy*2)), int2fx(it->y), mix/6));
            }else{
                x = it->x;
                y = it->y;
            }
        }else{
            x = it->x;
            y = it->y;
        }

        x += xoffset + push * savefile->settings.shake;
        y += yoffset - shake * savefile->settings.shake;

        it->sprite = &obj_buffer[19+i];
        obj_unhide(it->sprite,ATTR0_AFF_DBL);
        obj_set_attr(it->sprite, ATTR0_WIDE | ATTR0_AFF_DBL, ATTR1_SIZE(3) | ATTR1_AFF_ID(7+i), ATTR2_BUILD(650 + 32 * i, (game->zoneTimer != 0)?11:it->piece, 3 - (it->rotating != 0)));
        obj_set_pos(it->sprite, x, y);
        obj_aff_identity(&obj_aff_buffer[7+i]);
        obj_aff_rotscale(&obj_aff_buffer[7+i], ((flip)?-1:1) << 8, 1<<8, - 0x4000 * (r) + spin);

        it->timer--;
        it++;
    }
}

void addPlaceEffect(Tetris::Drop drop){
    if((int)placeEffectList.size() >= 3 || !savefile->settings.placeEffect || game->pawn.big)
        return;

    placeEffectList.push_back(PlaceEffect(drop.x, drop.y, drop.dx, drop.dy, drop.piece, drop.rotation, drop.rotating * (game->gameMode != CLASSIC)));
}

void hideMinos(){
    obj_hide(pawnSprite);
    obj_hide(pawnShadow);
    obj_hide(holdSprite);

    for(int i = 0; i < 5; i++)
        obj_hide(queueSprites[i]);

    for(int i = 0; i < 3; i++)
        obj_hide(&obj_buffer[19+i]);

    obj_hide(&obj_buffer[22]);
}

void showSpeedMeter(int fill){
    const int maxLength = 20;

    fill *= 2;

    OBJ_ATTR * sprite = &obj_buffer[22];

    //set palette
    memset16(&pal_obj_mem[13*16+1],0x03e0,1); //green
    memset16(&pal_obj_mem[13*16+2],0x001f,1); //red
    memset16(&pal_obj_mem[13*16+3],0x4a52,1); //gray

    TILE *dest;
    for(int x = 0; x < 4; x++){
        dest = (TILE *) &tile_mem[4][256 + x];
        for(int y = 0; y < 8; y++){
            dest->data[y] = 0;
        }
    }

    if(fill > maxLength)
        fill = maxLength;

    for(int x = 0; x < maxLength+1; x++){
        dest = (TILE *) &tile_mem[4][256 + x / 8];
        int shift = ((x%8) * 4);
        int c = (1 + (x < fill)) << shift;
        if(x < maxLength){
            dest->data[0] |= c;
            if(x > 0)
                dest->data[1] |= 3 << shift;
        }else{
            dest->data[1] |= 3 << shift;
        }
    }

    obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(1), ATTR2_BUILD(256, 13, 0));
    obj_set_pos(sprite,35, 139);
    obj_unhide(sprite, 0);

}

void disappear(){
    std::tuple<u8,u8> coords;
    u16* dest = (u16*)se_mem[25];

    bool found = false;

    while(!game->toDisappear.empty()){
        coords = game->toDisappear.front();

        int x = std::get<0>(coords);
        int y = std::get<1>(coords);

        dest[(y-20)*32 + x + 10] = 0;

        found = true;

        game->toDisappear.pop_front();
    }

    if(found){
        showBackground();
    }
}

INLINE int getBoard(int x, int y){
    if(game->disappearTimers[y][x] == 1)
        return 0;

    return game->board[y][x];
}

void showZoneMeter(){
    if(!(game->gameMode == MARATHON && subMode))
        return;

    OBJ_ATTR * sprite = &obj_buffer[23];

    obj_set_attr(sprite, ATTR0_SQUARE | ATTR0_MOSAIC, ATTR1_SIZE(2),ATTR2_BUILD(256 + 3, 15, 0));
    obj_set_pos(sprite, 14 + (push < 0) * push, 74);
    obj_unhide(sprite,0);

    const int color = 0x7fff;
    const int anticolor = 0x0c63;
    const int disabled = 0x5294;

    int n;
    if(!game->zoneTimer){
        n = game->zoneCharge / 3;
        if(game->zoneCharge == 32)
            n = 12;
    }else{
        n = game->zoneTimer / 100 + 1;
    }

    for(int i = 0; i < 12; i++){
        if(i < n && n <= 2 && !game->zoneTimer)
            memset16(&pal_obj_mem[15*16 + 4 + i], disabled , 1);
        else if (i < n)
            memset16(&pal_obj_mem[15*16 + 4 + i], color , 1);
        else
            memset16(&pal_obj_mem[15*16 + 4 + i], anticolor , 1);
    }

}

void zoneFlash(){

    if(!eventPauseTimer && game->gameMode == MARATHON)
        rainbowPalette();

    if(flashTimer == flashTimerMax){
        if(previousPalette != nullptr)
            delete previousPalette;
        previousPalette = new COLOR [512];
        memcpy32(&previousPalette[0], pal_bg_mem, 256);
    }

    flashTimer--;

    REG_MOSAIC = MOS_BUILD(flashTimer,flashTimer,flashTimer,flashTimer);

    int n = ((float)flashTimer/flashTimerMax) * 31;

    clr_fade_fast(previousPalette, 0x7fff, pal_obj_mem, 128, n);

    bool cond = ((flashTimer < flashTimerMax/2) && eventPauseTimer);

    if(cond && game->gameMode == MARATHON)
        gradient(true);

    memcpy16(&pal_bg_mem[cond],&pal_obj_mem[cond],(8 * 16));

    if(flashTimer < flashTimerMax/2 && game->gameMode == MASTER){
        if(!savefile->settings.lightMode)
            memset16(pal_bg_mem, 0x0000, 1);
        else
            memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    }
}

void resetZonePalette(){
    if(!holdingSave)
        return;

    holdingSave = false;

    savefile->settings = previousSettings;
    setPalette();
    setClearEffect();

    mmSetModuleTempo(1024);
    mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));

    memcpy16(&tile_mem[0][4],sprite5tiles_bin,16);

    if(game->lost)
        return;

    u16 * dest = (u16*) &se_mem[26];
    for(int i = 0; i < 20; i++){
        dest[i * 32 + 9] = 4 + 0x8000;
        dest[i * 32 + 20] = 4 + 0x8400;
    }

    if(!eventPauseTimer)
        flashTimer = 0;
}

void showFinesseCombo(){
    OBJ_ATTR * sprite = &obj_buffer[24];

    if(game->rotationSystem != SRS || game->finesseStreak < 3){
        obj_hide(sprite);
        return;
    }

    obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(0),ATTR2_BUILD(275, 15, 0));
    obj_set_pos(sprite, 52, 104);
    obj_unhide(sprite,0);

    memset32(&tile_mem[4][275],0x0000, 8 * 2);

    std::string text = "x" + std::to_string(game->finesseStreak);
    aprintsSprite(text,0,0,275);
}

void frameSnow(){
    const int fillAmount = 20;

    TILE * t = (TILE *) sprite5tiles_bin;
    TILE * dest = (TILE *) &tile_mem[0][4];
    TILE * topTile = (TILE *) &tile_mem[0][46];

    for(int i = 0; i < 8; i++){
        int add1 = 0;
        int add2 = 0;
        for(int j = 0; j < 4; j++){
            add1 += ((qran () % 100 < fillAmount) * 0xf) << ((j+4)*4);
            add2 += (((qran () % 100) < ((10-i) * 10) ) * 0xf) << ((j+4)*4);
        }

        dest->data[i] = t->data[i] & ~add1;
        topTile->data[i] = t->data[i] & ~add2;
    }

    u16 * dest2 = (u16 *) &se_mem[26];

    int i;
    for(i = 0; i < 20 - ((20 * game->zoneTimer) / game->zoneStart); i++){
        dest2[i * 32 + 9] = 0;
        dest2[i * 32 + 20] = 0;
    }

    if(i == 20)
        i = 19;

    dest2[i * 32 + 9] = 46;
    dest2[i * 32 + 20] = 46 + 0x400;

}

void rainbowPalette(){
    int n = (rainbowTimer >> 3) + 24;

    int color = 0;
    if(!savefile->settings.lightMode)
        color = RGB15(31, n, n);
    else
        color = RGB15(n, 31, 31);

    clr_rgbscale((COLOR *) rainbow,(COLOR *)&palette[0][1],5,color);

    if(savefile->settings.lightMode){
        memcpy16(&pal_obj_mem[1],rainbow,1);
        memcpy16(&pal_obj_mem[2],rainbow,4);
    }

    if(!savefile->settings.lightMode)
        clr_fade((COLOR *) rainbow,0,(COLOR*) rainbow,4,10);
    else
        clr_adj_brightness((COLOR *) rainbow,(COLOR*) rainbow,4,float2fx(0.25));

        // clr_adj_brightness((COLOR *) rainbow,(COLOR*) rainbow,4,float2fx(0.5));
        // clr_fade((COLOR *) rainbow,0,(COLOR*) rainbow,4,22);

    memcpy16(&pal_bg_mem[1],rainbow,4);

    if(!savefile->settings.lightMode)
        memcpy16(&pal_obj_mem[1],rainbow,4);

    if(savefile->settings.shadow != 3){
        memcpy16(&pal_obj_mem[10*16+1],rainbow,4);
    }else{
        if(!savefile->settings.lightMode)
            clr_fade((COLOR*)rainbow, 0x0000, &pal_obj_mem[10 * 16+1], 4, 14);
        else
            clr_adj_brightness(&pal_obj_mem[10 * 16+1], (COLOR*)rainbow, 4, float2fx(0.25));
    }
    memcpy16(&pal_obj_mem[11*16+1],rainbow,4);
}

void showZoneText(){
    if(game->zonedLines == 0 || !savefile->settings.floatText)
        return;

    aprintClearArea(10, 0, 10, 20);

    int height = (game->lengthY - game->zonedLines) / 2;

    char buff[4];

    posprintf(buff, "%d",game->zonedLines);

    std::string text = buff;

    text += " line";

    if(game->zonedLines > 1)
        text+= "s";

    const u16 pal[2][2] = {{0x5294,0x0421},{0x5294,0x318c}};

    if(!savefile->settings.lightMode)
        memcpy16(&pal_bg_mem[13 * 16 + 2],pal[0],2);
    else
        memcpy16(&pal_bg_mem[13 * 16 + 2 ],pal[1],2);

    aprintColor(text, 15 - text.size()/2, height , 2);
}

static FIXED creditCurrentHeight = int2fx(SCREEN_HEIGHT);
static s8 creditIndex = 0;
static u8 creditLastRead = 0;

const FIXED creditSpeed = float2fx(0.6);
const int creditSpace = 200;

void setupCredits(){
    creditCurrentHeight = int2fx(SCREEN_HEIGHT);
    creditIndex = 0;
    creditLastRead = 0;

    memset32(&tile_mem[4][256], 0x0000 , MAX_WORD_SPRITES * 12 * 8);
    for (int i = 0; i < MAX_WORD_SPRITES; i++){
        delete wordSprites[i];
        wordSprites[i] = new WordSprite(i,64 + i * 3, 300 + i * 12);
    }

    for(int i = 0; i < 2; i++){
        auto index = GameInfo::credits.begin();
        std::advance(index,creditLastRead++);

        std::string text = *index;
        std::size_t pos = text.find(" ");

        if(pos != std::string::npos){
            // aprint("            ", 9, 15 - height);
            std::string part1 = text.substr(0, pos);
            std::string part2 = text.substr(pos + 1);

            wordSprites[i*3+0]->setText(part1);
            wordSprites[i*3+1]->setText(part2);
        }else{
            wordSprites[i*3+1]->setText(text);
        }

        wordSprites[i*3+2]->setText("akouzoukos");
    }

    for(int i = 0; i < 9; i++){
        wordSprites[i]->priority = 3;
    }
}

void showCredits(){

    creditCurrentHeight -= creditSpeed;

    int height = fx2int(creditCurrentHeight);

    // profile_start();
    for(int i = 0; i < 3; i++){
        int yconst = height + creditSpace * i;
        int id = ((i+creditIndex)%3)*3;
        for(int j = 0; j < 3; j++){
            int y = 8 * (j + (j == 2)) + yconst;
            if(y > SCREEN_HEIGHT || y < -8){
                wordSprites[id + j]->hide();
            }else{
                wordSprites[id + j]->show(10 * 8, y, 15);
            }
        }
    }
    // log(std::to_string(profile_stop()));

    if(fx2int(creditCurrentHeight) <= -32)
        creditRefresh = true;
}

void refreshCredits(){
    creditRefresh = false;

    creditIndex++;

    creditCurrentHeight += int2fx(creditSpace);

    if(creditIndex >= 3)
        creditIndex = 0;

    auto index = GameInfo::credits.begin();
    std::advance(index,creditLastRead);

    std::string text;
    if(creditLastRead >= 10)
        text = "";
    else{
        text = *index;
        creditLastRead++;
    }

    std::size_t pos = text.find(" ");

    if(pos != std::string::npos){
        std::string part1 = text.substr(0, pos);
        std::string part2 = text.substr(pos + 1);


        wordSprites[creditIndex*3+0]->setText(part1);
        wordSprites[creditIndex*3+1]->setText(part2);
    }else{
        wordSprites[creditIndex*3+0]->setText("");
        wordSprites[creditIndex*3+1]->setText(text);
    }

    if(creditLastRead < 11)
        wordSprites[creditIndex*3+2]->setText("akouzoukos");
    else
        wordSprites[creditIndex*3+2]->setText("");

}

void showFullMeter(){
    if(game->zoneCharge != 32){
        if(fullMeterTimer < fullMeterAnimationLength){
            fullMeterTimer = 0;
            memcpy32(&tile_mem[4][256+3],meterTiles[0],meter1_tiles_bin_size/4);
        }
        return;
    }

    if(--fullMeterTimer <= 0){
        fullMeterTimer = fullMeterTimerMax;
        memcpy32(&tile_mem[4][256+3],meterTiles[0],meter1_tiles_bin_size/4);
        return;
    }

    if(fullMeterTimer < fullMeterAnimationLength){
        int n = 2 - (fullMeterTimer/(fullMeterAnimationLength/3));
        memcpy32(&tile_mem[4][256+3],meterTiles[n+1],meter1_tiles_bin_size/4);
    }
}

const Function gameFunctions[9] ={
    {&Game::keyLeft},
    {&Game::keyRight},
    {&Game::rotateCW},
    {&Game::rotateCCW},
    {&Game::rotateTwice},
    {&Game::keyDown},
    {&Game::keyDrop},
    {&Game::hold},
    {&Game::activateZone},
};

Function getActionFromKey(int key){
    int *keys = (int *) &savefile->settings.keys;

    int k = 0;
    for(int i = 0; i < 9; i++){
        if(keys[i] == key){
            k = i;
            break;
        }
    }

    return gameFunctions[k];
}

void liftKeys(){
    game->liftKeys();
}

void journeyFlash(){

    if(flashTimer == flashTimerMax){
        if(previousPalette != nullptr)
            delete previousPalette;
        previousPalette = new COLOR [512];
        memcpy32(&previousPalette[0], pal_bg_mem, 256);
    }

    flashTimer--;

    REG_MOSAIC = MOS_BUILD(flashTimer,flashTimer,flashTimer,flashTimer);

    int n = ((float)flashTimer/flashTimerMax) * 31;

    clr_fade_fast(previousPalette, 0x7fff, pal_obj_mem, 128, n);

    bool cond = ((flashTimer < flashTimerMax/2) && eventPauseTimer);

    if(cond && game->gameMode == MARATHON)
        gradient(true);

    memcpy16(&pal_bg_mem[cond],&pal_obj_mem[cond],(8 * 16));

    if(flashTimer < flashTimerMax/2 && game->gameMode == MASTER){
        if(!savefile->settings.lightMode)
            memset16(pal_bg_mem, 0x0000, 1);
        else
            memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    }
}

void setJourneyGraphics(Save *save, int level){
	switch (level){
		case 1:
			save->settings.backgroundGradient = 0x7dc8;
			save->settings.backgroundGrid = 5;
			save->settings.skin = 11;
			save->settings.shadow = 3;
			save->settings.palette = 5;
			save->settings.colors = 1;
			save->settings.edges = true;
			save->settings.lightMode = false;
			break;
		case 2:
			save->settings.backgroundGradient = RGB15(0,0,0);
			save->settings.backgroundGrid = 0;
			save->settings.skin = 0;
			save->settings.shadow = 3;
			save->settings.palette = 0;
			save->settings.colors = 1;
			save->settings.edges = false;
			save->settings.lightMode = false;
			break;
		case 3:
			save->settings.backgroundGradient = RGB15(0,0,0);
			save->settings.backgroundGrid = 0;
			save->settings.skin = 0;
			save->settings.shadow = 3;
			save->settings.palette = 0;
			save->settings.colors = 4;
			save->settings.edges = false;
			save->settings.lightMode = false;
			break;
		default:
			save->settings.edges = true;
			save->settings.backgroundGrid = qran() % (MAX_BACKGROUNDS - 1);
			save->settings.skin = qran() % (MAX_SKINS - 1);
			save->settings.palette = qran() % 6;
			save->settings.shadow = qran() % (MAX_SHADOWS - 1);
			save->settings.lightMode = (qran() % 2 == 0);
			save->settings.colors = qran() % (MAX_COLORS - 1);
			save->settings.backgroundGradient = RGB15(qran() % 31, qran() % 31, qran() % 31);
			break;
	}
}
