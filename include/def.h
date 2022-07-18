#include <string>
#include <tonc_types.h>
#include <maxmod.h>
#include "tetrisEngine.h"

extern void aprint(std::string, int, int);
extern void aprintColor(std::string, int, int, int);

extern void aprintf(int, int, int);

extern void aprints(std::string, int, int, int);

extern void setSmallTextArea(int, int, int, int, int);
extern void clearText();

INLINE void sfx(int);

extern const u16 fontTiles[1552];
#define fontTilesLen 3104
extern const u16 font3x5[96];

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
    int backgroundGrid;
    bool edges;
    int skin;
    int palette;
    int shadow;
    bool lightMode;
    bool songList[10];

}ALIGN(4) Settings;

typedef struct Test{
    bool t1[6];
    int t2[4];
}ALIGN(4) Test;

typedef struct Save{
    int placeholder[128];
    u8 newGame;

    Settings settings;
    int seed;
    char latestName[9];

    Scoreboard marathon[4];
    Timeboard sprint[3];
    Timeboard dig[3];
    Scoreboard ultra[4];

    // Tetris::Game savedGame;
    // bool canLoad;

}ALIGN(4) Save;

INLINE void sfx(int s){
	mm_sfxhand h = mmEffect(s);
	mmEffectVolume(h,255);
}
    
#define glowDuration 12

class FloatText {
public:
    std::string text;
    int timer = 0;

    FloatText() {}
    FloatText(std::string _t) {
        text = _t;
    }
};

class Effect {
public:
    int timer = 0;
    int duration;
    int type;
    int x;
    int y;

    Effect() {}
    Effect(int _type) {
        type = _type;
        duration = glowDuration * 3;
    }
    Effect(int _type, int _x, int _y) {
        type = _type;
        duration = glowDuration * (3 / 2);
        x = _x;
        y = _y;
    }
};


#define TRAINING_MESSAGE_MAX 300
#define MAX_SKINS 7
#define MAX_SHADOWS 5
#define MAX_BACKGROUNDS 6

#define MAX_MENU_SONGS 2
#define MAX_GAME_SONGS 4

#define GRADIENT_COLOR 0x71a6

#define SHOW_FINESSE 1
#define DIAGNOSE 0
#define SAVE_TAG 0x4c
#define ENABLE_BOT 0
// #define GRADIENT_COLOR 0x1a9d

extern void gameLoop();
extern void playSong(int,int);
extern void playSongRandom(int);
extern void settingsText();
extern void songListMenu();
extern void graphicTest();
extern void showTitleSprites();
extern void setLightMode();
extern void setSkin();
extern void update();

extern void showBackground();
extern void showPawn();
extern void showShadow();
extern void showQueue();
extern void showHold();
extern void showFrames();
extern void drawGrid();
extern void drawFrame();
extern void clearGlow();
extern void showClearText();

extern void reset();
extern void sleep();

extern void handleMultiplayer();
extern void startMultiplayerGame(int);
extern void progressBar();

extern void endScreen();
extern void pauseMenu();
extern void countdown();
extern void screenShake();

extern void saveToSram();
extern void addToResults(int,int);

extern void drawEnemyBoard();
extern void handleBotGame();
extern void showPPS();

extern std::string timeToString(int);

extern Save *savefile;
extern OBJ_ATTR obj_buffer[128];
extern OBJ_AFFINE* obj_aff_buffer;
extern Tetris::Game *game;
extern Tetris::Game *botGame;

extern u8 * blockSprite;

extern int shake;
extern int shakeMax;

extern bool onStates;

extern bool multiplayer;

extern bool pause;

extern bool restart;

extern std::list<Effect> effectList;
extern std::list<FloatText> floatingList;

extern int glow[20][10];

extern int nextSeed;

extern int push;
#define pushMax 2

extern int canDraw;
extern int gameSeconds;

extern bool playAgain;

extern int connected;
extern int multiplayerStartTimer;

extern int initialLevel;
extern int frameCounter;

extern OBJ_ATTR * titleSprites[2];
extern OBJ_ATTR * queueFrameSprites[3];

extern int enemyHeight;

extern Tetris::Bot *testBot;
extern int enemyBoard[20][10];
