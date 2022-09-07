#include "def.h"
#include "soundbank.h"
#include "sprites.h"
#include "tetrisEngine.h"
#include "tetromino.hpp"
#include "tonc.h"
#include "tonc_core.h"
#include "tonc_input.h"
#include "tonc_math.h"
#include "tonc_memdef.h"
#include "tonc_oam.h"
#include <string>
#include "text.h"

using namespace Tetris;

void diagnose();
void clearGlow();
void addGlow(Drop);
void countdown();
void drawFrame();
void drawGrid();
void progressBar();
void showBar(int, int, int, int);
void showBestMove();
bool checkDiagonal(int);
void showFinesse();

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
// int maxClearTimer = 1;

std::string clearTypeText = "";
int maxClearTextTimer = 100;
int clearTextHeight = 16;

int glow[20][10];

int push = 0;
int pushTimer = 0;
int trainingMessageTimer = 0;

int restartTimer = 0;
#define maxRestartTimer 20

int attackFlashTimer = 0;
int attackFlashMax = 10;

std::list<FloatText> floatingList;

std::list<Effect> effectList;

Bot *testBot;

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
    }
    if (game->sounds.invalid)
        sfx(SFX_INVALID);
    if (game->sounds.rotate)
        sfx(SFX_ROTATE);
    if (game->sounds.finesse){
        if(game->trainingMode)
            showBestMove();
        sfx(SFX_MENUCANCEL);
    }

    if (game->sounds.clear) {

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

    if (game->sounds.levelUp) {
        sfx(SFX_LEVELUPSOUND);
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
                    if (clearTimer < maxClearTimer - 10 + (9 - j) * 2)
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
    pawnSprite->attr2 = ATTR2_BUILD(16 * 7, n, 2);
    obj_set_pos(pawnSprite, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->pawn.y - 20) * 8 + shake * savefile->settings.shake);
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
        clr_fade_fast((COLOR*)&palette[savefile->settings.colors][n * 16], 0x0000, &pal_obj_mem[8 * 16], 16, (14) * bld);
    else
        clr_adj_brightness(&pal_obj_mem[8 * 16], (COLOR*)&palette[savefile->settings.colors][n * 16], 16, float2fx(0.15));

    obj_set_attr(pawnShadow, ATTR0_SQUARE, ATTR1_SIZE(2), 8);

    pawnShadow->attr2 = ATTR2_BUILD(16 * 8, 8, 2);
    obj_set_pos(pawnShadow, (10 + game->pawn.x) * 8 + push * savefile->settings.shake, (game->lowest() - 20) * 8 + shake * savefile->settings.shake);
}

void showHold() {
    holdSprite = &obj_buffer[2];
    holdFrameSprite = &obj_buffer[10];
    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6));

    obj_unhide(holdFrameSprite, 0);
    obj_set_attr(holdFrameSprite, ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
    holdFrameSprite->attr2 = ATTR2_BUILD(512, color, 3);
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
        holdSprite->attr2 = ATTR2_BUILD(9 * 16 + 8 * game->held, palette, 3);
        obj_set_pos(holdSprite, (5) * 8 + add * 3 + 1 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0));
    } else {
        obj_unhide(holdSprite, ATTR0_AFF);
        obj_set_attr(holdSprite, ATTR0_WIDE | ATTR0_AFF, ATTR1_SIZE(2) | ATTR1_AFF_ID(5), ATTR2_PALBANK(palette));
        holdSprite->attr2 = ATTR2_BUILD(16 * game->held, palette, 3);
        FIXED size = float2fx(1.4);
        obj_aff_scale(&obj_aff_buffer[5], size, size);
        obj_set_pos(holdSprite, (5) * 8 + add * 3 + 1 - 4 + (push < 0) * push, (10) * 8 - 3 * (game->held == 0) - 3);
    }
}

