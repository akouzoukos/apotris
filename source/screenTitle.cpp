#include "def.h"
#include "tetromino.hpp"
#include "tonc.h"
#include "text.h"
#include <string>
#include <list>
#include "soundbank.h"
#include "LinkConnection.h"
#include "flashSaves.h"

#include "posprintf.h"
#include "logging.h"
#include "sprites.h"
#include "classic_pal_bin.h"
#include "tonc_core.h"

void drawUIFrame(int, int, int, int);
void fallingBlocks();
void startText();
void settingsText();
void toggleBigMode();
std::string timeToStringHours(int frames);
void resetScoreboard(int,int,int);

using namespace Tetris;

class WordSprite {
public:
    std::string text = "";
    int startIndex;
    int startTiles;
    int id;
    OBJ_ATTR* sprites[3];

    void show(int x, int y, int palette) {
        for (int i = 0; i < 3; i++) {
            obj_unhide(sprites[i], 0);
            obj_set_attr(sprites[i], ATTR0_WIDE, ATTR1_SIZE(1), palette);
            sprites[i]->attr2 = ATTR2_BUILD(startTiles + i * 4, palette, 1);
            obj_set_pos(sprites[i], x + i * 32, y);
        }
    }

    void show(int x, int y, int palette, FIXED scale) {
        for (int i = 0; i < 3; i++) {
            int affId = id*3+i;
            obj_unhide(sprites[i], 0);
            obj_set_attr(sprites[i], ATTR0_WIDE | ATTR0_AFF, ATTR1_SIZE(1) | ATTR1_AFF_ID(affId), palette);
            sprites[i]->attr2 = ATTR2_BUILD(startTiles + i * 4, palette, 1);
            obj_set_pos(sprites[i], x + i * 32, y);
            obj_aff_identity(&obj_aff_buffer[affId]);
            obj_aff_scale(&obj_aff_buffer[affId], scale, scale);
        }
    }

    void hide() {
        for (int i = 0; i < 3; i++)
            obj_hide(sprites[i]);
    }

    void setText(std::string _text) {
        if (_text == text)
            return;

        text = _text;

        TILE* font = (TILE*)fontTiles;
        for (int i = 0; i < (int)text.length() && i < 12; i++) {
            int c = text[i] - 32;

            memcpy32(&tile_mem[4][startTiles + i], &font[c], 8);
        }
    }

