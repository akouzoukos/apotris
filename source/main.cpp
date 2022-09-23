#include <tonc.h>
#include <tonc_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>
#include <maxmod.h>

#include "classic1tiles_bin.h"
#include "def.h"
#include "tetrisEngine.h"
#include "sprites.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "LinkConnection.h"

#include "logging.h"
#include "tonc_core.h"

#include "text.h"

#include "flashSaves.h"

#include "posprintf.h"

#include "classic_pal_bin.h"
#include "tonc_memmap.h"

using namespace Tetris;

void control();
void showTimer();
mm_word myEventHandler();

LinkConnection* linkConnection = new LinkConnection();

int frameCounter = 1;

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE* obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;

int shakeMax = 10;
int shake = 0;

int gameSeconds;

bool pause = false;

int marathonClearTimer = 20;

int initialLevel = 0;

bool canDraw = false;

int profileResults[10];

void addToResults(int input, int index);

Save* savefile;

u8* blockSprite;

bool restart = false;

bool playAgain = false;
int nextSeed = 0;

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
            obj_aff_copy(obj_aff_mem, obj_aff_buffer, 32);
            update();
            showBackground();
            game->resetRefresh();
        }else if (game->clearLock){
            showBackground();
        }else{
            oam_copy(oam_mem, obj_buffer, 21);
            obj_aff_copy(obj_aff_mem, obj_aff_buffer, 32);
        }
        showTimer();
    }

    mmFrame();
}

void onHBlank() {
    if (REG_VCOUNT < 160) {
        clr_fade_fast((COLOR*)palette[savefile->settings.colors], GRADIENT_COLOR, pal_bg_mem, 2, (REG_VCOUNT / 20) * 2 + 16);
        memcpy16(&pal_bg_mem[1], &palette[savefile->settings.colors][1], 1);
    } else {
        clr_fade_fast((COLOR*)palette[savefile->settings.colors], GRADIENT_COLOR, pal_bg_mem, 2, 16);
        memcpy16(&pal_bg_mem[1], &palette[savefile->settings.colors][1], 1);
    }
}

mm_word myEventHandler(mm_word msg, mm_word param){
    if(msg == MMCB_SONGMESSAGE)
        return 0;

    playNextSong();

    return 0;
}

void initialize(){

    irq_init(NULL);
    irq_enable(II_VBLANK);
    irq_add(II_VBLANK, onVBlank);

    irq_add(II_SERIAL, LINK_ISR_SERIAL);
    irq_add(II_TIMER3, LINK_ISR_TIMER);

    irq_add(II_HBLANK, onHBlank);
    irq_disable(II_HBLANK);

    mmInitDefault((mm_addr)soundbank_bin, 10);
    mmSetEventHandler((mm_callback)myEventHandler);

    REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2);
    REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3);
    REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0);
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering
 	// Load bg palette

    // 	// //Load bg tiles
    memcpy16(&tile_mem[0][2], sprite3tiles_bin, sprite3tiles_bin_size / 2);
    memcpy16(&tile_mem[0][4], sprite5tiles_bin, sprite3tiles_bin_size / 2);
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

    //hold tileshodl
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

    for(int i = 0; i < 8; i ++)
        memcpy16(&tile_mem[5][128+i], moveSpriteTiles[i], 16);


    memcpy16(&tile_mem[2][0], fontTiles, fontTilesLen / 2);

    //load tetriminoes into tile memory for menu screen animation

    setPalette();
    setClearEffect();

    // REG_BLDCNT = BLD_BUILD(BLD_BG1,BLD_BACKDROP,BLD_STD);
    REG_BLDCNT = (1 << 6) + (1 << 13) + (1 << 1);
    REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(2);

    logInitMgba();
}

int main(void) {
    if(ENABLE_FLASH_SAVE)
        flash_init();

    loadSave();

    initialize();

    //start screen animation
    while(1){
        reset();

        if(!playAgain){
            startScreen();
        }else{
            playAgain = false;
            int goal = game->goal;
            int training = game->trainingMode;
            delete game;
            game = new Game(game->gameMode);
            game->setGoal(goal);
            game->setLevel(initialLevel);
            game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtectionFrames,savefile->settings.directionalDas);
            game->setTrainingMode(training);
        }

        gameLoop();
    }
}

void update() {
    clearText();
    showText();
    showTimer();
    showClearText();
}

std::string timeToString(int frames) {
    int t = (int)frames * 0.0167f;
    int millis = (int)(frames * 1.67f) % 100;
    int seconds = t % 60;
    int minutes = t / 60;

	char res[30];

	posprintf(res,"%02d:%02d:%02d",minutes,seconds,millis);

    std::string result = "";

    result = res;

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
    shake = 0;
    push = 0;

    floatingList.clear();

    // REG_DISPCNT &= ~(DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3);

    memset32(&se_mem[25], 0x0000, 32 * 20);
    memset32(&se_mem[26], 0x0000, 32 * 20);
    memset32(&se_mem[27], 0x0000, 32 * 20);

    //reset glow
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            glow[i][j] = 0;
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

        if (place == 0 && game->gameMode != 4 && (game->won || game->gameMode == 0 || game->gameMode == 2 || game->gameMode >= 5))
            aprint("New Record", 10, 5);

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
    case 7:
        setPalette();
        for(int i = 0; i < 8; i++)
            memcpy16(&tile_mem[0][48+i],classicTiles[i],classic1tiles_bin_size/2);

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
                    if(savefile->settings.skin != 7)
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], blockSprite, sprite1tiles_bin_size / 2);
                    else
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], classicTiles[i], sprite1tiles_bin_size / 2);
                }else{
                    memcpy16(&tile_mem[4][16 * i + j * 4 + k], &tile_mem[4][0], sprite1tiles_bin_size / 2);
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

void diagnose() {
    if (!DIAGNOSE)
        return;

    std::string str = "";

    for(int i = 0; i < 5; i++){
        str += std::to_string(profileResults[i]) + " ";
    }

    // str += std::to_string(game->previousBest.size()) + " " + std::to_string(game->moveHistory.size());

    // log(str);
}

void setPalette(){
    memcpy16(pal_bg_mem, palette[savefile->settings.colors], paletteLen / 2);
    memcpy16(pal_obj_mem, palette[savefile->settings.colors], paletteLen / 2);

    if(savefile->settings.skin == 7){
        for(int i = 0; i < 8; i++){
            memcpy16(&pal_bg_mem[i*16], classic_pal_bin,4);
            memcpy16(&pal_obj_mem[i*16], classic_pal_bin,4);
        }
    }
    memcpy16(&pal_obj_mem[13 * 16], title_pal_bin, title_pal_bin_size / 2);
    setLightMode();
}
