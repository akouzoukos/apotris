#include "maxmod.h"
#include "tonc.h"
#include "def.h"
#include "soundbank.h"
#include "LinkConnection.h"
#include "tetrisEngine.h"
#include "tonc_memdef.h"

using namespace Tetris;

void drawUIFrame(int, int, int, int);
void fallingBlocks();
void endAnimation();
void showStats();
int onRecord();
std::string nameInput(int);

OBJ_ATTR* titleSprites[2];

int backgroundArray[24][30];
int bgSpawnBlock = 0;
int bgSpawnBlockMax = 20;
int gravity = 0;
int gravityMax = 10;
int mode = 0;

Settings previousSettings;
bool saveExists = false;

Tetris::Game* quickSave;

std::list<std::string> menuOptions = { "Play","Settings","Credits" };
// std::list<std::string> gameOptions = { "Marathon","Sprint","Dig","Ultra","2P Battle","Training","Test" };
std::list<std::string> gameOptions = { "Marathon","Sprint","Dig","Ultra","2P Battle","Training"};

class WordSprite {
public:
    std::string text = "";
    int startIndex;
    int startTiles;
    OBJ_ATTR* sprites[3];

    void show(int x, int y, int palette) {
        for (int i = 0; i < 3; i++) {
            obj_unhide(sprites[i], 0);
            obj_set_attr(sprites[i], ATTR0_WIDE, ATTR1_SIZE(1), palette);
            sprites[i]->attr2 = ATTR2_BUILD(startTiles + i * 4, palette, 1);
            obj_set_pos(sprites[i], x + i * 32, y);
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

    WordSprite(int _index, int _tiles) {
        startIndex = _index;
        startTiles = _tiles;

        for (int i = 0; i < 3; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }
};

WordSprite* wordSprites[10];

void songListMenu() {
    int startX = 3;
    int endX = 24;

    int startY = 3;

    int selection = 0;
    int options = 7;

    mmStop();
    playSong(0, 0);

    while (1) {
        VBlankIntrWait();

        key_poll();

        if (key_hit(KEY_START)) {
            sfx(SFX_MENUCONFIRM);
            if (selection != options - 1) {
                selection = options - 1;
            } else {
                break;
            }
        }

        if (key_hit(KEY_B)) {
            sfx(SFX_MENUCANCEL);
            if (selection == options - 1)
                selection = 0;
            else
                break;
        }

        if (key_hit(KEY_UP)) {
            if (selection > 0)
                selection--;
            else
                selection = options - 1;
            sfx(SFX_MENUMOVE);

            mmStop();
            if (selection < 2)
                playSong(0, selection);
            else if (selection < 6)
                playSong(1, selection - 2);
        }

        if (key_hit(KEY_DOWN)) {
            if (selection < options - 1)
                selection++;
            else
                selection = 0;
            sfx(SFX_MENUMOVE);
            if (selection < 2)
                playSong(0, selection);
            else if (selection < 6)
                playSong(1, selection - 2);
        }

        if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT) || key_hit(KEY_A)) {
            sfx(SFX_MENUCONFIRM);
            if (selection == options - 1)
                break;
            else {
                savefile->settings.songList[selection] = !savefile->settings.songList[selection];
            }
        }

        aprint(" DONE ", 12, 17);

        aprint("Menu:", startX, startY);
        aprint("Track 1", startX, startY + 2);
        aprint("Track 2", startX, startY + 3);

        aprint("Game:", startX, startY + 6);
        aprint("Track 1", startX, startY + 8);
        aprint("Track 2", startX, startY + 9);
        aprint("Track 3", startX, startY + 10);
        aprint("Track 4", startX, startY + 11);

        for (int i = 0; i < 2; i++)
            aprint("   ", endX - 1, startY + 2 + i);

        for (int i = 0; i < 6; i++)
            aprint("   ", endX - 1, startY + 7 + i);

        if (savefile->settings.songList[0])
            aprint("x", endX, startY + 2);
        if (savefile->settings.songList[1])
            aprint("x", endX, startY + 3);

        if (savefile->settings.songList[2])
            aprint("x", endX, startY + 8);
        if (savefile->settings.songList[3])
            aprint("x", endX, startY + 9);
        if (savefile->settings.songList[4])
            aprint("x", endX, startY + 10);
        if (savefile->settings.songList[5])
            aprint("x", endX, startY + 11);

        if (selection == options - 1) {
            aprint("[", 12, 17);
            aprint("]", 17, 17);
        } else {
            aprint("[", endX - 1, startY + 2 + selection + (selection > 1) * 4);
            aprint("]", endX + 1, startY + 2 + selection + (selection > 1) * 4);
        }

        oam_copy(oam_mem, obj_buffer, 128);
    }

    mmStop();
    playSongRandom(0);
}

void settingsText() {
    int startX = 3;
    int startY = 4;
    int space = 1;
    aprint("Voice", startX, startY);
    aprint("Clear Text", startX, startY + space);
    aprint("Screen Shake", startX, startY + space * 2);
    aprint("Music", startX, startY + space * 3);
    aprint("Auto Repeat Delay", startX, startY + space * 4);
    aprint("Auto Repeat Rate", startX, startY + space * 5);
    aprint("Soft Drop Speed", startX, startY + space * 6);
    aprint("Drop Protection", startX, startY + space * 7);
    aprint("Show Finesse", startX, startY + space * 8);
    aprint("Graphics...", startX, startY + space * 9);
    aprint("Song List...", startX, startY + space * 10);
}

void graphicTest() {
    irq_disable(II_HBLANK);

    if (savefile->settings.lightMode)
        memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        memset16(pal_bg_mem, 0x0000, 1);

    memset16(&se_mem[26], 0x0000, 32 * 20);

    delete game;
    game = new Game(3);

    game->update();

    game->pawn.y++;

    bool showOptions = true;
    bool showGame = true;

    int startX = 5;
    int endX = 24;

    int startY = 5;
    int options = 8;
    int selection = 0;

    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering

    REG_DISPCNT &= ~DCNT_BG3;

    showBackground();
    while (1) {
        VBlankIntrWait();
        drawGrid();
        drawFrame();

        key_poll();

        aprint("R: Toggle", 0, 18);
        aprint("Options", 3, 19);

        if (key_hit(KEY_START) || key_hit(KEY_A)) {
            sfx(SFX_MENUCONFIRM);
            if (selection != options - 1) {
                selection = options - 1;
            } else {
                break;
            }
            setSkin();
            setLightMode();
        }

        if (key_hit(KEY_B)) {
            sfx(SFX_MENUCANCEL);
            if (selection == options - 1)
                selection = 0;
            else
                break;
        }

        if (key_hit(KEY_R)) {
            showOptions = !showOptions;

            if (!showOptions) {
                clearText();
                aprint("R: Toggle", 0, 18);
                aprint("Options", 3, 19);
            }
        }

        if (key_hit(KEY_UP)) {
            if (selection > 0)
                selection--;
            else
                selection = options - 1;
            sfx(SFX_MENUMOVE);
        }

        if (key_hit(KEY_DOWN)) {
            if (selection < options - 1)
                selection++;
            else
                selection = 0;
            sfx(SFX_MENUMOVE);
        }

        if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)) {
            switch (selection) {
            case 0:
                savefile->settings.effects = !savefile->settings.effects;
                if (savefile->settings.effects) {
                    effectList.push_back(Effect(1, 4, 5));
                }
                sfx(SFX_MENUMOVE);
                break;
            case 1:
                savefile->settings.edges = !savefile->settings.edges;
                sfx(SFX_MENUMOVE);
                break;
            case 2:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.backgroundGrid > 0) {
                        savefile->settings.backgroundGrid--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1) {
                        savefile->settings.backgroundGrid++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

                drawGrid();
                break;
            case 3:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.skin > 0) {
                        savefile->settings.skin--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.skin < MAX_SKINS - 1) {
                        savefile->settings.skin++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

                setSkin();
                setLightMode();
                break;
            case 4:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.palette > 0) {
                        savefile->settings.palette--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.palette < 7) {
                        savefile->settings.palette++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                break;
            case 5:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.shadow > 0) {
                        savefile->settings.shadow--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.shadow < MAX_SHADOWS - 1) {
                        savefile->settings.shadow++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                break;
            case 6:
                savefile->settings.lightMode = !savefile->settings.lightMode;
                setLightMode();
                sfx(SFX_MENUMOVE);
                break;
            }
        }

