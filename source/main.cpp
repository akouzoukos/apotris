#include <tonc.h>
#include <tonc_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>
#include <maxmod.h>

#include "def.h"
#include "tetrisEngine.h"
#include "sprites.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "LinkConnection.h"

#include "logging.hpp"
#include "tonc_core.h"

using namespace Tetris;

void control();
void showText();
void showTimer();
void update();
void showClearText();
void startScreen();
void fallingBlocks();
void endScreen();
void endAnimation();
void showStats();
void reset();
void loadSave();
void saveToSram();
void loadFromSram();
std::string timeToString(int);
void diagnose();
void progressBar();
void showBar(int, int, int, int);
void showFrames();
void clearGlow();
void handleMultiplayer();
void startMultiplayerGame(int);
void setSkin();
void showTitleSprites();
void drawEnemyBoard(int);
void handleBotGame();
void sleep();
void drawUIFrame(int, int, int, int);
void setLightMode();

LinkConnection* linkConnection = new LinkConnection();

int frameCounter = 1;

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE* obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;

int shakeMax = 10;
int shake = 0;

int gameSeconds;

bool pause = false;

int marathonClearTimer = 20;

u32 highscore = 0;
int bestTime = 0;
int initialLevel = 0;

int canDraw = 0;

int profileResults[10];

void addToResults(int input, int index);

Save* savefile;

u8* blockSprite;

bool restart = false;

bool playAgain = false;
int nextSeed = 0;

int attackFlashTimer = 0;
int attackFlashMax = 10;

bool multiplayer = false;
bool playingBotGame = false;

void onVBlank(void) {

    mmVBlank();
    LINK_ISR_VBLANK();

    if (canDraw) {
        canDraw = 0;

        control();
        showPawn();
        showShadow();

        showHold();
        showQueue();

        drawGrid();
        screenShake();
        showClearText();

        if (game->refresh) {
            oam_copy(oam_mem, obj_buffer, 21);
            obj_aff_copy((OBJ_AFFINE*)oam_mem, obj_aff_buffer, 10);
            update();
            showBackground();
            game->resetRefresh();
        }else if (game->clearLock){
            showBackground();
        }else{
            showTimer();
            oam_copy(oam_mem, obj_buffer, 21);
            obj_aff_copy((OBJ_AFFINE*)oam_mem, obj_aff_buffer, 10);
        }
    }

    mmFrame();
}

void onHBlank() {
    if (REG_VCOUNT < 160) {
        clr_fade_fast((COLOR*)palette, GRADIENT_COLOR, pal_bg_mem, 2, (REG_VCOUNT / 20) * 2 + 16);
        memcpy16(&pal_bg_mem[1], &palette[1], 1);
    } else {
        clr_fade_fast((COLOR*)palette, GRADIENT_COLOR, pal_bg_mem, 2, 16);
        memcpy16(&pal_bg_mem[1], &palette[1], 1);
    }
}

