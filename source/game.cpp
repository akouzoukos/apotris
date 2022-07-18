#include "def.h"
#include "soundbank.h"
#include "sprites.h"
#include "tetrisEngine.h"
#include "tonc.h"
#include "tonc_core.h"
#include "tonc_input.h"
#include "tonc_math.h"
#include "tonc_memdef.h"
#include "tonc_oam.h"
#include <string>

using namespace Tetris;

void diagnose();
void clearGlow();
void addGlow(Drop);
void countdown();
void drawFrame();
void drawGrid();

Game* game;
OBJ_ATTR* pawnSprite;
OBJ_ATTR* pawnShadow;
OBJ_ATTR* holdSprite;
OBJ_ATTR* holdFrameSprite;
OBJ_ATTR* queueFrameSprites[3];

OBJ_ATTR* queueSprites[5];

bool onStates = false;

int clearTimer = 0;
int maxClearTimer = 20;
// int maxClearTimer = 1;

std::string clearTypeText = "";
int maxClearTextTimer = 100;
int clearTextHeight = 16;

int glow[20][10];

int push = 0;
int pushTimer = 0;
int trainingMessageTimer = 0;

int restartTimer = 0;
#define maxRestartTimer 30

std::list<FloatText> floatingList;

std::list<Effect> effectList;

Bot *testBot;

void checkSounds() {
    if (game->sounds.hold == 1)
        sfx(SFX_HOLD);
    if (game->sounds.shift == 1)
        sfx(SFX_SHIFT);
    if (game->sounds.place == 1) {
        sfx(SFX_PLACE);
        shake = shakeMax/2;
    }
    if (game->sounds.invalid == 1)
        sfx(SFX_INVALID);
    if (game->sounds.rotate == 1)
        sfx(SFX_ROTATE);
    if (game->sounds.clear == 1) {

        int speed = game->comboCounter - 1;
        if (speed > 10)
            speed = 10;

        mm_sound_effect clear = {
{SFX_LEVELUP},
            (mm_hword)((1.0 + (float)speed / 10) * (1 << 10)),
            0,
            255,
            128,
        };

        mmEffectEx(&clear);

        int soundEffect = -1;

        clearTypeText = "";
        if (game->previousClear.isPerfectClear == 1) {
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
    if (game->sounds.levelUp == 1) {
        sfx(SFX_LEVELUPSOUND);
    }
    game->resetSounds();
}

void showBackground() {

    bool showEdges = savefile->settings.edges;

    u16* dest = (u16*)se_mem[25];

    std::list<int>::iterator l2c = game->linesToClear.begin();
    // n  int neib[3][3];
    bool up, down, left, right;
    bool before = false, after = false;

    dest += 10;
    for (int i = 20; i < 40; i++) {
        if (game->linesToClear.size() > 0) {
            std::list<int>::iterator l2c2 = game->linesToClear.begin();
            before = after = false;
            while (l2c2 != game->linesToClear.end()) {
                if (*l2c2 == i - 1) {
                    before = true;

                }
                if (*l2c2 == i + 1) {
                    after = true;
                }
                ++l2c2;
            }
        }

        for (int j = 0; j < 10; j++) {
            // draw white for clear animation
            if (!game->board[i][j] || (game->clearLock && i == *l2c && showEdges)) {
                if (!showEdges) {
                    *dest++ = 0;
                    continue;
                }

                up = (game->board[i - 1][j] > 0 && !before);
                left = (j - 1 >= 0 && game->board[i][j - 1] > 0 && !(i == *l2c && game->clearLock));
                right = (j + 1 <= 9 && game->board[i][j + 1] > 0 && !(i == *l2c && game->clearLock));
                down = (i + 1 <= 39 && game->board[i + 1][j] > 0 && !after);

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
            } else
                *dest++ = (1 + (((u32)(game->board[i][j] - 1)) << 12));
            if (game->clearLock && i == *l2c) {
                dest--;
                if (!showEdges)
                    *dest = 0;
                if (j < 5) {
                    if (clearTimer < maxClearTimer - 10 + j * 2)
                        *dest = 3 + savefile->settings.lightMode * 0x1000;
                } else {
                    if (clearTimer < maxClearTimer - 10 + (10 - j) * 2)
                        *dest = 3 + savefile->settings.lightMode * 0x1000;
                }
                dest++;
            }
        }
        if (i == *l2c)
            std::advance(l2c, 1);
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
            if (game->pawn.board[game->pawn.rotation][i][j] > 0)
                memcpy16(&tile_mem[4][16 * 7 + i * 4 + j], blockSprite, sprite1tiles_bin_size / 2);
            else
                memset16(&tile_mem[4][16 * 7 + i * 4 + j], 0, sprite1tiles_bin_size / 2);
        }
    }

    int n = game->pawn.current;
    obj_set_attr(pawnSprite, ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(n));
    pawnSprite->attr2 = ATTR2_BUILD(16 * 7, n, 1);
    obj_set_pos(pawnSprite, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->pawn.y - 20) * 8);
}

