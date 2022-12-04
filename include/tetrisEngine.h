#pragma once

#include <list>
#include <string>
#include "tetromino.hpp"
#include <tuple>
#include "platform.h"

namespace Tetris
{
    #define MAX_DISAPPEAR 300
    #define CREDITS_LENGTH 3240 //54 seconds * 60
    #define maxSleep 1;

    extern const u16 connectedConversion[24];
    extern const u16 connectedFix[3][24];
    extern int* getShape(int piece,int rotation,int rotationSystem);

    enum Modes{
        NO_MODE,
        MARATHON,
        SPRINT,
        DIG,
        BATTLE,
        ULTRA,
        BLITZ,
        COMBO,
        SURVIVAL,
        CLASSIC,
        MASTER,
        TRAINING,
    };

    enum RotationSystems{
        SRS,
        NRS,
        ARS,
        A_SRS,
    };

    enum Pieces{
        PIECE_I,
        PIECE_J,
        PIECE_L,
        PIECE_O,
        PIECE_S,
        PIECE_T,
        PIECE_Z,
    };

    class Stats{
    public:
        int clears[4] = {0,0,0,0};
        int tspins = 0;
        int perfectClears = 0;
        int maxStreak = 0;
        int maxCombo = 0;
        int holds = 0;
        int maxZonedLines = 0;

        Stats(){}
    };

    class Color {
    public:
        int r;
        int g;
        int b;
    };
    
    class Drop {
    public:
        int on = false;
        int startX = 0;
        int endX = 0;
        int startY = 0;
        int endY = 0;
        int rawEndY = 0;

        int x;
        int y;

        int dx;
        int dy;

        int piece;
        int rotation;
        int rotating;


        Drop(){}
        Drop(const Drop& oldDrop){
            on = oldDrop.on;
            startX = oldDrop.startX;
            endX = oldDrop.endX;
            startY = oldDrop.startY;
            endY = oldDrop.endY;
            rawEndY = oldDrop.rawEndY;
            x = oldDrop.x;
            y = oldDrop.y;
            dx = oldDrop.dx;
            dy = oldDrop.dy;
            piece = oldDrop.piece;
            rotation = oldDrop.rotation;
            rotating = oldDrop.rotating;
        }
    };

    class Move{
    public:
        int dx = 0;
        int rotation = 0;
        int height = 40;
        int holes = 0;
        int piece = -1;
        int clears = 0;
        int wells = 0;
        int score = -0x7fffffff;

        Move(){}

        Move(const Move& oldMove){
            dx = oldMove.dx;
            rotation = oldMove.rotation;
            height = oldMove.height;
            holes = oldMove.holes;
            piece = oldMove.piece;
            clears = oldMove.clears;
            score = oldMove.score;
        }
    };

    class Garbage {
    public:
        int id;
        int amount;
        int timer = 60;

        Garbage(int _id,int _amount){
            id = _id;
            amount = _amount;
        }
    };

    class Score {
    public:
        int linesCleared = 0;
        int score = 0;
        int combo = 0;
        int isTSpin = 0;
        int isPerfectClear = 0;
        int isBackToBack = 0;
        int isDifficult = 0;
        Drop drop;

        Score(){}
        Score(int lC, int sc, int co, int isTS, int isPC, int isBTB, int isDif, Drop drp) {
            linesCleared = lC;
            score = sc;
            combo = co;
            isTSpin = isTS;
            isPerfectClear = isPC;
            isBackToBack = isBTB;
            isDifficult = isDif;
            drop = drp;
        }

        Score(const Score& oldScore){
            linesCleared = oldScore.linesCleared;
            score = oldScore.score;
            combo = oldScore.combo;
            isTSpin = oldScore.isTSpin;
            isPerfectClear = oldScore.isPerfectClear; 
            isBackToBack = oldScore.isBackToBack;
            isDifficult = oldScore.isDifficult;
            drop = Drop(oldScore.drop);
        }
    };

    class SoundFlags {
    public:
        int shift = 0;
        int rotate = 0;
        int place = 0;
        int invalid = 0;
        int hold = 0;
        int clear = 0;
        int levelUp = 0;
        int finesse = 0;
        int section = 0;
        int disappear = 0;
        int zone = 0;
        int meter = 0;