int main(void) {
    irq_init(NULL);
    irq_enable(II_VBLANK);
    irq_add(II_VBLANK, onVBlank);

    irq_add(II_SERIAL, LINK_ISR_SERIAL);
    irq_add(II_TIMER3, LINK_ISR_TIMER);

    irq_add(II_HBLANK, onHBlank);
    irq_disable(II_HBLANK);

    mmInitDefault((mm_addr)soundbank_bin, 10);

    REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2);
    REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3);
    REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0);
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering
// 	// Load bg palette

    memcpy16(pal_bg_mem, palette, paletteLen / 2);
    // 	// //Load bg tiles
    memcpy16(&tile_mem[0][2], sprite3tiles_bin, sprite3tiles_bin_size / 2);
    memcpy16(&tile_mem[0][3], sprite4tiles_bin, sprite4tiles_bin_size / 2);
    memcpy16(&tile_mem[0][4], sprite5tiles_bin, sprite3tiles_bin_size / 2);
    memcpy16(&tile_mem[0][5], sprite7tiles_bin, sprite7tiles_bin_size / 2);
    memcpy16(&tile_mem[0][6], sprite8tiles_bin, sprite8tiles_bin_size / 2);
    memcpy16(&tile_mem[0][12], sprite10tiles_bin, sprite10tiles_bin_size / 2);
    memcpy16(&tile_mem[0][13], sprite11tiles_bin, sprite11tiles_bin_size / 2);
    memcpy16(&tile_mem[0][14], sprite12tiles_bin, sprite12tiles_bin_size / 2);
    memcpy16(&tile_mem[0][15], sprite13tiles_bin, sprite13tiles_bin_size / 2);
    memcpy16(&tile_mem[0][26], sprite17tiles_bin, sprite17tiles_bin_size / 2);
    memcpy16(&tile_mem[0][27], sprite20tiles_bin, sprite20tiles_bin_size / 2);
    memcpy16(&tile_mem[0][30], sprite22tiles_bin, sprite22tiles_bin_size / 2);
    memcpy16(&tile_mem[0][31], sprite23tiles_bin, sprite23tiles_bin_size / 2);
    memcpy16(&tile_mem[0][32], sprite24tiles_bin, sprite24tiles_bin_size / 2);

    memcpy16(&tile_mem[5][0], sprite6tiles_bin, sprite6tiles_bin_size / 2);

    //queue frame Tiles

    memcpy16(&tile_mem[5][16], sprite6tiles_bin, sprite6tiles_bin_size / 2 * 3 / 4);
    memcpy16(&tile_mem[5][16 + 12], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 1 / 4);
    memcpy16(&tile_mem[5][32], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 2 / 4);
    memcpy16(&tile_mem[5][32 + 8], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 2 / 4);
    memcpy16(&tile_mem[5][48], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 1 / 4);
    memcpy16(&tile_mem[5][48 + 4], &sprite6tiles_bin[128], sprite6tiles_bin_size / 2 * 3 / 4);

    memcpy16(&tile_mem[5][64], title1tiles_bin, title1tiles_bin_size / 2);
    memcpy16(&tile_mem[5][96], title2tiles_bin, title2tiles_bin_size / 2);

    memcpy16(&tile_mem[2][0], fontTiles, fontTilesLen / 2);

    //load tetriminoes into tile memory for menu screen animation

    memcpy16((COLOR*)MEM_PAL_OBJ, palette, paletteLen / 2);

    memcpy16(&pal_obj_mem[13 * 16], title_pal_bin, title_pal_bin_size / 2);

    // REG_BLDCNT = BLD_BUILD(BLD_BG1,BLD_BACKDROP,BLD_STD);
    REG_BLDCNT = (1 << 6) + (1 << 13) + (1 << 1);
    REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(2);

    loadSave();

    // //load mini sprite tiles
    // for(int i = 0; i < 7; i++)
    // 	memcpy16(&tile_mem[4][9*16+i*8],mini[1][i],16*7);

    setSkin();
    setLightMode();

    logInitMgba();

    //init glow 
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            glow[i][j] = 0;

    oam_init(obj_buffer, 128);

    // linkConnection->deactivate();

    playSongRandom(0);

    showTitleSprites();

    oam_copy(oam_mem, obj_buffer, 128);

    //start screen animation
    startScreen();

    oam_init(obj_buffer, 128);
    drawFrame();

    mmStop();

    showFrames();

    oam_copy(oam_mem, obj_buffer, 128);

    countdown();

    if (!(game->gameMode == 1 && game->goal == 0)) {
        playSongRandom(1);
    }

    update();

    gameLoop();
}

void update() {
    clearText();
    showText();
    showTimer();
}

std::string timeToString(int frames) {
    int t = (int)frames * 0.0167f;
    int millis = (int)(frames * 1.67f) % 100;
    int seconds = t % 60;
    int minutes = t / 60;

    std::string result = "";
    if (std::to_string(minutes).length() < 2)
        result += "0";
    result += std::to_string(minutes) + ":";
    if (std::to_string(seconds).length() < 2)
        result += "0";
    result += std::to_string(seconds) + ":";
    if (std::to_string(millis).length() < 2)
        result += "0";
    result += std::to_string(millis);

    return result;
}


void addToResults(int input, int index) {
    profileResults[index] = input;
}

