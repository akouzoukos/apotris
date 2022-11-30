#include "def.h"
#include "flashSaves.h"
#include "sprite1tiles_bin.h"
#include "sprites.h"
#include <string>
#include "logging.h"
#include <algorithm>

class SaveChange{
public:
    int saveId;
    int location;
    int size;

    SaveChange(int _id, int _location, int _size){
        saveId = _id;
        location = _location;
        size = _size;
    }
};

void setDefaultKeys();
void addStats();
void resetSkins(Save * save);
void setDefaults(Save * save, int depth);
void setDefaultGraphics(Save * save, int depth);
void applySaveChanges(u8* newSave, u8* oldSave,  int newSize);

const std::list<SaveChange> saveChanges = {
    SaveChange(0x50,132,20 * 4),//add zone + buffer to keys
    SaveChange(0x50,3120,4096),// add zone + buffer to scoreboards
};

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

    const int pmSettings = 540;
    const int pmSave = 3876;

    if (savefile->newGame == 0x4f) {
    }else if (savefile->newGame == 0x4e) {
        setDefaults(savefile,4);
    }else if (savefile->newGame == 0x4d) {
        Save* temp = new Save();
        const int oldSize = sizeof(Test3);

        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        for (int i = 0; i < oldSize; i++)
            tmp[i] = sf[i];

        memcpy16(&tmp[pmSettings+ sizeof(u8)], &sf[oldSize], (pmSave - oldSize) / 2);

        setDefaults(temp,3);

        memcpy32(savefile, temp, pmSave / 4);

        delete temp;
    }else if (savefile->newGame == 0x4c) {
        Save* temp = new Save();
        const int oldSize = sizeof(Test2) + sizeof(u8);

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
        const int oldSize = sizeof(Test) + sizeof(u8);

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

        savefile->newGame = SAVE_TAG;
    }

    //fix invalid values, since I messed up conversion at some point
    if ((savefile->settings.lightMode != 0) && (savefile->settings.lightMode != 1))
        savefile->settings.lightMode = false;

    if ((savefile->settings.rumble < 0) || (savefile->settings.rumble > 4))
        savefile->settings.rumble = 0;

    if(savefile->settings.irs < 0 || savefile->settings.irs > 1)
        savefile->settings.irs = 0;
    if(savefile->settings.ihs < 0 || savefile->settings.ihs > 1)
        savefile->settings.ihs = 0;

    if(savefile->newGame != SAVE_TAG){

        Save* temp = new Save();
        u8* tmp = (u8*)temp;

        u8* sf = (u8*)savefile;

        applySaveChanges(tmp, sf, sizeof(Save));

        setDefaults(temp,5);

        memcpy32(savefile, temp, sizeof(Save) / 4);

        delete temp;
        savefile->newGame = SAVE_TAG;
    }

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
    k.zone = KEY_SELECT;

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
        save->settings.cycleSongs = true;
        save->settings.dropProtectionFrames = 8;
        save->settings.abHold = true;
        save->settings.resetHold = false;

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

        // save->stats.timePlayed = 0;
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

        for(int i = 0; i < 4; i++)
            for(int j = 0; j < 5; j++)
                save->zone[i].highscores[j].score = 0;

        save->settings.diagonalType = save->settings.noDiagonals;
        save->settings.delaySoftDrop = false;
        save->settings.customDas = false;
        save->settings.irs = 0;
        save->settings.ihs = 0;

        //check if select is already bound - if not, bind it to zone activation
        int* keys = (int*) &save->settings.keys;

        bool found = false;

        std::list<int> foundKeys;
        for(int i = 0; i < 9; i++){
            foundKeys.clear();

            int k = keys[i];
            int counter = 0;
            do{
                if(k & (1 << counter)){
                    foundKeys.push_back(1<<counter);
                    k-= 1 << counter;
                }

                counter++;
            }while(k != 0);

            if(std::find(foundKeys.begin(), foundKeys.end(),(int) KEY_SELECT) != foundKeys.end()){
                found = true;
                break;
            }
        }

        if(!found)
            save->settings.keys.zone = KEY_SELECT;
        else
            save->settings.keys.zone = 0;
    }

    setDefaultGraphics(save, depth);
}

bool compareVersion(const SaveChange & first,const SaveChange & second){
    return first.saveId < second.saveId;
}

bool compareLocation(const SaveChange & first,const SaveChange & second){
    return first.location < second.location;
}

void applySaveChanges(u8* newSave, u8* oldSave, int newSize){

    const int migSize = 3876;
    const int version = savefile->newGame;

    int diff = newSize - migSize;
    int oldSize = migSize;

    std::list<SaveChange> sorted = saveChanges;

    //sort by descending location addresses
    sorted.sort(compareLocation);
    sorted.reverse();

    //sort by ascending save id
    sorted.sort(compareVersion);

    int applyingVersion = 0;
    int extraSize = 0;

    int sum = 0;
    SaveChange previous = SaveChange(0,0,0);
    for(auto const & change : sorted){
        if(change.saveId <= version)
            continue;

        if(change.saveId != applyingVersion){
            applyingVersion = change.saveId;
            oldSize += extraSize;
            extraSize = 0;
        }

        if(previous.saveId != 0 && previous.saveId != change.saveId){
            memcpy16(newSave,oldSave,previous.location / 2);

            memcpy16(oldSave,newSave,newSize/2);

            previous.location = 0;
        }

        int count = 0;
        if(previous.location == 0)
            count = oldSize-change.location;
        else
            count = previous.location - (change.location);

        memcpy16(&newSave[change.location + diff - sum],&oldSave[change.location], (count)/2);

        previous = change;
        sum += previous.size;
        extraSize += change.size;
    }

    if(previous.saveId != 0){
        memcpy16(newSave,oldSave,previous.location / 2);
    }
}

void setDefaultGraphics(Save *save, int depth){
    if(depth < 1){
        save->settings.floatText = true;
        save->settings.shake = true;
        save->settings.effects = true;
    }

    if(depth < 2){
        save->settings.edges = true;
        save->settings.backgroundGrid = 5;
        save->settings.skin = 11;
        save->settings.palette = 5;
        save->settings.shadow = 3;
        save->settings.lightMode = false;

    }

    if(depth < 3){
        save->settings.maxQueue = 5;
        save->settings.colors = 1;
        save->settings.clearEffect = 1;

        if(save->settings.shake)
            save->settings.shakeAmount = 2;
        else{
            save->settings.shake = true;
            save->settings.shakeAmount = 0;
        }
    }

    if(depth < 4){
        save->settings.placeEffect = true;
        save->settings.backgroundGradient = 0x7dc8;
    }
}
