#include "def.h"
#include "soundbank.h"
#include "text.h"

void audioText();
bool audioControl();

static int startX = 5;
static int startY = 5;
static int endX = 23;
static int space = 2;

static int selection;

static bool refreshText = true;

static int maxDas = 16;
static int dasVer = 0;
static int dasHor = 0;

static int maxArr = 3;
static int arr = 0;

static std::list<std::string> options = {
    "Music",
    "SFX",
    "Announcer",
    "Playback",
    "Song List..."
};

void audioSettings(){
    selection = 0;
    refreshText = true;

    bool previous = savefile->settings.cycleSongs;
    savefile->settings.cycleSongs = false;

    while (1) {
        VBlankIntrWait();

        key_poll();

        if(refreshText){
            audioText();

            refreshText = false;
        }

        if(audioControl())
            break;
    }

    savefile->settings.cycleSongs = previous;
}

bool audioControl(){
    if (key_hit(KEY_RIGHT) || key_hit(KEY_LEFT)) {
        if (selection == 0) {
            if (key_hit(KEY_RIGHT)){
                if(savefile->settings.volume < 10){
                    savefile->settings.volume++;
                    sfx(SFX_MENUMOVE);
                }else{
                    sfx(SFX_MENUCANCEL);
                }
            }

            if (key_hit(KEY_LEFT)){
                if(savefile->settings.volume > 0){
                    savefile->settings.volume--;
                    sfx(SFX_MENUMOVE);
                }else{
                    sfx(SFX_MENUCANCEL);
                }
            }

            mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
        }else if (selection == 1) {
            if (key_hit(KEY_RIGHT)){
                if(savefile->settings.sfxVolume < 10){
                    savefile->settings.sfxVolume++;
                    sfx(SFX_MENUMOVE);
                }else{
                    sfx(SFX_MENUCANCEL);
                }
            }

            if (key_hit(KEY_LEFT)){
                if(savefile->settings.sfxVolume > 0){
                    savefile->settings.sfxVolume--;
                    sfx(SFX_MENUMOVE);
                }else{
                    sfx(SFX_MENUCANCEL);
                }
            }
        }else if (selection == 2) {
            savefile->settings.announcer = !savefile->settings.announcer;
            sfx(SFX_MENUMOVE);
        }else if (selection == 3) {
            savefile->settings.cycleSongs = !savefile->settings.cycleSongs;
            sfx(SFX_MENUMOVE);
        }

        refreshText = true;
    }

    if(selection == 0){
        if (key_is_down(KEY_LEFT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if (savefile->settings.volume > 0) {
                if (arr++ > maxArr) {
                    arr = 0;
                    savefile->settings.volume--;
                    sfx(SFX_MENUMOVE);
                    refreshText = true;
                }
            }
            mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
            refreshText = true;
        } else if (key_is_down(KEY_RIGHT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if (savefile->settings.volume < 10) {
                if (arr++ > maxArr) {
                    arr = 0;
                    savefile->settings.volume++;
                    sfx(SFX_MENUMOVE);
                    refreshText = true;
                }
            }
            mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
            refreshText = true;
        } else {
            dasHor = 0;
        }
    }else if(selection == 1){
        if (key_is_down(KEY_LEFT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if (savefile->settings.sfxVolume > 0) {
                if (arr++ > maxArr) {
                    arr = 0;
                    savefile->settings.sfxVolume--;
                    sfx(SFX_MENUMOVE);
                    refreshText = true;
                }
            }
            refreshText = true;
        } else if (key_is_down(KEY_RIGHT)) {
            if (dasHor < maxDas) {
                dasHor++;
            } else if (savefile->settings.sfxVolume < 10) {
                if (arr++ > maxArr) {
                    arr = 0;
                    savefile->settings.sfxVolume++;
                    sfx(SFX_MENUMOVE);
                    refreshText = true;
                }
            }
            refreshText = true;
        } else {
            dasHor = 0;
        }
    }else if(selection == 4){
        if(key_hit(KEY_A)){
            clearText();
            sfx(SFX_MENUCONFIRM);
            songListMenu();
            clearText();
            refreshText = true;
        }
    }else if(selection == (int) options.size()){
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

    if(key_hit(KEY_B)){
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

void audioText(){

    aprint(" BACK ",12,17);

    auto option = options.begin();
    for(int i = 0; option != options.end(); i++){
        aprint(*option,startX,startY+space*i);
        option++;
    }

    for(int i = 0; i < (int) options.size(); i++){
       aprint("        ",endX-2,startY+space*i);
    }

    aprintf(savefile->settings.volume, endX, startY);

    aprintf(savefile->settings.sfxVolume, endX, startY + space);

    if (savefile->settings.announcer)
        aprint("ON", endX, startY+space*2);
    else
        aprint("OFF", endX, startY+space*2);

    if (savefile->settings.cycleSongs)
        aprint("CYCLE", endX, startY+space*3);
    else
        aprint("LOOP", endX, startY+space*3);//space necessary

    //show cursor
    if (selection == 0) {
        if (savefile->settings.volume > 0)
            aprint("<", endX - 1, startY);
        if (savefile->settings.volume < 10)
            aprint(">", endX + 1, startY);
    }else if (selection == 1) {
        if (savefile->settings.sfxVolume > 0)
            aprint("<", endX - 1, startY + space * selection);
        if (savefile->settings.sfxVolume < 10)
            aprint(">", endX + 1, startY + space * selection);
    }else if (selection == 2) {
        aprint("[", endX - 1, startY + space * selection);
        aprint("] ", endX + 2 + (!savefile->settings.announcer), startY + space * selection);
    }else if (selection == 3){
        aprint("[", endX - 1, startY + space * selection);
        aprint("]", endX + 4 + (savefile->settings.cycleSongs), startY + space * selection);
    }else if (selection == 4){
        aprint("[", endX - 1, startY + space * selection);
        aprint("]", endX + 2, startY + space * selection);
    }else if (selection == (int) options.size()){
        aprint("[",12,17);
        aprint("]",17,17);
    }
}
