#include <string>
#include <tonc_types.h>
#include "posprintf.h"
#include <maxmod.h>
#include "tetrisgame.h"

extern void aprint(std::string,int,int);
extern void aprintColor(std::string,int,int,int);

extern void aprintf(int, int, int);

extern void clearText();

INLINE void sfx(int);

extern const u16 fontTiles[1552];
#define fontTilesLen 3104

typedef struct Highscore{
    char name[9];
    int score;
}ALIGN(4) Highscore;

typedef struct Scoreboard{
    Highscore highscores[5];
}ALIGN(4) Scoreboard;

typedef struct Time{
    char name[9];
    int frames;
}ALIGN(4) Time;

typedef struct Timeboard{
    Time times[5];
}ALIGN(4) Timeboard;

typedef struct Settings{

    bool announcer;
    bool finesse;
    bool floatText;
    bool shake;
    bool effects;
    int volume;
    int das;
    int arr;
    int sfr;
    bool dropProtection;

}ALIGN(4) Settings;

typedef struct Save{
    u8 newGame;

    Settings settings;
    char latestName[9];

    Scoreboard marathon[4];
    Timeboard sprint[3];
    Timeboard dig[3];

    // Tetris::Game savedGame;
    // bool canLoad;

}ALIGN(4) Save;

INLINE void sfx(int s){
	mm_sfxhand h = mmEffect(s);
	mmEffectVolume(h,255);
}