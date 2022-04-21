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

    class Score {
    public:
        int linesCleared = 0;
        int score = 0;
        int combo = 0;
        int isTSpin = 0;
        int isPerfectClear = 0;
        int isBackToBack = 0;
        int isDifficult = 0;

        Score(int lC, int sc, int co, int isTS, int isPC, int isBTB, int isDif) {
            linesCleared = lC;
            score = sc;
            combo = co;
            isTSpin = isTS;
            isPerfectClear = isPC;
            isBackToBack = isBTB;
            isDifficult = isDif;
        }
    };

    class Drop {
    public:
        int on = false;
        int startX = 0;
        int endX = 0;
        int startY = 0;
        int endY = 0;
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

        SoundFlags() {};
    };

    class Pawn{
    public:
        int x;
        int y;
        int type;
        int current;
        int rotation;
        int board[4][4][4];
        int lowest;
        void setBlock();

        Pawn(int newX, int newY) {
            x = newX;
            y = newY;

            rotation = 0;
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
        int clear();
        void lockCheck();
        void next();
        void place();
        void generateGarbage(int);

        std::list<int> bag;
        int canHold = 1;
        float speed;
        float speedCounter = 0;
        int maxDas = 8;
        int arr = 2;
        int arrCounter = 0;
        int softDropCounter = 0;
        int maxSoftDrop = 10;
        int softDropSpeed = 2;
        int das = 0;
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
        Score previousClear = Score(0, 0, 0, 0, 0, 0, 0);
        int timer = 0;
        int refresh = 0;
        int won = 0;
        int goal;
        int finesse = 0;
        int garbageCleared = 0;
        int garbageHeight = 0;
        int pushDir = 0;
        int b2bCounter = 0;

        void rotateCW();
        void rotateCCW();
        Drop hardDrop();
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
    };
}