void reset() {
    oam_init(obj_buffer, 128);
    clearText();
    REG_BG0HOFS = 0;
    REG_BG1HOFS = 0;
    REG_BG0VOFS = 0;
    REG_BG1VOFS = 0;

    REG_DISPCNT &= ~(DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3);

    memset32(&se_mem[25], 0x0000, 32 * 20);
    memset32(&se_mem[26], 0x0000, 32 * 20);
    memset32(&se_mem[27], 0x0000, 32 * 20);

    SoftReset();
    RegisterRamReset(0xff);
}

std::string nameInput(int place) {

    std::string result = savefile->latestName;
    int cursor = 0;

    for (int i = 0; i < (int)result.size(); i++) {
        if (result[i] == ' ') {
            cursor = i;
            break;
        }
    }

    const static int nameHeight = 14;

    int timer = 0;

    int das = 0;
    int maxDas = 12;

    int arr = 0;
    int maxArr = 5;

    bool onDone = false;
    while (1) {
        VBlankIntrWait();

        if (place == 0 && game->won)
            aprint("New Record", 10, 7);

        aprint("Name: ", 11, nameHeight - 2);

        aprint("DONE", 14, 16);

        key_poll();

        u16 key = key_hit(KEY_FULL);

        aprint(result, 13, nameHeight);

        if (!onDone) {
            if (key == KEY_A || key == KEY_RIGHT) {
                cursor++;
                if (cursor > 7) {
                    onDone = true;
                    cursor = 7;
                    sfx(SFX_MENUCONFIRM);
                } else {
                    sfx(SFX_MENUMOVE);
                }
            }

            if (key == KEY_B) {
                result[cursor] = ' ';
                if (cursor > 0)
                    cursor--;
                sfx(SFX_MENUCANCEL);
            }

            if (key == KEY_LEFT) {
                if (cursor > 0)
                    cursor--;
                sfx(SFX_MENUMOVE);
            }

            if (key == KEY_START) {
                onDone = true;
                sfx(SFX_MENUCONFIRM);
            }

            char curr = result.at(cursor);
            if (key == KEY_UP) {
                if (curr == 'A')
                    result[cursor] = ' ';
                else if (curr == ' ')
                    result[cursor] = 'Z';
                else if (curr > 'A')
                    result[cursor] = curr - 1;

                sfx(SFX_MENUMOVE);
            } else if (key == KEY_DOWN) {
                if (curr == 'Z')
                    result[cursor] = ' ';
                else if (curr == ' ')
                    result[cursor] = 'A';
                else if (curr < 'Z')
                    result[cursor] = curr + 1;
                sfx(SFX_MENUMOVE);
            } else if (key_is_down(KEY_UP) || key_is_down(KEY_DOWN)) {
                if (das < maxDas)
                    das++;
                else {
                    if (arr++ > maxArr) {
                        arr = 0;
                        if (key_is_down(KEY_UP)) {
                            if (curr == 'A')
                                result[cursor] = ' ';
                            else if (curr == ' ')
                                result[cursor] = 'Z';
                            else if (curr > 'A')
                                result[cursor] = curr - 1;
                        } else {
                            if (curr == 'Z')
                                result[cursor] = ' ';
                            else if (curr == ' ')
                                result[cursor] = 'A';
                            else if (curr < 'Z')
                                result[cursor] = curr + 1;
                        }
                        sfx(SFX_MENUMOVE);
                    }
                }
            } else {
                das = 0;
                if (timer++ > 19)
                    timer = 0;

                if (timer < 10)
                    aprint("_", 13 + cursor, nameHeight);
            }

            aprint(" ", 12, 16);
        } else {
            aprint(">", 12, 16);

            if (key == KEY_A || key == KEY_START) {
                sfx(SFX_MENUCONFIRM);
                break;
            }

            if (key == KEY_B) {
                onDone = false;
                sfx(SFX_MENUCANCEL);
            }
        }
    }

    clearText();

    if (result.size() >= 8)
        result = result.substr(0, 8);

    for (int i = 0; i < 8; i++)
        savefile->latestName[i] = result.at(i);

    return result;
}


