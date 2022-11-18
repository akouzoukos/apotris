#include <tonc.h>
#include <string>
#include <maxmod.h>

#include "def.h"
#include "sprite37tiles_bin.h"
#include "sprite5tiles_bin.h"
#include "tetrisEngine.h"
#include "sprites.h"

#include "soundbank.h"
#include "soundbank_bin.h"

#include "LinkConnection.h"

#include "logging.h"

#include "text.h"

#include "flashSaves.h"

#include "posprintf.h"

#include "rumble.h"
#include "gbp_logo.hpp"
#include "classic1tiles_bin.h"
#include "classic_pal_bin.h"

#include "tetromino.hpp"
#include "tonc_bios.h"
#include "tonc_core.h"
#include "tonc_input.h"
#include "tonc_irq.h"
#include "tonc_memdef.h"
#include "tonc_memmap.h"
#include "tonc_oam.h"
#include "tonc_types.h"
#include "tonc_video.h"

#include "defaultGradient_bin.h"

using namespace Tetris;

mm_word myEventHandler();
bool unlock_gbp();
void initRumble();
int getClassicPalette();

LinkConnection* linkConnection = new LinkConnection();

int frameCounter = 1;

OBJ_ATTR obj_buffer[128];
OBJ_AFFINE* obj_aff_buffer = (OBJ_AFFINE*)obj_buffer;

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

bool rumble_enabled = false;

bool rumbleInitialized = false;

Scene * scene = nullptr;

u16 gradientTable[SCREEN_HEIGHT + 1];

bool gradientEnabled = false;

bool resumeJourney = false;
bool journeyLevelUp = false;
bool journeySaveExists = false;
Tetris::Game* journeySave;

void onVBlank(void) {

    mmVBlank();
    if(gradientEnabled)
        DMA_TRANSFER(&pal_bg_mem[0], gradientTable, 1, 0, DMA_HDMA);

    LINK_ISR_VBLANK();

    REG_IME = 1;
    if (canDraw) {
        canDraw = 0;

        scene->draw();
    }

    REG_IME = 0;
    frameCounter++;
    mmFrame();
}

void onHBlank() {
    if(REG_VCOUNT < 160)
        pal_bg_mem[0] = gradientTable[REG_VCOUNT];
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

    mmInitDefault((mm_addr)soundbank_bin, 12);
    mmSetEventHandler((mm_callback)myEventHandler);

    initRumble();

    REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2) | BG_MOSAIC;
    REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3) | BG_MOSAIC;
    REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0) | BG_MOSAIC;
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering

    	// //Load bg tiles
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

    //hold tiles
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

    for(int i = 0; i < 8; i++)
        memcpy16(&tile_mem[5][128+i], moveSpriteTiles[i], 16);

    memcpy16(&tile_mem[2][0], fontTiles, fontTilesLen / 2);

    //load tetriminoes into tile memory for menu screen animation

    setSkin();
    setClearEffect();

    // REG_BLDCNT = BLD_BUILD(BLD_BG1,BLD_BACKDROP,BLD_STD);
    REG_BLDCNT = (1 << 6) + (1 << 13) + (1 << 1);
    REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(2);

    setGradient(GRADIENT_COLOR);
}