        if (showOptions) {

            aprint(" DONE ", 12, 15);

            aprint("Effects", startX, startY);
            aprint("Block Edges", startX, startY + 1);
            aprint("Background", startX, startY + 2);
            aprint("Skin", startX, startY + 3);
            aprint("Frame Color", startX, startY + 4);
            aprint("Ghost Piece", startX, startY + 5);
            aprint("Light Mode", startX, startY + 6);

            for (int i = 0; i < options; i++)
                aprint("      ", endX - 1, startY + i);

            if (savefile->settings.effects)
                aprint("ON", endX, startY);
            else
                aprint("OFF", endX, startY);

            if (savefile->settings.edges)
                aprint("ON", endX, startY + 1);
            else
                aprint("OFF", endX, startY + 1);

            aprintf(savefile->settings.backgroundGrid + 1, endX, startY + 2);

            aprintf(savefile->settings.skin + 1, endX, startY + 3);

            aprintf(savefile->settings.palette + 1, endX, startY + 4);

            aprintf(savefile->settings.shadow + 1, endX, startY + 5);

            if (savefile->settings.lightMode)
                aprint("ON", endX, startY + 6);
            else
                aprint("OFF", endX, startY + 6);

            switch (selection) {
            case 0:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.effects), startY + selection);
                break;
            case 1:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.edges), startY + selection);
                break;
            case 2:
                if (savefile->settings.backgroundGrid > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 3:
                if (savefile->settings.skin > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.skin < MAX_SKINS - 1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 4:
                if (savefile->settings.palette > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.palette < 7)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 5:
                if (savefile->settings.shadow > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.shadow < MAX_SHADOWS)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 6:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.lightMode), startY + selection);
                break;
            case 7:
                aprint("[", 12, 15);
                aprint("]", 17, 15);
                break;
            }
        }

        if (showGame) {
            showBackground();
            showQueue();
            showPawn();
            showShadow();
            showBackground();
            showHold();
        } else {
            oam_init(obj_buffer, 128);
        }

        oam_copy(oam_mem, obj_buffer, 128);
    }
    REG_DISPCNT |= DCNT_BG3;
    irq_enable(II_HBLANK);
    memset32(&se_mem[25], 0x0000, 32 * 10);
    memset32(&se_mem[26], 0x0000, 32 * 10);
    memset32(&se_mem[27], 0x0000, 32 * 10);
    drawUIFrame(0, 0, 30, 20);
    oam_init(obj_buffer, 128);
    showTitleSprites();
    for (int i = 0; i < 2; i++)
        obj_hide(titleSprites[i]);
    oam_copy(oam_mem, obj_buffer, 128);
    drawUIFrame(0, 0, 30, 20);
}