void showShadow() {
    pawnShadow = &obj_buffer[1];
    if (game->clearLock) {
        obj_hide(pawnShadow);
        return;
    }

    u8* shadowTexture;

    bool bld = false;
    switch (savefile->settings.shadow) {
    case 0:
        shadowTexture = (u8*)sprite2tiles_bin;
        break;
    case 1:
        shadowTexture = (u8*)sprite15tiles_bin;
        break;
    case 2:
        shadowTexture = (u8*)sprite16tiles_bin;
        break;
    case 3:
        shadowTexture = blockSprite;
        bld = true;
        break;
    case 4:
        obj_hide(pawnShadow);
        return;
    default:
        shadowTexture = (u8*)sprite2tiles_bin;
        break;
    }

    obj_unhide(pawnShadow, 0);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (game->pawn.board[game->pawn.rotation][i][j] > 0)
                memcpy16(&tile_mem[4][16 * 8 + i * 4 + j], shadowTexture, sprite2tiles_bin_size / 2);
            else
                memset16(&tile_mem[4][16 * 8 + i * 4 + j], 0, sprite2tiles_bin_size / 2);
        }
    }

    int n = game->pawn.current;

    if (!savefile->settings.lightMode)
        clr_fade_fast((COLOR*)&palette[n * 16], 0x0000, &pal_obj_mem[8 * 16], 16, (10) * bld);
    else
        clr_adj_brightness(&pal_obj_mem[8 * 16], (COLOR*)&palette[n * 16], 16, float2fx(0.15));

    obj_set_attr(pawnShadow, ATTR0_SQUARE, ATTR1_SIZE(2), 8);

    pawnShadow->attr2 = ATTR2_BUILD(16 * 8, 8, 1);
    obj_set_pos(pawnShadow, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->lowest() - 20) * 8 + shake * savefile->settings.shake);
}

void showHold() {
    holdSprite = &obj_buffer[2];
    holdFrameSprite = &obj_buffer[10];
    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6));

    obj_unhide(holdFrameSprite, 0);
    obj_set_attr(holdFrameSprite, ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
    holdFrameSprite->attr2 = ATTR2_BUILD(512, color, 1);
    obj_set_pos(holdFrameSprite, 4 * 8 + 5 + (push < 0) * push, 9 * 8 - 2);

    if (game->held == -1) {
        obj_hide(holdSprite);
        return;
    }

    int add = !(game->held == 0 || game->held == 3);
    int palette = (game->canHold)? game->held : 7;

    if (savefile->settings.skin == 0 || savefile->settings.skin == 5) {
        obj_unhide(holdSprite, 0);
        obj_set_attr(holdSprite, ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_PALBANK(palette));
        holdSprite->attr2 = ATTR2_BUILD(9 * 16 + 8 * game->held, palette, 1);
        obj_set_pos(holdSprite, (5) * 8 + add * 3 + 1 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0));
    } else {
        obj_unhide(holdSprite, ATTR0_AFF);
        obj_set_attr(holdSprite, ATTR0_WIDE | ATTR0_AFF, ATTR1_SIZE(2) | ATTR1_AFF_ID(5), ATTR2_PALBANK(palette));
        holdSprite->attr2 = ATTR2_BUILD(16 * game->held, palette, 1);
        FIXED size = float2fx(1.4);
        obj_aff_scale(&obj_aff_buffer[5], size, size);
        obj_set_pos(holdSprite, (5) * 8 + add * 3 + 1 - 4 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0) - 3);
    }
}