int main(void) {
    if(DIAGNOSE)
        logInitMgba();

    if(ENABLE_FLASH_SAVE)
        flash_init();

    loadSave();

    initialize();

    //start screen animation
    while(1){
        reset();

        if(!playAgain && !journeyLevelUp){
            startScreen();
        }else if(journeyLevelUp){
			journeyLevelUp = false;
			if (journeySaveExists){		
				int goal = game->goal;
				int training = game->trainingMode;
				int rs = game->rotationSystem;
				
				delete game;
					
				game = new Game(*journeySave);				
				game->setGoal(goal);
				game->setTuning(getTuning());
				game->setTrainingMode(training);
				game->pawn.big = bigMode;
				game->bTypeHeight = goalSelection;
				game->setSubMode(subMode);
				game->setRotationSystem(rs);
				
				resumeJourney = true;
			}
		}else{
            playAgain = false;

            int goal = game->goal;
            int training = game->trainingMode;
            int rs = game->rotationSystem;

            delete game;
            game = new Game(game->gameMode,bigMode);
            game->setGoal(goal);
            game->setLevel(initialLevel);
            game->setTuning(getTuning());
            game->setTrainingMode(training);
            game->pawn.big = bigMode;
            game->bTypeHeight = goalSelection;
            game->setSubMode(subMode);
            game->setRotationSystem(rs);
        }

        gameLoop();
    }
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
    placeEffectList.clear();
    rumbleTimer = 0;
    rumble_set_state(RumbleState(rumble_stop));

    resetZonePalette();

    memset32(&se_mem[25], 0x0000, 32 * 20);
    memset32(&se_mem[26], 0x0000, 32 * 20);
    memset32(&se_mem[27], 0x0000, 32 * 20);

    REG_MOSAIC = MOS_BUILD(0,0,0,0);

    //reset glow
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            glow[i][j] = 0;

    int g = savefile->settings.backgroundGradient;
    if(g == 0)
        memcpy16(gradientTable,defaultGradient_bin,defaultGradient_bin_size/2);
    else
        setGradient(g);
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

        if (place == 0 && game->gameMode != BATTLE && (game->won || game->gameMode == MARATHON || game->gameMode >= BLITZ))
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
    setPalette();

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
        break;
    case 6:
        blockSprite = (u8*)sprite21tiles_bin;
        break;
    case 7:
        for(int i = 0; i < 8; i++)
            memcpy16(&tile_mem[0][48+i],classicTiles[0][i],classic1tiles_bin_size/2);

        blockSprite = (u8*)classicTiles[0][0];
        break;
    case 8:
        for(int i = 0; i < 8; i++)
            memcpy16(&tile_mem[0][48+i],classicTiles[1][i],classic1tiles_bin_size/2);

        blockSprite = (u8*)classicTiles[1][0];
        break;
    case 9:
        blockSprite = (u8*)sprite27tiles_bin;
        break;
    case 10:
        blockSprite = (u8*)sprite28tiles_bin;
        break;
    case 11:
        blockSprite = (u8*)&sprite38tiles_bin[12*32];

        memcpy32(&tile_mem[0][128],sprite38tiles_bin,sprite38tiles_bin_size/4);
        break;
    case 12:
        blockSprite = (u8*)&sprite39tiles_bin[12*32];

        memcpy32(&tile_mem[0][128],sprite39tiles_bin,sprite39tiles_bin_size/4);
        break;
    case 13:
        blockSprite = (u8*)&sprite40tiles_bin[12*32];

        memcpy32(&tile_mem[0][128],sprite40tiles_bin,sprite40tiles_bin_size/4);
        break;
    default:
        if(savefile->settings.skin < 0){
            int n = savefile->settings.skin;
            n *= -1;
            n--;

            blockSprite = (u8*)&savefile->customSkins[n].board;
        }
        break;
    }

    if(savefile->settings.skin < 7){
        VBlankIntrWait();
        if(savefile->settings.skin < 0){
            int n = savefile->settings.skin;
            n *= -1;
            n--;
            buildMini(&savefile->customSkins[n].smallBoard);
        }else{
            buildMini((TILE*)mini[savefile->settings.skin]);
        }
    }

    memcpy16(&tile_mem[0][1], blockSprite, sprite1tiles_bin_size / 2);
    memcpy16(&tile_mem[2][97], blockSprite, sprite1tiles_bin_size / 2);

    int** board;
    for (int i = 0; i < 7; i++) {
        board = getShape(i, 0, game->rotationSystem);

        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                if (board[j][k]) {
                    if(savefile->settings.skin == 11)
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], &sprite38tiles_bin[GameInfo::connectedConversion[(board[j][k])>>4] * 32], sprite1tiles_bin_size / 2);
                    else if(savefile->settings.skin == 12)
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], &sprite39tiles_bin[GameInfo::connectedConversion[(board[j][k])>>4] * 32], sprite1tiles_bin_size / 2);
                    else if(savefile->settings.skin == 13)
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], &sprite40tiles_bin[GameInfo::connectedConversion[(board[j][k])>>4] * 32], sprite1tiles_bin_size / 2);
                    else if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], blockSprite, sprite1tiles_bin_size / 2);
                    else
                        memcpy16(&tile_mem[4][16 * i + j * 4 + k], classicTiles[savefile->settings.skin-7][i], sprite1tiles_bin_size / 2);
                }else{
                    memcpy16(&tile_mem[4][16 * i + j * 4 + k], &tile_mem[4][0], sprite1tiles_bin_size / 2);
                }
            }
        }

        for (int i = 0; i < 4; i++)
            delete[] board[i];
        delete[] board;
    }
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

    setGradient(savefile->settings.backgroundGradient);
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
            clearText();
            sfx(SFX_MENUCANCEL);
            return;
        }
    }

    irq_disable(II_VBLANK);
    gradient(false);
    int stat_value = REG_SNDSTAT;
    int dsc_value = REG_SNDDSCNT;
    int dmg_value = REG_SNDDMGCNT;

    REG_DISPCNT |= 0x0080;
    REG_SNDSTAT = 0;
    REG_SNDDSCNT = 0;
    REG_SNDDMGCNT = 0;

    bool isLinked = linkConnection->isActive();

    if(isLinked)
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

    if(isLinked)
        linkConnection->activate();

    irq_enable(II_TIMER3);
    irq_enable(II_SERIAL);
    irq_delete(II_KEYPAD);
    irq_enable(II_VBLANK);
    gradient(true);
}