void showTitleSprites() {
    for (int i = 0; i < 2; i++)
        titleSprites[i] = &obj_buffer[14 + i];
    for (int i = 0; i < 2; i++) {
        obj_unhide(titleSprites[i], 0);
        obj_set_attr(titleSprites[i], ATTR0_WIDE, ATTR1_SIZE(3), ATTR2_PALBANK(13));
        titleSprites[i]->attr2 = ATTR2_BUILD(512 + 64 + i * 32, 13, 0);
        obj_set_pos(titleSprites[i], 120 - 64 + 64 * i, 24);
    }
}

void startText(bool onSettings, int selection, int goalSelection, int level, int toStart) {

    if (!onSettings) {
        // aprint("APOTRIS",12,4);
        aprint("v3.0.0a", 0, 19);

        aprint("akouzoukos", 20, 19);

    } else {
        if (toStart == 2) {//Marathon Options
            int levelHeight = 2;
            int goalHeight = 6;

            aprint("Level: ", 12, levelHeight);
            aprint("Lines: ", 12, goalHeight);
            aprint("START", 12, 17);

            aprint(" ||||||||||||||||||||    ", 2, levelHeight + 2);
            aprintColor(" 150   200   300   Endless ", 1, goalHeight + 2, 1);

            std::string levelText = std::to_string(level);
            aprint(levelText, 27 - levelText.length(), levelHeight + 2);

            for (int i = 0; i < 5; i++) {
                aprint(std::to_string(i + 1) + ".", 3, 11 + i);

                aprint("                       ", 5, 11 + i);
                if (savefile->marathon[goalSelection].highscores[i].score == 0)
                    continue;

                aprint(savefile->marathon[goalSelection].highscores[i].name, 6, 11 + i);
                std::string score = std::to_string(savefile->marathon[goalSelection].highscores[i].score);

                aprint(score, 25 - (int)score.length(), 11 + i);
            }

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

        } else if (toStart == 1) {//Sprint Options
            int goalHeight = 4;
            aprint("START", 12, 17);
            aprint("Lines: ", 12, goalHeight);
            aprintColor(" 20   40   100 ", 8, goalHeight + 2, 1);

            for (int i = 0; i < 5; i++) {
                aprint(std::to_string(i + 1) + ".", 3, 11 + i);
                aprint("                       ", 5, 11 + i);
                if (savefile->sprint[goalSelection].times[i].frames == 0)
                    continue;

                aprint(savefile->sprint[goalSelection].times[i].name, 6, 11 + i);
                std::string time = timeToString(savefile->sprint[goalSelection].times[i].frames);

                aprint(time, 25 - (int)time.length(), 11 + i);
            }

            aprint(" ", 10, 17);
            if (selection == 0) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 8, goalHeight + 2);
                    aprint("]", 11, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 13, goalHeight + 2);
                    aprint("]", 16, goalHeight + 2);
                    break;
                case 2:
                    aprint("[", 18, goalHeight + 2);
                    aprint("]", 22, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                aprint(">", 10, 17);
            }

            switch (goalSelection) {
            case 0:
                aprint("20", 9, goalHeight + 2);
                break;
            case 1:
                aprint("40", 14, goalHeight + 2);
                break;
            case 2:
                aprint("100", 19, goalHeight + 2);
                break;
            }
        } else if (toStart == 3) {//Dig Options
            int goalHeight = 4;
            aprint("START", 12, 17);
            aprint("Lines: ", 12, goalHeight);
            aprintColor(" 10   20   100 ", 8, goalHeight + 2, 1);

            for (int i = 0; i < 5; i++) {
                aprint(std::to_string(i + 1) + ".", 3, 11 + i);
                aprint("                       ", 5, 11 + i);
                if (savefile->dig[goalSelection].times[i].frames == 0)
                    continue;

                aprint(savefile->dig[goalSelection].times[i].name, 6, 11 + i);
                std::string time = timeToString(savefile->dig[goalSelection].times[i].frames);

                aprint(time, 25 - (int)time.length(), 11 + i);
            }

            aprint(" ", 10, 17);
            if (selection == 0) {
                switch (goalSelection) {
                case 0:
                    aprint("[", 8, goalHeight + 2);
                    aprint("]", 11, goalHeight + 2);
                    break;
                case 1:
                    aprint("[", 13, goalHeight + 2);
                    aprint("]", 16, goalHeight + 2);
                    break;
                case 2:
                    aprint("[", 18, goalHeight + 2);
                    aprint("]", 22, goalHeight + 2);
                    break;
                }
            } else if (selection == 1) {
                aprint(">", 10, 17);
            }

            switch (goalSelection) {
            case 0:
                aprint("10", 9, goalHeight + 2);
                break;
            case 1:
                aprint("20", 14, goalHeight + 2);
                break;
            case 2:
                aprint("100", 19, goalHeight + 2);
                break;
            }
        } else if (toStart == 5) {//Ultra Options
            int goalHeight = 4;
            aprint("START", 12, 17);
            aprint("Minutes: ", 12, goalHeight);
            aprintColor(" 3    5    10 ", 8, goalHeight + 2, 1);

            for (int i = 0; i < 5; i++) {
                aprint(std::to_string(i + 1) + ".", 3, 11 + i);
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
        } else if (toStart == -1) {
            int startY = 4;
            int space = 1;
            aprint(" SAVE ", 12, 17);

            int endX = 23;

            for (int i = 0; i < 11; i++)
                aprint("       ", endX - 1, startY + space * i);

            if (savefile->settings.announcer)
                aprint("ON", endX, startY);
            else
                aprint("OFF", endX, startY);

            if (savefile->settings.floatText)
                aprint("ON", endX, startY + space);
            else
                aprint("OFF", endX, startY + space);

            if (savefile->settings.shake)
                aprint("ON", endX, startY + space * 2);
            else
                aprint("OFF", endX, startY + space * 2);

            // if(savefile->settings.effects)
            // 	aprint("ON",endX,startY+space*3);
            // else
            // 	aprint("OFF",endX,startY+space*3);

            aprintf(savefile->settings.volume, endX, startY + space * 3);

            if (savefile->settings.das == 8)
                aprint("V.FAST", endX, startY + space * 4);
            else if (savefile->settings.das == 9)
                aprint("FAST", endX, startY + space * 4);
            else if (savefile->settings.das == 11)
                aprint("MID", endX, startY + space * 4);
            else if (savefile->settings.das == 16)
                aprint("SLOW", endX, startY + space * 4);

            if (savefile->settings.arr == 0)
                aprint("V.FAST", endX, startY + space * 5);
            else if (savefile->settings.arr == 1)
                aprint("FAST", endX, startY + space * 5);
            else if (savefile->settings.arr == 2)
                aprint("MID", endX, startY + space * 5);
            else if (savefile->settings.arr == 3)
                aprint("SLOW", endX, startY + space * 5);

            if (savefile->settings.sfr == 0)
                aprint("V.FAST", endX, startY + space * 6);
            else if (savefile->settings.sfr == 1)
                aprint("FAST", endX, startY + space * 6);
            else if (savefile->settings.sfr == 2)
                aprint("MID", endX, startY + space * 6);
            else if (savefile->settings.sfr == 3)
                aprint("SLOW", endX, startY + space * 6);

            if (savefile->settings.dropProtection)
                aprint("ON", endX, startY + space * 7);
            else
                aprint("OFF", endX, startY + space * 7);

            if (savefile->settings.finesse)
                aprint("ON", endX, startY + space * 8);
            else
                aprint("OFF", endX, startY + space * 8);

            if (selection == 0) {
                aprint("[", endX - 1, startY + space * selection);
                aprint("]", endX + 2 + (!savefile->settings.announcer), startY + space * selection);
            } else if (selection == 1) {
                aprint("[", endX - 1, startY + space * selection);
                aprint("]", endX + 2 + (!savefile->settings.floatText), startY + space * selection);
            } else if (selection == 2) {
                aprint("[", endX - 1, startY + space * selection);
                aprint("]", endX + 2 + (!savefile->settings.shake), startY + space * selection);
            } else if (selection == 3) {
                if (savefile->settings.volume > 0)
                    aprint("<", endX - 1, startY + space * selection);
                if (savefile->settings.volume < 10)
                    aprint(">", endX + 1, startY + space * selection);
            } else if (selection == 4) {
                if (savefile->settings.das > 8)
                    aprint(">", endX + 3 + (savefile->settings.das != 11), startY + space * selection);
                if (savefile->settings.das < 16)
                    aprint("<", endX - 1, startY + space * selection);
            } else if (selection == 5) {
                if (savefile->settings.arr < 3)
                    aprint("<", endX - 1, startY + space * selection);
                if (savefile->settings.arr > 0)
                    aprint(">", endX + 3 + (savefile->settings.arr != 2), startY + space * selection);

            } else if (selection == 6) {
                if (savefile->settings.sfr < 3)
                    aprint("<", endX - 1, startY + space * selection);
                if (savefile->settings.sfr > 0)
                    aprint(">", endX + 3 + (savefile->settings.sfr != 2), startY + space * selection);
            } else if (selection == 7) {

                aprint("[", endX - 1, startY + space * selection);
                aprint("]", endX + 2 + (!savefile->settings.dropProtection), startY + space * selection);
            } else if (selection == 8) {

                aprint("[", endX - 1, startY + space * selection);
                aprint("]", endX + 2 + (!savefile->settings.finesse), startY + space * selection);
            } else if (selection == 9 || selection == 10) {
                aprint("[", endX - 1, startY + space * selection);
                aprint("]", endX + 2, startY + space * selection);
            } else if (selection == 11) {
                aprint("[", 12, 17);
                aprint("]", 17, 17);
            }
        } else if (toStart == -2) {
            int startX = 4;
            int startY = 2;

            int startY2 = 9;

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
        }
    }
}

