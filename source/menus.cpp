#include "maxmod.h"
#include "rumble.h"
#include "tonc.h"
#include "def.h"
#include "soundbank.h"
#include "tetrisEngine.h"
#include "logging.h"
#include "text.h"
#include "tonc_memmap.h"
#include "tonc_oam.h"
#include <string>
#include <map>

using namespace Tetris;

void endAnimation();
void showScore();
void showStats(bool, std::string, std::string);
int onRecord();
std::string nameInput(int);

int mode = 0;
bool showingStats = false;

bool saveExists = false;
Tetris::Game* quickSave;

const std::string modeStrings[9] = {
    "Marathon",
    "Sprint",
    "Dig",
    "Battle",
    "Ultra",
    "Blitz",
    "Combo",
    "Survival",
    "Classic",
};

const std::string modeOptionStrings[9][4] = {
    {"150","200","300","Endless"},
    {"20","40","100"},
    {"10","20","100"},
    {""},
    {"3","5","10"},
    {""},
    {""},
    {"EASY","MEDIUM","HARD"},
    {"",""},
};

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

int endScreen() {
    clearSmallText();
    setSmallTextArea(100, 1, 1, 10, 20);

    mmStop();

    int selection = 0;

    //calculate pps
    FIXED t = gameSeconds * float2fx(0.0167f);
    FIXED pps =  fxdiv(int2fx(game->pieceCounter),(t));

    std::string ppsStr = std::to_string(fx2int(pps)) + ".";

    int fractional = pps & 0xff;
    for(int i = 0; i < 2; i++){
        fractional *= 10;
        ppsStr += '0' + (fractional >> 8);
        fractional &= 0xff;
    }

    std::string totalTime = timeToString(gameSeconds);

    endAnimation();

    int prevBld = REG_BLDCNT;
    REG_BLDCNT = (1 << 6) + (0b1111 << 9) + (1);
    memset16(&se_mem[25], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);

    if(savefile->settings.announcer){
        if (game->gameMode == ULTRA || game->gameMode == BLITZ) {
            if (game->won == 1)
                sfx(SFX_TIME);
            else if (game->lost == 1)
                sfx(SFX_GAMEOVER);
        }else if (game->gameMode != BATTLE) {
            if (game->won == 1)
                sfx(SFX_CLEAR);
            else if (game->lost == 1)
                sfx(SFX_GAMEOVER);
        } else {
            if (game->won == 1)
                sfx(SFX_YOUWIN);
            else if (game->lost == 1)
                sfx(SFX_YOULOSE);
        }
    }

    playSongRandom(0);

    showScore();
    int record = onRecord();

    if (multiplayer) {
        enemyHeight = 0;
        progressBar();
    }

    savefile->stats.gamesCompleted++;
    if(game->lost)
        savefile->stats.gamesLost++;

    while (1) {
        handleMultiplayer();
        VBlankIntrWait();
        key_poll();

        showScore();
        showStats(showingStats, totalTime, ppsStr);

        if (record != -1 && game->gameMode != BATTLE && (game->won || game->gameMode == MARATHON || game->gameMode >= ULTRA)){
            std::string str;
            switch(record){
                case 0: str = "1st"; break;
                case 1: str = "2nd"; break;
                case 2: str = "3rd"; break;
                case 3: str = "4th"; break;
                case 4: str = "5th"; break;
            }
            aprint(str+" Place", 11, 5);
        }

        if(game->gameMode > 0 && game->gameMode <= 9){
            int counter = 0;
            std::string str;
            std::string str2;

            str = modeStrings[game->gameMode-1];

            if(game->gameMode == SPRINT && game->goal == 0)
                str = "Training";

            aprintColor(str,30-str.size(),counter++,0);

            str = "";
            if(game->gameMode == SPRINT && game->goal == 0){
                if(game->trainingMode)
                    str = "Finesse";
            }else{
                if(game->gameMode != CLASSIC)
                    str = modeOptionStrings[game->gameMode-1][mode];
                else
                    str = modeOptionStrings[game->gameMode-1][0];
            }

            if(str != "")
                aprintColor(str,30-str.size(),counter++,0);

            str = "";
            str2 = "";
            if(game->subMode){
                switch(game->gameMode){
                case SPRINT: str = "Attack"; break;
                case DIG: str = "Efficiency"; break;
                case CLASSIC:
                    str = "B-Type";
                    str2 = std::to_string(initialLevel) + "-" + std::to_string(game->bTypeHeight);
                    break;
                }
            }else{
                if(game->gameMode == CLASSIC)
                    str = "A-Type";
            }

            if(str != "")
                aprintColor(str,30-str.size(),counter++,0);
            if(str2 != "")
                aprintColor(str2,30-str2.size(),counter++,0);

            if(bigMode)
                aprintColor("BIG MODE",22,counter++,0);
        }

        aprint("Play", 12, 11);
        aprint("Again", 14, 12);

        aprint("Change", 12, 14);
        aprint("Options", 14, 15);

        aprint("Main", 12, 17);
        aprint("Menu", 14, 18);

        if (selection == 0) {
            aprint(">", 10, 11);
            aprint(" ", 10, 14);
            aprint(" ", 10, 17);
        } else if (selection == 1) {
            aprint(" ", 10, 11);
            aprint(">", 10, 14);
            aprint(" ", 10, 17);
        } else if (selection == 2) {
            aprint(" ", 10, 11);
            aprint(" ", 10, 14);
            aprint(">", 10, 17);
        }

        u16 key = key_hit(KEY_FULL);

        if (playAgain && multiplayer) {
            startMultiplayerGame(nextSeed);

            mmStop();
            playSongRandom(1);

            floatingList.clear();
            placeEffectList.clear();

            drawFrame();
            clearText();

            oam_init(obj_buffer, 128);
            showHold();
            showQueue();
            oam_copy(oam_mem, obj_buffer, 128);
            memset16(&se_mem[25], 0, 32 * 20);
            countdown();
            update();
            break;
        }

        if (key == KEY_A || key == KEY_START) {
            if (selection == 0) {
                shake = 0;

 if (!multiplayer) {
                    sfx(SFX_MENUCONFIRM);
                    playAgain = true;
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

            } else if (selection == 1){
                sfx(SFX_MENUCANCEL);
                REG_BLDCNT = prevBld;
                goToOptions = true;
                return 1;
            } else if (selection == 2){
                sfx(SFX_MENUCANCEL);
                REG_BLDCNT = prevBld;
                return 1;
            }
        }

        if (key == KEY_UP) {
            if (selection == 0)
                selection = 2;
            else
                selection--;
            sfx(SFX_MENUMOVE);
        }

        if (key == KEY_DOWN || key == KEY_SELECT) {
            if (selection == 2)
                selection = 0;
            else
                selection++;
            sfx(SFX_MENUMOVE);
        }

        if(key == KEY_L){
            sfx(SFX_MENUCONFIRM);
            showingStats = !showingStats;
            clearSmallText();
        }
    }

    REG_BLDCNT = prevBld;
    return 0;
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

    rumble_set_state(RumbleState(rumble_stop));

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

void showScore(){
    if (game->gameMode == BATTLE) {
        if (game->lost)
            aprint("YOU LOSE", 11, 3);
        else
            aprint("YOU WIN!", 11, 3);

        aprint("Lines Sent", 10, 5);
        aprintf(game->linesSent, 14, 7);

    } else if (game->gameMode == MARATHON || game->lost || game->gameMode >= ULTRA || (game->gameMode == DIG && subMode == 1)) {
        std::string score;

        if(game->gameMode == DIG)
            score = std::to_string(game->pieceCounter);
        else if (game->gameMode != 7)
            score = std::to_string(game->score);
        else
            score = std::to_string(game->linesCleared);

        if (game->lost)
            aprint("GAME OVER", 11, 3);
        else {
            if (game->gameMode != ULTRA && game->gameMode != BLITZ)
                aprint("CLEAR!", 12, 3);
            else
                aprint("TIME!", 13, 3);
        }

        if (game->gameMode == MARATHON || game->gameMode == ULTRA || game->gameMode == BLITZ || game->gameMode == COMBO || game->gameMode == DIG || game->gameMode == CLASSIC)
            aprint(score, 15 - ((int)score.size() / 2), 7);
        else if (game->gameMode == SURVIVAL)
            aprint(timeToString(gameSeconds), 11, 7);
    } else {
        aprint("CLEAR!", 12, 3);

        aprint(timeToString(gameSeconds), 11, 7);
    }
}

void showStats(bool moreStats, std::string time, std::string pps) {
    if(!moreStats){
        aprints("Press L", 0, 0, 2);
        aprints("for Stats", 0, 7, 2);
        return;
    }

    int counter = 0;

    int g = game->gameMode;

    aprints("Time: " + time,0,7*counter++,2);
    if(g == MARATHON || g == BLITZ || g == CLASSIC)
        aprints("Level: " + std::to_string(game->level),0,7*counter++,2);

    aprints("Lines: " + std::to_string(game->linesCleared),0,7*counter++,2);
    aprints("Pieces: " + std::to_string(game->pieceCounter), 0, 7*counter++, 2);

    aprints("PPS: " + pps, 0, 7*counter++, 2);

    aprints("Finesse: " + std::to_string(game->finesseFaults), 0, 7*counter++, 2);

    aprints("Singles: " + std::to_string(game->statTracker.clears[0]), 0, 7*counter++, 2);
    aprints("Doubles: " + std::to_string(game->statTracker.clears[1]), 0, 7*counter++, 2);
    aprints("Triples: " + std::to_string(game->statTracker.clears[2]), 0, 7*counter++, 2);
    aprints("Quads: " + std::to_string(game->statTracker.clears[3]), 0, 7*counter++, 2);
    aprints("T-Spins: " + std::to_string(game->statTracker.tspins), 0, 7*counter++, 2);
    aprints("Perfect Clears: " + std::to_string(game->statTracker.perfectClears), 0, 7*counter++, 2);
    aprints("Max Streak: " + std::to_string(game->statTracker.maxStreak), 0, 7*counter++, 2);
    aprints("Max Combo: " + std::to_string(game->statTracker.maxCombo), 0, 7*counter++, 2);
    aprints("Times Held: " + std::to_string(game->statTracker.holds), 0, 7*counter++, 2);
}

int pauseMenu(){
    int selection = 0;
    int maxSelection;

    int optionsHeight = 10;
    int optionsCounter = 0;

    // for (int i = 0; i < 20; i++)
    //     aprint("          ", 10, i);
    clearText();

    int prevBld = REG_BLDCNT;
    REG_BLDCNT = (1 << 6) + (0b1111 << 9) + (1);
    memset16(&se_mem[25], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);

    hideMinos();
    obj_hide(&obj_buffer[24]); //hide finesse combo counter

    oam_copy(oam_mem, obj_buffer, 128);

    while (1) {
        if (!onStates){
            if(game->goal == 0 && game->gameMode == SPRINT)
                maxSelection = 5;
            else
                maxSelection = 4;
        } else
            maxSelection = 3;

        VBlankIntrWait();
        key_poll();

        aprint("PAUSE!", 12, 4);

        for (int i = 0; i < maxSelection; i++)
            aprint(" ", 10, optionsHeight + 2 * i);

        aprint(">", 10, optionsHeight + 2 * selection);

        u16 key = key_hit(KEY_FULL);

        if (!onStates) {
            optionsCounter = 0;
            aprint("Resume", 12, optionsHeight + optionsCounter++ * 2);
            aprint("Restart", 12, optionsHeight + optionsCounter++ * 2);

            if(game->goal == 0 && game->gameMode == SPRINT)
                aprint("Saves", 12, optionsHeight + optionsCounter++ * 2);

            aprint("Sleep", 12, optionsHeight + optionsCounter++ * 2);
            aprint("Quit", 12, optionsHeight + optionsCounter++ * 2);

            if (key == KEY_A) {
                int n = selection;
                if(!(game->goal == 0 && game->gameMode == SPRINT) && selection >= 2)
                    n++;

                if (n == 0) {
                    sfx(SFX_MENUCONFIRM);
                    clearText();
                    update();
                    pause = false;
                    mmResume();
                    break;
                } else if (n == 1) {
                    // restart = true;
                    playAgain = true;
                    pause = false;
                    sfx(SFX_MENUCONFIRM);
                    break;
                } else if (n == 2) {
                    onStates = true;
                    selection = 0;
                    clearText();
                    update();
                } else if (n == 3) {
                    sleep();
                } else if (n == 4) {
                    REG_BLDCNT = prevBld;
                    sfx(SFX_MENUCANCEL);
                    return 1;
                }
            }
        } else {
            optionsCounter = 0;
            aprintColor("Load", 12, optionsHeight + optionsCounter++ * 2, !(saveExists));
            aprint("Save", 12, optionsHeight + optionsCounter++ * 2);
            aprint("Back", 12, optionsHeight + optionsCounter++ * 2);

            if (key == KEY_A) {
                if (selection == 0) {
                    if (saveExists) {
                        delete game;
                        game = new Game(*quickSave);
                        game->setTuning(getTuning());
                        game->pawn.big = bigMode;
                        update();
                        showBackground();
                        showPawn();
                        showShadow();
                        showHold();
                        showQueue();
                        update();
                        floatingList.clear();
                        placeEffectList.clear();
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
                } else if (selection == 1) {
                    delete quickSave;
                    quickSave = new Game(*game);
                    saveExists = true;

                    aprint("Saved!", 23, 18);
                    sfx(SFX_MENUCONFIRM);
                } else if (selection == 2) {
                    sfx(SFX_MENUCONFIRM);
                    clearText();
                    update();
                    onStates = false;
                    selection = 0;
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
            if(onStates){
                clearText();
                onStates = false;
                selection = 0;
                update();
            }else{
                clearText();
                update();
                pause = false;
                mmResume();
            }
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

    REG_BLDCNT = prevBld;
    memset16(&se_mem[25], 0 , 32 * 20);
    showBackground();

    return 0;
}

int onRecord() {
    int place = -1;

    if(bigMode)
        return place;

    for (int i = 0; i < 5; i++) {
        if (game->gameMode == MARATHON) {
            if (game->score < savefile->marathon[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->marathon[mode].highscores[j + 1] = savefile->marathon[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->marathon[mode].highscores[i].score = game->score;
            strncpy(savefile->marathon[mode].highscores[i].name, name.c_str(), 9);

        } else if (game->gameMode == SPRINT && game->won == 1) {
            if(subMode == 0){
                if (gameSeconds > savefile->sprint[mode].times[i].frames && savefile->sprint[mode].times[i].frames)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->sprint[mode].times[j + 1] = savefile->sprint[mode].times[j];

                std::string name = nameInput(i);

                savefile->sprint[mode].times[i].frames = gameSeconds;
                strncpy(savefile->sprint[mode].times[i].name, name.c_str(), 9);
            }else{
                if (gameSeconds > savefile->sprintAttack[mode].times[i].frames && savefile->sprintAttack[mode].times[i].frames)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->sprintAttack[mode].times[j + 1] = savefile->sprintAttack[mode].times[j];

                std::string name = nameInput(i);

                savefile->sprintAttack[mode].times[i].frames = gameSeconds;
                strncpy(savefile->sprintAttack[mode].times[i].name, name.c_str(), 9);
            }
        } else if (game->gameMode == DIG && game->won == 1) {
            if(subMode == 0){
                if (gameSeconds > savefile->dig[mode].times[i].frames && savefile->dig[mode].times[i].frames)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->dig[mode].times[j + 1] = savefile->dig[mode].times[j];

                std::string name = nameInput(i);

                savefile->dig[mode].times[i].frames = gameSeconds;
                strncpy(savefile->dig[mode].times[i].name, name.c_str(), 9);
            }else{
                if (game->pieceCounter > savefile->digEfficiency[mode].highscores[i].score && savefile->digEfficiency[mode].highscores[i].score)
                    continue;

                for (int j = 3; j >= i; j--)
                    savefile->digEfficiency[mode].highscores[j + 1] = savefile->digEfficiency[mode].highscores[j];

                std::string name = nameInput(i);

                savefile->digEfficiency[mode].highscores[i].score = game->pieceCounter;
                strncpy(savefile->digEfficiency[mode].highscores[i].name, name.c_str(), 9);
            }
        } else if (game->gameMode == ULTRA) {
            if (game->score < savefile->ultra[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->ultra[mode].highscores[j + 1] = savefile->ultra[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->ultra[mode].highscores[i].score = game->score;
            strncpy(savefile->ultra[mode].highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == BLITZ) {
            if (game->score < savefile->blitz[mode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->blitz[mode].highscores[j + 1] = savefile->blitz[mode].highscores[j];

            std::string name = nameInput(i);

            savefile->blitz[mode].highscores[i].score = game->score;
            strncpy(savefile->blitz[mode].highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == COMBO) {
            if (game->linesCleared < savefile->combo.highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->combo.highscores[j + 1] = savefile->combo.highscores[j];

            std::string name = nameInput(i);

            savefile->combo.highscores[i].score = game->linesCleared;
            strncpy(savefile->combo.highscores[i].name, name.c_str(), 9);
        } else if (game->gameMode == SURVIVAL) {
            if (gameSeconds < savefile->survival[mode].times[i].frames && savefile->survival[mode].times[i].frames)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->survival[mode].times[j + 1] = savefile->survival[mode].times[j];

            std::string name = nameInput(i);

            savefile->survival[mode].times[i].frames = gameSeconds;
            strncpy(savefile->survival[mode].times[i].name, name.c_str(), 9);
        } else if (game->gameMode == CLASSIC) {
            if (game->score < savefile->classic[subMode].highscores[i].score)
                continue;

            for (int j = 3; j >= i; j--)
                savefile->classic[subMode].highscores[j + 1] = savefile->classic[subMode].highscores[j];

            std::string name = nameInput(i);

            savefile->classic[subMode].highscores[i].score = game->score;
            strncpy(savefile->classic[subMode].highscores[i].name, name.c_str(), 9);
        }

        place = i;

        saveToSram();
        break;
    }

    return place;
}
