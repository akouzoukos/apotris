#include "maxmod.h"
#include "mm_types.h"
#include "soundbank.h"
#include "def.h"

int currentMenu = 0;
int currentlyPlayingSong = 0;

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

    currentMenu = menuId;
    currentlyPlayingSong = songId;

    mmStart(song, (savefile->settings.cycleSongs)? MM_PLAY_ONCE:MM_PLAY_LOOP);
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

    if (max <= 0)
        return;

    int index = qran() % max;

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

void playNextSong(){
    int songId = -1;
    int max = 0;

    int menuId = currentMenu;

    if (menuId == 0) {
        for (int i = 0; i < MAX_MENU_SONGS; i++)
            max += (savefile->settings.songList[i]);
    } else if (menuId == 1) {
        for (int i = 0; i < MAX_GAME_SONGS; i++)
            max += (savefile->settings.songList[i + MAX_MENU_SONGS]);
    }

    if (max <= 0)
        return;

    int index = currentlyPlayingSong+1;
    if(index >= max)
        index = 0;

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

std::string getSongTitle(){
	switch (currentlyPlayingSong) {
	case 0:
		return "Nikku4211 - Thirno";
	case 1:
		return "kb-zip - oh my god!";
	case 2:
		return "curt cool - unsuspected <h>";
	case 3:
		return "Basq - Warning Infected!";
	default:
		return "Unknown Artist - Unknown Song";
	}
}