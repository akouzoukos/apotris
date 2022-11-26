#pragma once

#include <string>
#include <tonc_types.h>
#include <maxmod.h>
#include "tetrisEngine.h"
#include "tonc_core.h"
#include "tonc_video.h"

INLINE FIXED lerp(FIXED a, FIXED b, FIXED mix){
    return a + (((b-a)*mix)>>8);
}

INLINE int flipSign(int x, int y){
    return (((x > 0) - (x < 0)) ^ y) - y;
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
    int backgroundGradient;
    bool customDas;
    int ihs;
    int irs;

    int placeHolder[94];
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
#define DIAGNOSE 1
#define SAVE_TAG 0x50
#define ENABLE_BOT 1

#define ENABLE_FLASH_SAVE 1

extern void sfx(int);
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
extern void showTimer();
extern void setGradient(int);
extern void setDefaultGradient();
extern void gradient(bool state);
extern std::string timeToString(int);
extern void setDefaultGraphics(Save * save, int depth);
extern void showCredits();
extern void setupCredits();
extern void refreshCredits();

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

extern bool resumeJourney;
extern bool journeyLevelUp;
extern bool journeySaveExists;
extern Tetris::Game* journeySave;

extern int connected;
extern int multiplayerStartTimer;

extern int initialLevel;
extern int frameCounter;

extern OBJ_ATTR * titleSprites[2];
extern OBJ_ATTR * queueFrameSprites[3];

extern int enemyHeight;

extern Tetris::Bot *testBot;
extern u8 enemyBoard[20][10];

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

extern bool gradientEnabled;

#define shakeMax 10

class Scene{
public:
    virtual void draw();

    Scene(){};
    virtual ~Scene(){};
};

class GameScene : public Scene{
    void draw();
};

class TitleScene : public Scene{
    void draw();
};

class GraphicsScene : public Scene{
    void draw();
};

class EditorScene : public Scene{
    void draw();
};

extern void changeScene(Scene * newScene);

class WordSprite {
public:
    std::string text = "";
    int startIndex;
    int startTiles;
    int id;
    int priority = 1;
    OBJ_ATTR* sprites[3];

    void show(int x, int y, int palette) {
        for (int i = 0; i < 3; i++) {
            obj_unhide(sprites[i], 0);
            obj_set_attr(sprites[i], ATTR0_WIDE, ATTR1_SIZE(1), ATTR2_BUILD(startTiles + i * 4, palette, priority));
            obj_set_pos(sprites[i], x + i * 32, y);
        }
    }

    void show(int x, int y, int palette, FIXED scale) {
        for (int i = 0; i < 3; i++) {
            int affId = id*3+i;
            obj_unhide(sprites[i], 0);
            obj_set_attr(sprites[i], ATTR0_WIDE | ATTR0_AFF, ATTR1_SIZE(1) | ATTR1_AFF_ID(affId), palette);
            sprites[i]->attr2 = ATTR2_BUILD(startTiles + i * 4, palette, 1);
            obj_set_pos(sprites[i], x + i * 32, y);
            obj_aff_identity(&obj_aff_buffer[affId]);
            obj_aff_scale(&obj_aff_buffer[affId], scale, scale);
        }
    }

    void hide() {
        for (int i = 0; i < 3; i++)
            obj_hide(sprites[i]);
    }

    void setTextFast(char** _text, int size) {
        if (*_text == text)
            return;

        text = *_text;

        if(text == ""){
            memset32(&tile_mem[4][startTiles], 0, 12 * 8);
            return;
        }

        int n = min(size,12);

        TILE* font = (TILE*)fontTiles;
        int i;
        for (i = 0; i < n; i++) {
            int c = *_text[i] - 32;

            // memcpy32(&tile_mem[4][startTiles + i], &font[c], 8);
            dma3_cpy(&tile_mem[4][startTiles + i], &font[c], 32);
        }

        if(i < 12){
            memset32(&tile_mem[4][startTiles + i], 0, (12-i) * 8);
        }
    }

    void setText(std::string _text) {
        if (_text == text)
            return;

        text = _text;


        if(text == ""){
            memset32(&tile_mem[4][startTiles], 0, 12 * 8);
            return;
        }

        int n = min((int)text.size(),12);

        TILE* font = (TILE*)fontTiles;
        int i;
        for (i = 0; i < n; i++) {
            int c = text[i] - 32;

            // memcpy32(&tile_mem[4][startTiles + i], &font[c], 8);
            dma3_cpy(&tile_mem[4][startTiles + i], &font[c], 32);
        }

        if(i < 12){
            memset32(&tile_mem[4][startTiles + i], 0, (12-i) * 8);
        }
    }

    WordSprite(int _id,int _index, int _tiles) {
        id = _id;
        startIndex = _index;
        startTiles = _tiles;

        for (int i = 0; i < 3; i++) {
            sprites[i] = &obj_buffer[startIndex + i];
        }
    }
};

#define MAX_WORD_SPRITES 15
extern WordSprite* wordSprites[MAX_WORD_SPRITES];

class Function{
    private:

    public:
        void (Tetris::Game::*gameFunction)(int);
};