void showQueue() {
    int maxQueue = savefile->settings.maxQueue;

    for (int i = 0; i < maxQueue; i++)
        queueSprites[i] = &obj_buffer[3 + i];

    for (int i = 0; i < 3; i++)
        queueFrameSprites[i] = &obj_buffer[11 + i];
    int color = (savefile->settings.palette + 2 * (savefile->settings.palette > 6));

    if(maxQueue > 1){
        for (int i = 0; i < 3; i++) {
            obj_unhide(queueFrameSprites[i], 0);
            obj_set_attr(queueFrameSprites[i], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
            queueFrameSprites[i]->attr2 = ATTR2_BUILD(512 + 16 + 16 * i, color, 3);
            obj_set_pos(queueFrameSprites[i], 173 + (push > 0) * push, 12 + 32 * i - (i * 9 * (5-maxQueue)));
        }
    }else{
        obj_unhide(queueFrameSprites[0], 0);
        obj_set_attr(queueFrameSprites[0], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_PALBANK(color));
        queueFrameSprites[0]->attr2 = ATTR2_BUILD(512, color, 3);
        obj_set_pos(queueFrameSprites[0], 173 + (push > 0) * push, 12);

        obj_hide(queueFrameSprites[1]);
        obj_hide(queueFrameSprites[2]);
    }

    int startX = 22 * 8 + 1;
    int yoffset = 3 * (maxQueue == 1);

    std::list<int>::iterator q = game->queue.begin();
    for (int k = 0; k < 5; k++) {

        if(k >= maxQueue){
            obj_hide(queueSprites[k]);
            q++;
            continue;
        }

        int n = *q;

        int add = !(n == 0 || n == 3);
        if ((savefile->settings.skin == 0 || savefile->settings.skin == 5)) {
            obj_unhide(queueSprites[k], 0);
            obj_set_attr(queueSprites[k], ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_PALBANK(n));
            queueSprites[k]->attr2 = ATTR2_BUILD(16 * 9 + 8 * n, n, 3);
            obj_set_pos(queueSprites[k], startX + add * 3 + (push > 0) * push, (3 + (k * 3)) * 6 - 3 * (n == 0) + yoffset);
        } else {
            obj_unhide(queueSprites[k], ATTR0_AFF);
            obj_set_attr(queueSprites[k], ATTR0_SQUARE | ATTR0_AFF, ATTR1_SIZE(2) | ATTR1_AFF_ID(k), ATTR2_PALBANK(n));
            queueSprites[k]->attr2 = ATTR2_BUILD(16 * n, n, 3);
            obj_aff_identity(&obj_aff_buffer[k]);
            FIXED size = 358;//~1.4
            obj_aff_scale(&obj_aff_buffer[k], size, size);
            obj_set_pos(queueSprites[k], startX + add * 3 + (push > 0) * push - 4, (3 + (k * 3)) * 6 - 3 * (n == 0) - 4 + yoffset);
        }
        ++q;
    }
}

void control() {
    if (pause)
        return;

    key_poll();

    Keys k = savefile->settings.keys;

    if (key_hit(KEY_START) && !multiplayer) {
        sfx(SFX_MENUCONFIRM);
        pause = true;
        mmPause();
        clearText();
        update();
    }

    if (key_hit(k.hold) && !checkDiagonal(k.hold)) {
        game->hold();
    }

    if (key_hit(k.moveLeft) && !checkDiagonal(k.moveLeft))
        game->keyLeft(1);
    if (key_hit(k.moveRight) && !checkDiagonal(k.moveRight))
        game->keyRight(1);

    if (key_hit(k.hardDrop) && !checkDiagonal(k.hardDrop))
        game->keyDrop();

    if (key_hit(k.softDrop) && !checkDiagonal(k.softDrop))
        game->keyDown(1);

    if (key_is_down(KEY_A) && key_is_down(KEY_B) && savefile->settings.abHold){
        game->hold();
    }else{
        if (key_hit(k.rotateCW) && !checkDiagonal(k.rotateCW))
            game->rotateCW();

        if (key_hit(k.rotateCCW) && !checkDiagonal(k.rotateCCW))
            game->rotateCCW();
    }

    if (key_hit(k.rotate180)){
        game->rotateTwice();
    }

    if (key_hit(KEY_SELECT) && game->goal == 0 && game->gameMode == 1) {
        sfx(SFX_MENUCONFIRM);
        pause = true;
        mmPause();
        clearText();
        update();
        onStates = true;
    }

    if (key_released(k.softDrop))
        game->keyDown(0);

    if (key_released(k.moveLeft))
        game->keyLeft(0);

    if (key_released(k.moveRight))
        game->keyRight(0);

    if (key_is_down(KEY_L) && key_is_down(KEY_R) && (game->gameMode != 4)) {
        if(restartTimer++ > maxRestartTimer || !savefile->settings.resetHold)
            playAgain = true;
    }else{
        restartTimer = 0;
    }
}

void showTimer() {
    if (!(game->gameMode == 1 && game->goal == 0)) {
        std::string timer = timeToString(gameSeconds);
        aprint(timer, 1, 1);
    }

    if(game->trainingMode){
    //     clearSmallText();
    //     aprints("PPS:",0,0,2);
    //     showPPS();
        // aprints("Finesse:",0,7,2);
        // showFinesse();
        aprint("Finesse", 1, 14);
        aprintf(game->finesse, 4, 15);
    }

    if (game->goal == 0 && trainingMessageTimer < TRAINING_MESSAGE_MAX) {
        if (++trainingMessageTimer == TRAINING_MESSAGE_MAX) {
            aprint("     ", 1, 3);
            aprint("      ", 1, 5);
            aprint("         ", 1, 7);
        }
    }
}

void showText() {
    if (game->gameMode == 0 || game->gameMode == 2 || game->gameMode == 5 || game->gameMode == 6) {

        aprint("Score", 3, 3);

        std::string score = std::to_string(game->score);
        aprint(score, 8 - score.size(), 5);

        if (game->gameMode != 5) {
            aprint("Level", 2, 14);

            aprintf(game->level, 4, 15);
        }

    } else if (game->gameMode == 1) {
        // if (savefile->settings.finesse) {
        //     aprint("Finesse", 1, 14);
        //     aprintf(game->finesse, 4, 15);
        // }
        if (game->goal == 0) {
            aprint("Training", 1, 1);
        }
    }

    if (game->gameMode != 4 && game->gameMode != 6){
        aprint("Lines", 2, 17);
        if (game->gameMode == 3)
            aprintf(game->garbageCleared, 4, 18);
        else
            aprintf(game->linesCleared, 4, 18);

    } else if(game->gameMode == 6){
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

        aprint(str,2,18);

    } else {
        aprint("Attack", 2, 17);
        aprintf(game->linesSent, 4, 18);
    }

    if (game->goal == 0 && trainingMessageTimer < TRAINING_MESSAGE_MAX && game->gameMode != 7) {
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
    for(int i = 0; i < 2; i++){
        fractional *= 10;
        str += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

    aprints(str,25,0,2);
}

void showFinesse(){
    if(gameSeconds == 0)
        return;
    aprints(std::to_string(game->finesse),4*9,7,2);
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

void gameLoop(){
    clearSmallText();
    setSmallTextArea(100, 2, 14, 8, 17);
    gameSeconds = 0;
    update();

    oam_init(obj_buffer, 128);
    drawFrame();

    mmStop();

    showHold();
    showQueue();

    oam_copy(oam_mem, obj_buffer, 128);

    countdown();

    if (!(game->gameMode == 1 && game->goal == 0)) {
        playSongRandom(1);
    }

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

        Tetris::Drop latestDrop = game->getDrop();

        if (latestDrop.on)
            addGlow(latestDrop);

        canDraw = true;
        VBlankIntrWait();

        if (clearTimer == maxClearTimer || (game->gameMode == 8 && clearTimer)) {
            game->removeClearLock();
            shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
            clearTimer = 0;
            update();
        }

        if (game->won || game->lost){
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
        }

        if (playAgain) {
            return;
        }

        sqran(qran() % frameCounter++);
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

void progressBar() {
    if (game->goal == 0)
        return;

    int current;
    int max = game->goal;

    if (game->gameMode == 3)
        current = game->garbageCleared;
    else if (game->gameMode == 5 || game->gameMode == 6)
        current = game->timer;
    else if (game->gameMode != 8)
        current = game->linesCleared;
    else
        return;

    int color = savefile->settings.palette + 2 * (savefile->settings.palette > 6);

    showBar(current, max, 20, color);

    if (game->gameMode == 4) {
        if (++attackFlashTimer > attackFlashMax)
            attackFlashTimer = 0;

        memcpy16(&pal_bg_mem[8 * 16], &palette[savefile->settings.colors][color * 16], 16);
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
    if(!savefile->settings.noDiagonals)
        return false;
    return ((key == KEY_DOWN || key == KEY_UP) && (key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)));
}