void startScreen() {

    for (int i = 0; i < 10; i++)
        wordSprites[i] = new WordSprite(64 + i * 3, 256 + i * 12);

    //initialise background array
    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 30; j++)
            backgroundArray[i][j] = 0;

    int selection = 0;
    int previousSelection = 0;

    int options = (int)menuOptions.size();

    int toStart = 0;
    bool onSettings = false;
    int level = 1;

    int goalSelection = 0;

    bool refreshText = true;

    int maxDas = 16;
    int dasHor = 0;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    bool onPlay = false;

    REG_DISPCNT &= ~DCNT_BG1;
    REG_DISPCNT &= ~DCNT_BG3;
    drawUIFrame(0, 0, 30, 20);

    while (1) {
        VBlankIntrWait();
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

            startText(onSettings, selection, goalSelection, level, toStart);
        }

        key_poll();

        u16 key = key_hit(KEY_FULL);


        if (!onSettings) {

            fallingBlocks();
            for (int i = 0; i < 2; i++)
                obj_unhide(titleSprites[i], 0);
            if (savefile->settings.lightMode)
                memset16(pal_bg_mem, 0x5ad6, 1);//background gray
            else
                memset16(pal_bg_mem, 0x0000, 1);

            int startX, startY, space = 2;
            startX = 12;
            startY = 11;

            std::list<std::string>::iterator index = menuOptions.begin();
            for (int i = 0; i < (int)menuOptions.size(); i++) {
                wordSprites[i]->setText(*index);
                wordSprites[i]->show((startX - 5 * onPlay) * 8, (startY + i * space) * 8, 15 - !((onPlay && i == 0) || (!onPlay && selection == i)));
                ++index;
            }

            int offset = (int)menuOptions.size();

            index = gameOptions.begin();
            for (int i = 0; i < (int)gameOptions.size(); i++) {
                wordSprites[i + offset]->setText(*index);
                int height = i - selection;
                if (onPlay && height >= -2 && height < 4) {
                    wordSprites[i + offset]->show((startX + 5) * 8, (startY + height * space) * 8, 15 - (selection != i));
                } else
                    wordSprites[i + offset]->hide();
                ++index;
            }

            for (int i = 0; i < 5; i++)
                aprint(" ", startX - 2, startY + space * i);
            if (!onPlay) {
                aprint(">", startX - 2, startY + space * selection);
                aprint(" ", 15, startY);
            } else {
                aprint(">", 15, startY);
            }

            if (key == KEY_A || key == KEY_START || key == KEY_RIGHT) {
                int n = 0;
                if (!onPlay) {
                    if (selection == 0) {
                        onPlay = !onPlay;
                        if (onPlay)
                            options = (int)gameOptions.size();
                        else
                            options = (int)menuOptions.size();
                    } else if (selection == 1) {
                        n = -1;
                        previousSettings = savefile->settings;
                        options = 12;
                    } else if (selection == 2) {
                        n = -2;
                    }

                } else {
                    if (selection == 0) {
                        n = 2;
                        options = 3;
                        // maxClearTimer = marathonClearTimer;
                    } else if (selection == 1) {
                        n = 1;
                        options = 2;
                        goalSelection = 1;// set default goal to 40 lines for sprint
                        // maxClearTimer = 1;
                    } else if (selection == 2) {
                        n = 3;
                        options = 2;
                    } else if (selection == 3) {
                        options = 2;
                        n = 5;
                    } else if (selection == 4) {
                        n = -3;
                        linkConnection->activate();

                    } else if (selection == 5) {
                        sfx(SFX_MENUCONFIRM);
                        delete game;
                        game = new Game(1);
                        game->setLevel(1);
                        game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtection);
                        game->setGoal(0);

                        memset16(&se_mem[25], 0, 20 * 32);
                        memset16(&se_mem[26], 0, 20 * 32);
                        memset16(&se_mem[27], 0, 20 * 32);

                        REG_DISPCNT |= DCNT_BG1;
                        REG_DISPCNT |= DCNT_BG3;

                        clearText();
                        break;
                    }else if (selection == 6){

                        int seed = qran ();
                        startMultiplayerGame(seed);
                        multiplayer = false;

                        delete botGame;
                        botGame = new Game(4,seed);
                        botGame->setGoal(100);
                        botGame->setLevel(1);

                        delete testBot;
                        testBot = new Bot(botGame);
                        
                        memset16(&se_mem[25], 0, 20 * 32);
                        memset16(&se_mem[26], 0, 20 * 32);
                        memset16(&se_mem[27], 0, 20 * 32);

                        REG_DISPCNT |= DCNT_BG1;
                        REG_DISPCNT |= DCNT_BG3;

                        clearText();
                        break;
                    }
                }

                sfx(SFX_MENUCONFIRM);
                if (n != 0) {
                    toStart = n;
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
                    sfx(SFX_MENUCANCEL);
                }
            }

        } else {
            for (int i = 0; i < 10; i++)
                wordSprites[i]->hide();

            for (int i = 0; i < 2; i++)
                obj_hide(titleSprites[i]);

            if (toStart == 2) {
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
            } else if (toStart == 1 || toStart == 3 || toStart == 5) {
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
            } else if (toStart == -1) {
                if (key == KEY_RIGHT || key == KEY_LEFT) {
                    if (selection == 0) {
                        savefile->settings.announcer = !savefile->settings.announcer;
                    } else if (selection == 1) {
                        savefile->settings.floatText = !savefile->settings.floatText;
                    } else if (selection == 2) {
                        savefile->settings.shake = !savefile->settings.shake;
                    } else if (selection == 3) {
                        if (key == KEY_RIGHT && savefile->settings.volume < 10)
                            savefile->settings.volume++;
                        else if (key == KEY_LEFT && savefile->settings.volume > 0)
                            savefile->settings.volume--;

                        if (key_is_down(KEY_LEFT)) {
                            if (dasHor < maxDas) {
                                dasHor++;
                            } else if (savefile->settings.volume > 0) {
                                if (arr++ > maxArr) {
                                    arr = 0;
                                    savefile->settings.volume--;
                                    sfx(SFX_MENUMOVE);
                                    refreshText = true;
                                }
                            }
                        } else if (key_is_down(KEY_RIGHT)) {
                            if (dasHor < maxDas) {
                                dasHor++;
                            } else if (savefile->settings.volume < 10) {
                                if (arr++ > maxArr) {
                                    arr = 0;
                                    savefile->settings.volume++;
                                    sfx(SFX_MENUMOVE);
                                    refreshText = true;
                                }
                            }
                        } else {
                            dasHor = 0;
                        }

                        mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
                    } else if (selection == 4) {
                        if (key == KEY_RIGHT) {
                            if (savefile->settings.das == 16)
                                savefile->settings.das = 11;
                            else if (savefile->settings.das == 11)
                                savefile->settings.das = 9;
                            else if (savefile->settings.das == 9)
                                savefile->settings.das = 8;
                        } else if (key == KEY_LEFT) {
                            if (savefile->settings.das == 8)
                                savefile->settings.das = 9;
                            else if (savefile->settings.das == 9)
                                savefile->settings.das = 11;
                            else if (savefile->settings.das == 11)
                                savefile->settings.das = 16;
                        }
                    } else if (selection == 5) {
                        if (key == KEY_RIGHT && savefile->settings.arr > 0)
                            savefile->settings.arr--;
                        else if (key == KEY_LEFT && savefile->settings.arr < 3)
                            savefile->settings.arr++;
                    } else if (selection == 6) {
                        if (key == KEY_RIGHT && savefile->settings.sfr > 0)
                            savefile->settings.sfr--;
                        else if (key == KEY_LEFT && savefile->settings.sfr < 3)
                            savefile->settings.sfr++;
                    } else if (selection == 7) {
                        savefile->settings.dropProtection = !savefile->settings.dropProtection;
                    } else if (selection == 8) {
                        savefile->settings.finesse = !savefile->settings.finesse;
                    }
                    if (selection != options - 1) {
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }

                if(selection == 3){
                    if (key_is_down(KEY_LEFT)) {
                        if (dasHor < maxDas) {
                            dasHor++;
                        } else if (savefile->settings.volume > 0) {
                            if (arr++ > maxArr) {
                                arr = 0;
                                savefile->settings.volume--;
                                sfx(SFX_MENUMOVE);
                                refreshText = true;
                            }
                        }
                    } else if (key_is_down(KEY_RIGHT)) {
                        if (dasHor < maxDas) {
                            dasHor++;
                        } else if (savefile->settings.volume < 10) {
                            if (arr++ > maxArr) {
                                arr = 0;
                                savefile->settings.volume++;
                                sfx(SFX_MENUMOVE);
                                refreshText = true;
                            }
                        }
                    } else {
                        dasHor = 0;
                    }

                    mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
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

            if ((key == KEY_A || key == KEY_START) && toStart != -3) {
                if (onSettings && toStart == -1 && selection == options - 3 && key == KEY_A) {
                    clearText();
                    sfx(SFX_MENUCONFIRM);
                    graphicTest();
                    clearText();
                    refreshText = true;
                    settingsText();
                } else if (onSettings && toStart == -1 && selection == options - 2 && key == KEY_A) {
                    clearText();
                    sfx(SFX_MENUCONFIRM);
                    songListMenu();
                    clearText();
                    refreshText = true;
                    settingsText();
                } else if (selection != options - 1 && toStart != -2) {
                    selection = options - 1;
                    sfx(SFX_MENUCONFIRM);
                    refreshText = true;
                } else {
                    if (toStart != -1 && toStart != -2) {
                        if (toStart == 2 && goalSelection == 3)
                            toStart = 0;

                        initialLevel = level;

                        delete game;
                        game = new Game(toStart);
                        game->setLevel(level);
                        game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtection);
                        mode = goalSelection;

                        int goal = 0;

                        switch (toStart) {
                        case 1:
                            if (goalSelection == 0)
                                goal = 20;
                            else if (goalSelection == 1)
                                goal = 40;
                            else if (goalSelection == 2)
                                goal = 100;
                            break;
                        case 2:
                            if (goalSelection == 0)
                                goal = 150;
                            else if (goalSelection == 1)
                                goal = 200;
                            else if (goalSelection == 2)
                                goal = 300;
                            break;
                        case 3:
                            if (goalSelection == 0)
                                goal = 10;
                            else if (goalSelection == 1)
                                goal = 20;
                            else if (goalSelection == 2)
                                goal = 100;
                            break;
                        case 5:
                            if (goalSelection == 0)
                                goal = 3 * 3600;
                            else if (goalSelection == 1)
                                goal = 5 * 3600;
                            else if (goalSelection == 2)
                                goal = 10 * 3600;
                            break;
                        }

                        if (goal)
                            game->setGoal(goal);

                        sfx(SFX_MENUCONFIRM);
                        break;
                    } else {
                        if (toStart == -1)
                            saveToSram();
                        onSettings = false;
                        options = (int)menuOptions.size();
                        selection = 0;
                        clearText();
                        refreshText = true;
                        sfx(SFX_MENUCONFIRM);
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
                sfx(SFX_MENUMOVE);
                refreshText = true;
            }

            if (key == KEY_DOWN || key == KEY_SELECT) {
                if (selection == options - 1)
                    selection = 0;
                else
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
                        sfx(SFX_MENUMOVE);
                        refreshText = true;
                    }
                }
            } else {
                dasVer = 0;
            }
        }

        sqran(qran() * frameCounter++);

        oam_copy(oam_mem, obj_buffer, 128);
    }
    VBlankIntrWait();
    clearText();
    onSettings = false;
    irq_disable(II_HBLANK);
    memset16(pal_bg_mem, 0x0000, 1);

    memset16(&se_mem[26], 0x0000, 32 * 20);
    memset16(&se_mem[27], 0x0000, 32 * 20);


    if (savefile->settings.lightMode)
        memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        memset16(pal_bg_mem, 0x0000, 1);
}

