#include "def.h"
#include "tonc.h"
#include "text.h"
#include "soundbank.h"
#include "sprites.h"
#include <string>
#include "posprintf.h"
#include "logging.h"
#include "tonc_irq.h"

using namespace Tetris;

void showLabels();
void gradientEditor();

static const int startX = 5;
static const int endX = 24;

static const int startY = 2;
static const int options = 15;

void graphicTest() {
    gradient(false);
    irq_enable(II_HBLANK);

    setPalette();
    setGradient(savefile->settings.backgroundGradient);

    if (savefile->settings.lightMode)
        memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        memset16(pal_bg_mem, 0x0000, 1);

    memset16(&se_mem[26], 0x0000, 32 * 20);

    delete game;
    game = new Game(DIG,bigMode);
    game->setGoal(0);
    game->setLevel(1);
    game->rotationSystem = SRS;

    game->update();

    game->pawn.y++;

    bool showOptions = true;
    bool showGame = true;

    int selection = 0;

    const int maxDas = 16;
    int dasVer = 0;
    int dasHor = 0;
    int dasOffset = 0;

    const int maxArr = 3;
    int arr = 0;

    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering

    int prevBld = REG_BLDCNT;
    REG_BLDCNT = (1 << 6) + (0b111111 << 8) + (1 << 3) ;
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(1);
    memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);

    showBackground();
    showLabels();

    while (1) {
        VBlankIntrWait();
        drawGrid();
        drawFrame();
        screenShake();
        showPlaceEffect();

        if(savefile->settings.floatText){
            showClearText();

            if(!floatingList.empty() && showOptions)
                showLabels();
        }

        key_poll();

        aprint("R: Toggle", 0, 18);
        aprint("Options", 3, 19);

        if (key_hit(KEY_START) || (key_hit(KEY_A) && selection != 0)) {
            sfx(SFX_MENUCONFIRM);
            if (selection != options - 1) {
                selection = options - 1;
            } else {
                break;
            }
            setSkin();
            // setLightMode();
        }

        if (key_hit(KEY_B)) {
            sfx(SFX_MENUCANCEL);
            if (selection == options - 1)
                selection = 0;
            else
                break;
        }

        if (key_hit(KEY_R)) {
            showOptions = !showOptions;

            if (!showOptions) {
                clearText();
                REG_DISPCNT &= ~DCNT_BG3;
                aprint("R: Toggle", 0, 18);
                aprint("Options", 3, 19);
            }else{
                REG_DISPCNT |= DCNT_BG3;
                showLabels();
            }
        }

        if (key_hit(KEY_UP)) {
            if (selection > 0)
                selection--;
            else
                selection = options - 1;
            sfx(SFX_MENUMOVE);
        }

        if (key_hit(KEY_DOWN)) {
            if (selection < options - 1)
                selection++;
            else
                selection = 0;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_SELECT)){
            setDefaultGraphics(savefile, 0);
            setSkin();
            setPalette();
            memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);
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
        } else if (key_is_down(KEY_DOWN)) {
            if (dasVer < maxDas) {
                dasVer++;
            } else if(selection != options-1){
                if (arr++ > maxArr) {
                    arr = 0;
                    selection++;
                    sfx(SFX_MENUMOVE);
                }
            }
        } else {
            dasVer = 0;
        }

        if(selection == 2){
            if (key_is_down(KEY_LEFT)) {
                if (dasHor < maxDas) {
                    dasHor++;
                } else if(savefile->settings.skin+dasOffset > - MAX_CUSTOM_SKINS){
                    if (arr++ > maxArr) {
                        arr = 0;
                        // savefile->settings.skin--;
                        dasOffset--;
                        sfx(SFX_MENUMOVE);
                    }
                }
            } else if (key_is_down(KEY_RIGHT)) {
                if (dasHor < maxDas) {
                    dasHor++;
                } else if(savefile->settings.skin+dasOffset < MAX_SKINS-1){
                    if (arr++ > maxArr) {
                        arr = 0;
                        // savefile->settings.skin++;
                        dasOffset++;
                        sfx(SFX_MENUMOVE);
                    }
                }
            } else {
                if(dasHor == maxDas){
                    savefile->settings.skin += dasOffset;
                    dasOffset = 0;
                    setSkin();
                    showPawn();
                    showShadow();
                }
                dasHor = 0;
            }
        }

        if(key_hit(KEY_A) && selection == 0){
            clearText();

            oam_init(obj_buffer, 128);
            oam_copy(oam_mem, obj_buffer, 128);

            gradientEditor();
            if(showOptions)
                showLabels();
        }

        if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)) {
            switch (selection) {
            case 0:
                break;
            case 1:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.backgroundGrid > 0) {
                        savefile->settings.backgroundGrid--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1) {
                        savefile->settings.backgroundGrid++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

                drawGrid();
                break;
            case 2:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.skin > -(MAX_CUSTOM_SKINS)) {
                        savefile->settings.skin--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.skin < MAX_SKINS - 1) {
                        savefile->settings.skin++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

                setSkin();
                showPawn();
                showShadow();
                break;
            case 3:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.shadow > 0) {
                        savefile->settings.shadow--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.shadow < MAX_SHADOWS - 1) {
                        savefile->settings.shadow++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                break;
            case 4:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.palette > 0) {
                        savefile->settings.palette--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.palette < 7) {
                        savefile->settings.palette++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                setPalette();
                break;
            case 5:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.colors > 0) {
                        savefile->settings.colors--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.colors < MAX_COLORS-1) {
                        savefile->settings.colors++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                setPalette();
                break;
            case 6:
                savefile->settings.edges = !savefile->settings.edges;
                sfx(SFX_MENUMOVE);
                break;
            case 7:
                savefile->settings.lightMode = !savefile->settings.lightMode;
                memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);
                setSkin();
                // setLightMode();

                sfx(SFX_MENUMOVE);
                break;
            case 8:
                savefile->settings.effects = !savefile->settings.effects;
                if (savefile->settings.effects) {
                    effectList.push_back(Effect(1, 4, 5));
                }
                sfx(SFX_MENUMOVE);
                break;
            case 9:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.clearEffect > 0) {
                        savefile->settings.clearEffect--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.clearEffect < MAX_CLEAR_EFFECTS - 1) {
                        savefile->settings.clearEffect++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }

                setClearEffect();
                break;
            case 10:
                savefile->settings.placeEffect = !savefile->settings.placeEffect;
                if(savefile->settings.placeEffect && (int)placeEffectList.size() <= 3){
                    placeEffectList.push_back(PlaceEffect(game->pawn.x, game->pawn.y-20, 0, 0, game->pawn.current, 0, false));
                }

                sfx(SFX_MENUMOVE);
                break;
            case 11:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.shakeAmount > 0) {
                        savefile->settings.shakeAmount--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.shakeAmount < 4) {
                        savefile->settings.shake = true;
                        savefile->settings.shakeAmount++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                shake = -shakeMax * (savefile->settings.shakeAmount) / 4;
                break;
            case 12:
                savefile->settings.floatText = !savefile->settings.floatText;

                if(savefile->settings.floatText){
                    floatingList.push_back(FloatText("quad"));
                }else{
                    for(auto const & floating : floatingList){
                        int height;
                        if (floating.timer < 2 * 100 / 3)
                            height = 5 * (float)floating.timer / ((float)2 * 100 / 3);
                        else
                            height = (30 * (float)(floating.timer) / 100) - 15;
                        aprintClearArea(9, 15-height, 12, 1);
                    }

                    if(showOptions)
                        showLabels();

                    floatingList.clear();
                }
                sfx(SFX_MENUMOVE);
                break;
            case 13:
                if (key_hit(KEY_LEFT)) {
                    if (savefile->settings.maxQueue > 1) {
                        savefile->settings.maxQueue--;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                } else {
                    if (savefile->settings.maxQueue < 5) {
                        savefile->settings.maxQueue++;
                        sfx(SFX_MENUMOVE);
                    } else {
                        sfx(SFX_MENUCANCEL);
                    }
                }
                break;
            }
        }

        if (showOptions) {

            aprint(" DONE ", 12, 17);

            int counter = 0;

            for (int i = 0; i < options; i++)
                aprint("       ", endX - 2, startY + i);

            aprint("...", endX, startY + counter++);

            aprintf(savefile->settings.backgroundGrid + 1, endX, startY + counter++);

            if(savefile->settings.skin+dasOffset >= 0)
                aprintf(savefile->settings.skin + 1 + dasOffset, endX, startY + counter);
            else
                aprint("C" + std::to_string(-(savefile->settings.skin + dasOffset)), endX-1, startY + counter);

            counter++;

            aprintf(savefile->settings.shadow + 1, endX, startY + counter++);

            aprintf(savefile->settings.palette + 1, endX, startY + counter++);

            aprintf(savefile->settings.colors+1, endX, startY + counter++);

            if (savefile->settings.edges)
                aprint("ON", endX, startY + counter);
            else
                aprint("OFF", endX, startY + counter);

            counter++;

            if (savefile->settings.lightMode)
                aprint("ON", endX, startY + counter);
            else
                aprint("OFF", endX, startY + counter);

            counter++;

            if (savefile->settings.effects)
                aprint("ON", endX, startY + counter);
            else
                aprint("OFF", endX, startY + counter);

            counter++;

            aprintf(savefile->settings.clearEffect + 1, endX, startY + counter);

            counter++;

            if (savefile->settings.placeEffect)
                aprint("ON", endX, startY + counter);
            else
                aprint("OFF", endX, startY + counter);

            counter++;

            std::string shakeString = std::to_string(savefile->settings.shakeAmount * 25) + "%";
            aprint(shakeString, endX, startY + counter);

            counter++;

            if (savefile->settings.floatText)
                aprint("ON", endX, startY + counter);
            else
                aprint("OFF", endX, startY + counter);

            counter++;

            aprintf(savefile->settings.maxQueue, endX, startY + counter++);

            switch (selection) {
            case 0:
                aprint("[",endX-1,startY+selection);
                aprint("]",endX+3,startY+selection);
                break;
            case 1:
                if (savefile->settings.backgroundGrid > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 2:
                if (savefile->settings.skin+dasOffset > -(MAX_CUSTOM_SKINS))
                    aprint("<", endX - 1 - (savefile->settings.skin+dasOffset < 0), startY + selection);
                if (savefile->settings.skin+dasOffset < MAX_SKINS - 1)
                    aprint(">", endX + 1 + (savefile->settings.skin+dasOffset > 8), startY + selection);
                break;
            case 3:
                if (savefile->settings.shadow > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.shadow < MAX_SHADOWS-1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 4:
                if (savefile->settings.palette > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.palette < 7)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 5:
                if (savefile->settings.colors > 1)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.colors < MAX_COLORS)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 6:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.edges), startY + selection);
                break;
            case 7:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.lightMode), startY + selection);
                break;
            case 8:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.effects), startY + selection);
                break;
            case 9:
                if (savefile->settings.clearEffect > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.clearEffect < MAX_CLEAR_EFFECTS-1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 10:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.placeEffect), startY + selection);
                break;
            case 11:
                if (savefile->settings.shakeAmount > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.shakeAmount < 4)
                    aprint(">", endX + shakeString.size(), startY + selection);
                break;
            case 12:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.floatText), startY + selection);
                break;
            case 13:
                if (savefile->settings.maxQueue > 1)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.maxQueue < 5)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 14:
                aprint("[", 12, 17);
                aprint("]", 17, 17);
                break;
            }
        }

        if (showGame) {
            showQueue();
            showPawn();
            showShadow();
            showHold();
            showBackground();

            if(selection == 9){
                u16 *dest = (u16*) &se_mem[25];
                for(int i = 0; i < game->lengthX; i++){
                    dest[10+i+32*18] = 3 + savefile->settings.lightMode * 0x1000;
                    dest[10+i+32*19] = 3 + savefile->settings.lightMode * 0x1000;
                }
            }
        } else {
            oam_init(obj_buffer, 128);
        }

        oam_copy(oam_mem, obj_buffer, 128);
    }

    reset();

    REG_DISPCNT |= DCNT_BG3;
    REG_BLDCNT = prevBld;
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);

    irq_disable(II_HBLANK);
    gradient(true);
}

