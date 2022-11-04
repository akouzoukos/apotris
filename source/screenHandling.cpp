
#include "def.h"
#include "tonc.h"
#include "soundbank.h"
#include "text.h"
#include "tonc_memdef.h"

void handlingText();
bool handlingControl();

static const int startX = 3;
static const int startY = 3;
static const int endX = 23;
static const int space = 2;

static int selection;

static bool refreshText = true;

static const int maxDas = 16;
static int dasVer = 0;
static int dasHor = 0;

static const int maxArr = 3;
static int arr = 0;

static std::list<std::string> options = {
    "Auto Repeat Delay",
    "Auto Repeat Rate",
    "Soft Drop Speed",
    "Delay Soft Drop",
    "Drop Protection",
    "Directional Delay",
    "Disable Diagonals"
};

void handlingSettings(){
    selection = 0;
    refreshText = true;

    while (1) {
        VBlankIntrWait();

        key_poll();

        if(refreshText){
            handlingText();

            refreshText = false;
        }

        if(handlingControl())
            break;
    }
}

bool handlingControl(){
    if (key_hit(KEY_RIGHT) || key_hit(KEY_LEFT)) {
        if (selection == 0) {
            if (key_hit(KEY_RIGHT)) {
                if (savefile->settings.das == 16)
                    savefile->settings.das = 11;
                else if (savefile->settings.das == 11)
                    savefile->settings.das = 9;
                else if (savefile->settings.das == 9)
                    savefile->settings.das = 8;
            } else if (key_hit(KEY_LEFT)) {
                if (savefile->settings.das == 8)
                    savefile->settings.das = 9;
                else if (savefile->settings.das == 9)
                    savefile->settings.das = 11;
                else if (savefile->settings.das == 11)
                    savefile->settings.das = 16;
            }
        } else if (selection == 1) {
            if (key_hit(KEY_RIGHT) && savefile->settings.arr > -1)
                savefile->settings.arr--;
            else if (key_hit(KEY_LEFT) && savefile->settings.arr < 3)
                savefile->settings.arr++;
        } else if (selection == 2) {
            if (key_hit(KEY_RIGHT) && savefile->settings.sfr > -1)
                savefile->settings.sfr--;
            else if (key_hit(KEY_LEFT) && savefile->settings.sfr < 3)
                savefile->settings.sfr++;
        } else if (selection == 3) {
            savefile->settings.delaySoftDrop = !savefile->settings.delaySoftDrop;
        } else if (selection == 4) {
            if (key_hit(KEY_LEFT) && savefile->settings.dropProtectionFrames > 0)
                savefile->settings.dropProtectionFrames--;
            else if (key_hit(KEY_RIGHT) && savefile->settings.dropProtectionFrames < 20)
                savefile->settings.dropProtectionFrames++;
        }else if (selection == 5){
            savefile->settings.directionalDas = !savefile->settings.directionalDas;
        }else if (selection == 6){
            if (key_hit(KEY_LEFT) && savefile->settings.diagonalType > 0)
                savefile->settings.diagonalType--;
            else if (key_hit(KEY_RIGHT) && savefile->settings.diagonalType < 2)
                savefile->settings.diagonalType++;
        }

        sfx(SFX_MENUMOVE);
        refreshText = true;
    }

    if(selection == (int) options.size()){
        if(key_hit(KEY_A)){
            sfx(SFX_MENUCANCEL);
            return true;
        }
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

    if(selection == 4){
        if (key_is_down(KEY_LEFT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if(savefile->settings.dropProtectionFrames > 0){
                if (arr++ > maxArr) {
                    arr = 0;
                    savefile->settings.dropProtectionFrames--;
                    sfx(SFX_MENUMOVE);
                }
            }
            refreshText = true;
        } else if (key_is_down(KEY_RIGHT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if(savefile->settings.dropProtectionFrames < 20){
                if (arr++ > maxArr) {
                    arr = 0;
                    savefile->settings.dropProtectionFrames++;
                    sfx(SFX_MENUMOVE);
                }
            }
            refreshText = true;
        } else {
            dasHor = 0;
        }

    }

    if(key_hit(KEY_B) || (key_hit(KEY_START) && selection == (int) options.size())){
        sfx(SFX_MENUCANCEL);
        return true;
    }

    if(key_hit(KEY_START)){
        selection = options.size();
        sfx(SFX_MENUMOVE);
        refreshText = true;
    }

    return false;
}

void handlingText(){

    aprint(" BACK ",12,17);

    auto option = options.begin();
    for(int i = 0; option != options.end(); i++){
        aprint(*option,startX,startY+space*i);
        option++;
    }

    for(int i = 0; i < (int) options.size(); i++){
       aprint("        ",endX-2,startY+space*i);
    }

    if (savefile->settings.das == 8)
        aprint("V.FAST", endX - 1, startY);
    else if (savefile->settings.das == 9)
        aprint("FAST", endX, startY);
    else if (savefile->settings.das == 11)
        aprint("MID", endX, startY);
    else if (savefile->settings.das == 16)
        aprint("SLOW", endX, startY);

    if (savefile->settings.arr == -1)
        aprint("INSTANT", endX - 1, startY + space * 1);
    else if (savefile->settings.arr == 0)
        aprint("V.FAST", endX - 1, startY + space * 1);
    else if (savefile->settings.arr == 1)
        aprint("FAST", endX, startY + space * 1);
    else if (savefile->settings.arr == 2)
        aprint("MID", endX, startY + space * 1);
    else if (savefile->settings.arr == 3)
        aprint("SLOW", endX, startY + space * 1);

    if (savefile->settings.sfr == -1)
        aprint("INSTANT", endX - 1, startY + space * 2);
    else if (savefile->settings.sfr == 0)
        aprint("V.FAST", endX -1, startY + space * 2);
    else if (savefile->settings.sfr == 1)
        aprint("FAST", endX, startY + space * 2);
    else if (savefile->settings.sfr == 2)
        aprint("MID", endX, startY + space * 2);
    else if (savefile->settings.sfr == 3)
        aprint("SLOW", endX, startY + space * 2);

    if (savefile->settings.delaySoftDrop)
        aprint("ON", endX, startY + space * 3);
    else
        aprint("OFF", endX, startY + space * 3);

    aprintf(savefile->settings.dropProtectionFrames,endX,startY + space * 4);

    if (savefile->settings.directionalDas)
        aprint("ON", endX, startY + space * 5);
    else
        aprint("OFF", endX, startY + space * 5);

    std::string diagonalString;
    switch(savefile->settings.diagonalType){
    case 0: diagonalString = "OFF"; break;
    case 1: diagonalString = "SOFT"; break;
    case 2: diagonalString = "STRICT"; break;
    }

    aprint(diagonalString, endX, startY + space * 6);

    //show cursor
    if (selection == 0) {
        if (savefile->settings.das > 8)
            aprint(">", endX + 3 + (savefile->settings.das != 11), startY);
        if (savefile->settings.das < 16)
            aprint("<", endX - 1 - (savefile->settings.das == 8), startY);
    } else if (selection == 1) {
        if (savefile->settings.arr < 3)
            aprint("<", endX - 1 - (savefile->settings.arr <= 0), startY + space * selection);
        if (savefile->settings.arr > - 1)
            aprint(">", endX + 3 + (savefile->settings.arr != 2) + (savefile->settings.arr <= 0), startY + space * selection);
    } else if (selection == 2) {
        if (savefile->settings.sfr < 3)
            aprint("<", endX - 1 - (savefile->settings.sfr <= 0), startY + space * selection);
        if (savefile->settings.sfr > - 1)
            aprint(">", endX + 3 + (savefile->settings.sfr != 2) + (savefile->settings.sfr <= 0), startY + space * selection);
    } else if (selection == 3) {
        aprint("[", endX - 1, startY + space * selection);
        aprint("]", endX + 2 + (!savefile->settings.delaySoftDrop), startY + space * selection);
    } else if (selection == 4) {
        if (savefile->settings.dropProtectionFrames > 0)
            aprint("<", endX - 1, startY + space * selection);
        if (savefile->settings.dropProtectionFrames < 20)
            aprint(">", endX + 1 + (savefile->settings.dropProtectionFrames > 9), startY + space * selection);
    } else if (selection == 5) {
        aprint("[", endX - 1, startY + space * selection);
        aprint("]", endX + 2 + (!savefile->settings.directionalDas), startY + space * selection);
    } else if (selection == 6) {
        if(savefile->settings.diagonalType != 0)
            aprint("<", endX - 1, startY + space * selection);
        if(savefile->settings.diagonalType != 2)
            aprint(">", endX + diagonalString.size(), startY + space * selection);
    }else if (selection == (int) options.size()){
        aprint("[",12,17);
        aprint("]",17,17);
    }
}