void diagnose() {
    if (!DIAGNOSE)
        return;

    std::string str = "";

    // for(int i = 0; i < 5; i++){
    //     // str += std::to_string(profileResults[i]) + " ";

    // }
    // str+= "Garbage Height: " + std::to_string(game->garbageHeight);

    // str += std::to_string(game->previousBest.size()) + " " + std::to_string(game->moveHistory.size());

    // str += std::to_string(profileResults[0]);

}

void setPalette(){
    int n = (savefile->settings.colors < 2)?savefile->settings.colors:0;

    for(int i = 0; i < 16; i++){

        if (i < 8 && savefile->settings.colors < 2){
            memcpy16(&pal_bg_mem[i*16], &palette[n][i*16], 8);
            memcpy16(&pal_obj_mem[i*16], &palette[n][i*16], 8);
        }else if (i < 8){
            memcpy16(&pal_bg_mem[i*16+4], &palette[n][i*16+4], 4);
            memcpy16(&pal_obj_mem[i*16+4], &palette[n][i*16+4], 4);
        }else if (i > 9){
            memcpy16(&pal_bg_mem[i*16], &palette[n][i*16], 16);
            memcpy16(&pal_obj_mem[i*16], &palette[n][i*16], 16);
        }
    }

    int color = savefile->settings.palette + 2 * (savefile->settings.palette > 6);

    if(savefile->settings.colors == 2){
        for(int i = 0; i < 9; i++){
            memcpy16(&pal_bg_mem[i*16], classic_pal_bin,4);
            memcpy16(&pal_obj_mem[i*16], classic_pal_bin,4);
        }
    }else if(savefile->settings.colors == 3){
        //set frame color
        memcpy16(&pal_obj_mem[8 * 16], &palette[n][color * 16], 16);
        memcpy16(&pal_bg_mem[8 * 16], &palette[n][color * 16], 16);

        int n = getClassicPalette();
        for(int i = 0; i < 8; i++){
            memcpy16(&pal_bg_mem[i*16+1], &nesPalette[n][0],4);
            memcpy16(&pal_obj_mem[i*16+1], &nesPalette[n][0],4);
        }

    }else if(savefile->settings.colors == 4){
        for(int i = 0; i < 9; i++){
            memcpy16(&pal_bg_mem[i*16+1], &monoPalette[savefile->settings.lightMode],4);
            memcpy16(&pal_obj_mem[i*16+1], &monoPalette[savefile->settings.lightMode],4);
        }

        if(!savefile->settings.lightMode){
            memset16(&pal_bg_mem[7], 0x7fff,2);
            memset16(&pal_obj_mem[7], 0x7fff,2);
        }else{
            memset16(&pal_bg_mem[7], 0x0421,2);
            memset16(&pal_obj_mem[7], 0x0421,2);
        }

    }else if(savefile->settings.colors == 5){
        for(int i = 0; i < 8; i++){
            memcpy16(&pal_bg_mem[i*16+1], &arsPalette[0][i],4);
            memcpy16(&pal_obj_mem[i*16+1], &arsPalette[0][i],4);
        }

        //set frame color
        memcpy16(&pal_obj_mem[8 * 16], &palette[0][color * 16], 16);
        memcpy16(&pal_bg_mem[8 * 16], &palette[0][color * 16], 16);
    }else if(savefile->settings.colors == 6){
        for(int i = 0; i < 8; i++){
            memcpy16(&pal_bg_mem[i*16+1], &arsPalette[1][i],4);
            memcpy16(&pal_obj_mem[i*16+1], &arsPalette[1][i],4);
        }
        //set frame color
        memcpy16(&pal_obj_mem[8 * 16], &palette[1][color * 16], 16);
        memcpy16(&pal_bg_mem[8 * 16], &palette[1][color * 16], 16);
    }else{
        //set frame color
        memcpy16(&pal_obj_mem[8 * 16], &palette[n][color * 16], 16);
        memcpy16(&pal_bg_mem[8 * 16], &palette[n][color * 16], 16);

    }

    memcpy16(&pal_obj_mem[13 * 16], title_pal_bin, title_pal_bin_size / 2);
    setLightMode();
}