void setClearEffect(){
    switch(savefile->settings.clearEffect){
    case 0:
        memcpy16(&tile_mem[0][3], sprite4tiles_bin, sprite4tiles_bin_size / 2);
        break;
    case 1:
        memcpy16(&tile_mem[0][3], sprite25tiles_bin, sprite25tiles_bin_size / 2);
        break;
    case 2:
        memcpy16(&tile_mem[0][3], sprite26tiles_bin, sprite26tiles_bin_size / 2);
        break;
    }
}

static const std::list<std::string> labels = {
    "Background",
    "Grid",
    "Skin",
    "Ghost Piece",
    "Frame Color",
    "Palette",
    "Block Edges",
    "Light Mode",
    "Board Effects",
    "Clear Style",
    "Place Effect",
    "Screen Shake",
    "Clear Text",
    "Previews",
};

void showLabels(){
    int counter = 0;

    for(auto const& label : labels){
        aprint(label,startX, startY + counter++);
    }
}

void gradientEditor(){
    irq_enable(II_HBLANK);

    int display_value = REG_DISPCNT;
    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG2; //Disable all backgrounds except text

    int color[3];

    for(int i = 0; i < 3; i++){
        color[i] = (savefile->settings.backgroundGradient >> (5*i)) & 0x1f;
    }

    int selection = 0;

    int das = 0;

    const int dasMax = 10;
    const int arrMax = 4;

    int c = 0;

    aprint("Press B to Exit",7,18);

    while(1){
        VBlankIntrWait();
        key_poll();

        if(key_hit(KEY_START) || key_hit(KEY_B)){
            sfx(SFX_MENUCANCEL);
            break;
        }

        if(key_hit(KEY_LEFT)){
            if(selection > 0)
                selection--;

            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_RIGHT)){
            if(selection < 2)
                selection++;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_UP)){
            if(color[selection] < 31)
                color[selection]++;
            sfx(SFX_SHIFT2);
        }

        if(key_hit(KEY_DOWN)){
            if(color[selection] > 0)
                color[selection]--;
            sfx(SFX_SHIFT2);
        }

        if(key_is_down(KEY_UP) || key_is_down(KEY_DOWN)){
            das++;

            if(das == dasMax){
                das -= arrMax;

                if(key_is_down(KEY_UP)){
                    if(color[selection] < 31){
                        color[selection]++;
                        sfx(SFX_SHIFT2);
                    }
                }else if(key_is_down(KEY_DOWN)){
                    if(color[selection] > 0){
                        color[selection]--;
                        sfx(SFX_SHIFT2);
                    }
                }
            }
        }else{
            das = 0;
        }

        const int space = 7;
        const int height = 11;

        u16 * dest = (u16*) &se_mem[29];

        c = 0;

        const int tile = 105 + 0xf000;

        for(int i = 0; i < 3; i++){
            int x = 14 + (i-1)*space;

            std::string str = "";
            switch(i){
                case 0: str = "RED"; break;
                case 1: str = "GREEN"; break;
                case 2: str = "BLUE"; break;
            }
            aprint(str,x-str.length()/2 + (i - 1) * (i > 0), height - 4);

            char buff[2];
            posprintf(buff, "%02d",color[i]);

            aprint(buff, x, height);

            if(i == selection){
                dest[32 * (height - 1) + x] = tile;
                dest[32 * (height - 1) + x+1] = tile + 1;
                dest[32 * (height + 1) + x] = tile + 0x800;
                dest[32 * (height + 1) + x+1] = tile + 0x800 + 1;
            }else{
                aprint("  " , x,height-1);
                aprint("  " , x,height+1);
            }

            c += color[i] << (5*i);
        }

        setGradient(c);
    }

    savefile->settings.backgroundGradient = c;

    REG_DISPCNT = display_value;
    clearText();

}
