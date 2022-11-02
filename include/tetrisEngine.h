#pragma once

#include <cstring>
#include <list>
#include <string>
#include "tonc.h"

namespace Tetris
{
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
    };

    class Stats{
    public:
        int clears[4] = {0,0,0,0};
        int tspins = 0;
        int perfectClears = 0;
        int maxStreak = 0;
        int maxCombo = 0;
        int holds = 0;


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

        Move(){}

        Move(const Move& oldMove){
            dx = oldMove.dx;
            rotation = oldMove.rotation;
            height = oldMove.height;
            holes = oldMove.holes;
            piece = oldMove.piece;
            clears = oldMove.clears;
        }
    };

    class Garbage {
    public:
        int id;
        int amount;
        int timer = 10;

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
        int lowest;
        bool big = false;
        void setBlock(bool alt);

        Pawn(int newX, int newY) {
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
    };

    class Game {
    private:
        void fillBag();
        void fillQueue(int);
        void fillQueueSeed(int,int);
        void moveLeft();
        void moveRight();
        void moveDown();
        int clear(Drop);
        void lockCheck();
        void next();
        void place();
        void generateGarbage(int,int);
        Drop calculateDrop();

        std::list<int> bag;

        float speed;
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

        int maxLockTimer = 30;
        int lockTimer = maxLockTimer;
        int lockMoveCounter = 15;

        int left = 0;
        int right = 0;
        int down = 0;

        int lastMoveRotation = 0;
        int lastMoveDx = 0;
        int lastMoveDy = 0;

        int finesseCounter = 0;
        bool dropping = false;

        Drop lastDrop;

        bool dropProtection = true;
        bool directionCancel = true;

        int dropLockTimer = 0;
        int dropLockMax = 8;

        bool specialTspin = false;

        int pieceHistory = -1;

        int gracePeriod = 0;

        int initialLevel = 0;

        bool initialHold = false;

        int initialRotate = 0;

        bool delaySoftDrop = true;

    public:
        int lengthX = 10;
        int lengthY = 40;
        int** board;
        std::list<int> queue;
        Pawn pawn = Pawn(0, 0);
        int held = -1;
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
        int holdCounter = 0;
        bool trainingMode = false;
        int seed = 0;
        int initSeed = 0;

        int entryDelay = 0;
        int bTypeHeight = 0;

        Stats statTracker;

        int subMode = 0;

        int checkRotation(int, int, int);
        void rotateCW();
        void rotateCCW();
        void rotateTwice();
        void hardDrop();
        void update();
        int lowest();
        Color color(int);
        void hold();
        int** getShape(int,int);
        void keyLeft(int);
        void keyRight(int);
        void keyDown(int);
        void keyDrop();
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
        }


        ~Game(){
            for(int i = 0; i < lengthY; i++)
                delete[] board[i];
            delete[] board;

        }
    };

    class Bot{
    public:
        bool thinking = true;
        int thinkingI = -6;
        int thinkingJ = 0;
        Move current;
        Move best;
        int maxSleep = 5;
        int sleepTimer = 0;

        int dx = 0;
        int rotation = 0;
        Game* game;
        int **testBoard;

        int currentHoles = 0;
        int currentHeight = 0;

        void run(){
            if(thinking){
                if(game->clearLock)
                    return;

                if(thinkingI == -6){
                    currentHoles = countHoles(game->board,game->lengthX,game->lengthY);
                    currentHeight = getHeight(game->board,game->lengthX,game->lengthY);
                }

                if(thinkingI < 6){
                    for(int i = 0; i < 1; i++){
                        if(thinkingJ < 4){
                            while(!findBestDrop(thinkingI,thinkingJ)){
                                if(thinkingJ < 3){
                                    thinkingJ++;
                                }else{
                                    thinkingJ = 0;
                                    if(thinkingI < 6)
                                        thinkingI++;
                                    else
                                        break;
                                }
                            }
                            thinkingJ++;
                        }else{
                            thinkingJ = 0;
                            thinkingI++;
                        }
                    }
                }else{
                    thinkingI = -6;
                    thinkingJ = 0;
                    thinking = false;
                    current = best;
                    best = Move();
                }
            }else{
                move();
            }
        }

        void move(){
            if(--sleepTimer > 0)
                return;
            else
                sleepTimer = maxSleep;

            if(rotation != current.rotation){
                game->rotateCW();
                rotation++;
            }else if(dx > current.dx){
                game->keyLeft(1);
                game->keyLeft(0);
                dx--;
            }else if(dx < current.dx){
                game->keyRight(1);
                game->keyRight(0);
                dx++;
            }else{
                game->keyDrop();
                dx = 0;
                rotation = 0;
                thinking = true;
                sleepTimer = maxSleep;
            }
        }

        int getHeight(int **board, int lengthX, int lengthY){
            int max = 40;
            for(int j = 0; j < lengthX; j++){
                for(int i = 0; i < lengthY; i++){
                    if(board[i][j]){
                        if(i < max)
                            max = i;
                        break;
                    }
                }
            }

            return 40-max;
        }

        int countHoles(int **board,int lengthX, int lengthY){
            int holes = 0;
            for(int i = 0; i < lengthY-1; i++){
                for(int j = 0; j < lengthX; j++){
                    if(board[i][j] != 0 && board[i+1][j] == 0)
                        holes++;
                }
            }

            return holes;
        }

        int countClears(int **board,int lengthX, int lengthY){
            int clears = 0;
            for(int i = 0; i < lengthY; i++){
                bool found = false;
                for(int j = 0; j < lengthX; j++){
                    if(board[i][j] == 0)
                        found = true;
                }

                if(!found)
                    clears++;
            }

            return clears;
        }

        int countWells(int **board,int lengthX, int lengthY){
            int sum = 0;
            for(int j = lengthX-1; j >= 0; j--){
                int counter = 0;
                for(int i = 0; i < lengthY; i++){
                    if(j == 0 || board[i][j] || ((j != 0 && !board[i][j-1]) && (j != lengthX-1 && !board[i][j+1]))){
                        if(counter > 2)
                            sum++;
                        counter = 0;
                    }else
                        counter++;
                }
            }

            return sum;   
        }

        int findBestDrop(int ii,int jj){
            if(!game->checkRotation(ii,0,jj))
                return 0;
            
            int blocks[4];

            for(int i = 0; i < 4; i++)//initialize height array
                blocks[i] = -1;

            for(int i = 3; i >= 0; i--)//find height of lowest block in column
                for(int j = 0; j < 4; j++)
                    if(game->pawn.board[jj][i][j] && blocks[j] == -1)
                        blocks[j] = i;

            int lowest = game->pawn.y;
            bool escape = false;

            for(int i = 0; i < game->lengthY - game->pawn.y; i++){
                for(int j = 0; j < 4; j++){
                    if(blocks[j] == -1)
                        continue;
                    int x = game->pawn.x+j+ii;
                    int y = game->pawn.y+i;
                    if(y+blocks[j] >= game->lengthY || game->board[y+blocks[j]][x]){
                        lowest = y-1;
                        escape = true;
                        break;
                    }
                }
                if(escape)
                    break;
            }
            
            for(int i = 0; i < 20; i++)
                for(int j = 0; j < game->lengthX; j++)
                    testBoard[i][j] = game->board[i+20][j];

            for(int i = 0; i < 4; i++){
                for(int j = 0; j < 4; j++){
                    if(game->pawn.board[jj][i][j] == 0)
                        continue;
                    int x = game->pawn.x+ii+j;
                    int y = lowest+i;

                    if(y > game->lengthY-1 || x > game->lengthX)
                        continue;

                    testBoard[lowest+i-20][x] = 1;
                }
            }

            int afterHoles = countHoles(testBoard,game->lengthX,20);
            int clears = countClears(testBoard,game->lengthX,20);
            int afterHeight = getHeight(testBoard,game->lengthX,20);
            
            int holeDiff = afterHoles-currentHoles;
            int heightDiff = afterHeight-currentHeight;

            if((clears > best.clears) || ((holeDiff < best.holes) || (heightDiff < best.height && holeDiff == best.holes) || (heightDiff-best.height < -1 && holeDiff <= best.holes+1))){
                best.height = heightDiff;
                best.dx = ii;
                best.rotation = jj;
                best.holes = holeDiff;
                best.piece = game->pawn.current;
                best.clears = clears;
            }

            return 1;
        }


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
