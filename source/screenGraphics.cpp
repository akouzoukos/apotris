#include "def.h"
#include "tonc.h"
#include "text.h"
#include "soundbank.h"
#include "tonc_bios.h"
#include "sprites.h"
#include <string>

using namespace Tetris;

void graphicTest() {
    irq_disable(II_HBLANK);
    setPalette();

    if (savefile->settings.lightMode)
        memset16(pal_bg_mem, 0x5ad6, 1);//background gray
    else
        memset16(pal_bg_mem, 0x0000, 1);

    memset16(&se_mem[26], 0x0000, 32 * 20);

    delete game;
    game = new Game(3,bigMode);
    game->setGoal(0);

    game->update();

    game->pawn.y++;

    bool showOptions = true;
    bool showGame = true;

    int startX = 5;
    int endX = 24;

    int startY = 2;
    int options = 14;
    int selection = 0;

    int maxDas = 16;
    int dasVer = 0;

    int maxArr = 3;
    int arr = 0;

    REG_DISPCNT = 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering

    int prevBld = REG_BLDCNT;
    REG_BLDCNT = (1 << 6) + (0b11111 << 8) + (1 << 3) ;
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(1);
    memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);

    showBackground();
    while (1) {
        VBlankIntrWait();
        drawGrid();
        drawFrame();
        screenShake();
        showPlaceEffect();

        if(savefile->settings.floatText){
            showClearText();
        }

        key_poll();

        aprint("R: Toggle", 0, 18);
        aprint("Options", 3, 19);

        if(selection == 8){
            if(!game->clearLock)
                game->demoClear();
        }else{
            if(game->clearLock)
                game->demoFill();
        }

        if (key_hit(KEY_START) || key_hit(KEY_A)) {
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

        if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)) {
            switch (selection) {
            case 0:
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
            case 1:
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
                // setLightMode();
                break;
            case 2:
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
            case 3:
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
            case 4:
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
            case 5:
                savefile->settings.edges = !savefile->settings.edges;
                sfx(SFX_MENUMOVE);
                break;
            case 6:
                savefile->settings.lightMode = !savefile->settings.lightMode;
                memset16(&se_mem[27], 12+4*0x1000 * (savefile->settings.lightMode), 32 * 20);
                setSkin();
                // setLightMode();

                sfx(SFX_MENUMOVE);
                break;
            case 7:
                savefile->settings.effects = !savefile->settings.effects;
                if (savefile->settings.effects) {
                    effectList.push_back(Effect(1, 4, 5));
                }
                sfx(SFX_MENUMOVE);
                break;
            case 8:
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
            case 9:
                savefile->settings.placeEffect = !savefile->settings.placeEffect;
                if(savefile->settings.placeEffect && (int)placeEffectList.size() <= 3){
                    placeEffectList.push_back(PlaceEffect(game->pawn.x, game->pawn.y-20, 0, 0, game->pawn.current, 0, false));
                }

                sfx(SFX_MENUMOVE);
                break;
            case 10:
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
            case 11:
                savefile->settings.floatText = !savefile->settings.floatText;

                if(savefile->settings.floatText){
                    floatingList.push_back(FloatText("quad"));
                }else{
                    floatingList.clear();
                }
                sfx(SFX_MENUMOVE);
                break;
            case 12:
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

            aprint(" DONE ", 12, 16);
            int counter = 0;

            aprint("Background", startX, startY + counter++);
            aprint("Skin", startX, startY + counter++);
            aprint("Ghost Piece", startX, startY + counter++);
            aprint("Frame Color", startX, startY + counter++);
            aprint("Palette", startX, startY + counter++);
            aprint("Block Edges", startX, startY + counter++);
            aprint("Light Mode", startX, startY + counter++);
            aprint("Board Effects", startX, startY + counter++);
            aprint("Clear Style", startX, startY + counter++);
            aprint("Place Effect", startX, startY + counter++);
            aprint("Screen Shake", startX, startY + counter++);
            aprint("Clear Text", startX, startY + counter++);
            aprint("Previews", startX, startY + counter++);

            counter = 0;

            for (int i = 0; i < options; i++)
                aprint("       ", endX - 2, startY + i);

            aprintf(savefile->settings.backgroundGrid + 1, endX, startY + counter++);

            if(savefile->settings.skin >= 0)
                aprintf(savefile->settings.skin + 1, endX, startY + counter);
            else
                aprint("C" + std::to_string(-savefile->settings.skin), endX-1, startY + counter);

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
                if (savefile->settings.backgroundGrid > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.backgroundGrid < MAX_BACKGROUNDS - 1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 1:
                if (savefile->settings.skin > -(MAX_CUSTOM_SKINS))
                    aprint("<", endX - 1 - (savefile->settings.skin < 0), startY + selection);
                if (savefile->settings.skin < MAX_SKINS - 1)
                    aprint(">", endX + 1 + (savefile->settings.skin > 8), startY + selection);
                break;
            case 2:
                if (savefile->settings.shadow > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.shadow < MAX_SHADOWS-1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 3:
                if (savefile->settings.palette > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.palette < 7)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 4:
                if (savefile->settings.colors > 1)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.colors < MAX_COLORS)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 5:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.edges), startY + selection);
                break;
            case 6:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.lightMode), startY + selection);
                break;
            case 7:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.effects), startY + selection);
                break;
            case 8:
                if (savefile->settings.clearEffect > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.clearEffect < MAX_CLEAR_EFFECTS-1)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 9:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.placeEffect), startY + selection);
                break;
            case 10:
                if (savefile->settings.shakeAmount > 0)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.shakeAmount < 4)
                    aprint(">", endX + shakeString.size(), startY + selection);
                break;
            case 11:
                aprint("[", endX - 1, startY + selection);
                aprint("]", endX + 2 + !(savefile->settings.floatText), startY + selection);
                break;
            case 12:
                if (savefile->settings.maxQueue > 1)
                    aprint("<", endX - 1, startY + selection);
                if (savefile->settings.maxQueue < 5)
                    aprint(">", endX + 1, startY + selection);
                break;
            case 13:
                aprint("[", 12, 16);
                aprint("]", 17, 16);
                break;
            }
        }

        if (showGame) {
            showBackground();
            showQueue();
            showPawn();
            showShadow();
            showBackground();
            showHold();
        } else {
            oam_init(obj_buffer, 128);
        }

        oam_copy(oam_mem, obj_buffer, 128);
    }

    reset();

    REG_DISPCNT |= DCNT_BG3;
    irq_enable(II_HBLANK);
    REG_BLDCNT = prevBld;
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
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
        memcpy16(&tile_mem[0][3], sprite26tiles_bin, sprite25tiles_bin_size / 2);
        break;
    }
}
