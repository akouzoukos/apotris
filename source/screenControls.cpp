#include "def.h"
#include "soundbank.h"
#include "text.h"
#include <map>
#include <string>
#include <algorithm>
#include "logging.h"

std::map<int,std::string> keyMap = {
    {KEY_LEFT,"Left"},
    {KEY_RIGHT,"Right"},
    {KEY_UP,"Up"},
    {KEY_DOWN,"Down"},
    {KEY_A,"A"},
    {KEY_B,"B"},
    {KEY_L,"L"},
    {KEY_R,"R"},
    {KEY_SELECT,"Select"},
};

void controlsText();
bool controlsControl();
void showKey(int key, int x, int y, bool selected);
void changeInput(int selection);

static int startX = 3;
static int startY = 3;
static int endX = 23;
static int space = 1;

static int selection;

static bool refreshText = true;

static int maxDas = 16;
static int dasVer = 0;

static int maxArr = 3;
static int arr = 0;

static const std::list<std::string> options = {
    "Move Left",
    "Move Right",
    "Rotate Left",
    "Rotate Right",
    "Rotate 180",
    "Soft Drop",
    "Hard Drop",
    "Hold",
    "Activate Zone",
    "A+B to Hold",
    "Quick Reset",
    "Rumble",
    "Reset Controls",
};

void controlsSettings(){
    selection = 0;
    refreshText = true;

    int i = 0;
    for(auto const & option : options)
        aprint(option,startX,startY+space*(i++));

    while (1) {
        VBlankIntrWait();

        key_poll();

        if(refreshText){
            controlsText();

            refreshText = false;
        }

        if(controlsControl())
            break;
    }
}

bool controlsControl(){
    if (key_hit(KEY_RIGHT) || key_hit(KEY_LEFT) || key_hit(KEY_A)) {
        sfx(SFX_MENUMOVE);

        if (selection < 9){
            changeInput(selection);
            refreshText = true;
            return false;
        }else if (selection == 9) {
            savefile->settings.abHold = !savefile->settings.abHold;
        }else if (selection == 10) {
            savefile->settings.resetHold = !savefile->settings.resetHold;
        }else if (selection == 11) {
            if (key_hit(KEY_LEFT)) {
                if (savefile->settings.rumble > 0) {
                    savefile->settings.rumble--;
                }
            } else {
                if (savefile->settings.rumble < 4) {
                    savefile->settings.rumble++;
                }
            }

        }else if (selection == 12) {
            setDefaultKeys();
        }

        refreshText = true;
    }

    if(selection == (int) options.size()){
        if(key_hit(KEY_A)){
            sfx(SFX_MENUCANCEL);
            return true;
        }
    }

    if(key_hit(KEY_B)){
        sfx(SFX_MENUCANCEL);
        return true;
    }

    if (key_hit(KEY_UP)) {
        if (selection > 0)
            selection--;
        else
            selection = (int) options.size();
        sfx(SFX_MENUMOVE);
        refreshText = true;
    }

    if (key_hit(KEY_DOWN)) {
        if (selection < (int) options.size())
            selection++;
        else
            selection = 0;
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
            }
        }
        refreshText = true;
    } else if (key_is_down(KEY_DOWN)) {
        if (dasVer < maxDas) {
            dasVer++;
        } else if(selection != (int) options.size()){
            if (arr++ > maxArr) {
                arr = 0;
                selection++;
                sfx(SFX_MENUMOVE);
            }
        }
        refreshText = true;
    } else {
        dasVer = 0;
    }

    // if(key_hit(KEY_B) || (key_hit(KEY_START) && selection == (int) options.size())){
    //     sfx(SFX_MENUCANCEL);
    //     return true;
    // }

    if(key_hit(KEY_START)){
        if(selection < 8){
            int* keys = (int*) &savefile->settings.keys;

            //QUICK FIX FOR ROTATE LABELS, GONNA CHANGE NEXT MAJOR UPDATE
            int temp = selection;
            if(selection == 2)
                temp++;
            else if(selection == 3)
                temp--;

            keys[temp] = 0;

        }else{
            selection = options.size();
        }

        sfx(SFX_MENUMOVE);
        refreshText = true;
    }

    return false;
}