void progressBar() {
    if (game->goal == 0)
        return;

    int current;
    int max = game->goal;

    if (game->gameMode == 3)
        current = game->garbageCleared;
    else if (game->gameMode == 5)
        current = game->timer;
    else
        current = game->linesCleared;

    int color = savefile->settings.palette + 2 * (savefile->settings.palette > 6);

    showBar(current, max, 20, color);

    if (game->gameMode == 4) {
        if (++attackFlashTimer > attackFlashMax)
            attackFlashTimer = 0;

        memcpy16(&pal_bg_mem[8 * 16], &palette[color * 16], 16);
        if (attackFlashTimer < attackFlashMax / 2) {
            memset32(&pal_bg_mem[8 * 16 + 5], 0x421f, 1);
        } else {
            memset32(&pal_bg_mem[8 * 16 + 5], 0x7fff, 1);
        }
        //attack bar
        showBar(game->getIncomingGarbage(), 20, 9, 8);

        //enemy height
        // u16*dest = (u16*)se_mem[27];

        // for(int i = 19; i >= 0; i--){
        // 	if(i <= (19-enemyHeight))
        // 		dest[32*i+29] = 0x000d + savefile->settings.palette * 0x1000;
        // 	else
        // 		dest[32*i+29] = 0x000e + savefile->settings.palette * 0x1000;
        // }
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

void setSkin() {
    for (int i = 0; i < 7; i++)
        memcpy16(&tile_mem[4][9 * 16 + i * 8], mini[0][i], 16 * 7);

    switch (savefile->settings.skin) {
    case 0:
        blockSprite = (u8*)sprite1tiles_bin;
        break;
    case 1:
        blockSprite = (u8*)sprite7tiles_bin;
        break;
    case 2:
        blockSprite = (u8*)sprite9tiles_bin;
        break;
    case 3:
        blockSprite = (u8*)sprite14tiles_bin;
        break;
    case 4:
        blockSprite = (u8*)sprite18tiles_bin;
        break;
    case 5:
        blockSprite = (u8*)sprite19tiles_bin;
        //load mini sprite tiles
        for (int i = 0; i < 7; i++)
            memcpy16(&tile_mem[4][9 * 16 + i * 8], mini[1][i], 16 * 7);
        break;
    case 6:
        blockSprite = (u8*)sprite21tiles_bin;
        break;
    }

    memcpy16(&tile_mem[0][1], blockSprite, sprite1tiles_bin_size / 2);
    memcpy16(&tile_mem[2][97], blockSprite, sprite1tiles_bin_size / 2);
    memcpy16(&pal_bg_mem[8 * 16], &palette[savefile->settings.palette * 16], 16);

    int** board;
    for (int i = 0; i < 7; i++) {
        board = game->getShape(i, 0);
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                if (board[j][k]) {
                    memcpy16(&tile_mem[4][16 * i + j * 4 + k], blockSprite, sprite1tiles_bin_size / 2);
                }
            }
        }
    }
    for (int i = 0; i < 4; i++)
        delete[] board[i];
    delete[] board;
}

void setLightMode() {

    if (savefile->settings.lightMode) {
        memset16(pal_bg_mem, 0x5ad6, 1);//background gray
        memset16(&pal_bg_mem[8 * 16 + 4], 0x0421, 1);//progressbar

        //activated font
        memset16(&pal_bg_mem[15 * 16 + 2], 0x0000, 1);//font main
        memset16(&pal_bg_mem[15 * 16 + 3], 0x5ad6, 1);//font shadow
        memset16(&pal_obj_mem[15 * 16 + 2], 0x0000, 1);//obj font main
        memset16(&pal_obj_mem[15 * 16 + 3], 0x5ad6, 1);//boj font shadow

        //unactivated font
        memset16(&pal_bg_mem[14 * 16 + 2], 0x7fff, 1);//font main
        memset16(&pal_bg_mem[14 * 16 + 3], 0x5ad6, 1);//font shadow
        memset16(&pal_obj_mem[14 * 16 + 2], 0x7fff, 1);//obj font main
        memset16(&pal_obj_mem[14 * 16 + 3], 0x5ad6, 1);//boj font shadow
    } else {
        memset16(pal_bg_mem, 0x0000, 1);
        memset16(&pal_bg_mem[8 * 16 + 4], 0x7fff, 1);

        //activated font
        memset16(&pal_bg_mem[15 * 16 + 2], 0x7fff, 1);
        memset16(&pal_bg_mem[15 * 16 + 3], 0x294a, 1);
        memset16(&pal_obj_mem[15 * 16 + 2], 0x7fff, 1);
        memset16(&pal_obj_mem[15 * 16 + 3], 0x294a, 1);

        //unactivated font
        memset16(&pal_bg_mem[14 * 16 + 2], 0x318c, 1);//font main
        memset16(&pal_bg_mem[14 * 16 + 3], 0x0421, 1);//font shadow
        memset16(&pal_obj_mem[14 * 16 + 2], 0x318c, 1);//obj font main
        memset16(&pal_obj_mem[14 * 16 + 3], 0x0421, 1);//boj font shadow
    }
}