void initRumble(){
    if(!savefile->settings.rumble)
        return;

    rumbleInitialized = true;
    RegisterRamReset(RESET_VRAM);
    REG_DISPCNT = DCNT_MODE0 | DCNT_BG0;
    *((volatile u16*)0x4000008) = 0x0088;

    if (unlock_gbp()) {

        RumbleGBPConfig conf{[](void (*rumble_isr)(void)) {
            irq_enable(II_SERIAL);
            irq_add(II_SERIAL, rumble_isr);
        }};

        rumble_init(&conf);
    } else {
        rumble_init(nullptr);
    }
}

bool unlock_gbp(){
    bool gbp_detected = false;

    memcpy16((u16*)0x6008000, gbp_logo_pixels, (sizeof gbp_logo_pixels)/2);
    memcpy16((u16*)0x6000000, gbp_logo_tiles, (sizeof gbp_logo_tiles)/2);
    memcpy16(pal_bg_mem, gbp_logo_palette, (sizeof gbp_logo_palette)/2);

    static volatile u32* keys = (volatile u32*)0x04000130;

    for(int i = 0; i < 120; i++){
        // if(key_is_down(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)){
        if(*keys==0x030f){
            gbp_detected = true;
        }
        VBlankIntrWait();
    }

    RegisterRamReset(RESET_VRAM);

    return gbp_detected;
}

int getClassicPalette(){
    int n = 0;
    int mode = game->gameMode;
    if((mode == MARATHON || mode == BLITZ || mode == CLASSIC ) && game->level >= 0 && game->level < 255)
        n = (game->level-(game->gameMode != CLASSIC)) % 10;
    else
        n = abs(game->initSeed) % 10;

    return n;
}

void buildMini(TILE * customSkin){
    memset32(&tile_mem[4][9*16],0,8*8*7);

    int add = (game->rotationSystem == NRS || game->rotationSystem == ARS);

    for (int i = 0; i < 7; i++){

        TILE * t;
        int** p = getShape(i, 0, game->rotationSystem);
        int tileStart = 9 * 16 + i * 8;

        for(int y = 0; y < 2; y++){
            for(int x = 0; x < 4; x++){
                if(!p[y+add][x])
                    continue;

                for(int ii = 0; ii < 6; ii++){
                    for(int jj = 0; jj < 6; jj++){
                        t = &tile_mem[4][tileStart + (x*6+jj)/8 + ((y*6+ii)/8)*4];

                        t->data[(y*6+ii)%8] |= ((customSkin->data[ii] >> (4*jj)) & 0xf) << (((x*6+jj)%8)*4);
                    }
                }
            }
        }

        for(int i = 0; i < 4; i++)
            delete p[i];
        delete p;
    }
}

Tuning getTuning(){
    Tuning t = {
        t.das = savefile->settings.das,
        t.arr = savefile->settings.arr,
        t.sfr = savefile->settings.sfr,
        t.dropProtection = savefile->settings.dropProtectionFrames,
        t.directionalDas = savefile->settings.directionalDas,
        t.delaySoftDrop = savefile->settings.delaySoftDrop,
    };

    return t;
}

void changeScene(Scene *newScene){
    if(scene != nullptr){
        delete scene;
    }

    scene = newScene;
}

void setGradient(int color){
    REG_DMA0CNT &= ~(1 << 0x1f);

    int n = (savefile->settings.colors < 2)?savefile->settings.colors:0;

    COLOR * src = (COLOR *) &palette[n];

    if(!(savefile->settings.lightMode && color == 0)){
        for(int i = 0; i < SCREEN_HEIGHT; i++){
            clr_fade(src, color, (COLOR *) &gradientTable[i], 1, ((SCREEN_HEIGHT-1-i) / 20) * 2 + 16);
        }

        gradientTable[SCREEN_HEIGHT-1] = gradientTable[0];
    }else{
        for(int i = 0; i < SCREEN_HEIGHT; i++)
            gradientTable[i] = 0x5ad6;
    }

}

void sfx(int s){
	mm_sfxhand h = mmEffect(s);
	mmEffectVolume(h, 255 * (float)savefile->settings.sfxVolume / 10);

    if((s == SFX_PLACE) && game->zoneTimer){
        mmEffectRate(h,512);
    }
}

void setDefaultGradient(){
    memcpy16(gradientTable,defaultGradient_bin,defaultGradient_bin_size/2);
}

void gradient(bool state){
    if(state){
    }else{
        REG_DMA0CNT &= ~(1 << 0x1f);
    }

    gradientEnabled = state;
}
