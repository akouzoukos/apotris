#include "maxmod.h"
#include "soundbank.h"
#include "def.h"
#include "tonc_core.h"

void playSong(int menuId, int songId) {

    int song = 0;

    if (menuId == 0) {
        switch (songId) {
        case 0:
            song = MOD_MENU;
            break;
        case 1:
            song = MOD_OPTIKAL_INNOVATION;
            break;
        default:
            return;
        }
    } else if (menuId == 1) {
        switch (songId) {
        case 0:
            song = MOD_THIRNO;
            break;
            // case 1:
            // 	song = MOD_ALDEBARAN_SHORT;
            // 	break;
        case 1:
            song = MOD_OH_MY_GOD;
            break;
        case 2:
            song = MOD_UNSUSPECTED_H;
            break;
        case 3:
            song = MOD_WARNING_INFECTED;
            break;
        default:
            return;
        }
    }

    mmStart(song, MM_PLAY_LOOP);
    mmSetModuleVolume(512 * ((float)savefile->settings.volume / 10));
}

void playSongRandom(int menuId) {

    int songId = -1;
    int max = 0;


    if (menuId == 0) {
        for (int i = 0; i < MAX_MENU_SONGS; i++)
            max += (savefile->settings.songList[i]);
    } else if (menuId == 1) {
        for (int i = 0; i < MAX_GAME_SONGS; i++)
            max += (savefile->settings.songList[i + MAX_MENU_SONGS]);
    }

    int index = 0;
    if (max > 0) {
        index = qran() % max;
    } else {
        return;
    }

    int start = 0;
    if (menuId == 1)
        start = MAX_MENU_SONGS;

    int counter = 0;
    for (int i = start; i < 10;i++) {
        if (counter == index && savefile->settings.songList[i]) {
            songId = i;
            if (menuId == 1)
                songId -= MAX_MENU_SONGS;
            break;
        }
        if (savefile->settings.songList[i])
            counter++;
    }

    if (songId == -1)
        return;

    playSong(menuId, songId);
}
