#include "def.h"
#include "flashSaves.h"
#include "tonc.h"
#include "sprite1tiles_bin.h"
#include "sprites.h"

void setDefaultKeys();
void addStats();
void resetSkins(Save * save);
void setDefaults(Save * save, int depth);

void saveToSram() {
    addStats();

    volatile u8* sf = (volatile u8*)sram_mem;

    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)sizeof(Save); i++)
        sf[i] = (u8)arr[i];

    if(ENABLE_FLASH_SAVE)
        save_sram_flash();
}

void loadFromSram() {
    volatile u8* sf = (volatile u8*)sram_mem;

    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)sizeof(Save); i++)
        arr[i] = (u8)sf[i];
}

void loadSave() {
    delete savefile;
    savefile = new Save();
    loadFromSram();

    if (savefile->newGame == 0x4f) {
        setDefaults(savefile,5);
    }else if (savefile->newGame == 0x4e) {
        setDefaults(savefile,4);
    }else if (savefile->newGame == 0x4d) {
        Save* temp = new Save();
        int oldSize = sizeof(Test3);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[sizeof(Settings)+ sizeof(u8)], &sf[oldSize], (sizeof(Save) - oldSize) / 2);

        setDefaults(temp,3);

        memcpy32(savefile, temp, sizeof(Save) / 4);

        delete temp;
    }else if (savefile->newGame == 0x4c) {
        Save* temp = new Save();
        int oldSize = sizeof(Test2) + sizeof(u8);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[sizeof(Settings) + sizeof(u8)], &sf[oldSize], (sizeof(Save) - oldSize) / 2);

        setDefaults(temp,2);

        memcpy32(savefile, temp, sizeof(Save) / 4);

        setDefaultKeys();

        delete temp;
    }else if (savefile->newGame == 0x4b) {
        Save* temp = new Save();
        int oldSize = sizeof(Test) + sizeof(u8);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[sizeof(Settings) + sizeof(u8)], &sf[oldSize], (sizeof(Save) - oldSize) / 2);

        setDefaults(temp,1);

        setDefaultKeys();

        memcpy32(savefile, temp, sizeof(Save) / 4);

        delete temp;

    } else if (savefile->newGame != SAVE_TAG) {
        savefile = new Save();

        setDefaults(savefile,0);

        setDefaultKeys();

        resetSkins(savefile);
    }

    //fix invalid values, since I messed up conversion at some point
    if ((savefile->settings.lightMode != 0) && (savefile->settings.lightMode != 1))
        savefile->settings.lightMode = false;

    if ((savefile->settings.rumble < 0) || (savefile->settings.rumble > 4))
        savefile->settings.rumble = 0;

    savefile->newGame = SAVE_TAG;

    u8* dump = (u8*)savefile;
    int sum = 0;
    for (int i = 0; i < (int)sizeof(Save); i++)
        sum += dump[i];
    sqran(sum);

    savefile->seed = qran();
    saveToSram();
}

void setDefaultKeys(){
    Keys k;
    k.moveLeft = KEY_LEFT;
    k.moveRight = KEY_RIGHT;
    k.rotateCW = KEY_A;
    k.rotateCCW = KEY_B;
    k.rotate180 = 0;
    k.softDrop = KEY_DOWN;
    k.hardDrop = KEY_UP;
    k.hold = KEY_R | KEY_L;

    savefile->settings.keys = k;
}

void addStats(){
    savefile->stats.timePlayed += frameCounter;
    frameCounter = 1;
}

void resetSkins(Save *save){
    for(int i = 0; i < MAX_CUSTOM_SKINS; i++){
        memcpy16(&save->customSkins[i].board,sprite1tiles_bin,sprite1tiles_bin_size/2);
        memcpy16(&save->customSkins[i].smallBoard,mini[0],sprite1tiles_bin_size/2);
    }
}

void setDefaults(Save *save, int depth){

    if(depth < 1){
        save->settings.announcer = true;
        save->settings.finesse = false;
        save->settings.floatText = true;
        save->settings.shake = true;
        save->settings.effects = true;
        save->settings.das = 11;
        save->settings.arr = 2;
        save->settings.sfr = 2;
        save->settings.dropProtection = true;
        save->settings.volume = 10;

        for (int i = 0; i < 8; i++)
            savefile->latestName[i] = ' ';
        savefile->latestName[8] = '\0';

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 5; j++)
                savefile->marathon[i].highscores[j].score = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                savefile->sprint[i].times[j].frames = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                savefile->dig[i].times[j].frames = 0;
    }

    if(depth < 2){
        save->settings.edges = false;
        save->settings.backgroundGrid = 0;
        save->settings.skin = 0;
        save->settings.palette = 6;
        save->settings.shadow = 0;
        save->settings.lightMode = false;

        for (int i = 0; i < 10; i++)
            save->settings.songList[i] = true;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                save->ultra[i].highscores[j].score = 0;
    }

    if(depth < 3){
        save->settings.sfxVolume = 10;
        save->settings.directionalDas = false;
        save->settings.noDiagonals = false;
        save->settings.maxQueue = 5;
        save->settings.colors = 0;
        save->settings.cycleSongs = true;
        save->settings.dropProtectionFrames = 8;
        save->settings.abHold = true;
        save->settings.clearEffect = 0;
        save->settings.resetHold = false;

        if(save->settings.shake)
            save->settings.shakeAmount = 2;
        else{
            save->settings.shake = true;
            save->settings.shakeAmount = 0;
        }

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 5; j++)
                save->blitz[i].highscores[j].score = 0;

        for (int j = 0; j < 5; j++)
            save->combo.highscores[j].score = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                save->survival[i].times[j].frames = 0;
    }

    if(depth < 4){
        save->settings.placeEffect = false;
        save->settings.rumble = 0;

    }

    if(depth < 5){
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                save->sprintAttack[i].times[j].frames = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                save->digEfficiency[i].highscores[j].score = 0;

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 5; j++)
                save->classic[i].highscores[j].score = 0;

        save->stats.timePlayed = 0;
        save->stats.gamesStarted = 0;
        save->stats.gamesCompleted = 0;
        save->stats.gamesLost = 0;

        resetSkins(save);
    }

    if(depth < 6){
        for (int i = 0; i < 2; i++){
            for (int j = 0; j < 5; j++){
                save->master[i].times[j].frames = 0;
                save->master[i].grade[j] = -1;
            }
        }
    }
}


//TODO: marathon zone saving