void showQueue() {
    for (int i = 0; i < 5; i++)
        queueSprites[i] = &obj_buffer[3 + i];

    for (int i = 0; i < 3; i++)
        queueFrameSprites[i] = &obj_buffer[11 + i];
    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6));
    for (int i = 0; i < 3; i++) {
        obj_unhide(queueFrameSprites[i], 0);
        obj_set_attr(queueFrameSprites[i], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
        queueFrameSprites[i]->attr2 = ATTR2_BUILD(512 + 16 + 16 * i, color, 1);
        obj_set_pos(queueFrameSprites[i], 173 + (push > 0) * push, 12 + 32 * i);
    }

    std::list<int>::iterator q = game->queue.begin();
    for (int k = 0; k < 5; k++) {

        int n = *q;

        int add = !(n == 0 || n == 3);
        if (savefile->settings.skin == 0 || savefile->settings.skin == 5) {
            obj_unhide(queueSprites[k], 0);
            obj_set_attr(queueSprites[k], ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_PALBANK(n));
            queueSprites[k]->attr2 = ATTR2_BUILD(16 * 9 + 8 * n, n, 1);
            obj_set_pos(queueSprites[k], (22) * 8 + add * 3 + 1 + (push > 0) * push, (3 + (k * 3)) * 6 - 3 * (n == 0));
        } else {
            obj_unhide(queueSprites[k], ATTR0_AFF);
            obj_set_attr(queueSprites[k], ATTR0_SQUARE | ATTR0_AFF, ATTR1_SIZE(2) | ATTR1_AFF_ID(k), ATTR2_PALBANK(n));
            queueSprites[k]->attr2 = ATTR2_BUILD(16 * n, n, 1);
            obj_aff_identity(&obj_aff_buffer[k]);
            FIXED size = 358;//~1.4
            obj_aff_scale(&obj_aff_buffer[k], size, size);
            obj_set_pos(queueSprites[k], (22) * 8 + add * 3 + 1 - 4 + (push > 0) * push, (3 + (k * 3)) * 6 - 3 * (n == 0) - 4);
        }

        ++q;
    }
}

void control() {
    if (pause)
        return;

    key_poll();
    u16 key = key_hit(KEY_FULL);

    if (KEY_START == (key & KEY_START) && !multiplayer) {
        sfx(SFX_MENUCONFIRM);
        pause = true;
        mmPause();
        clearText();
        update();
    }

    if (KEY_R == (key & KEY_R)) {
        game->hold();
    }

    if (KEY_UP == (key & KEY_UP))
        game->keyDrop();

    if (KEY_DOWN == (key & KEY_DOWN))
        game->keyDown(1);

    if (KEY_LEFT == (key & KEY_LEFT))
        game->keyLeft(1);

    if (KEY_RIGHT == (key & KEY_RIGHT))
        game->keyRight(1);

    if (key_is_down(KEY_A) && key_is_down(KEY_B)){
        game->hold();
        // update();
    }else{
        if (KEY_A == (key & KEY_A))
            game->rotateCW();

        if (KEY_B == (key & KEY_B))
            game->rotateCCW();
    }

    // if (KEY_L == (key & KEY_L) || KEY_R == (key & KEY_R)) {
    //     game->hold();
    //     update();
    // }


    if (KEY_L == (key & KEY_L)){
        game->rotateTwice();
    }

    if (KEY_SELECT == (key & KEY_SELECT) && game->goal == 0 && game->gameMode == 1) {
        sfx(SFX_MENUCONFIRM);
        pause = true;
        mmPause();
        clearText();
        update();
        onStates = true;
    }

    key = key_released(KEY_FULL);

    if (KEY_DOWN == (key & KEY_DOWN))
        game->keyDown(0);

    if (KEY_LEFT == (key & KEY_LEFT))
        game->keyLeft(0);

    if (KEY_RIGHT == (key & KEY_RIGHT))
        game->keyRight(0);

    if (key_is_down(KEY_L) && key_is_down(KEY_R) && (game->gameMode != 4)) {
        if(restartTimer++ > maxRestartTimer)
            restart = true;
    }else{
        restartTimer = 0;
    }
}

void showTimer() {
    if (!(game->gameMode == 1 && game->goal == 0)) {
        std::string timer = timeToString(gameSeconds);
        aprint(timer, 1, 1);
    }

    if(game->gameMode == 1){
        showPPS();
    }
}

