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
        int canHold = 1;
        
        float speed;
        float speedCounter = 0;

        int seed = 0;


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
        int linesSent = 0;
        std::list<Garbage> attackQueue;
        std::list<Garbage> garbageQueue;
        std::list<int> moveHistory;
        std::list<int> previousBest;
        int previousKey = 0;
        bool softDrop = false;

        int checkRotation(int, int, int);
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
        void clearAttack(int);
        void setWin();
        void addToGarbageQueue(int,int);
        std::list<int> getBestFinesse(int,int,int);
        int getIncomingGarbage();
        Move findBestDrop();

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
                generateGarbage(9,0);
            }
            
        }

        Game(int gm, int sd){
            gameMode = gm;
            seed = sd;
            board = new int* [lengthY];

            for (int i = 0; i < lengthY; i++) {
                board[i] = new int[lengthX];
                for (int j = 0; j < lengthX; j++)
                    board[i][j] = 0;
            }
            
            fillBag();
            fillQueueSeed(5,seed);
            linesToClear = std::list<int>();

            pawn = Pawn((int)lengthX / 2 - 2, 0);
            next();

            if(gameMode == 1)
                goal = 40;
            else if(gameMode == 2)
                goal = 150;
            else if(gameMode == 3){
                goal = 100;
                generateGarbage(9,0);
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

        void run(){
            if(thinking){
                if(game->clearLock)
                    return;

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

            int currentHoles = countHoles(game->board,game->lengthX,game->lengthY);
            int currentHeight = getHeight(game->board,game->lengthX,game->lengthY);

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
            
            for(int i = 0; i < game->lengthY; i++)
                for(int j = 0; j < game->lengthX; j++)
                    testBoard[i][j] = game->board[i][j];

            for(int i = 0; i < 4; i++){
                for(int j = 0; j < 4; j++){
                    if(game->pawn.board[jj][i][j] == 0)
                    continue;
                    int x = game->pawn.x+ii+j;
                    int y = lowest+i;

                    if(y > game->lengthY-1 || x > game->lengthX)
                        continue;

                    testBoard[lowest+i][game->pawn.x+ii+j] = 1;
                }
            }

            int afterHoles = countHoles(testBoard,game->lengthX,game->lengthY);
            int clears = countClears(testBoard,game->lengthX,game->lengthY);
            int afterHeight = getHeight(testBoard,game->lengthX,game->lengthY);
            
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
            testBoard = new int*[game->lengthY];
            for(int i = 0; i < game->lengthY; i++){
                testBoard[i] = new int[game->lengthX];
                for(int j = 0; j < game->lengthX; j++)
                    testBoard[i][j] = game->board[i][j];
            }
        }

        ~Bot(){
            for(int i = 0; i < game->lengthY; i++)
                delete[] testBoard[i];
            delete[] testBoard;
        }
    };
}