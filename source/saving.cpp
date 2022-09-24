#include "def.h"
#include "flashSaves.h"
#include "tonc.h"
#include "tonc_memdef.h"

void setDefaultKeys();

void saveToSram() {
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

    if (savefile->newGame == 0x4d) {
        Save* temp = new Save();
        int oldSize = sizeof(Test3);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[sizeof(Settings)+ sizeof(u8)], &sf[oldSize], (sizeof(Save) - oldSize) / 2);

        temp->newGame = SAVE_TAG;
        temp->settings.placeEffect = false;

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

        temp->newGame = SAVE_TAG;
        temp->settings.sfxVolume = 10;
        temp->settings.directionalDas = false;

        if(temp->settings.shake)
            temp->settings.shakeAmount = 2;
        else{
            temp->settings.shake = true;
            temp->settings.shakeAmount = 0;
        }

        temp->settings.noDiagonals = false;
        temp->settings.maxQueue = 5;
        temp->settings.colors = 0;
        temp->settings.cycleSongs = true;
        temp->settings.dropProtectionFrames = 8;
        temp->settings.abHold = true;
        temp->settings.clearEffect = 0;
        temp->settings.resetHold = false;

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 5; j++)
                temp->blitz[i].highscores[j].score = 0;

        for (int j = 0; j < 5; j++)
            temp->combo.highscores[j].score = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                temp->survival[i].times[j].frames = 0;

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

        temp->newGame = SAVE_TAG;
        temp->settings.edges = false;
        temp->settings.backgroundGrid = 0;
        temp->settings.skin = 0;
        temp->settings.palette = 6;
        temp->settings.shadow = 0;
        temp->settings.lightMode = false;
        temp->settings.sfxVolume = 10;
        temp->settings.directionalDas = false;
        temp->settings.noDiagonals = false;
        temp->settings.maxQueue = 5;
        temp->settings.colors = 0;
        temp->settings.cycleSongs = true;
        temp->settings.dropProtectionFrames = 8;
        temp->settings.abHold = true;
        temp->settings.clearEffect = 0;
        temp->settings.resetHold = false;

        if(temp->settings.shake)
            temp->settings.shakeAmount = 2;
        else{
            temp->settings.shake = true;
            temp->settings.shakeAmount = 0;
        }

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 5; j++)
                temp->blitz[i].highscores[j].score = 0;

        for (int j = 0; j < 5; j++)
            temp->combo.highscores[j].score = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                temp->survival[i].times[j].frames = 0;

        for (int i = 0; i < 10; i++)
            temp->settings.songList[i] = true;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                temp->ultra[i].highscores[j].score = 0;

        memcpy32(savefile, temp, sizeof(Save) / 4);

        delete temp;

    } else if (savefile->newGame != SAVE_TAG) {
        savefile = new Save();
        savefile->newGame = SAVE_TAG;

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

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                savefile->ultra[i].highscores[j].score = 0;

        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 5; j++)
                savefile->blitz[i].highscores[j].score = 0;

        for (int j = 0; j < 5; j++)
            savefile->combo.highscores[j].score = 0;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                savefile->survival[i].times[j].frames = 0;

        savefile->settings.announcer = true;
        savefile->settings.finesse = false;
        savefile->settings.floatText = true;
        savefile->settings.shake = true;
        savefile->settings.effects = true;
        savefile->settings.das = 11;
        savefile->settings.arr = 2;
        savefile->settings.sfr = 2;
        savefile->settings.dropProtection = true;
        savefile->settings.edges = false;
        savefile->settings.backgroundGrid = 0;
        savefile->settings.skin = 0;
        savefile->settings.palette = 6;
        savefile->settings.shadow = 0;
        savefile->settings.lightMode = false;

        savefile->settings.sfxVolume = 10;
        savefile->settings.directionalDas = false;
        savefile->settings.shakeAmount = 2;
        savefile->settings.noDiagonals = false;
        savefile->settings.maxQueue = 5;
        savefile->settings.colors = 0;
        savefile->settings.cycleSongs = true;
        savefile->settings.dropProtectionFrames = 8;
        savefile->settings.abHold = true;
        savefile->settings.clearEffect = 0;
        savefile->settings.resetHold = false;

        savefile->settings.volume = 10;

        for (int i = 0; i < 10; i++)
            savefile->settings.songList[i] = true;

        setDefaultKeys();
    }

    if ((savefile->settings.lightMode != 0) && (savefile->settings.lightMode != 1))
        savefile->settings.lightMode = false;

    u8* dump = (u8*)savefile;

    int sum = 0;

    for (int i = 0; i < (int)sizeof(Save); i++) {
        sum += dump[i];
    }

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