void showText() {
    if (game->gameMode == 0 || game->gameMode == 2 || game->gameMode == 5) {

        aprint("Score", 3, 3);

        std::string score = std::to_string(game->score);
        aprint(score, 8 - score.size(), 5);

        if (game->gameMode != 5) {
            aprint("Level", 2, 14);

            aprintf(game->level, 4, 15);
        }

    } else if (game->gameMode == 1) {
        if (savefile->settings.finesse) {
            aprint("Finesse", 1, 14);
            aprintf(game->finesse, 4, 15);
        }
        if (game->goal == 0) {
            aprint("Training", 1, 1);
            // std::string bagCount = std::to_string(game->bagCounter-1)
            // aprint(bagCount,26,1);
        }

        aprints("PPS:",0,0,2);
        showPPS();
    }

    if (game->gameMode != 4) {
        aprint("Lines", 2, 17);
        std::string lines;
        if (game->gameMode == 3)
            aprintf(game->garbageCleared, 4, 18);
        else
            aprintf(game->linesCleared, 4, 18);
    } else {
        aprint("Attack", 2, 17);
        aprintf(game->linesSent, 4, 18);
    }

    if (game->goal == 0 && trainingMessageTimer < TRAINING_MESSAGE_MAX) {
        aprint("Press", 1, 3);
        aprint("SELECT", 1, 5);
        aprint("for Saves", 1, 7);
    }
}

void showPPS(){
    FIXED t = gameSeconds * float2fx(0.0167f);

    if(t <= 0)
        return;

    FIXED pps =  fxdiv(int2fx(game->pieceCounter),(t));

    std::string str = "";

    str += std::to_string(fx2int(pps)) + ".";

    int fractional = pps & 0xff;
    for(int i = 0; i < 3; i++){
        fractional *= 10;
        str += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

    aprints(str,25,0,2);
}

void showClearText() {
    if (game->comboCounter > 1) {
        aprint("Combo x", 21, clearTextHeight - 1);

        aprintf(game->comboCounter, 28, clearTextHeight - 1);
    } else {
        aprint("          ", 20, clearTextHeight - 1);
    }

    if (game->b2bCounter > 0) {
        aprint("Streak", 22, clearTextHeight + 1);

        aprint("x", 24, clearTextHeight + 2);
        aprintf(game->b2bCounter + 1, 25, clearTextHeight + 2);
    } else {
        aprint("          ", 20, clearTextHeight + 1);
        aprint("          ", 20, clearTextHeight + 2);
    }

    std::list<FloatText>::iterator index = floatingList.begin();

    for (int i = 0; i < (int)floatingList.size(); i++) {
        std::string text = (*index).text;
        if (index->timer++ > maxClearTextTimer) {
            index = floatingList.erase(index);
            aprint("          ", 10, 0);
        } else {
            int height = 0;
            if (index->timer < 2 * maxClearTextTimer / 3)
                height = 5 * (float)index->timer / (2 * maxClearTextTimer / 3);
            else
                height = (30 * (float)(index->timer) / maxClearTextTimer) - 15;
            if (text.size() <= 10) {
                aprint(text, 15 - text.size() / 2, 15 - height);
            } else {
                aprint("          ", 10, 15 - height);

                std::size_t pos = text.find(" ");
                std::string part1 = text.substr(0, pos);
                std::string part2 = text.substr(pos + 1);

                if (15 - height - 1 > 0)
                    aprint(part1, 15 - part1.size() / 2, 15 - height - 1);
                aprint(part2, 15 - part2.size() / 2, 15 - height);
            }
            aprint("          ", 10, 15 - height + 1);
            std::advance(index, 1);
        }
    }
}


void showFrames() {
    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6));
    obj_unhide(holdFrameSprite, 0);
    obj_set_attr(holdFrameSprite, ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
    holdFrameSprite->attr2 = ATTR2_BUILD(512, color, 1);
    obj_set_pos(holdFrameSprite, 4 * 8 + 5, 9 * 8 - 2);

    for (int i = 0; i < 3; i++) {
        obj_unhide(queueFrameSprites[i], 0);
        obj_set_attr(queueFrameSprites[i], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
        queueFrameSprites[i]->attr2 = ATTR2_BUILD(512 + 16 + 16 * i, color, 1);
        obj_set_pos(queueFrameSprites[i], 173, 12 + 32 * i);
    }
}

void gameLoop(){
    setSmallTextArea(100, 2, 14, 8, 16);
    update();

    while (1) {
        diagnose();
        if (!game->lost && !pause) {
            game->update();
        }
        checkSounds();
        handleMultiplayer();

        progressBar();

        if(ENABLE_BOT){
            profile_start();
            testBot->run();
            botGame->update();
            handleBotGame();
            addToResults(profile_stop(),0);
        }

        gameSeconds = game->timer;

        if (game->clearLock) {
            clearTimer++;
        }

        if (clearTimer == maxClearTimer) {
            game->removeClearLock();
            shake = -shakeMax;
            clearTimer = 0;
        }

        Tetris::Drop latestDrop = game->getDrop();

        if (latestDrop.on)
            addGlow(latestDrop);

        canDraw = 1;
        VBlankIntrWait();

        if (game->won || game->lost)
            endScreen();

        if (pause)
            pauseMenu();

        if (game->goal == 0 && trainingMessageTimer < TRAINING_MESSAGE_MAX) {
            if (++trainingMessageTimer == TRAINING_MESSAGE_MAX) {
                aprint("     ", 1, 3);
                aprint("      ", 1, 5);
                aprint("         ", 1, 7);
            }
        }

        if (restart) {
            restart = false;
            int oldGoal = game->goal;

            mmStop();

            delete game;
            game = new Game(game->gameMode);
            game->setGoal(oldGoal);
            game->setLevel(initialLevel);
            game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtection);

            memset32(&se_mem[25], 0x0000, 32 * 10);
            memset32(&se_mem[27], 0x0000, 32 * 10);

            VBlankIntrWait();
            oam_init(obj_buffer, 128);
            oam_copy(oam_mem, obj_buffer, 128);

            clearText();

            showFrames();
            oam_copy(oam_mem, obj_buffer, 128);
            progressBar();

            clearGlow();
            floatingList.clear();

            countdown();

            if (!(game->gameMode == 1 && game->goal == 0)) {
                playSongRandom(1);
            }

            update();
        }

        sqran(qran() % frameCounter++);

        // addToResults(profile_stop(),3);
    }
}

