#include <string>
#include <tonc_types.h>
#include "posprintf.h"
#include <maxmod.h>

extern void aprint(std::string,int,int);

extern void aprintf(int, int, int);

extern void clearText();

INLINE void sfx(int);

extern const u16 fontTiles[1552];
#define fontTilesLen 3104

typedef struct Save{
    u8 highscore[4];
    u8 bestTime[3];
}ALIGN(4) Save;

INLINE void sfx(int s){
	mm_sfxhand h = mmEffect(s);
	mmEffectVolume(h,255);
}