void sleep() {
    int display_value = REG_DISPCNT;
    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG2; //Disable all backgrounds except text
    clearText();

    oam_init(obj_buffer, 128);
    oam_copy(oam_mem, obj_buffer, 128);

    while (1) {
        aprint("Entering sleep...", 7, 4);
        aprint("Press", 13, 8);
        aprint("L + R + SELECT", 9, 10);
        aprint("to leave sleep", 8, 12);

        aprint("A: Sleep      B: Cancel", 4, 17);

        VBlankIntrWait();

        key_poll();

        if (key_hit(KEY_A)) {
            clearText();
            break;
        }

        if (key_hit(KEY_B)) {
            REG_DISPCNT = display_value;
            update();
            showPawn();
            showHold();
            showShadow();
            showQueue();
            showTimer();
            return;
        }

    }

    irq_disable(II_VBLANK);
    int stat_value = REG_SNDSTAT;
    int dsc_value = REG_SNDDSCNT;
    int dmg_value = REG_SNDDMGCNT;

    REG_DISPCNT |= 0x0080;
    REG_SNDSTAT = 0;
    REG_SNDDSCNT = 0;
    REG_SNDDMGCNT = 0;

    linkConnection->deactivate();
    irq_disable(II_TIMER3);
    irq_disable(II_SERIAL);

    REG_P1CNT = 0b1100001100000100;
    irq_add(II_KEYPAD, nullptr);

    Stop();

    REG_DISPCNT = display_value;
    REG_SNDSTAT = stat_value;
    REG_SNDDSCNT = dsc_value;
    REG_SNDDMGCNT = dmg_value;

    linkConnection->activate();
    irq_enable(II_TIMER3);
    irq_enable(II_SERIAL);
    irq_delete(II_KEYPAD);
    irq_enable(II_VBLANK);

    update();
    showPawn();
    showHold();
    showShadow();
    showQueue();
    showTimer();
}

void drawUIFrame(int x, int y, int w, int h) {
    u16* dest = (u16*)&se_mem[26];
    u16* dest2 = (u16*)&se_mem[27];

    dest2 += y * 32 + x;

    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6)) * 0x1000;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int tile = 0;
            if ((i == 0 && (j == 0 || j == w - 1)) || (i == h - 1 && (j == 0 || j == w - 1))) {
                tile = 29 + (i > 0) * 0x800 + (j > 0) * 0x400;
            } else if (i == 0 || i == h - 1) {
                tile = 28 + (i > 0) * 0x800;
            } else if (j == 0 || j == w - 1) {
                tile = 4 + (j > 0) * 0x400;
            }
            if (tile)
                *dest2++ = tile + color * (tile != 12);
            else
                dest2++;
        }
        dest2 += 32 - w;
    }

    dest += (y + 1) * 32 + x + 1;
    for (int i = 1; i < h - 1; i++) {
        for (int j = 1; j < w - 1; j++) {
            *dest++ = 12 + 4 * 0x1000 * (savefile->settings.lightMode);
        }
        dest += 32 - w + 2;
    }
}

void diagnose() {
    if (!DIAGNOSE)
        return;
    // aprintf(game->linesSent, 0, 0);
    //
    // aprintf(game->moveCounter,0,0);
    // aprintf(game->timer,0,5);

    // int statHeight = 4;

    std::string str = "";

    for(int i = 0; i < 5; i++){
    	// aprint("       ",20,statHeight+i);
    	// std::string n = std::to_string(profileResults[i]);
    	// aprint(n,20,statHeight+i);
        str += std::to_string(profileResults[i]) + " ";
    }
    log(str);
}