        SoundFlags() {}
        SoundFlags(const SoundFlags& oldFlags){
            shift = oldFlags.shift;
            rotate = oldFlags.rotate;
            place = oldFlags.place;
            invalid = oldFlags.invalid;
            hold = oldFlags.hold;
            clear = oldFlags.clear;
            levelUp = oldFlags.levelUp;
            finesse = oldFlags.finesse;
            section = oldFlags.section;
            disappear = oldFlags.disappear;
            zone = oldFlags.zone;
            meter = oldFlags.meter;
        }
    };

    class Pawn{
    public:
        int x;
        int y;
        int type;
        int current = -1;
        int rotation = 0;
        int board[4][4][4];
        int boardLowest[4][4];
        int heighest[4];
        int lowest;
        bool big = false;
        void setBlock(int system);

        Pawn(int newX, int newY) {
            for(int i = 0; i < 4; i++){
                heighest[i] = -1;
                for(int j = 0; j < 4; j++)
                    boardLowest[i][j] = -1;
            }

            x = newX;
            y = newY;
        }

        Pawn(const Pawn& oldPawn){
            x = oldPawn.x;
            y = oldPawn.y;

            type = oldPawn.type;
            current = oldPawn.current;
            rotation = oldPawn.rotation;
            lowest = oldPawn.lowest;

            for(int i = 0; i < 4; i++)
                for(int j = 0; j < 4; j++)
                    for(int k = 0; k < 4; k++)
                        board[i][j][k] = oldPawn.board[i][j][k];

            for(int i = 0; i < 4; i++)
                for(int j = 0; j < 4; j++)
                    boardLowest[i][j] = oldPawn.boardLowest[i][j];

            for(int i = 0; i < 4; i++)
                heighest[i] = oldPawn.heighest[i];
        }
    };

    class Tuning{
    public:
            int das = 8;
            int arr = 2;
            int sfr = 2;
            int dropProtection = 8;
            bool directionalDas = false;
            bool delaySoftDrop = true;
            bool ihs = false;
            bool irs = false;
    };

    class Options{
    public:
            int mode = 0;
            int goal = 0;
            int level = 0;
            Tuning tuning;

            bool trainingMode = false;
            bool bigMode = false;

            int bTypeHeight = 0;
            int subMode = 0;
            int rotationSystem = SRS;
    };

    class Game {
    private:
        void fillBag();
        void fillQueue(int);
        void fillQueueSeed(int,int);
        bool moveLeft();
        bool moveRight();
        void moveDown();
        int clear(Drop);
        void lockCheck();
        void next();
        void place();
        void generateGarbage(int,int);
        Drop calculateDrop();
        void setMasterTuning();
        void rotate(int);
        int checkSpecialRotation(int,int);
        int checkITouching(int,int);
        void rotatePlace(int,int,int,int);
        void updateDisappear();
        void endZone();
        void clearBoard();
        void fixConnected(std::list<int>);
        void connectBoard(int startY, int endY);

        int bigBag[35];

        std::list<int> bag;

        float speedCounter = 0;

        //7  117
        //8  133
        //9  150 
        //11 183
        //16 267

        int maxDas = 8;
        int das = 0;
        int arr = 2;
        int arrCounter = 0;

        int softDropCounter = 0;
        int maxSoftDrop = 10;
        int softDropSpeed = 2;
        int softDropRepeatTimer = 0;

        int lockMoveCounter = 15;

        int left = 0;
        int right = 0;
        int down = 0;

        int lastMoveRotation = 0;
        int lastMoveDx = 0;
        int lastMoveDy = 0;

        int finesseCounter = 0;

        Drop lastDrop;

        bool dropProtection = true;
        bool directionCancel = true;

        int dropLockTimer = 0;
        int dropLockMax = 8;

        bool specialTspin = false;

        int pieceHistory = -1;

        int gracePeriod = 0;

        int rotating = 0;

        int section = 0;
        int sectionStart = 0;
        int previousSectionTime = 1000000;

        int internalGrade = 0;
        int gradePoints = 0;

        bool cool = false;
        bool regret = false;
        int decayTimer = 0;

        bool stopLockReset = false;
        bool fromLock = false;

        std::list<int> historyList;

        int pieceDrought[7];
        bool delaySoftDrop = true;

        bool ihs = false;
        bool irs = false;

        bool rotates[3] = {false,false,false};