void fallingBlocks() {
    gravity++;
    bgSpawnBlock++;

    int i, j;
    if (gravity > gravityMax) {
        gravity = 0;

        for (i = 23; i >= 0; i--) {
            for (j = 0; j < 30; j++) {
                if (i == 0)
                    backgroundArray[i][j] = 0;
                else
                    backgroundArray[i][j] = backgroundArray[i - 1][j];
            }
        }
    }

    u16* dest = (u16*)se_mem[25];

    for (i = 4; i < 24; i++) {
        for (j = 0; j < 30; j++) {
            if (!backgroundArray[i][j])
                *dest++ = 2 * (!savefile->settings.lightMode);
            else
                *dest++ = (1 + (((u32)(backgroundArray[i][j] - 1)) << 12));
        }
        dest += 2;
    }

    if (bgSpawnBlock > bgSpawnBlockMax) {
        bgSpawnBlock = 0;
        int x = qran() % 27;
        int n = qran() % 7;
        int** p = game->getShape(n, qran() % 4);

        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                if (backgroundArray[i][j + x])
                    return;

        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
                if (p[i][j])
                    backgroundArray[i][j + x] = n + 1;
    }
}

void endScreen() {
    mmStop();

    int selection = 0;

    endAnimation();

    if (game->gameMode != 4 && game->gameMode != 5) {
        if (game->won == 1)
            sfx(SFX_CLEAR);
        else if (game->lost == 1)
            sfx(SFX_GAMEOVER);
    } else if (game->gameMode == 5) {
        if (game->won == 1)
            sfx(SFX_TIME);
        else if (game->lost == 1)
            sfx(SFX_GAMEOVER);
    } else {
        if (game->won == 1)
            sfx(SFX_YOUWIN);
        else if (game->lost == 1)
            sfx(SFX_YOULOSE);
    }

    playSongRandom(0);

    showStats();
    int record = onRecord();

    if (multiplayer) {
        enemyHeight = 0;
        progressBar();
    }

    while (1) {
        handleMultiplayer();
        VBlankIntrWait();
        key_poll();

        showStats();

        if (record != -1 && game->gameMode != 4){
            std::string str;
            switch(record){
            case 0:
                str = "1st";
                break;
            case 1:
                str = "2nd";
                break;
            case 2:
                str = "3rd";
                break;
            case 3:
                str = "4th";
                break;
            case 4:
                str = "5th";
                break;
            }
            aprint(str+" Place", 10, 7);
        }

        aprint("Play", 12, 14);
        aprint("Again", 14, 15);

        aprint("Main", 12, 17);
        aprint("Menu", 14, 18);

        if (selection == 0) {
            aprint(">", 10, 14);
            aprint(" ", 10, 17);
        } else if (selection == 1) {
            aprint(" ", 10, 14);
            aprint(">", 10, 17);
        }

        u16 key = key_hit(KEY_FULL);

        if (playAgain) {
            playAgain = false;
            startMultiplayerGame(nextSeed);

            mmStop();
            playSongRandom(1);

            floatingList.clear();

            drawFrame();
            clearText();

            oam_init(obj_buffer, 128);
            showFrames();
            oam_copy(oam_mem, obj_buffer, 128);
            countdown();
            update();
            break;
        }

        if (key == KEY_A || key == KEY_START) {
            if (!selection) {
                shake = 0;

                if (!multiplayer) {
                    sfx(SFX_MENUCONFIRM);
                    int goal = game->goal;
                    delete game;
                    game = new Game(game->gameMode);
                    game->setGoal(goal);
                    game->setLevel(initialLevel);
                    game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtection);

                    mmStop();

                    floatingList.clear();

                    drawFrame();
                    clearText();

                    oam_init(obj_buffer, 128);
                    showFrames();
                    oam_copy(oam_mem, obj_buffer, 128);
                    countdown();

                    if (!(game->gameMode == 1 && game->goal == 0)) {
                        playSongRandom(1);
                    }

                    update();

                    if (ENABLE_BOT) {
                        delete testBot;
                        testBot = new Bot(game);
                    }

                    break;

                } else {
                    if (connected == 1) {
                        sfx(SFX_MENUCONFIRM);
                        multiplayerStartTimer = 5;
                        nextSeed = (u16)qran();
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

            } else {
                sfx(SFX_MENUCONFIRM);
                reset();
            }
        }

        if (key == KEY_UP) {
            if (selection == 0)
                selection = 1;
            else
                selection--;
            sfx(SFX_MENUMOVE);
        }

        if (key == KEY_DOWN || key == KEY_SELECT) {
            if (selection == 1)
                selection = 0;
            else
                selection++;
            sfx(SFX_MENUMOVE);
        }
    }
}

void endAnimation() {
    clearText();
    oam_init(obj_buffer, 128);
    showHold();
    showQueue();
    oam_copy(oam_mem, obj_buffer, 128);

    REG_BG0HOFS = 0;
    REG_BG1HOFS = 0;
    REG_BG0VOFS = 0;
    REG_BG1VOFS = 0;

    // int timer = 0;
    // int maxTimer = 20;

    // while(timer++ < maxTimer)
    // 	VBlankIntrWait();

    sfx(SFX_END);

    for (int i = 0; i < 41; i++) {
        VBlankIntrWait();
        drawGrid();
        showClearText();
        REG_BG0VOFS = -1 * i * 4;
        u16* dest = (u16*)se_mem[25];

        if (i % 2 != 0)
            continue;

        dest += 32 * (20 - i / 2) + 10;

        for (int j = 0; j < 10; j++) {
            *dest++ = 0;
        }
    }

    while (!floatingList.empty()) {
        VBlankIntrWait();
        showClearText();
    }
    clearText();

    // u16*dest,*dest2;
    // for(int i = 19; i >= 0; i--){
    // 	dest = (u16*)se_mem[25];
    // 	dest += i * 32;

    // 	dest2 = (u16*)se_mem[26];
    // 	dest2 += i * 32;

    // 	// int timer = 0;
    // 	VBlankIntrWait();

    // 	for(int j = 0; j < 32; j++)
    // 		*dest++ = *dest2++ = 0;
    // }

    REG_BG0HOFS = 0;
    REG_BG1HOFS = 0;
    REG_BG0VOFS = 0;
    REG_BG1VOFS = 0;
}

void showStats() {
    if (game->gameMode == 4) {
        if (game->lost)
            aprint("YOU LOSE", 11, 4);
        else
            aprint("YOU WIN!", 11, 4);

        aprint("Lines Sent", 10, 8);
        aprintf(game->linesSent, 14, 10);

    } else if (game->gameMode == 0 || game->gameMode == 2 || game->lost || game->gameMode == 5) {
        std::string score = std::to_string(game->score);

        if (game->lost)
            aprint("GAME  OVER", 10, 4);
        else {
            if (game->gameMode != 5)
                aprint("CLEAR!", 12, 4);
            else
                aprint("TIME!", 12, 4);
        }

        if (game->gameMode == 0 || game->gameMode == 2 || game->gameMode == 5)
            aprint(score, 15 - ((int)score.size() / 2), 9);
    } else {
        aprint("CLEAR!", 12, 4);

        aprint(timeToString(gameSeconds), 11, 8);
    }
}

void pauseMenu() {

    int selection = 0;
    int maxSelection;

    if (!onStates)
        maxSelection = 4;
    else
        maxSelection = 3;

    for (int i = 0; i < 20; i++)
        aprint("          ", 10, i);


    while (1) {
        VBlankIntrWait();
        key_poll();

        aprint("PAUSE!", 12, 6);

        for (int i = 0; i < maxSelection; i++)
            aprint(" ", 10, 11 + 2 * i);

        aprint(">", 10, 11 + 2 * selection);

        u16 key = key_hit(KEY_FULL);

        if (!onStates) {
            aprint("Resume", 12, 11);
            aprint("Restart", 12, 13);
            aprint("Sleep", 12, 15);
            aprint("Quit", 12, 17);

            if (key == KEY_A) {
                if (selection == 0) {
                    sfx(SFX_MENUCONFIRM);
                    clearText();
                    update();
                    pause = false;
                    mmResume();
                    break;
                } else if (selection == 1) {
                    restart = true;
                    pause = false;
                    sfx(SFX_MENUCONFIRM);
                    break;
                } else if (selection == 2) {
                    sleep();
                } else if (selection == 3) {
                    sfx(SFX_MENUCONFIRM);
                    reset();
                }
            }
        } else {
            aprint("Resume", 12, 11);
            aprintColor("Load", 12, 13, !(saveExists));
            aprint("Save", 12, 15);

            if (key == KEY_A) {
                if (selection == 0) {
                    sfx(SFX_MENUCONFIRM);
                    clearText();
                    update();
                    pause = false;
                    onStates = false;
                    mmResume();
                    break;
                } else if (selection == 1) {
                    if (saveExists) {
                        delete game;
                        game = new Game(*quickSave);
                        update();
                        showBackground();
                        showPawn();
                        showShadow();
                        showHold();
                        update();
                        floatingList.clear();
                        clearGlow();

                        if (ENABLE_BOT) {
                            delete testBot;
                            testBot = new Bot(game);
                        }

                        aprint("Loaded!", 22, 18);
                        sfx(SFX_MENUCONFIRM);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else if (selection == 2) {
                    delete quickSave;
                    quickSave = new Game(*game);
                    saveExists = true;

                    aprint("Saved!", 23, 18);
                    sfx(SFX_MENUCONFIRM);
                }
            }
        }

        if (key == KEY_START || key == KEY_SELECT) {
            sfx(SFX_MENUCONFIRM);
            clearText();
            update();
            pause = false;
            onStates = false;
            mmResume();
            break;
        }

        if (key == KEY_B) {
            sfx(SFX_MENUCONFIRM);
            clearText();
            update();
            pause = false;
            onStates = false;
            mmResume();
            break;
        }

        if (key == KEY_UP) {
            if (selection == 0)
                selection = maxSelection - 1;
            else
                selection--;
            sfx(SFX_MENUMOVE);
        }

        if (key == KEY_DOWN) {
            if (selection == maxSelection - 1)
                selection = 0;
            else
                selection++;
            sfx(SFX_MENUMOVE);
        }

        oam_copy(oam_mem, obj_buffer, 128);
    }
}

int onRecord() {
    int place = -1;

    for (int i = 0; i < 5; i++) {
        if (game->gameMode == 0 || game->gameMode == 2) {
            if (game->score < savefile->marathon[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->marathon[mode].highscores[j + 1] = savefile->marathon[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->marathon[mode].highscores[i].score = game->score;
            strncpy(savefile->marathon[mode].highscores[i].name, name.c_str(), 9);

        } else if (game->gameMode == 1 && game->won == 1) {
            if (gameSeconds > savefile->sprint[mode].times[i].frames && savefile->sprint[mode].times[i].frames)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->sprint[mode].times[j + 1] = savefile->sprint[mode].times[j];

            std::string name = nameInput(i);

            savefile->sprint[mode].times[i].frames = gameSeconds;
            strncpy(savefile->sprint[mode].times[i].name, name.c_str(), 9);

        } else if (game->gameMode == 3 && game->won == 1) {
            if (gameSeconds > savefile->dig[mode].times[i].frames && savefile->dig[mode].times[i].frames)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->dig[mode].times[j + 1] = savefile->dig[mode].times[j];

            std::string name = nameInput(i);

            savefile->dig[mode].times[i].frames = gameSeconds;
            strncpy(savefile->dig[mode].times[i].name, name.c_str(), 9);
        } else if (game->gameMode == 5) {
            if (game->score < savefile->ultra[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->ultra[mode].highscores[j + 1] = savefile->ultra[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->ultra[mode].highscores[i].score = game->score;
            strncpy(savefile->ultra[mode].highscores[i].name, name.c_str(), 9);

        }

        place = i;

        saveToSram();
        break;
    }

    return place;
}