    WordSprite(int _id,int _index, int _tiles) {
        id = _id;
        startIndex = _index;
        startTiles = _tiles;

        for (int i = 0; i < 3; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }
};

#define MAX_WORD_SPRITES 15
WordSprite* wordSprites[MAX_WORD_SPRITES];
int titleFloat = 0;
OBJ_ATTR* titleSprites[2];
u8 backgroundArray[30][30];
int bgSpawnBlock = 0;
int bgSpawnBlockMax = 20;
int gravity = 0;
int gravityMax = 10;
int previousOptionScreen = -1;
bool goToOptions = false;
int previousSelection = 0;
int previousOptionMax = 0;

std::list<int> keyHistory;

Settings previousSettings;
Skin previousSkins[MAX_CUSTOM_SKINS];

bool bigMode = false;

int bigModeMessageTimer = 0;
int bigModeMessageMax = 180;

const std::list<std::string> menuOptions = { "Play","Settings","Credits" };
const std::list<std::string> gameOptions = { "Marathon","Sprint","Dig","Ultra","Blitz","Combo","Survival","Classic","Master","2P Battle","Training"};

const int secretCombo[11] = {KEY_UP,KEY_UP,KEY_DOWN,KEY_DOWN,KEY_LEFT,KEY_RIGHT,KEY_LEFT,KEY_RIGHT,KEY_B,KEY_A,KEY_START};

static int selection = 0;
int goalSelection = 0;
int level = 1;
static int toStart = 0;
static bool onSettings = false;
int subMode = 0;
bool proMode = false;

void startScreen() {

    int options = (int)menuOptions.size();

    bool refreshText = true;

    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool onPlay = false;

    bool moving = false;
    bool movingHor = false;
    int movingTimer = 0;
    int movingDirection = 0;

    selection = 0;

    VBlankIntrWait();
    REG_DISPCNT &= ~DCNT_BG1;
    REG_DISPCNT &= ~DCNT_BG3;
    drawUIFrame(0, 0, 30, 20);

    setSkin();
    setLightMode();

    oam_init(obj_buffer, 128);

    playSongRandom(0);

    clearText();

    if(goToOptions){
        goToOptions = false;
        onSettings = true;
        onPlay = true;
        toStart = previousOptionScreen;
        options = previousOptionMax;

        u16* dest = (u16*)se_mem[25];
        memset16(dest, 0, 20 * 32);

        clearText();

        refreshText = true;
    }else{
        showTitleSprites();

        memset32(&tile_mem[4][256], 0x0000 , MAX_WORD_SPRITES * 12 * 8);

        for (int i = 0; i < MAX_WORD_SPRITES; i++)
            wordSprites[i] = new WordSprite(i,64 + i * 3, 256 + i * 12);

        //initialise background array
        for (int i = 0; i < 30; i++)
            for (int j = 0; j < 30; j++)
                backgroundArray[i][j] = 0;

        oam_copy(oam_mem, obj_buffer, 128);
    }

    while (1) {
        VBlankIntrWait() ;
        if (!onSettings) {
            irq_disable(II_HBLANK);
        } else {
            irq_enable(II_HBLANK);
        }

        if (refreshText) {
            refreshText = false;
            if (onSettings) {
                REG_DISPCNT |= DCNT_BG1;
                REG_DISPCNT |= DCNT_BG3;
            } else {
                REG_DISPCNT &= ~DCNT_BG1;
                REG_DISPCNT &= ~DCNT_BG3;
            }

            startText();

        }

        key_poll();

        u16 key = key_hit(KEY_FULL);

        if (!onSettings) {

            fallingBlocks();
            showTitleSprites();

            if (savefile->settings.lightMode)
                memset16(pal_bg_mem, 0x5ad6, 1);//background gray
            else
                memset16(pal_bg_mem, 0x0000, 1);

            int startX, startY, space = 2;
            startX = 12;
            startY = 11;

            // std::list<std::string>::iterator index = menuOptions.begin();
            // for (int i = 0; i < (int)menuOptions.size(); i++) {
            int i = 0;
            for(auto const & option : menuOptions){
                int x = (startX - 5 * onPlay) * 8;

                int y = (startY + i * space) * 8;

                if(movingHor){
                    if(movingDirection == -1)
                        x=lerp((startX-5)*8,x,64*movingTimer);
                    else
                        x=lerp((startX)*8,x,64*movingTimer);
                }

                wordSprites[i]->setText(option);
                wordSprites[i]->show(x, y , 15 - !((onPlay && i == 0) || (!onPlay && selection == i)));
                i++;
            }

            int offset = (int)menuOptions.size();

            // index = gameOptions.begin();
            // for (int i = 0; i < (int)gameOptions.size(); i++) {
            i = 0;
            for(auto const& option : gameOptions){
                wordSprites[i + offset]->setText(option);

                int height = i - selection;

                if (onPlay && height >= -2 && height < 4) {
                    int x = (startX+5)*8;

                    int y = (startY+height*space)*8;

                    if(moving){
                        y=lerp((startY+(height+movingDirection)*space)*8,y,64*movingTimer);
                    }

                    // FIXED scale = abs(fxdiv(int2fx(y),int2fx((startY+2)*space*8)));

                    // scale = int2fx(1) + fxmul(scale,scale);

                    wordSprites[i + offset]->show(x, y, 15 - (selection != i));

                } else
                    wordSprites[i + offset]->hide();

                i++;
            }

            if(moving || movingHor){
                movingTimer++;
                if(movingTimer > 4){
                    moving = false;
                    movingHor = false;
                    movingTimer = 0;
                    movingDirection = 0;
                }
            }

            for (int i = 0; i < 5; i++)
                aprint(" ", startX - 2, startY + space * i);
            if (!onPlay) {
                aprint(">", startX - 2, startY + space * selection);
                aprint(" ", 15, startY);
            } else {
                aprint(">", 15, startY);
            }

            bool correct = false;
            if(key_hit(KEY_FULL)){

                if((int)keyHistory.size() == 11)
                    keyHistory.pop_front();

                keyHistory.push_back(key_hit(KEY_FULL));
                if((int)keyHistory.size() == 11){

                    correct = true;
                    int counter = 0;
                    auto index = keyHistory.begin();
                    while(index != keyHistory.end()){
                        if(*index != secretCombo[counter++]){
                            correct = false;
                            break;
                        }
                        ++index;
                    }
                }
            }

            if (key == KEY_A || key == KEY_START || key == KEY_RIGHT) {
                if(correct){
                    toggleBigMode();
                    sfx(SFX_SECRET);
                    continue;
                }

                int n = 0;
                if (!onPlay) {
                    if (selection == 0) {
                        onPlay = true;
                        options = (int)gameOptions.size();
                        movingHor = true;
                        movingDirection = 1;
                    } else if (selection == 1) {
                        n = -1;
                        previousSettings = savefile->settings;

                        for (int i = 0; i < MAX_CUSTOM_SKINS; i++)
                            previousSkins[i] = savefile->customSkins[i];

                        options = 6;
                    } else if (selection == 2) {
                        n = -2;
                    }

                } else {
                    if (selection == 0) {//marathon
                        n = MARATHON;
                        options = 3;
                        if(level < 1)
                            level = 1;
                    } else if (selection == 1) {//sprint
                        n = SPRINT;
                        options = 3;
                        goalSelection = 1;// set default goal to 40 lines for sprint
                        // maxClearTimer = 1;
                    } else if (selection == 2) {//Dig
                        n = DIG;
                        options = 3;
                    } else if (selection == 3) {//Ultra
                        options = 2;
                        n = ULTRA;
                    } else if (selection == 4) {//Blitz
                        options = 1;
                        n = BLITZ;
                    } else if (selection == 5) {//Combo
                        options = 1;
                        n = COMBO;
                    } else if (selection == 6) {//Survival
                        options = 2;
                        n = SURVIVAL;
                    } else if (selection == 7) {//Classic
                        options = 4;
                        n = CLASSIC;
                        if(level > 19)
                            level = 19;
                    } else if (selection == 8) {//Master
                        options = 2;
                        n = MASTER;
                    } else if (selection == 9) {//2p Battle
                        n = -3;
                        linkConnection->activate();

                    } else if (selection == 10) {//Training
                        options = 2;
                        n = -4;
                        level = 0;

                        // sfx(SFX_MENUCONFIRM);
                        // delete game;
                        // game = new Game(1);
                        // game->setLevel(1);
                        // game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtectionFrames,savefile->settings.directionalDas);
                        // game->setGoal(0);
                        // game->setTrainingMode(true);

                        // memset16(&se_mem[25], 0, 20 * 32);
                        // memset16(&se_mem[26], 0, 20 * 32);
                        // memset16(&se_mem[27], 0, 20 * 32);

                        // REG_DISPCNT |= DCNT_BG1;
                        // REG_DISPCNT |= DCNT_BG3;

                        // clearText();
                        // break;


                    // }else if (selection == 6){

                        // int seed = qran ();
                        // startMultiplayerGame(seed);
                        // multiplayer = false;

                        // delete botGame;
                        // botGame = new Game(4,seed);
                        // botGame->setGoal(100);
                        // game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtection);
                        // botGame->setLevel(1);

                        // delete testBot;
                        // testBot = new Bot(botGame);

                        // memset16(&se_mem[25], 0, 20 * 32);
                        // memset16(&se_mem[26], 0, 20 * 32);
                        // memset16(&se_mem[27], 0, 20 * 32);

                        // REG_DISPCNT |= DCNT_BG1;
                        // REG_DISPCNT |= DCNT_BG3;

                        // clearText();
                        // break;
                    }
                }

                sfx(SFX_MENUCONFIRM);
                if (n != 0) {
                    previousOptionScreen = toStart = n;
                    onSettings = true;

                    u16* dest = (u16*)se_mem[25];
                    memset16(dest, 0, 20 * 32);

                    clearText();
                    previousSelection = selection;
                    selection = 0;

                    refreshText = true;
                    if (n == -1)
                        settingsText();
                }
            }

            if (key == KEY_B || key == KEY_LEFT) {
                if (onPlay) {
                    onPlay = false;
                    selection = 0;
                    options = (int)menuOptions.size();
                    movingHor = true;
                    movingDirection = -1;
                    sfx(SFX_MENUCANCEL);
                }
            }

        } else {
            for (int i = 0; i < MAX_WORD_SPRITES; i++)
                wordSprites[i]->hide();

            for (int i = 0; i < 2; i++)
                obj_hide(titleSprites[i]);

            if(key_held(KEY_L) && key_held(KEY_R) && key_held(KEY_SELECT)){
                resetScoreboard(toStart,goalSelection,subMode);
                refreshText = true;
            }

            if (toStart == MARATHON) {
                if (selection == 0) {
                    if (key == KEY_RIGHT && level < 20) {
                        level++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && level > 1) {
                        level--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key_is_down(KEY_LEFT)) {
                        if (dasHor < maxDas) {
                            dasHor++;
                        } else if (level > 1) {
                            if (arr++ > maxArr) {
                                arr = 0;
                                level--;
                                sfx(SFX_MENUMOVE);
                                refreshText = true;
                            }
                        }
                    } else if (key_is_down(KEY_RIGHT)) {
                        if (dasHor < maxDas) {
                            dasHor++;
                        } else if (level < 20) {
                            if (arr++ > maxArr) {
                                arr = 0;
                                level++;
                                sfx(SFX_MENUMOVE);
                                refreshText = true;
                            }
                        }
                    } else {
                        dasHor = 0;
                    }
                } else if (selection == 1) {
                    if (key == KEY_RIGHT && goalSelection < 3) {
                        goalSelection++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && goalSelection > 0) {
                        goalSelection--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else if (toStart == SPRINT || toStart == DIG){
                if (selection == 0) {
                    if (key == KEY_RIGHT && subMode < 1) {
                        subMode++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && subMode > 0) {
                        subMode--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }else if (selection == 1) {
                    if (key == KEY_RIGHT && goalSelection < 2) {
                        goalSelection++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && goalSelection > 0) {
                        goalSelection--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else if (toStart == ULTRA || toStart == SURVIVAL) {
                if (selection == 0) {
                    if (key == KEY_RIGHT && goalSelection < 2) {
                        goalSelection++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && goalSelection > 0) {
                        goalSelection--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else if (toStart == CLASSIC){
                if (selection == 0) {
                    if (key == KEY_RIGHT && subMode < 1) {
                        subMode++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && subMode > 0) {
                        subMode--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }else if(selection == 1){
                    if (key == KEY_RIGHT && level < 20) {
                        level++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && level > 1) {
                        level--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key_is_down(KEY_LEFT)) {
                        if (dasHor < maxDas) {
                            dasHor++;
                        } else if (level > 1) {
                            if (arr++ > maxArr) {
                                arr = 0;
                                level--;
                                sfx(SFX_MENUMOVE);
                                refreshText = true;
                            }
                        }
                    } else if (key_is_down(KEY_RIGHT)) {
                        if (dasHor < maxDas) {
                            dasHor++;
                        } else if (level < 20) {
                            if (arr++ > maxArr) {
                                arr = 0;
                                level++;
                                sfx(SFX_MENUMOVE);
                                refreshText = true;
                            }
                        }
                    } else {
                        dasHor = 0;
                    }
                }else if (selection == 2 && subMode) {
                    if (key == KEY_RIGHT && goalSelection < 5) {
                        goalSelection++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && goalSelection > 0) {
                        goalSelection--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }

            }else if (toStart == MASTER){
                if (selection == 0) {
                    if (key == KEY_RIGHT && subMode < 1) {
                        subMode++;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }

                    if (key == KEY_LEFT && subMode > 0) {
                        subMode--;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else if (toStart == -4) {
                if (selection == 0) {
                    if (key == KEY_LEFT || key == KEY_RIGHT) {
                        goalSelection = !goalSelection;
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else if (toStart == -1) {
                if(key == KEY_A){
                    if(selection == 0){//open graphics settings
                        clearText();
                        sfx(SFX_MENUCONFIRM);
                        graphicTest();

                        drawUIFrame(0, 0, 30, 20);
                        oam_init(obj_buffer, 128);
                        showTitleSprites();
                        for (int i = 0; i < 2; i++)
                            obj_hide(titleSprites[i]);
                        oam_copy(oam_mem, obj_buffer, 128);
                        drawUIFrame(0, 0, 30, 20);

                        clearText();
                        refreshText = true;
                        settingsText();
                    }else if(selection != options-1){
                        clearText();
                        sfx(SFX_MENUCONFIRM);
                        switch(selection){
                            case 1: audioSettings(); break;
                            case 2: controlsSettings(); break;
                            case 3: handlingSettings(); break;
                            case 4: skinEditor(); break;
                        }
                        clearText();
                        refreshText = true;
                        settingsText();
                    }else if (selection == options-1){
                        saveToSram();
                        irq_enable(II_HBLANK);
                        onSettings = false;
                        options = (int)menuOptions.size();
                        selection = 0;
                        clearText();
                        refreshText = true;
                        sfx(SFX_MENUCONFIRM);
                    }
                }

                if(key == KEY_START){
                    if(selection != options - 1){
                        selection = options - 1;
                        refreshText = true;
                        sfx(SFX_MENUMOVE);
                    }else{
                        saveToSram();
                        irq_enable(II_HBLANK);
                        onSettings = false;
                        options = (int)menuOptions.size();
                        selection = 0;
                        clearText();
                        refreshText = true;
                        sfx(SFX_MENUCONFIRM);
                    }
                }

            } else if (toStart == -3) {
                if (multiplayerStartTimer) {
                    if (--multiplayerStartTimer == 0) {
                        startMultiplayerGame(nextSeed);
                        break;
                    } else {
                        linkConnection->send((u16)nextSeed + 100);
                    }
                } else {
                    auto linkState = linkConnection->linkState.get();

                    if (linkState->isConnected()) {
                        u16 data[LINK_MAX_PLAYERS];
                        for (u32 i = 0; i < LINK_MAX_PLAYERS; i++)
                            data[i] = 0;
                        for (u32 i = 0; i < linkState->playerCount; i++) {
                            while (linkState->hasMessage(i))
                                data[i] = linkState->readMessage(i);
                        }

                        for (u32 i = 0; i < LINK_MAX_PLAYERS; i++) {
                            if (data[i] == 2) {
                                if (connected < 1) {
                                    refreshText = true;
                                    clearText();
                                }
                                connected = 1;
                            } else if (data[i] > 100)
                                nextSeed = data[i] - 100;
                        }

                        if (linkState->playerCount == 2) {
                            if (linkState->currentPlayerId != 0) {
                                if (nextSeed > 100) {
                                    startMultiplayerGame(nextSeed);
                                    break;
                                }
                                aprint("Waiting", 12, 15);
                                aprint("for host...", 12, 16);
                            } else {
                                aprint("Press Start", 10, 15);

                            }
                        } else if (linkState->playerCount == 1) {
                            if (connected > -1) {
                                refreshText = true;
                                clearText();
                            }
                            connected = -1;
                        }

                        if (key == KEY_START && linkState->currentPlayerId == 0) {
                            nextSeed = (u16)qran() & 0x1fff;
                            multiplayerStartTimer = 3;
                        } else {
                            linkConnection->send(2);
                        }

                        aprint("             ", 0, 19);
                    } else {
                        if (connected > -1) {
                            refreshText = true;
                            clearText();
                        }
                        connected = -1;
                    }
                }
            }

            if ((key == KEY_A || key == KEY_START) && (toStart >= 0 || toStart == -4)) {
                if (selection != options - 1 && toStart != -2 && !(toStart == CLASSIC && !subMode && selection == options - 2)) {
                    selection = options - 1;

                    if(toStart == CLASSIC && !subMode)
                        selection--;

                    sfx(SFX_MENUCONFIRM);
                    refreshText = true;
                } else {
                    if (toStart != -1 && toStart != -2) {
                        bool training = false;
                        if(toStart == -4){
                            toStart = SPRINT;
                            training = true;
                        }

                        if(key_is_down(KEY_R)){
                            proMode = true;
                        }else{
                            proMode = false;
                        }

                        initialLevel = level - (toStart == CLASSIC || toStart == MASTER);
                        // initialLevel = 990;

                        if(toStart == MARATHON)
                            subMode = 1;

                        previousOptionMax = options;

                        //START GAME
                        delete game;
                        game = new Game(toStart,bigMode);
                        game->setLevel(initialLevel);
                        game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtectionFrames,savefile->settings.directionalDas);
                        game->bTypeHeight = goalSelection;
                        game->setSubMode(subMode);
                        mode = goalSelection;

                        game->pawn.big = bigMode;

                        if(training && goalSelection)
                            game->setTrainingMode(true);

                        int goal = 0;

                        switch (toStart) {
                        case MARATHON:
                            if (goalSelection == 0)
                                goal = 150;
                            else if (goalSelection == 1)
                                goal = 200;
                            else if (goalSelection == 2)
                                goal = 300;
                            else if (goalSelection == 3)
                                goal = 0x7fffffff;
                            break;
                        case SPRINT:
                            if(training)
                                break;
                            if (goalSelection == 0)
                                goal = 20;
                            else if (goalSelection == 1)
                                goal = 40;
                            else if (goalSelection == 2)
                                goal = 100;
                            break;
                        case DIG:
                            if (goalSelection == 0)
                                goal = 10;
                            else if (goalSelection == 1)
                                goal = 20;
                            else if (goalSelection == 2)
                                goal = 100;
                            break;
                        case ULTRA:
                            if (goalSelection == 0)
                                goal = 3 * 3600;
                            else if (goalSelection == 1)
                                goal = 5 * 3600;
                            else if (goalSelection == 2)
                                goal = 10 * 3600;
                            break;
                        case BLITZ:
                            goal = 2 * 3600;
                            break;
                        case SURVIVAL:
                            goal = goalSelection+1;
                            break;
                        case CLASSIC:
                            if(subMode)
                                goal = 25;
                            game->setRotationSystem(NRS);
                            break;
                        case MASTER:
                            // game->setRotationSystem(ARS);
                            break;
                        }
                        game->setGoal(goal);

                        savefile->stats.gamesStarted++;

                        sfx(SFX_MENUCONFIRM);
                        break;
                    }
                }
            }

            if (key == KEY_B) {
                if (toStart != -1) {
                    if (selection == 0 || toStart == -2) {
                        onSettings = false;
                        if (onPlay)
                            options = (int)gameOptions.size();
                        else
                            options = (int)menuOptions.size();
                        clearText();
                        sfx(SFX_MENUCANCEL);
                        refreshText = true;
                        selection = previousSelection;
                        subMode = 0;
                    } else {
                        selection = 0;
                        sfx(SFX_MENUCANCEL);
                        refreshText = true;
                    }
                } else {
                    onSettings = false;
                    options = (int)menuOptions.size();
                    clearText();
                    sfx(SFX_MENUCANCEL);
                    savefile->settings = previousSettings;

                        for (int i = 0; i < MAX_CUSTOM_SKINS; i++)
                            savefile->customSkins[i] = previousSkins[i];

                    mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
                    setSkin();
                    setLightMode();
                    drawUIFrame(0, 0, 30, 20);
                    refreshText = true;
                    selection = previousSelection;
                }

                goalSelection = 0;
                sfx(SFX_MENUCONFIRM);
            }
        }

        if (!(onSettings && toStart == -2)) {
            if (key == KEY_UP) {
                if (selection == 0)
                    selection = options - 1;
                else
                    selection--;
                if(onPlay){
                    moving = true;
                    movingDirection = -1;
                }

                if(onSettings && toStart == CLASSIC && !subMode && selection == 2)
                    selection--;

                sfx(SFX_MENUMOVE);
                refreshText = true;
            }

            if (key == KEY_DOWN || key == KEY_SELECT) {
                if (selection == options - 1)
                    selection = 0;
                else
                    selection++;
                if(onPlay){
                    moving = true;
                    movingDirection = 1;
                }

                if(onSettings && toStart == CLASSIC && !subMode && selection == 2)
                    selection++;

                sfx(SFX_MENUMOVE);
                refreshText = true;
            }

            if (key_is_down(KEY_UP)) {
                if (dasVer < maxDas) {
                    dasVer++;
                } else if(selection != 0){
                    if (arr++ > maxArr) {
                        arr = 0;
                        selection--;
                        if(onPlay){
                            moving = true;
                            movingDirection = -1;
                        }
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else if (key_is_down(KEY_DOWN)) {
                if (dasVer < maxDas) {
                    dasVer++;
                } else if(selection != options-1){
                    if (arr++ > maxArr) {
                        arr = 0;
                        selection++;
                        if(onPlay){
                            moving = true;
                            movingDirection = 1;
                        }
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else {
                dasVer = 0;
            }
        }

        sqran(qran() * frameCounter);

        oam_copy(oam_mem, obj_buffer, 128);
    }
    VBlankIntrWait();
    clearText();
    onSettings = false;
    irq_disable(II_HBLANK);
    setPalette();
    memset16(pal_bg_mem, 0x0000, 1);

    memset16(&se_mem[26], 0x0000, 32 * 20);
    memset16(&se_mem[27], 0x0000, 32 * 20);

    if (savefile->settings.lightMode)
        memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        memset16(pal_bg_mem, 0x0000, 1);
}

void startText() {
    char buff[5];

    const int titleX = 1;
    const int titleY = 1;

    if (!onSettings) {
        aprint("v3.3.1", 0, 19);

        aprint("akouzoukos", 20, 19);

        if(!rumbleInitialized && savefile->settings.rumble){
            aprint("Reboot to enable rumble.",0,0);
        }

    } else {
        if (toStart == MARATHON) {//Marathon Options
            aprintColor("Marathon",titleX,titleY,1);
            const int levelHeight = 3;
            const int goalHeight = 7;

            aprint("Level: ", 12, levelHeight);
            aprint("Lines: ", 12, goalHeight);
            aprint("START", 12, 17);

            aprintColor(" ||||||||||||||||||||    ", 2, levelHeight + 2,1);
            aprintColor(" 150   200   300   Endless ", 1, goalHeight + 2, 1);

            const std::string levelText = std::to_string(level);
            aprint(levelText, 27 - levelText.length(), levelHeight + 2);

            aprint(" ", 10, 17);
            if (selection == 0) {
                aprint("<", 2, levelHeight + 2);
                aprint(">", 23, levelHeight + 2);
            } else if (selection == 1) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 1, goalHeight + 2);
                    aprint("]", 5, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 7, goalHeight + 2);
                    aprint("]", 11, goalHeight + 2);
                    break;
                case 2:
                    aprint("[", 13, goalHeight + 2);
                    aprint("]", 17, goalHeight + 2);
                    break;
                case 3:
                    aprint("[", 19, goalHeight + 2);
                    aprint("]", 27, goalHeight + 2);
                    break;
                }
            } else if (selection == 2) {
                aprint(">", 10, 17);
            }

            switch (goalSelection) {
            case 0:
                aprint("150", 2, goalHeight + 2);
                break;
            case 1:
                aprint("200", 8, goalHeight + 2);
                break;
            case 2:
                aprint("300", 14, goalHeight + 2);
                break;
            case 3:
                aprint("Endless", 20, goalHeight + 2);
                break;
            }

            // show level cursor
            u16* dest = (u16*)se_mem[29];
            dest += (levelHeight + 2) * 32 + 2 + level;

            *dest = 0x5061;

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);

                aprint("                       ", 5, 11 + i);
                if (savefile->marathon[goalSelection].highscores[i].score == 0)
                    continue;

                aprint(savefile->marathon[goalSelection].highscores[i].name, 6, 11 + i);
                std::string score = std::to_string(savefile->marathon[goalSelection].highscores[i].score);

                aprint(score, 25 - (int)score.length(), 11 + i);
            }

        } else if (toStart == SPRINT) {//Sprint Options
            aprintColor("Sprint",titleX,titleY,1);
            const int goalHeight = 3;
            aprint("START", 12, 17);

            aprint("Type: ", 12, goalHeight);
            aprintColor(" Normal    Attack ", 6, goalHeight + 2, 1);

            aprint("Lines: ", 12, goalHeight + 4);
            aprintColor(" 20   40   100 ", 8, goalHeight + 6, 1);

            aprint(" ", 10, 17);
            if (selection == 0) {
                switch (subMode) {
                case 0:
                    aprint("[", 6, goalHeight + 2);
                    aprint("]", 13, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 16, goalHeight + 2);
                    aprint("]", 23, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 8, goalHeight + 6);
                    aprint("]", 11, goalHeight + 6);
                    break;
                case 1:
                    aprint("[", 13, goalHeight + 6);
                    aprint("]", 16, goalHeight + 6);
                    break;
                case 2:
                    aprint("[", 18, goalHeight + 6);
                    aprint("]", 22, goalHeight + 6);
                    break;
                }
            } else if (selection == 2) {
                aprint(">", 10, 17);
            }

            switch(subMode){
                case 0: aprint("Normal", 7, goalHeight + 2); break;
                case 1: aprint("Attack", 17, goalHeight + 2); break;
            }

            switch (goalSelection) {
                case 0: aprint("20", 9, goalHeight + 6); break;
                case 1: aprint("40", 14, goalHeight + 6); break;
                case 2: aprint("100", 19, goalHeight + 6); break;
            }

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);
                if(subMode == 0){
                    aprint("               ", 5, 11 + i);
                    if (savefile->sprint[goalSelection].times[i].frames == 0){
                        aprint("        ", 17, 11 + i);
                        continue;
                    }

                    aprint(savefile->sprint[goalSelection].times[i].name, 6, 11 + i);
                    std::string time = timeToString(savefile->sprint[goalSelection].times[i].frames);

                    aprint(time, 25 - (int)time.length(), 11 + i);

                }else if(subMode == 1){
                    aprint("               ", 5, 11 + i);
                    if (savefile->sprintAttack[goalSelection].times[i].frames == 0){
                        aprint("        ", 17, 11 + i);
                        continue;
                    }

                    aprint(savefile->sprintAttack[goalSelection].times[i].name, 6, 11 + i);
                    std::string time = timeToString(savefile->sprintAttack[goalSelection].times[i].frames);

                    aprint(time, 25 - (int)time.length(), 11 + i);
                }
            }

        } else if (toStart == DIG) {//Dig Options
            aprintColor("Dig",titleX,titleY,1);

            const int goalHeight = 3;
            aprint("START", 12, 17);

            aprint("Type: ", 12, goalHeight);
            aprintColor(" Normal    Efficiency ", 4, goalHeight + 2, 1);

            aprint("Lines: ", 12, goalHeight + 4);
            aprintColor(" 10   20   100 ", 8, goalHeight + 6, 1);

            switch(subMode){
                case 0: aprint("Normal", 5, goalHeight + 2); break;
                case 1: aprint("Efficiency", 15, goalHeight + 2); break;
            }

            aprint(" ", 10, 17);
            if (selection == 0) {
                switch (subMode) {
                case 0:
                    aprint("[", 4, goalHeight + 2);
                    aprint("]", 11, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 14, goalHeight + 2);
                    aprint("]", 25, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 8, goalHeight + 6);
                    aprint("]", 11, goalHeight + 6);
                    break;
                case 1:
                    aprint("[", 13, goalHeight + 6);
                    aprint("]", 16, goalHeight + 6);
                    break;
                case 2:
                    aprint("[", 18, goalHeight + 6);
                    aprint("]", 22, goalHeight + 6);
                    break;
                }
            } else if (selection == 2) {
                aprint(">", 10, 17);
            }

            switch (goalSelection) {
                case 0: aprint("10", 9, goalHeight + 6); break;
                case 1: aprint("20", 14, goalHeight + 6); break;
                case 2: aprint("100", 19, goalHeight + 6); break;
            }

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);

                if(subMode == 0){
                    aprint("               ", 5, 11 + i);
                    if (savefile->dig[goalSelection].times[i].frames == 0){
                        aprint("        ", 17, 11 + i);
                        continue;
                    }

                    aprint(savefile->dig[goalSelection].times[i].name, 6, 11 + i);
                    std::string time = timeToString(savefile->dig[goalSelection].times[i].frames);

                    aprint(time, 25 - (int)time.length(), 11 + i);
                }else if(subMode == 1){
                    aprint("                       ", 5, 11 + i);
                    if (savefile->digEfficiency[goalSelection].highscores[i].score == 0)
                        continue;

                    aprint(savefile->digEfficiency[goalSelection].highscores[i].name, 6, 11 + i);
                    std::string score = std::to_string(savefile->digEfficiency[goalSelection].highscores[i].score);

                    aprint(score, 25 - (int)score.length(), 11 + i);
                }

            }
        } else if (toStart == ULTRA) {//Ultra Options
            aprintColor("Ultra",titleX,titleY,1);

            const int goalHeight = 4;
            aprint("START", 12, 17);
            aprint("Minutes: ", 12, goalHeight);
            aprintColor(" 3    5    10 ", 8, goalHeight + 2, 1);

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);

                aprint("                       ", 5, 11 + i);
                if (savefile->ultra[goalSelection].highscores[i].score == 0)
                    continue;

                aprint(savefile->ultra[goalSelection].highscores[i].name, 6, 11 + i);
                std::string score = std::to_string(savefile->ultra[goalSelection].highscores[i].score);

                aprint(score, 25 - (int)score.length(), 11 + i);
            }

            aprint(" ", 10, 17);
            if (selection == 0) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 8, goalHeight + 2);
                    aprint("]", 10, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 13, goalHeight + 2);
                    aprint("]", 15, goalHeight + 2);
                    break;
                case 2:
                    aprint("[", 18, goalHeight + 2);
                    aprint("]", 21, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                aprint(">", 10, 17);
            }

            switch (goalSelection) {
            case 0:
                aprint("3", 9, goalHeight + 2);
                break;
            case 1:
                aprint("5", 14, goalHeight + 2);
                break;
            case 2:
                aprint("10", 19, goalHeight + 2);
                break;
            }
        } else if (toStart == BLITZ) {//Blitz Options
            aprintColor("Blitz",titleX,titleY,1);

            aprint("START", 12, 17);
            aprint(">", 10, 17);

            const int leaderboardHeight = 8;

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,leaderboardHeight+i);

                aprint("                       ", 5, leaderboardHeight + i);
                if (savefile->blitz[goalSelection].highscores[i].score == 0)
                    continue;

                aprint(savefile->blitz[goalSelection].highscores[i].name, 6, leaderboardHeight + i);
                std::string score = std::to_string(savefile->blitz[goalSelection].highscores[i].score);

                aprint(score, 25 - (int)score.length(), leaderboardHeight + i);
            }
        } else if (toStart == COMBO) {//Combo Options
            aprintColor("Combo",titleX,titleY,1);

            aprint("START", 12, 17);
            aprint(">", 10, 17);

            const int leaderboardHeight = 8;

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,leaderboardHeight+i);

                aprint("                       ", 5, leaderboardHeight + i);
                if (savefile->combo.highscores[i].score == 0)
                    continue;

                aprint(savefile->combo.highscores[i].name, 6, leaderboardHeight + i);
                std::string score = std::to_string(savefile->combo.highscores[i].score);

                aprint(score, 25 - (int)score.length(), leaderboardHeight + i);
            }
        } else if (toStart == SURVIVAL) {//Survival Options
            aprintColor("Survival",titleX,titleY,1);

            const int goalHeight = 4;
            aprint("START", 12, 17);
            const std::string str = "Difficulty:";
            aprint(str, 14-str.size()/2, goalHeight);
            aprintColor(" EASY   MEDIUM   HARD ", 4, goalHeight + 2, 1);

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);

                aprint("                       ", 5, 11 + i);
                if (savefile->survival[goalSelection].times[i].frames == 0)
                    continue;

                aprint(savefile->survival[goalSelection].times[i].name, 6, 11 + i);
                std::string time = timeToString(savefile->survival[goalSelection].times[i].frames);

                aprint(time, 25 - (int)time.length(), 11 + i);
            }
            aprint(" ", 10, 17);
            if (selection == 0) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 4, goalHeight + 2);
                    aprint("]", 9, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 11, goalHeight + 2);
                    aprint("]", 18, goalHeight + 2);
                    break;
                case 2:
                    aprint("[", 20, goalHeight + 2);
                    aprint("]", 25, goalHeight + 2);
                    break;
                }
            } else if (selection == 1){
                aprint(">", 10, 17);
            }

            switch (goalSelection) {
            case 0:
                aprint("EASY", 5, goalHeight + 2);
                break;
            case 1:
                aprint("MEDIUM", 12, goalHeight + 2);
                break;
            case 2:
                aprint("HARD", 21, goalHeight + 2);
                break;
            }
        } else if (toStart == CLASSIC) {//Classic Options
            aprintColor("Classic",titleX,titleY,1);
            const int levelHeight = 5;
            const int goalHeight = 1;

            const int diffHeight = 9;

            aprint("Level: ", 12, levelHeight);
            aprint("START", 12, 17);

            aprintColor(" ||||||||||||||||||||    ", 2, levelHeight + 2,1);
            aprint("Type:",4,goalHeight+2);
            aprintColor(" A-TYPE   B-TYPE ", 10, goalHeight + 2, 1);

            if(subMode){
                aprint("Height:",4,diffHeight);
                aprintColor(" 0 1 2 3 4 5 ", 12, diffHeight, 1);

                aprintf(goalSelection,goalSelection*2+13,diffHeight);
            }else{
                aprint("                      ",4,diffHeight);
            }

            std::string levelText = std::to_string(level-1);
            aprint(levelText, 27 - levelText.length(), levelHeight + 2);

            aprint(" ", 10, 17);
            if (selection == 0) {
                switch(subMode){
                case 0:
                    aprint("[", 10, goalHeight + 2);
                    aprint("]", 17, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 19, goalHeight + 2);
                    aprint("]", 26, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                aprint("<", 2, levelHeight + 2);
                aprint(">", 23, levelHeight + 2);
            } else if (selection == 2 && subMode) {
                aprint("<", 12, diffHeight);
                aprint(">", 24, diffHeight);
            } else if (selection == 2 || selection == 3) {
                aprint(">", 10, 17);
            }

            switch(subMode){
            case 0: aprint("A-TYPE", 11, goalHeight + 2); break;
            case 1: aprint("B-TYPE", 20, goalHeight + 2); break;
            }

            // show level cursor
            u16* dest = (u16*)se_mem[29];
            dest += (levelHeight + 2) * 32 + 2 + level;

            *dest = 0x5061;

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);

                aprint("                       ", 5, 11 + i);
                if (savefile->classic[subMode].highscores[i].score == 0)
                    continue;

                aprint(savefile->classic[subMode].highscores[i].name, 6, 11 + i);
                std::string score = std::to_string(savefile->classic[subMode].highscores[i].score);

                aprint(score, 25 - (int)score.length(), 11 + i);
            }
        } else if (toStart == MASTER) {//Master Options
            aprintColor("Master",titleX,titleY,1);

            const int goalHeight = 4;
            aprint("  START", 10, 17);
            aprint("Rules: ", 12, goalHeight);
            aprintColor(" Normal     Classic ", 5, goalHeight + 2, 1);

            for (int i = 0; i < 5; i++) {
                posprintf(buff,"%d.",i+1);
                aprint(buff,3,11+i);

                aprint("                       ", 5, 11 + i);
                if (savefile->master[subMode].times[i].frames == 0)
                    continue;

                aprint(savefile->master[subMode].times[i].name, 6, 11 + i);
                std::string text;
                text += GameInfo::masterGrades[savefile->master[subMode].grade[i]];
                text += " " + timeToString(savefile->master[subMode].times[i].frames);

                aprint(text, 25 - (int)text.length(), 11 + i);
            }

            if (selection == 0) {
                switch (subMode) {
                case 0:
                    aprint("[", 5, goalHeight + 2);
                    aprint("]", 12, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 16, goalHeight + 2);
                    aprint("]", 24, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                aprint(">", 10, 17);
            }

            switch (subMode) {
            case 0:
                aprint("Normal", 6, goalHeight + 2);
                break;
            case 1:
                aprint("Classic", 17, goalHeight + 2);
                break;
            }
        } else if (toStart == -1) {
            const int startY = 5;
            const int space = 2;
            aprint(" SAVE ", 12, 17);

            for(int i = 0; i < 5; i++)
                aprint(" ",10,startY+i*space);

            if(selection != 5)
                aprint(">", 10, startY+selection * space);
            else{
                aprint("[",12,17);
                aprint("]",17,17);
            }
        } else if (toStart == -2) {
            const int startX = 4;
            const int startY = 2;

            const int startY2 = 9;

            aprint("Menu Music:", startX - 1, startY);
            aprint("-veryshorty-extended", startX, startY + 1);
            aprint("by supernao", startX + 3, startY + 2);
            aprint("-optikal innovation", startX, startY + 3);
            aprint("by substance", startX + 3, startY + 4);

            aprint("In-Game Music:", startX - 1, startY2);
            aprint("-Thirno", startX, startY2 + 1);
            aprint("by Nikku4211", startX + 3, startY2 + 2);
            aprint("-oh my god!", startX, startY2 + 3);
            aprint("by kb-zip", startX + 3, startY2 + 4);
            aprint("-unsuspected <h>", startX, startY2 + 5);
            aprint("by curt cool", startX + 3, startY2 + 6);
            aprint("-Warning Infected!", startX, startY2 + 7);
            aprint("by Basq", startX + 3, startY2 + 8);
        } else if (toStart == -3) {
            if (connected < 1) {
                aprint("Waiting for", 7, 7);
                aprint("Link Cable", 9, 9);
                aprint("connection...", 11, 11);
            } else {
                aprint("Connected!", 10, 6);
            }
        } else if (toStart == -4) {
            aprintColor("Training",titleX,titleY,1);

            int goalHeight = 9;
            aprint("START", 12, 17);
            aprint("Finesse Training: ", 3, goalHeight);
            // aprintColor(" 3    5    10 ", 8, goalHeight + 2, 1);

            if(goalSelection == 0){
                aprint(" OFF ", 3 + 18, goalHeight);
            }else if(goalSelection == 1){
                aprint(" ON  ", 3 + 18, goalHeight);
            }

            aprint(" ", 10, 17);
            if (selection == 0) {
                aprint("[", 3 + 18, goalHeight);
                aprint("]", 3 + 21 + (!goalSelection), goalHeight);
            } else if (selection == 1) {
                aprint(">", 10, 17);
            }

            // switch (goalSelection) {
            // case 0:
            //     aprint("3", 9, goalHeight + 2);
            //     break;
            // case 1:
            //     aprint("5", 14, goalHeight + 2);
            //     break;
            // case 2:
            //     aprint("10", 19, goalHeight + 2);
            //     break;
            // }

        }
    }
}

void drawUIFrame(int x, int y, int w, int h) {
    memset32(&se_mem[26], 0x0000, 32 * 20);

    u16* dest = (u16*)&se_mem[26];
    u16* dest2 = (u16*)&se_mem[27];

    dest2 += y * 32 + x;

    // int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6)) * 0x1000;
    int color = 8 * 0x1000;

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

void showTitleSprites() {
    for (int i = 0; i < 2; i++)
        titleSprites[i] = &obj_buffer[14 + i];
    for (int i = 0; i < 2; i++) {
        obj_unhide(titleSprites[i], 0);
        obj_set_attr(titleSprites[i], ATTR0_WIDE, ATTR1_SIZE(3), ATTR2_PALBANK(13));
        titleSprites[i]->attr2 = ATTR2_BUILD(512 + 64 + i * 32, 13, 0);
        int offset = ((sin_lut[titleFloat]*3)>>12);

        if(offset == 3)
            offset = 2;
        obj_set_pos(titleSprites[i], 120 - 64 + 64 * i, 24 + offset);
        // obj_set_pos(titleSprites[i], 120 - 64 + 64 * i, 24 + 40);
    }

    titleFloat+=3;
    if(titleFloat >= 512)
        titleFloat = 0;
}

void fallingBlocks() {
    gravity++;
    bgSpawnBlock++;

    int i, j;
    if (gravity > gravityMax) {
        gravity = 0;

        for (i = 29; i >= 0; i--) {
            for (j = 0; j < 30; j++) {
                if (i == 0)
                    backgroundArray[i][j] = 0;
                else
                    backgroundArray[i][j] = backgroundArray[i - 1][j];
            }
        }
    }

    u16* dest = (u16*)se_mem[25];
    for (i = 4+6*bigMode; i < 24+6*bigMode; i++) {
        for (j = 0; j < 30; j++) {
            if (!backgroundArray[i][j])
                *dest++ = 2 * (!savefile->settings.lightMode);
            else{
                if(savefile->settings.skin < 7 || savefile->settings.skin > 8)
                    *dest++ = (1 + (((u32)(backgroundArray[i][j] - 1)) << 12));
                else{
                    *dest++ = (48 + backgroundArray[i][j] - 1 + (((u32)(backgroundArray[i][j] - 1)) << 12));
                }
            }
        }
        dest += 2;
    }

    if (bgSpawnBlock > bgSpawnBlockMax) {

        int n = qran() % 7;
        int** p = game->getShape(n, qran() % 4);

        bool found = false;

        if(!bigMode){
            int x = qran() % 27;

            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                    if (backgroundArray[i][j + x])
                        found = true;

            if(!found)
                for (i = 0; i < 4; i++)
                    for (j = 0; j < 4; j++)
                        if (p[i][j])
                            backgroundArray[i][j + x] = n + 1;
        }else{
            int x = (qran() % 13) * 2;

            for (i = 0; i < 8; i++)
                for (j = 0; j < 8; j++)
                    if (backgroundArray[i][j + x*2])
                        found = true;

            if(!found){
                for (i = 0; i < 4; i++){
                    for (j = 0; j < 4; j++){
                        int xoffset = (j+x)*2;
                        int yoffset = i*2;

                        if (!p[i][j] || yoffset < 0 || yoffset > 23 || xoffset < 0 || xoffset > 29)
                            continue;

                        backgroundArray[yoffset][xoffset] = n + 1;
                        backgroundArray[yoffset][xoffset+1] = n + 1;
                        backgroundArray[yoffset+1][xoffset] = n + 1;
                        backgroundArray[yoffset+1][xoffset+1] = n + 1;
                    }
                }
            }
        }

        for(i = 0; i < 4; i++)
            delete p[i];
        delete p;

        bgSpawnBlock = 0;
    }

    if(bigModeMessageTimer > 0){
        if(--bigModeMessageTimer == 0){
            aprint("                  ",0,0);
            aprint("                    ",0,1);
        }
    }
}

void settingsText() {
    const std::list<std::string> options = {
        "Graphics",
        "Audio",
        "Controls",
        "Handling",
        "Skin Editor"
    };

    const int startX = 12;
    const int startY = 5;
    const int space = 2;

    int i = 0;
    for(auto const& option : options){
        aprint(option,startX,startY+space*i);
        i++;
    }

    std::string str = "Playtime: " + timeToStringHours(savefile->stats.timePlayed);
    aprint(str,15-str.size()/2,1);
}

void toggleBigMode(){
    bigMode = !bigMode;

    //empty background array
    for (int i = 0; i < 30; i++)
        for (int j = 0; j < 30; j++)
            backgroundArray[i][j] = 0;

    bigModeMessageTimer = bigModeMessageMax;

    if(bigMode){
        aprint("Big Mode enabled!",0,0);
        aprint("Highscores disabled!",0,1);
    }else
        aprint("Big Mode disabled!",0,0);
}

std::string timeToStringHours(int frames) {
    int t = (int)frames * 0.0167f;
    int minutes = (t / 60)%60;
    int hours = t / 3600;

	char res[30];

	posprintf(res,"%02d:%02d",hours,minutes);

    std::string result = "";

    result = res;

    return result;
}

void resetScoreboard(int mode, int goal, int subMode){
    // int display_value = REG_DISPCNT;
    // REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG2; //Disable all backgrounds except text
    clearText();

    oam_init(obj_buffer, 128);
    oam_copy(oam_mem, obj_buffer, 128);


    int timer = 0;

    int s = 1;

    while (1) {

        aprint("Are you sure you", 6, 6);
        aprint("want to reset the", 6 , 8);
        aprint("current scoreboard?", 6 , 10);


        VBlankIntrWait();
        key_poll();

        if(timer < 200){
            timer++;
        }else{
            aprint(" YES      NO ", 8, 15);

            if(s == 0){
                aprint("[",8,15);
                aprint("]",12,15);
            }else{
                aprint("[",17,15);
                aprint("]",20,15);
            }

            if (key_hit(KEY_A)) {
                clearText();
                if(s == 0)
                    break;
                else
                    return;
            }

            if (key_hit(KEY_B)) {
                clearText();
                return;
            }

            if(key_hit(KEY_LEFT)){
                if(s > 0)
                    s--;
            }

            if(key_hit(KEY_RIGHT)){
                if(s < 1)
                    s++;
            }
        }
    }

    switch(mode){
    case MARATHON:
        for (int j = 0; j < 5; j++)
            savefile->marathon[goal].highscores[j].score = 0;
        break;
    case SPRINT:
        if(!subMode){
            for (int j = 0; j < 5; j++)
                savefile->sprint[goal].times[j].frames = 0;
        }else{
            for (int j = 0; j < 5; j++)
                savefile->sprintAttack[goal].times[j].frames = 0;
        }
        break;
    case DIG:
        if(!subMode){
            for (int j = 0; j < 5; j++)
                savefile->dig[goal].times[j].frames = 0;
        }else{
            for (int j = 0; j < 5; j++)
                savefile->digEfficiency[goal].highscores[j].score = 0;
        }
        break;
    case ULTRA:
        for (int j = 0; j < 5; j++)
            savefile->ultra[goal].highscores[j].score = 0;
        break;
    case BLITZ:
        for (int j = 0; j < 5; j++)
            savefile->blitz[goal].highscores[j].score = 0;
        break;
    case COMBO:
        for (int j = 0; j < 5; j++)
            savefile->combo.highscores[j].score = 0;
        break;
    case SURVIVAL:
        for (int j = 0; j < 5; j++)
            savefile->survival[goal].times[j].frames = 0;
        break;
    case CLASSIC:
        for (int j = 0; j < 5; j++)
            savefile->classic[goal].highscores[j].score = 0;
        break;
    case MASTER:
        for (int j = 0; j < 5; j++){
            savefile->master[goal].times[j].frames = 0;
            savefile->master[goal].grade[j] = -1;
        }
        break;
    }

    saveToSram();
    clearText();
}