    public:
        const int lengthX = 10;
        const int lengthY = 40;
        int** board;
        std::list<int> queue;
        Pawn pawn = Pawn(0, 0);
        int held = -1;
        float speed;
        int linesCleared = 0;
        int level = 0;
        int score = 0;
        int comboCounter = 0;
        std::list<int> linesToClear;
        int clearLock = 0;
        int lost = 0;
        int gameMode;
        SoundFlags sounds;
        Score previousClear = Score(0, 0, 0, 0, 0, 0, 0, Drop());
        int timer = 0;
        int refresh = 0;
        int won = 0;
        int goal = 40;
        int finesseFaults = 0;
        int garbageCleared = 0;
        int garbageHeight = 0;
        int pushDir = 0;
        int b2bCounter = 0;
        int bagCounter = 0;
        int linesSent = 0;
        int moveCounter = 0;
        int pieceCounter = 0;
        std::list<Garbage> attackQueue;
        std::list<Garbage> garbageQueue;
        std::list<int> moveHistory;
        std::list<int> previousBest;
        int previousKey = 0;
        bool softDrop = false;
        bool canHold = true;
        bool holding = false;
        int holdCounter = 0;
        bool trainingMode = false;
        int seed = 0;
        int initSeed = 0;
        int initialLevel = 0;
        int eventTimer = 0;

        bool dropping = false;
        int entryDelay = 0;
        int areMax = 0;
        int lineAre = 0;
        int maxClearDelay = 1;

        int bTypeHeight = 0;

        Stats statTracker;

        int subMode = 0;
        int zoneCharge = 0;
        int zoneTimer = 0;
        int zonedLines = 0;
        int zoneClearCounter = 0;
        int zoneScore = 0;
        int zoneStart = 0;
        bool fullZone = false;
        bool inversion = false;

        int grade = 0;
        int coolCount = 0;
        int regretCount = 0;

        int rotationSystem = SRS;

        int maxLockTimer = 30;
        int lockTimer = maxLockTimer;

        u16 ** disappearTimers;
        int disappearing = 0;


        std::list<std::tuple<u8,u8>> toDisappear;

        float creditGrade = 0;

        bool eventLock = false;

        int finesseStreak = 0;

        int inGameTimer = 0;

        int stackHeight = lengthY;

        int checkRotation(int, int, int);
        void rotateCW(int dir);
        void rotateCCW(int dir);
        void rotateTwice(int dir);
        void hardDrop();
        void update();
        int lowest();
        Color color(int);
        void hold(int dir);
        void keyLeft(int);
        void keyRight(int);
        void keyDown(int);
        void keyDrop(int);
        void removeClearLock();
        void resetSounds();
        void resetRefresh();
        void setLevel(int);
        void setGoal(int);
        Drop getDrop();
        void setTuning(Tuning);
        void clearAttack(int);
        void setWin();
        void addToGarbageQueue(int,int);
        std::list<int> getBestFinesse(int,int,int);
        int getIncomingGarbage();
        Move findBestDrop();
        void setTrainingMode(bool);
        void demoClear();
        void demoFill();
        void bType(int);
        void setSubMode(int);
        void setSpeed();
        void setRotationSystem(int);
        void removeEventLock();
        void activateZone(int);
        void liftKeys();
        void setOptions(Options newOptions);

        Game(){
            seed = initSeed = qran();
        }

        Game(int gm, int sd, bool big){
            gameMode = gm;
            seed = initSeed = sd;
            board = new int* [lengthY];

            for (int i = 0; i < lengthY; i++) {
                board[i] = new int[lengthX];
                for (int j = 0; j < lengthX; j++)
                    board[i][j] = 0;
            }

            fillBag();

            if(gameMode == CLASSIC){
                maxLockTimer = 1;
                fillQueue(1);

                maxDas = 16;
                arr = 6;
                softDropSpeed = 2;
                gracePeriod = 90;

                rotationSystem = NRS;
            }else
                fillQueue(5);

            pawn = Pawn(0,0);
            pawn.big = big;

            if(gameMode == SPRINT)
                goal = 40;
            else if(gameMode == MARATHON)
                goal = 150;
            else if(gameMode == DIG){
                goal = 100;
                if(!big)
                    generateGarbage(9,0);
                else
                    generateGarbage(4,0);
            }else if(gameMode == COMBO){
                if(!big){
                    for(int i = lengthY/2-1; i < lengthY; i++){
                        for(int j = 0; j < 10; j++){
                            if(j > 2 && j < 7 && !(i == lengthY-2 && j < 5) && !(i == lengthY-1 && j < 4))
                                continue;
                            board[i][j] = i % 7 + 1;
                        }
                        connectBoard(i, i);
                    }
                }else{
                    for(int i = (lengthY/4-1); i < lengthY/2; i++){
                        for(int j = 0; j < 5; j++){
                            if(j != 0 && !(i == (lengthY/2)-2 && j < 3) && !(i == (lengthY/2)-1 && j < 2))
                                continue;
                            board[i*2][j*2] = i % 7 + 1;
                            board[i*2][j*2+1] = i % 7 + 1;
                            board[i*2+1][j*2] = i % 7 + 1;
                            board[i*2+1][j*2+1] = i % 7 + 1;
                        }
                    }
                }
            }else if(gameMode == MASTER){
                level = 0;
                speed = GameInfo::masterGravity[0][1];
                setMasterTuning();
            }

        }

