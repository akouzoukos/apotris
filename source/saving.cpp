#include "def.h"
#include "tonc.h"

void saveToSram() {
    volatile u8* sf = (volatile u8*)sram_mem;

    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)sizeof(Save); i++)
        sf[i] = (u8)arr[i];
}

void loadFromSram() {
    volatile u8* sf = (volatile u8*)sram_mem;

    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)sizeof(Save); i++)
        arr[i] = (u8)sf[i];
}

void loadSave() {
    savefile = new Save();
    loadFromSram();

    if (savefile->newGame == 0x4b) {
        Save* temp = new Save();
        int oldSize = sizeof(Test) + sizeof(u8);

        // memcpy32(temp,savefile,oldSize/4);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        // for(int i = 0; i < (int) sizeof(Save)-oldSize; i++)
        // 	tmp[i+sizeof(Settings)+sizeof(u8)-4] = sf[i+oldSize];
        memcpy16(&tmp[sizeof(Settings) + sizeof(u8) + sizeof(int) - 4], &sf[oldSize], (sizeof(Save) - oldSize) / 2);

        temp->newGame = SAVE_TAG;
        temp->settings.edges = false;
        temp->settings.backgroundGrid = 0;
        temp->settings.skin = 0;
        temp->settings.palette = 6;
        temp->settings.shadow = 0;
        temp->settings.lightMode = false;

        for (int i = 0; i < 10; i++)
            temp->settings.songList[i] = true;

        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 5; j++)
                temp->ultra[i].highscores[j].score = 0;

        memcpy32(savefile, temp, sizeof(Save) / 4);

        delete temp;

    } else if (savefile->newGame != SAVE_TAG) {
        // delete savefile;
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

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 5; j++)
                savefile->ultra[i].highscores[j].score = 0;

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

        savefile->settings.volume = 10;

        for (int i = 0; i < 10; i++)
            savefile->settings.songList[i] = true;

        // savefile->savedGame = Game(0);
        // savefile->canLoad = false;
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