void addGlow(Tetris::Drop location) {
    for (int i = 0; i < location.endY && i < 20; i++)
        for (int j = location.startX; j < location.endX; j++)
            glow[i][j] = glowDuration;

    if (game->comboCounter > 0) {
        int xCenter = (location.endX - location.startX) / 2 + location.startX;
        if (game->previousClear.isTSpin) {
            for (int i = 0; i < 20; i++)
                for (int j = 0; j < 10; j++)
                    glow[i][j] = glowDuration + abs(xCenter - j) + abs(location.endY - i);

            if (game->previousClear.isBackToBack == 1) {
                effectList.push_back(Effect(2, xCenter, location.endY));
            }
        } else {
            for (int i = 0; i < 20; i++)
                for (int j = 0; j < 10; j++)
                    glow[i][j] = glowDuration + Sqrt(abs(xCenter - j) * abs(xCenter - j) + abs(location.endY - i) * abs(location.endY - i));

            if (game->previousClear.isBackToBack == 1) {
                effectList.push_back(Effect(1, xCenter, location.endY));
            }

        }
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
    showFrames();
    oam_copy(oam_mem, obj_buffer, 128);


    while (timer++ < timerMax - 1) {
        VBlankIntrWait();

        if (timer < timerMax / 3) {
            aprint("READY?", 12, 10);
            if (timer == 1)
                sfx(SFX_READY);
        } else if (timer < 2 * timerMax / 3){
            key_poll();
            aprint("READY?", 12, 10);
            // if(key_is_down(KEY_L) && key_is_down(KEY_R)){
            //     restart = true;
            //     break;
            // }
        }else {
            aprint("  GO  ", 12, 10);
            if (timer == 2 * timerMax / 3)
                sfx(SFX_GO);
        }
    }
    showBackground();
    clearText();
}

void screenShake() {
    if (!savefile->settings.shake)
        return;

    REG_BG0VOFS = -shake;
    REG_BG1VOFS = -shake;

    if (shake != 0) {
        if (shake > 0)
            shake--;
        else
            shake++;
        shake *= -1;
    }

    REG_BG0HOFS = -push;
    REG_BG1HOFS = -push;
    if (game->pushDir != 0) {
        if (abs(push) < pushMax)
            push += game->pushDir;
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

    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6)) * 0x1000;

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
        obj_set_attr(queueFrameSprites[i], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
        queueFrameSprites[i]->attr2 = ATTR2_BUILD(512 + 16 + 16 * i, color, 0);
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

    u16* dest = (u16*)se_mem[26];
    dest += 10;

    u32 gridTile;
    int palOffset = 4;

    switch (savefile->settings.backgroundGrid) {
    case 0:
        gridTile = 0x0002;
        break;
    case 1:
        gridTile = 0x000c;
        break;
    case 2:
        gridTile = 0x001a;
        break;
    case 3:
        gridTile = 0x001e;
        break;
    case 4:
        gridTile = 0x001f;
        break;
    case 5:
        gridTile = 0x0020;
        break;
    default:
        gridTile = 0x0002;
        break;
    }

    if (savefile->settings.lightMode)
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
