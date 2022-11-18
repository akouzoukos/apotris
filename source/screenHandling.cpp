
#include "def.h"
#include "tonc.h"
#include "soundbank.h"
#include "text.h"
#include "tonc_memdef.h"
#include <string>

void handlingText();
bool handlingControl();
void setDas(int index);

static const int startX = 3;
static const int startY = 4;
static const int endX = 23;
static const int space = 1;

static int selection;

static bool refreshText = true;

static const int maxDas = 16;
static int dasVer = 0;
static int dasHor = 0;

static const int maxArr = 3;
static int arr = 0;

static int dasSelection = 0;

static std::list<std::string> options = {
    "Auto Repeat Delay",
    "Auto Repeat Rate",
    "Soft Drop Speed",
    "Delay Soft Drop",
    "Drop Protection",
    "Directional Delay",
    "Disable Diagonals",
    "Initial Holding",
    "Initial Rotation",
};

void handlingSettings(){
    setSmallTextArea(110, endX, startY-1, endX+5, startY-1);
    clearSmallText();
    clearText();

    selection = 0;
    refreshText = true;

    if(savefile->settings.customDas){
        dasSelection = savefile->settings.das-30;
    }else{
        switch(savefile->settings.das){
            case 16: dasSelection = 1; break;
            case 11: dasSelection = 2; break;
            case  9: dasSelection = 3; break;
            case  8: dasSelection = 4; break;
        }
    }

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

    setSmallTextArea(110, 0, 0, 1, 1);
    clearSmallText();
    clearText();
}

bool handlingControl(){
    if (key_hit(KEY_RIGHT) || key_hit(KEY_LEFT)) {
        if (selection == 0) {
            if(key_hit(KEY_RIGHT)){
                if(dasSelection < 4)
                    dasSelection++;
            }
            if(key_hit(KEY_LEFT)){
                if(dasSelection > -29)
                    dasSelection--;
            }
            setDas(dasSelection);

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
        }else if (selection == 7){
            savefile->settings.ihs = !savefile->settings.ihs;
        }else if (selection == 8){
            savefile->settings.irs = !savefile->settings.irs;
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

    if(selection == 0){
        if (key_is_down(KEY_LEFT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if(dasSelection > -29){
                if (arr++ > maxArr) {
                    arr = 0;
                    dasSelection--;
                    sfx(SFX_MENUMOVE);
                }
            }
            setDas(dasSelection);
            refreshText = true;
        } else if (key_is_down(KEY_RIGHT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if(dasSelection < 4){
                if (arr++ > maxArr) {
                    arr = 0;
                    dasSelection++;
                    sfx(SFX_MENUMOVE);
                }
            }
            setDas(dasSelection);
            refreshText = true;
        } else {
            dasHor = 0;
        }

    }else if(selection == 4){
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
        aprint(*option,startX,startY+space*i + (i > 2) + (i > 6));
        option++;
    }

    aprintClearArea(endX-2,startY,8,options.size()+2);

    if(!savefile->settings.customDas){
        if (savefile->settings.das == 8)
            aprint("V.FAST", endX - 1, startY);
        else if (savefile->settings.das == 9)
            aprint("FAST", endX, startY);
        else if (savefile->settings.das == 11)
            aprint("MID", endX, startY);
        else if (savefile->settings.das == 16)
            aprint("SLOW", endX, startY);
    }else{
        clearSmallText();
        switch(savefile->settings.das){
        case 16: aprints("slow",4,0,1); break;
        case 11: aprints("mid",8,0,1); break;
        case 9: aprints("fast",0,0,1); break;
        case 8: aprints("v.fast",0,0,1); break;
        }
        aprint(std::to_string(savefile->settings.das) + "f", endX, startY);
    }

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
        aprint("ON", endX, startY + space * 4);
    else
        aprint("OFF", endX, startY + space * 4);

    aprintf(savefile->settings.dropProtectionFrames,endX,startY + space * 5);

    if (savefile->settings.directionalDas)
        aprint("ON", endX, startY + space * 6);
    else
        aprint("OFF", endX, startY + space * 6);

    std::string diagonalString;
    switch(savefile->settings.diagonalType){
    case 0: diagonalString = "OFF"; break;
    case 1: diagonalString = "SOFT"; break;
    case 2: diagonalString = "STRICT"; break;
    }

    aprint(diagonalString, endX, startY + space * 7);

    if (savefile->settings.ihs)
        aprint("ON", endX, startY + space * 9);
    else
        aprint("OFF", endX, startY + space * 9);

    if (savefile->settings.irs)
        aprint("ON", endX, startY + space * 10);
    else
        aprint("OFF", endX, startY + space * 10);

    //show cursor
    if (selection == 0) {
        if(!savefile->settings.customDas){
            if (savefile->settings.das > 8)
                aprint(">", endX + 3 + (savefile->settings.das != 11), startY);
            aprint("<", endX - 1 - (savefile->settings.das == 8), startY);
        }else{
            aprint(">", endX + 2 + (savefile->settings.das > 9), startY);
            if (savefile->settings.das > 1)
                aprint("<", endX - 1, startY);
        }
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
        aprint("[", endX - 1, startY + space * (selection + 1));
        aprint("]", endX + 2 + (!savefile->settings.delaySoftDrop), startY + space * (selection + 1));
    } else if (selection == 4) {
        if (savefile->settings.dropProtectionFrames > 0)
            aprint("<", endX - 1, startY + space * selection + 1);
        if (savefile->settings.dropProtectionFrames < 20)
            aprint(">", endX + 1 + (savefile->settings.dropProtectionFrames > 9), startY + space * (selection + 1));
    } else if (selection == 5) {
        aprint("[", endX - 1, startY + space * (selection + 1));
        aprint("]", endX + 2 + (!savefile->settings.directionalDas), startY + space * (selection + 1));
    } else if (selection == 6) {
        if(savefile->settings.diagonalType != 0)
            aprint("<", endX - 1, startY + space * (selection + 1));
        if(savefile->settings.diagonalType != 2)
            aprint(">", endX + diagonalString.size(), startY + space * (selection + 1));
    } else if (selection == 7) {
        aprint("[", endX - 1, startY + space * (selection + 2));
        aprint("]", endX + 2 + (!savefile->settings.ihs), startY + space * (selection + 2));
    } else if (selection == 8) {
        aprint("[", endX - 1, startY + space * (selection + 2));
        aprint("]", endX + 2 + (!savefile->settings.irs), startY + space * (selection + 2));
    }else if (selection == (int) options.size()){
        aprint("[",12,17);
        aprint("]",17,17);
    }
}

void setDas(int index){
    int n = 0;
    if(index > 0){
        switch(index){
            case 1: n = 16; break;
            case 2: n = 11; break;
            case 3: n = 9; break;
            case 4: n = 8; break;
        }
        savefile->settings.customDas = false;
        savefile->settings.das = n;
    }else{
        n = 30 + index;
        savefile->settings.customDas = true;
        savefile->settings.das = n;
    }
}
