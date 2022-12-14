#include "def.h"
#include "tetrisEngine.h"
#include <string>
#include <tuple>
#include "logging.h"
#include "posprintf.h"
#include "tonc_bios.h"
#include "tonc_core.h"
#include "tonc_input.h"
#include "tonc_memdef.h"

int botThinkingSpeed = 10;
int botSleepDuration = 1;

using namespace Tetris;

void showTestBoard();

INLINE bool bitGet(u16 *bitboard,int x, int y){
    return (bitboard[y] >> x) & 0b1;
}

const u8 bitcount[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

int countBits(u16 n) {
    int result = 0;

    for(int i = 0; i < 2; i++){
        result += bitcount[(n >> (8*i) & 0xff)];
    }

    return result;
}

int countTransitions(u16 a){
    return countBits((a >> 1) ^ a);
}

Values evaluate(u16 *board, int *columnHeights, int lengthX, int lengthY, int startY){
    int start = max(1,startY-1);

    int rowTrans = 0;
    int colTrans = 0;
    int holes = 0;
    int wells = 0;
    int quadWell = 0;

    for(int i = start; i < lengthY; i++){
        if(board[i])
            rowTrans += countTransitions(board[i]);
    }

    for(int j = 0; j < lengthX; j++){
        bool prev = false;
        for(int i = lengthY - columnHeights[j]; i < lengthY; i++){
            bool n = bitGet(board, j, i);

            if(n != prev){
                colTrans++;

                if(!n)
                    holes++;
            }

            prev = n;
        }
    }

    int lowestX = 0;
    int lowestCount = 255;

    // profile_start();

    for(int x = 0; x < lengthX; x++){
        int count = 0;
        int n = columnHeights[x];

        if(n < lowestCount){
            lowestCount = n;
            lowestX = x;
        }

        while(((x == 0 || n < columnHeights[x-1]) && (x == lengthX-1 || n < columnHeights[x+1]))){
            n++;
            count += count + 1;
        }

        wells += count;
    }

    // log("calc: " + std::to_string(profile_stop()));


    int i = lengthY - columnHeights[lowestX] - 1;
    while(countBits(board[i]) == 9 && i >= 0){
        i--;
        quadWell++;
    }

    Values result;
    result.rowTrans = rowTrans;
    result.colTrans = colTrans;
    result.holes = holes;
    result.wells = wells;
    result.quadWell = quadWell;

    if(key_is_down(KEY_SELECT)){
        for(int i = lengthY-20; i < lengthY; i++){
            std::string str;

            for(int j = 0; j < 16; j++)
                str += std::to_string((board[i] >> j) & 0b1);

            log(str);
        }

        std::string str;
        int *r = (int*) &result;
        for(int i = 0; i < 5; i++)
            str += std::to_string(r[i]) + " ";
        log(str);
        log(std::to_string(start) + " " + std::to_string(lengthY));
        for(int i = start; i < lengthY; i++){
            if(board[i])
                log(std::to_string(countTransitions(board[i])));
        }

        while(1){
            VBlankIntrWait();
            key_poll();

            if(key_hit(KEY_A)){
                break;
            }
        }
    }

    return result;
}

int countClears(u16 *board, int lengthX, int lengthY , int startY){//row
    int clears = 0;
    if(startY < 0)
        startY = 0;
    const int len = min(startY+4,lengthY);
    for(int i = startY; i < len; i++){
        if(board[i] == 0x3ff)
            clears++;
    }

    return clears;
}

void Bot::run(){
    if(--sleepTimer > 0)
        return;
    if(thinking){
        if(game->clearLock || game->dropping || game->pawn.current == -1)
            return;

        if(thinkingI == -6){
            // profile_start();
            currentValues = evaluate(game->bitboard,game->columnHeights,game->lengthX,game->lengthY,game->stackHeight);
            // log("first: " + std::to_string(profile_stop())); //3-8k cycle;

            // char buff[30];
            // posprintf(buff,"%d", currentValues.quadWell);

            // log(buff);

            delete quickSave;
            quickSave = new Game(*game);
        }

        if(thinkingI < 6){
            for(int i = 0; i < botThinkingSpeed; i++){
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

            if(thinkingI >= 6){
                thinkingI = -6;
                thinkingJ = 0;
                // thinking = false;
                sleepTimer = botSleepDuration;

                if(checking == 1){
                    thinking = false;
                    checking = 0;
                    // game->pawn.current = previous;
                    // game->pawn.setBlock(SRS);
                    delete game;
                    game = new Game(*quickSave);
                    game->setTuning(getTuning());
                } else{
                    checking++;

                    previous = game->pawn.current;

                    if(game->held != -1)
                        game->pawn.current = game->held;
                    else
                        game->pawn.current = game->queue.front();

                    // std::cout << "previous: " << std::to_string(previous) << " current: " << std::to_string(game->pawn.current) << "\n";

                    game->pawn.setBlock(SRS);
                    return;
                }
                current = best;
                best = Move();

                // log("x: " + std::to_string(current.dx + game->pawn.x) + " y: " + std::to_string(current.rotation));
            }
        }
    }

    if(!thinking)
        move();
}

void Bot::move(){
    if(game->dropping)
        return;
    if(--sleepTimer > 0)
        return;
    else
        sleepTimer = maxSleep;

    if(game->pawn.current != current.piece){
        log("current: " + std::to_string(game->pawn.current) + " best: " + std::to_string(current.piece));
        game->hold(1);
        game->hold(0);
        return;
    }

    if(game->pawn.rotation != current.rotation){
        // if(current.rotation == 3){
        //     game->rotateCCW(1);
        //     game->rotateCCW(0);
        //     // rotation = 3;
        // }else{
            while(game->pawn.rotation != current.rotation){
                game->rotateCW(1);
                game->rotateCW(0);
                // rotation++;
            }
        // }
    }

    if(dx > current.dx){
        while(dx != current.dx){
            game->keyLeft(1);
            game->keyLeft(0);
            dx--;
        }
    }else if(dx < current.dx){
        while(dx != current.dx){
            game->keyRight(1);
            game->keyRight(0);
            dx++;
        }
    }

    if(dx == current.dx && game->pawn.rotation == current.rotation){
        // log("dx: " + std::to_string(game->pawn.x) + " dy: " + std::to_string(game->pawn.rotation));
        game->keyDrop(1);
        dx = 0;
        rotation = 0;
        thinking = true;
        sleepTimer = maxSleep;

        return;
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

    const int height = 32;

    memcpy16(testBoard,&game->bitboard[8],height);
    memcpy32(columnHeights,game->columnHeights,10);

    // for(int i = 0; i < game->lengthY; i++)
    //     testBoard[i] = game->bitboard[i];
        // memcpy32(testBoard[i],game->board[i+20],10);


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

            if(y > game->lengthY-1 || y < 0 || x > game->lengthX-1)
                continue;

            // testBoard[y-20][x] = 1;
            testBoard[y-8] |= 1 << x;
            if(height-(y-8) > columnHeights[x])
                columnHeights[x] = height-(y-8);

            if(++count == 4)
                break;
        }
        if(count == 4)
            break;
    }

    // // showTestBoard();

    int clears = countClears(testBoard,game->lengthX,height,lowest+minH-8);
    int afterHeight = min(game->stackHeight,lowest+minH) + clears;

    Values v = evaluate(testBoard, columnHeights, game->lengthX,height,afterHeight-8);
    // int afterJag = v.jag;
    int afterColTrans = v.colTrans;
    int afterRowTrans = v.rowTrans;
    int afterHoles = v.holes;
    int afterWells = v.wells;
    int afterQuad = v.quadWell;
    // int holeDiff = afterHoles - currentValues.holes;
    int wellDiff = afterWells - currentValues.wells;
    // int rowDiff = afterRowTrans - currentValues.rowTrans;
    // int colDiff = afterColTrans - currentValues.colTrans;
    // int quadDiff = afterQuad - currentValues.quadWell;

    int holeDiff = afterHoles;
    // int wellDiff = afterWells;
    int rowDiff = afterRowTrans;
    int colDiff = afterColTrans;
    int quadDiff = afterQuad;

    int score = (clears * clears) * weights.clears + holeDiff * (weights.holes) + (game->lengthY-lowest) * weights.lowest +
        wellDiff * (weights.well) + rowDiff * (weights.row) + colDiff * (weights.col) + quadDiff * (weights.quadWell);

    // log( "score " + std::to_string(score) + " best score: " + std::to_string(best.score));

    if(score > best.score){
        best.score = score;
        best.dx = ii;
        best.rotation = jj;

        // log("qd: " + std::to_string(quadDiff));

        // log("setting to: " + std::to_string(game->pawn.current));
        best.piece = game->pawn.current;
    }

    return 1;
}

void showTestBoard(){
    // OBJ_ATTR* enemyBoardSprite = &obj_buffer[25];
    // obj_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    // obj_set_attr(enemyBoardSprite, ATTR0_TALL | ATTR0_AFF_DBL, ATTR1_SIZE(2) | ATTR1_AFF_ID(6), 0);
    // enemyBoardSprite->attr2 = ATTR2_BUILD(768, 0, 1);
    // obj_set_pos(enemyBoardSprite, 43, 24);
    // obj_aff_identity(&obj_aff_buffer[6]);
    // obj_aff_scale(&obj_aff_buffer[6], float2fx(0.5), float2fx(0.5));

    // TILE* dest2;

    // for(int height = 10; height < 20; height++){
    //     for (int j = 0; j < 10; j++) {
    //         dest2 = (TILE*)&tile_mem[5][256 + ((height) / 8) * 2 + (j) / 8];

    //         if (testBot->testBoard[height][j])
    //             dest2->data[(height) % 8] |= (4 + 2 * savefile->settings.lightMode) << ((j % 8) * 4);
    //         else
    //             dest2->data[(height) % 8] &= ~(0xffff << ((j % 8) * 4));
    //     }
    // }
}
