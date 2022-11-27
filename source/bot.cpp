#include "tetrisEngine.h"
#include <string>
#include "logging.h"
#include "posprintf.h"

using namespace Tetris;

void showTestBoard();

int countRightSide(int **board,int lengthX, int lengthY, int startY){
    int sum = 0;

    for(int i = std::max(startY-1,0); i < lengthY; i++){
        if(board[i][lengthX-1])
            sum++;
    }

    return sum;
}

int countHoles(int **board,int lengthX, int lengthY, int startY){
    int holes = 0;
    for(int i = std::max(startY-1,0); i < lengthY-1; i++){
        for(int j = 0; j < lengthX; j++){
            if(board[i][j] != 0 && board[i+1][j] == 0)
                holes++;
        }
    }

    return holes;
}

int countClears(int **board,int lengthX, int lengthY , int startY){
    int clears = 0;
    if(startY < 0)
        startY = 0;
    const int len = std::min(startY+4,lengthY);
    for(int i = startY; i < len; i++){
        bool found = false;
        for(int j = 0; j < lengthX; j++){
            if(board[i][j] == 0){
                found = true;
                break;
            }
        }

        if(!found)
            clears++;
    }

    return clears;
}

int countJag(int **board,int lengthX, int lengthY, int startY){
    int sum = 0;
    int prev = 0;

    if(startY < 0)
        startY = 0;

    for(int j = 0; j < lengthX; j++){
        for(int i = startY; i < lengthY; i++){
            if(board[i][j] != 0 || i == lengthY-1){
                if(j != 0){
                    sum+= abs(prev-i);
                }
                prev = i;
                break;
            }
        }
    }

    return sum;
}

void Bot::run(){
    if(thinking){
        if(game->clearLock || game->dropping)
            return;

        if(thinkingI == -6){
            currentHoles = countHoles(game->board,game->lengthX,game->lengthY,game->stackHeight);
            currentJag = countJag(game->board,game->lengthX,game->lengthY,game->stackHeight);
            // currentRight = countRightSide(game->board,game->lengthX,game->lengthY,game->stackHeight);

            // log(std::to_string(currentHoles));
        }

        if(thinkingI < 6){
            for(int i = 0; i < 4; i++){
                if(thinkingI >= 6)
                    break;
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
            // if(checking == 1){
            //     thinking = false;
            //     checking = 0;
            //     game->pawn.current = previous;
            //     game->pawn.setBlock(SRS);
            // } else{
            //     checking++;

            //     previous = game->pawn.current;

            //     if(game->held != -1)
            //         game->pawn.current = game->held;
            //     else
            //         game->pawn.current = game->queue.front();

            //     // std::cout << "previous: " << std::to_string(previous) << " current: " << std::to_string(game->pawn.current) << "\n";

            //     game->pawn.setBlock(SRS);
            //     return;
            // }
            current = best;
            best = Move();
        }
    }else{
        move();
    }
}

void Bot::move(){
    if(game->dropping)
        return;
    if(--sleepTimer > 0)
        return;
    else
        sleepTimer = maxSleep;

    if(game->pawn.current != current.piece){
        // std::cout << "current: " << std::to_string(game->pawn.current) << " best: " << std::to_string(current.piece) << "\n";
        game->hold(1);
        game->hold(0);
        return;
    }

    if(dx == current.dx && rotation == current.rotation){
        game->keyDrop(1);
        dx = 0;
        rotation = 0;
        thinking = true;
        sleepTimer = maxSleep;
        return;
    }
    
    if(rotation != current.rotation){
        if(current.rotation == 3){
            game->rotateCCW(1);
            game->rotateCCW(0);
            rotation = 3;
        }else{
            game->rotateCW(1);
            game->rotateCW(0);
            rotation++;
        }
    }else if(dx > current.dx){
        game->keyLeft(1);
        game->keyLeft(0);
        dx--;
    }else if(dx < current.dx){
        game->keyRight(1);
        game->keyRight(0);
        dx++;
    }
}

int Bot::findBestDrop(int ii,int jj){
    if(!game->checkRotation(ii,0,jj))
        return 0;

    int prevR = game->pawn.rotation;
    int prevX = game->pawn.x;
    game->pawn.rotation = jj;
    game->pawn.x += ii;
    int lowest = game->lowest();

    game->pawn.rotation = prevR;
    game->pawn.x = prevX;

    for(int i = 0; i < 20; i++)
        memcpy32(testBoard[i],game->board[i+20],10);

    int minH = -1;
    int count = 0;
    for(int i = 0; i < 4; i++){
        int y = lowest+i;
        for(int j = 0; j < 4; j++){
            if(game->pawn.board[jj][i][j] == 0)
                continue;

            if(minH == -1)
                minH = i;

            int x = game->pawn.x+ii+j;

            if(y > game->lengthY-1 || y-20 < 0 || x > game->lengthX-1)
                continue;

            testBoard[y-20][x] = 1;

            if(++count == 4)
                break;
        }
        if(count == 4)
            break;
    }

    // showTestBoard();

    int clears = countClears(testBoard,game->lengthX,20,game->pawn.y+minH-20);
    int afterHeight = min(game->stackHeight,lowest+minH) + clears;
    int afterHoles = countHoles(testBoard,game->lengthX,20,afterHeight-20);
    int afterJag = countJag(testBoard,game->lengthX,20,afterHeight-20);
    // int afterRight = countRightSide(testBoard,game->lengthX,20,afterHeight-20);

    int holeDiff = afterHoles-currentHoles;
    // int heightDiff = game->stackHeight-afterHeight;
    int jagDiff = afterJag - currentJag;
    // int rightDiff = afterRight - currentRight;

    int score = (clears * clears) * weights.clears + holeDiff * -(weights.holes) + (afterHeight-30) * (weights.height) + lowest * weights.lowest + jagDiff * - (weights.jag);

    if(score > best.score){
        // log(std::to_string(holeDiff));
        // log(std::to_string(afterHoles));
        best.score = score;
        best.dx = ii;
        best.rotation = jj;
        best.piece = game->pawn.current;
        // best.holes = holeDiff;
        // best.height = heightDiff;
    }

    return 1;
}

void showTestBoard(){
    OBJ_ATTR* enemyBoardSprite = &obj_buffer[25];
    obj_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    obj_set_attr(enemyBoardSprite, ATTR0_TALL | ATTR0_AFF_DBL, ATTR1_SIZE(2) | ATTR1_AFF_ID(6), 0);
    enemyBoardSprite->attr2 = ATTR2_BUILD(768, 0, 1);
    obj_set_pos(enemyBoardSprite, 43, 24);
    obj_aff_identity(&obj_aff_buffer[6]);
    obj_aff_scale(&obj_aff_buffer[6], float2fx(0.5), float2fx(0.5));

    TILE* dest2;

    for(int height = 10; height < 20; height++){
        for (int j = 0; j < 10; j++) {
            dest2 = (TILE*)&tile_mem[5][256 + ((height) / 8) * 2 + (j) / 8];

            if (testBot->testBoard[height][j])
                dest2->data[(height) % 8] |= (4 + 2 * savefile->settings.lightMode) << ((j % 8) * 4);
            else
                dest2->data[(height) % 8] &= ~(0xffff << ((j % 8) * 4));
        }
    }
}
