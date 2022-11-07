#pragma once

#include <string>
#include <tonc_types.h>
#include <maxmod.h>
#include "tetrisEngine.h"

INLINE void sfx(int);

INLINE FIXED lerp(FIXED a, FIXED b, FIXED mix){
    return a + (((b-a)*mix)>>8);
}

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

typedef struct Gradeboard{
    Time times[5];
    s8 grade[5];
}ALIGN(4) Gradeboard;

typedef struct Keys{
    int moveLeft;
    int moveRight;
    int rotateCW;
    int rotateCCW;
    int rotate180;
    int softDrop;
    int hardDrop;
    int hold;
    int zone;

    int placeHolder[19];
}ALIGN(4) Keys;

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
    int sfxVolume;
    bool directionalDas;
    int shakeAmount;
    bool noDiagonals;
    int maxQueue;
    int colors;
    bool cycleSongs;
    int dropProtectionFrames;
    bool abHold;
    struct Keys keys;
    int clearEffect;
    bool resetHold;
    bool placeEffect;
    int rumble;
    int diagonalType;
    bool delaySoftDrop;

    int placeHolder[98];
}ALIGN(4) Settings;

typedef struct Test{
    bool t1[6];
    int t2[4];
}ALIGN(4) Test;

typedef struct Test2{
    bool t1[18];
    int t2[10];
}ALIGN(4) Test2;

typedef struct Test3{
    bool t1[14];//14
    int t2[30];//22
}ALIGN(4) Test3;

typedef struct TotalStats{
    int timePlayed;
    int gamesStarted;
    int gamesCompleted;
    int gamesLost;
}ALIGN(4) TotalStats;

typedef struct Skin{
    TILE board;
    TILE smallBoard;
    int previewStyle;
}ALIGN(4) Skin;

#define MAX_CUSTOM_SKINS 5

typedef struct Save{
    u8 newGame;

    Settings settings;
    int seed;
    char latestName[9];

    Scoreboard marathon[4];
    Timeboard sprint[3];
    Timeboard dig[3];
    Scoreboard ultra[3];
    Scoreboard blitz[2];
    Scoreboard combo;
    Timeboard survival[3];
    Timeboard sprintAttack[3];
    Scoreboard digEfficiency[3];
    Scoreboard classic[2];
    Gradeboard master[2];
    Scoreboard zone[4];

    int placeHolder[1000];

    TotalStats stats;

    int placeHolder2[100];

    Skin customSkins[MAX_CUSTOM_SKINS];
}ALIGN(4) Save;

extern Save *savefile;

INLINE void sfx(int s){
	mm_sfxhand h = mmEffect(s);
	mmEffectVolume(h, 255 * (float)savefile->settings.sfxVolume / 10);
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

class PlaceEffect{
public:

        int x;
        int y;
        int dx;
        int dy;
        int piece;
        int rotation;
        int rotating;

        int timer = 12;

        OBJ_ATTR* sprite;

        PlaceEffect(){}
        PlaceEffect(int _x, int _y, int _dx, int _dy, int _piece, int _rotation, int _rotating){
            x = (_x + 10) * 8 - 16 - 32;
            y = _y * 8 - 16;
            dx = _dx;
            dy = _dy;
            piece = _piece;
            rotation = _rotation;
            rotating = _rotating;
        }
};

#define TRAINING_MESSAGE_MAX 300
#define MAX_SKINS 14
#define MAX_SHADOWS 5
#define MAX_BACKGROUNDS 6
#define MAX_COLORS 7
#define MAX_CLEAR_EFFECTS 3

#define MAX_MENU_SONGS 2
#define MAX_GAME_SONGS 4

#define GRADIENT_COLOR 0x71a6

#define SHOW_FINESSE 1
#define DIAGNOSE 0
#define SAVE_TAG 0x50
#define ENABLE_BOT 0

#define ENABLE_FLASH_SAVE 1

extern void gameLoop();
extern void playSong(int,int);
extern void playSongRandom(int);
extern void playNextSong();
extern void settingsText();
extern void songListMenu();
extern void graphicTest();
extern void audioSettings();
extern void handlingSettings();
extern void controlsSettings();
extern void showTitleSprites();
extern void setLightMode();
extern void setSkin();
extern void update();
extern void setDefaultKeys();
extern void setClearEffect();

extern void showBackground();
extern void showPawn();
extern void showShadow();
extern void showQueue();
extern void showHold();
extern void drawGrid();
extern void drawFrame();
extern void clearGlow();
extern void showClearText();
extern void hideMinos();

extern void reset();
extern void sleep();

extern void handleMultiplayer();
extern void startMultiplayerGame(int);
extern void progressBar();

extern int endScreen();
extern int pauseMenu();
extern void countdown();
extern void screenShake();

extern void saveToSram();
extern void addToResults(int,int);

extern void drawEnemyBoard();
extern void handleBotGame();
extern void showPPS();
extern void showFinesse();
extern void setPalette();
extern void loadSave();
extern void startScreen();
extern void showText();
extern void showPlaceEffect();
extern void checkSounds();
extern int getClassicPalette();
extern void skinEditor();
extern void maxModInit();
extern void drawUIFrame(int,int,int,int);
extern void buildMini(TILE *);
extern void showZoneMeter();
extern void resetZonePalette();
extern void showBestMove();
extern Tetris::Tuning getTuning();
extern void showFinesseCombo();

extern std::string timeToString(int);

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
extern std::list<PlaceEffect> placeEffectList;

extern s16 glow[20][10];

extern int nextSeed;

extern int push;
#define pushMax 4

extern bool canDraw;
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
extern bool enemyBoard[20][10];

extern int mode;

extern int currentlyPlayingSong;
extern int currentMenu;

extern int previousOptionScreen;
extern bool goToOptions;

extern int rumbleTimer;
extern int rumbleMax;

extern bool rumbleInitialized;
extern bool bigMode;

extern int subMode;
extern int goalSelection;
extern int level;

extern TILE* customSkin;

extern bool proMode;

#define shakeMax 10