        Game(int gm,bool big) : Game(gm,qran(),big){}

        Game(const Game& oldGame){
            canHold = oldGame.canHold;
            bag = oldGame.bag;

            board = new int* [lengthY];

            for (int i = 0; i < lengthY; i++) {
                board[i] = new int[lengthX];
                for (int j = 0; j < lengthX; j++)
                    board[i][j] = oldGame.board[i][j];
            }
            
            queue = oldGame.queue;

            pawn = Pawn(oldGame.pawn);

            held = oldGame.held;
            linesCleared = oldGame.linesCleared;
            level = oldGame.level;
            score = oldGame.score;
            comboCounter = oldGame.comboCounter;
            linesCleared = oldGame.linesCleared;
            clearLock = oldGame.clearLock;
            lost = oldGame.lost;
            gameMode = oldGame.gameMode;
            sounds = SoundFlags(oldGame.sounds);
            previousClear = Score(oldGame.previousClear);
            timer = oldGame.timer;
            inGameTimer = oldGame.inGameTimer;
            refresh = oldGame.refresh;
            won = oldGame.won;
            goal = oldGame.goal;
            finesseFaults = oldGame.finesseFaults;
            garbageCleared = oldGame.garbageCleared;
            garbageHeight = oldGame.garbageHeight;
            pushDir = oldGame.pushDir;
            b2bCounter = oldGame.b2bCounter;
            bagCounter = oldGame.bagCounter;
            seed = oldGame.seed;
            initSeed = oldGame.initSeed;
            subMode = oldGame.subMode;
            initialLevel = oldGame.initialLevel;
            trainingMode = oldGame.trainingMode;
            clearLock = oldGame.clearLock;
            linesToClear = oldGame.linesToClear;
            pawn.big = oldGame.pawn.big;
        }


        ~Game(){
            for(int i = 0; i < lengthY; i++)
                delete[] board[i];
            delete[] board;

            if(disappearing){
                for(int i = 0; i < lengthY; i++)
                    delete[] disappearTimers[i];
                delete[] disappearTimers;
            }
        }
    };

    typedef struct Weights{
        int clears = 0;
        int holes = 0;
        int height = 0;
        int lowest = 0;
        int jag = 0;
        int well = 0;
        int row = 0;
        int col = 0;
    }Weights;

    class Bot{
    public:
        bool thinking = true;
        int thinkingI = -6;
        int thinkingJ = 0;
        Move current;
        Move best;
        int sleepTimer = 0;

        int dx = 0;
        int rotation = 0;
        Game* game;
        int **testBoard;

        int currentHoles = 0;
        int currentJag = 0;
        int currentWells = 0;
        int currentRowTrans = 0;
        int currentColTrans = 0;

        int checking = 0;
        int previous = 0;

        Weights weights{
        992,
        -206,
        799,
        142,
        400,
        -140,
        -797,
        -969
        };

        void run();

        void move();

        int findBestDrop(int ii,int jj);

        Bot(){}
        Bot(Game* _game){
            game = _game;
            testBoard = new int*[20];
            for(int i = 0; i < 20; i++){
                testBoard[i] = new int[game->lengthX];
                for(int j = 0; j < game->lengthX; j++)
                    testBoard[i][j] = game->board[i+20][j];
            }
        }

        ~Bot(){
            for(int i = 0; i < 20; i++)
                delete[] testBoard[i];
            delete[] testBoard;
        }
    };

}