void controlsText(){

    aprint(" BACK ",12,17);

    for(int i = 0; i < (int) options.size(); i++){
       aprint("           ",endX-6,startY+space*i);
    }

    Keys k = savefile->settings.keys;

    showKey(k.moveLeft, endX, startY, (selection == 0));

    showKey(k.moveRight, endX, startY+space, (selection == 1));

    showKey(k.rotateCCW, endX, startY+space*2, (selection == 2));

    showKey(k.rotateCW, endX, startY+space*3, (selection == 3));

    showKey(k.rotate180, endX, startY+space*4, (selection == 4));

    showKey(k.softDrop, endX, startY+space*5, (selection == 5));

    showKey(k.hardDrop, endX, startY+space*6, (selection == 6));

    showKey(k.hold, endX, startY+space*7, (selection == 7));

    showKey(k.zone, endX, startY+space*8, (selection == 8));

    if(savefile->settings.abHold)
        aprint("ON", endX-1, startY+space*9);
    else
        aprint("OFF", endX-1, startY+space*9);

    if(savefile->settings.resetHold)
        aprint("HOLD", endX-1, startY+space*10);
    else
        aprint("PRESS", endX-1, startY+space*10);

    std::string rumbleString = std::to_string(savefile->settings.rumble * 25) + "%";

    aprint(rumbleString, endX+2-rumbleString.size(), startY + 11);

    //show cursor
    if (selection == 9) {
        aprint("[", endX - 2, startY + space * selection);
        aprint("]", endX + 1 + (!savefile->settings.abHold), startY + space * selection);
    }else if (selection == 10) {
        aprint("[", endX - 2, startY + space * selection);
        aprint("]", endX + 3 + (!savefile->settings.resetHold), startY + space * selection);
    } else if (selection == 11) {
        if (savefile->settings.rumble > 0)
            aprint("<", endX - 1 - (savefile->settings.rumble != 0) - (savefile->settings.rumble == 4), startY + selection);
        if (savefile->settings.rumble < 4)
            aprint(">", endX + 2, startY + selection);
    } else if (selection == 12) {
        aprint("[", endX - 2, startY + space * selection);
        aprint("]", endX + 1, startY + space * selection);
    }else if (selection == (int) options.size()){
        aprint("[",12,17);
        aprint("]",17,17);
    }

}

void showKey(int key, int x, int y,bool selected){
    if((key & (key - 1)) == 0)
        if(!selected)
            aprint(keyMap[key], x - keyMap[key].size()/2, y);
        else
            aprint("["+keyMap[key]+"]", x - keyMap[key].size()/2-1, y);
    else{
        std::string result = "";

        if(selected)
            result+= "[";

        int k = key;

        int counter = 0;
        do{
            if(k & (1 << counter)){
                result+= keyMap[1<<counter] + " ";
                k-= 1 << counter;
            }

            counter++;
        }while(k != 0);

        if(selected){
            result.pop_back();
            result+= "]";
        }

        aprint(result, x - result.size()/2 - (selected), y);

    }
}

void changeInput(int selection){

    int* keys = (int*) &savefile->settings.keys;
    std::list<int> foundKeys;

    aprint("           ",endX-6,startY+space*selection);
    aprint("[...]",endX-2,startY+space*selection);

    while(1){
        VBlankIntrWait();
        key_poll();

        if(key_hit(KEY_START)){
            sfx(SFX_MENUCANCEL);
            break;
        }

        u16 key = key_hit(KEY_FULL);

        if(key != 0){
            std::list<int> currentKeys;

            int temp = key;

            int counter = 0;
            do{
                if(temp & (1 << counter)){
                    currentKeys.push_back(1<<counter);
                    temp-= 1 << counter;
                }

                counter++;
            }while(temp != 0);

            for(int i = 0; i < 9; i++){
                foundKeys.clear();

                int k = keys[i];
                counter = 0;
                do{
                    if(k & (1 << counter)){
                        foundKeys.push_back(1<<counter);
                        k-= 1 << counter;
                    }

                    counter++;
                }while(k != 0);

                for(auto const & cur : currentKeys){
                    if(std::find(foundKeys.begin(), foundKeys.end(),(int) cur) != foundKeys.end())
                        keys[i]-=cur;
                }
            }

            //QUICK FIX FOR ROTATE LABELS, GONNA CHANGE NEXT MAJOR UPDATE. (Nov. 12th, next update I swear Copium)
            if(selection == 2)
                selection++;
            else if(selection == 3)
                selection--;

            keys[selection] = key;
            break;
        }
    }
}
