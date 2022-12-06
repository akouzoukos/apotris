#include "def.h"
#include "tetrisEngine.h"
#include <string>
#include <tuple>
#include "logging.h"
#include "posprintf.h"

int botThinkingSpeed = 3;
int botSleepDuration = 1;

using namespace Tetris;

void showTestBoard();

typedef struct Values{
    int rowTrans;
    int colTrans;
    int holes;
    int wells;
    int jag;
}Values;

INLINE bool bitGet(u16 *bitboard,int x, int y){
    return (bitboard[y] >> x) & 0b1;
}

Values evaluate(u16 *board,int lengthX, int lengthY, int startY){
    int start = max(1,startY-1);

    int rowTrans = 0;
    int wells = 0;
    int holes = 0;
    // int jagSum = 0;
    int colTrans = 0;

    for(int i = start; i < lengthY+1; i++){
        bool prevTrans = true;
        for(int j = 0; j < lengthX+1; j++){
            if(i == lengthY){
                colTrans += !bitGet(board, j, i-1);
                break;
            }

            if(j == lengthX){
                rowTrans += !prevTrans;
                break;
            }
            bool cur = (bitGet(board, j, i));
            colTrans += (cur != (bitGet(board, j, i-1)));


            rowTrans += (cur != prevTrans);
            prevTrans = cur;

            if(i < lengthY-1 && (bitGet(board, j, i)) && !(bitGet(board, j, i+1))){
                holes++;
            }
        }
    }

    for(int x = 0; x < game->lengthX; x++){
        int count = 0;
        int n = game->columnHeights[x];
        while(((x == 0 || n < game->columnHeights[x-1]) && (x == game->lengthX-1 || n < game->columnHeights[x+1]))){
            n++;
            count += count + 1;
        }

        wells += count;
    }

    Values result;
    result.colTrans = colTrans;
    result.rowTrans = rowTrans;
    result.holes = holes;
    result.wells = wells;
    // result.jag = jagSum;

    return result;
}

int countClears(u16 *board,int lengthX, int lengthY , int startY){//row
    int clears = 0;
    if(startY < 0)
        startY = 0;
    const int len = min(startY+4,lengthY);
    for(int i = startY; i < len; i++){
        bool found = false;
        for(int j = 0; j < lengthX; j++){
            if(bitGet(board, j, i)){
                found = true;
                break;
            }
        }

        if(!found)
            clears++;
    }

    return clears;
}

// std::tuple<int,int> countColValues(int **board,int lengthX, int lengthY, int startY){

//     return std::make_tuple(jagSum,transitions);
// }

void Bot::run(){
    if(thinking){
        if(game->clearLock || game->dropping || game->pawn.current == -1)
            return;

        if(thinkingI == -6){
            profile_start();
            Values v = evaluate(game->bitboard,game->lengthX,game->lengthY,game->stackHeight);
            log("first: " + std::to_string(profile_stop())); //3-8k cycle;
            currentJag = v.jag;
            currentColTrans = v.colTrans;
            currentRowTrans = v.rowTrans;
            currentHoles = v.holes;
            currentWells = v.wells;

            // char buff[30];
            // posprintf(buff,"%d %d %d %d", currentColTrans, currentRowTrans, currentHoles, currentWells);

            // log(buff);
            // log(std::to_string(currentWells));

            // std::tie(currentJag,currentColTrans) = countColValues(game->board,game->lengthX,game->lengthY,game->stackHeight);
            // std::tie(currentRowTrans,currentWells,currentHoles) = countRowValues(game->board,game->lengthX,game->lengthY,game->stackHeight);

            // currentRight = countRightSide(game->board,game->lengthX,game->lengthY,game->stackHeight);

            // log(std::to_string(currentHoles));

            // delete quickSave;
            // quickSave = new Game(*game);
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
        }else{
            thinkingI = -6;
            thinkingJ = 0;
            // thinking = false;
            // delete game;
            // game = new Game(*quickSave);
            // game->setTuning(getTuning());

            if(checking == 1){
                thinking = false;
                checking = 0;
                game->pawn.current = previous;
                game->pawn.setBlock(SRS);
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
        // log("current: " + std::to_string(game->pawn.current) + " best: " + std::to_string(current.piece));
        game->hold(1);
        game->hold(0);
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

    if(dx == current.dx && rotation == current.rotation){
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

    memcpy16(testBoard,&game->bitboard[8],32);
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

            if(++count == 4)
                break;
        }
        if(count == 4)
            break;
    }

    // // showTestBoard();

    int clears = countClears(testBoard,game->lengthX,32,lowest+minH-8);
    int afterHeight = min(game->stackHeight,lowest+minH) + clears;

    Values v = evaluate(testBoard,game->lengthX,32,afterHeight-8);
    // int afterJag = v.jag;
    int afterColTrans = v.colTrans;
    int afterRowTrans = v.rowTrans;
    int afterHoles = v.holes;
    int afterWells = v.wells;

    int holeDiff = afterHoles-currentHoles;
    // int jagDiff = afterJag - currentJag;
    int wellDiff = afterWells - currentWells;
    int rowDiff = afterRowTrans - currentRowTrans;
    int colDiff = afterColTrans - currentColTrans;

    int score = (clears * clears) * weights.clears + holeDiff * (weights.holes) + (game->lengthY-lowest) * weights.lowest +
        wellDiff * (weights.well) + rowDiff * (weights.row) + colDiff * (weights.col);

    // log( "score " + std::to_string(score) + " best score: " + std::to_string(best.score));

    if(score > best.score){
        best.score = score;
        best.dx = ii;
        best.rotation = jj;

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
