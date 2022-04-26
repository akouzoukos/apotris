#pragma once

#include <cstring>
#include <iostream>
#include <list>
#include <string>

namespace Tetris
{

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

        Drop(){}
        Drop(const Drop& oldDrop){
            on = oldDrop.on;
            startX = oldDrop.startX;
            endX = oldDrop.endX;
            startY = oldDrop.startY;
            endY = oldDrop.endY;
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

        SoundFlags() {}

        SoundFlags(const SoundFlags& oldFlags){
            shift = oldFlags.shift;
            rotate = oldFlags.rotate;
            place = oldFlags.place;
            invalid = oldFlags.invalid;
            hold = oldFlags.hold;
            clear = oldFlags.clear;
            levelUp = oldFlags.levelUp;
        }
    };

    class Pawn{
    public:
        int x;
        int y;
        int type;
        int current;
        int rotation = 0;
        int board[4][4][4];
        int lowest;
        void setBlock();

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

    class Game {
    private:
        int checkRotation(int, int, int);
        void fillBag();
        void fillQueue(int);
        void moveLeft();
        void moveRight();
        void moveDown();
        int clear(Drop);
        void lockCheck();
        void next();
        void place();
        void generateGarbage(int);
        Drop calculateDrop();

        std::list<int> bag;
        int canHold = 1;
        
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
        int lockMoveCounter = 0;

        int left = 0;
        int right = 0;
        int down = 0;

        int lastMoveRotation = 0;
        int finesseCounter = 0;
        bool dropping = false;
        Drop lastDrop;

        bool dropProtection = true;

        int dropLockTimer = 0;
        int dropLockMax = 8;

        bool specialTspin = false;

    public:
        int lengthX = 10;
        int lengthY = 40;
        int** board;
        std::list<int> queue;
        Pawn pawn = Pawn(0, 0);
        int held = -1;
        int linesCleared = 0;
        int level = 1;
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
        int finesse = 0;
        int garbageCleared = 0;
        int garbageHeight = 0;
        int pushDir = 0;
        int b2bCounter = 0;
        int bagCounter = 0;

        void rotateCW();
        void rotateCCW();
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
        void setTuning(int,int,int,bool);

        Game(){}
        Game(int gm) {
            gameMode = gm;
            board = new int* [lengthY];

            for (int i = 0; i < lengthY; i++) {
                board[i] = new int[lengthX];
                for (int j = 0; j < lengthX; j++)
                    board[i][j] = 0;
            }
            
            fillBag();
            fillQueue(5);
            linesToClear = std::list<int>();

            pawn = Pawn((int)lengthX / 2 - 2, 0);
            next();

            if(gameMode == 1)
                goal = 40;
            else if(gameMode == 2)
                goal = 150;
            else if(gameMode == 3){
                goal = 100;
                generateGarbage(9);
            }
            
        }

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
            finesse = oldGame.finesse;
            garbageCleared = oldGame.garbageCleared;
            garbageHeight = oldGame.garbageHeight;
            pushDir = oldGame.pushDir;
            b2bCounter = oldGame.b2bCounter;
            bagCounter = oldGame.bagCounter;

        }

        ~Game(){
            for(int i = 0; i < lengthY; i++)
                delete[] board[i];
            delete[] board;
            
        }
    };
}