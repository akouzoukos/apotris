
#include "tetromino.hpp"
#include "tetrisEngine.h"
#include <iostream>
#include <algorithm>
#include <tuple>
#include "logging.h"
#include "posprintf.h"
#include "tonc_core.h"

using namespace Tetris;
using namespace GameInfo;

int getNbr(int** board,int x, int y, int ex, int sy, int ey);

int Game::checkRotation(int dx, int dy, int r) {
    const int x = dx + pawn.x;
    const int y = dy + pawn.y;

    int total = 0;
    for (int j = 0; j < 4; j++) {
        for (int i = pawn.boardLowest[r][j]; i >= 0; i--) {
            if (pawn.board[r][i][j] == 0)
                continue;
            
            if(!pawn.big){
                if (x + j < 0 || x + j > lengthX - 1 || y + i < 0 || y + i > lengthY - 1 || board[i+y][j+x] != 0)
                    return 0;
            }else{
                for(int k = 0; k < 2; k++){
                    for(int l = 0; l < 2; l++){
                        int xoffset = (x+j) * 2 + l;
                        int yoffset = (y+i) * 2 + k;
                        if (xoffset < 0 || xoffset > lengthX - 1 || yoffset < 0 || yoffset > lengthY - 1 || board[yoffset][xoffset] != 0)
                            return 0;
                    }
                }
            }

            if(++total >= 4)
                return 1;
        }
    }

    return 1;
}

int Game::checkSpecialRotation(int dx, int r){
    int x = dx + pawn.x;

    int total = 0;
    for (int i = 0; i < 4; i++) {
        if(total >= 4)
            break;


        for (int j = 0; j < 4; j++) {
            if (pawn.board[r][i][j] == 0)
                continue;

            total++;

            if(!pawn.big){
                if (x + j < 0 || x + j > lengthX - 1 || pawn.y + i < 0 || pawn.y + i > lengthY - 1 || board[i+pawn.y][j+x] != 0)
                    return i * 4 + j + 1;
            }else{
                for(int k = 0; k < 2; k++){
                    for(int l = 0; l < 2; l++){
                        int xoffset = (x+j) * 2 + l;
                        int yoffset = (pawn.y+i) * 2 + k;
                        if (xoffset < 0 || xoffset > lengthX - 1 || yoffset < 0 || yoffset > lengthY - 1 || board[yoffset][xoffset] != 0)
                            return i * 4 + j + 1;
                    }
                }
            }

            if(total >= 4)
                break;
        }
    }

    return 0;
}

void Pawn::setBlock(int system) {
    memset32(boardLowest,-1,16);
    memset32(heighest,-1,4);

    for (int i = 0; i < 4; i++){
        int * shape = getShape(current,i,system);

        for(int iy = 0; iy < 4; iy++){
            for(int ix = 0; ix < 4; ix++){
                board[i][iy][ix] = shape[iy * 4 + ix];

                if(shape[iy * 4 + ix]){
                    if(boardLowest[i][ix] < iy)
                        boardLowest[i][ix] = iy;
                    if(heighest[i] == -1)
                        heighest[i] = iy;
                }
            }
        }

        delete[] shape;
    }
}

int getNbr(int **board, int x, int y, int ex, int sy , int ey){
    int count = 0;

    if(y - 1 >= sy && board[y-1][x])
        count++;
    if(y + 1 <= ey-1 && board[y+1][x])
        count++;
    if(x - 1 >= 0 && board[y][x-1])
        count++;
    if(x + 1 <= ex-1 && board[y][x+1])
        count++;

    if(y - 1 >= sy && board[y-1][x] && x - 1 >= 0 && board[y][x-1] && board[y-1][x-1])
        count++;
    if(y - 1 >= sy && board[y-1][x] && x + 1 <= ex && board[y][x+1] && board[y-1][x+1])
        count++;
    if(y + 1 <= ey && board[y+1][x] && x - 1 >= 0 && board[y][x-1] && board[y+1][x-1])
        count++;
    if(y + 1 <= ey && board[y+1][x] && x + 1 <= ex && board[y][x+1] && board[y+1][x+1])
        count++;

    return count;
}

void Game::rotateCW(int dir) {
    rotates[0] = dir;

    if(!dir)
        return;

    rotate(1);
}

void Game::rotateCCW(int dir) {
    rotates[1] = dir;

    if(!dir)
        return;

    rotate(-1);
}

void Game::rotateTwice(int dir) {
    rotates[2] = dir;

    if(!dir)
        return;

    rotate(2);
}

void Game::rotate(int dir){
    if ((clearLock || entryDelay)){
        return;
    }else if(eventLock){
        return;
    }

    if(rotationSystem == ARS && dir == 2)
        dir = -1;

    int dx = 0;
    int dy = 0;
    int r = pawn.rotation + dir;
    if(r > 3)
        r -= 4;
    else if(r < 0)
        r += 4;

    moveCounter++;
    lastMoveRotation = dir;

    if(rotationSystem == SRS){
        for (int i = 0; i < 5 + (dir > 1); i++) {
            int dx;
            int dy;
            if(dir <= 1){
                dx = kicks[(pawn.current == 0)][(dir < 0)][pawn.rotation][i][0];
                dy = kicks[(pawn.current == 0)][(dir < 0)][pawn.rotation][i][1];
            }else{
                dx = kickTwice[pawn.rotation][i][0];
                dy = kickTwice[pawn.rotation][i][1];
            }

            if (checkRotation(dx, dy, r)) {
                rotatePlace(dir,dx,dy,r);
                return;

            }else{
                sounds.invalid = 1;
            }
        }

    }else if(rotationSystem == NRS){
        if(checkRotation(0, 0, r)){
            rotatePlace(dir,dx,dy,r);
            return;
        }else
            sounds.invalid = 1;

    }else if(rotationSystem == ARS){

        if((pawn.current == 1 || pawn.current == 2 || pawn.current == 5) && (pawn.rotation % 2 == 0)){
            int n = checkSpecialRotation(0, r);
            if((n-1) % 4 == 1)
                return;
        }else if(pawn.current == 0){
            if(checkRotation(0, 0, r)){
                rotatePlace(dir,dx,dy,r);
                return;
            }else{
                int touching = checkITouching(r, (pawn.rotation % 2 == 0));
                if(pawn.rotation % 2 == 1){
                    if(touching == 1)
                        dx = 2;
                    else
                        dx = -1;
                }else{
                    if(touching)
                        dy = -2;
                }

                if(checkRotation(dx, dy, r)){
                    rotatePlace(dir,dx,dy,r);
                    return;
                }else
                    sounds.invalid = 1;
            }
            return;
        }

        for(int i = 0; i < 3; i++){
            dx = 0;
            dy = 0;

            if(i == 1 && pawn.current == 5 && r == 2){
                dy = -1;
            }else if(i == 1)
                dx = 1;
            else if(i == 2)
                dx = -1;

            if(checkRotation(dx, dy, r)){
                rotatePlace(dir,dx,dy,r);

                if((pawn.current == 0 || pawn.current == 5) && dy)
                    stopLockReset = true;

                return;
            }else
                sounds.invalid = 1;

            if(dy)
                break;
        }
    }
}

void Game::rotatePlace(int dir, int dx, int dy, int r){
    pawn.rotation = r;

    lockCheck();
    pawn.x += dx;
    pawn.y += dy;

    sounds.rotate = 1;

    switch(dir){
        case -1: moveHistory.push_front(5); break;
        case  1: moveHistory.push_front(4); break;
        case  2: moveHistory.push_front(6); break;
    }

    lastMoveDx = dx;
    lastMoveDy = dy;
}

bool Game::moveLeft() {
    if(das != maxDas)
        moveHistory.push_back(0);
    else if(!(moveHistory.size() > 0 && moveHistory.back() == 2)){
        if(moveHistory.size() > 0 && moveHistory.back() == 0)
            moveHistory.pop_back();

        moveHistory.push_back(2);
    }


    if (checkRotation(-1, 0, pawn.rotation)) {
        pawn.x--;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
        return true;
    }else{
        pushDir = -1;
        if(gameMode == CLASSIC)
            das = maxDas;
        return false;
    }
}

bool Game::moveRight() {
    if(das != maxDas)
        moveHistory.push_back(1);
    else if(!(moveHistory.size() > 0 && moveHistory.back() == 3)){
        if(moveHistory.size() > 0 && moveHistory.back() == 1)
            moveHistory.pop_back();

        moveHistory.push_back(3);
    }

    if (checkRotation(1, 0, pawn.rotation)) {
        pawn.x++;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
        return true;
    }else{
        pushDir = 1;
        if(gameMode == CLASSIC)
            das = maxDas;
        return false;
    }
}

void Game::moveDown() {
    if (checkRotation(0, 1, pawn.rotation)){
        pawn.y++;

        if(rotationSystem == ARS && !stopLockReset)
            lockTimer = maxLockTimer;

        if(gameMode != CLASSIC)
            sounds.shift = 1 ;
        lastMoveRotation = 0;
    }else if(rotationSystem == NRS || rotationSystem == ARS){
        place();
        down = 0;
    }
}

void Game::hardDrop() {
    if (clearLock || eventLock || dropLockTimer || entryDelay)
        return;

    int diff = pawn.lowest - pawn.y;

    if(diff)
        lastMoveRotation = 0;

    int offset = 0;
    for(int i = 0; i < 4; i++)
        if(pawn.boardLowest[pawn.rotation][i] != -1)
            offset = i;

    std::list<int> bestFinesse = getBestFinesse(pawn.current,pawn.x+offset,pawn.rotation);
    previousBest = bestFinesse;

    int difference = 0;
    bool correct = false;
    if(moveHistory.size() > bestFinesse.size()-1){
        for(const auto& move : bestFinesse){
            if(move == 6 && moveHistory.size() == bestFinesse.size()){
                correct = true;
                break;
            }
        }
        if(!correct){
            difference = moveHistory.size() - (bestFinesse.size() - 1);
        }
    }else{
        correct = true;
    }


    if(!correct && !softDrop){
        finesseFaults += difference;
        finesseStreak = 0;

        if(trainingMode){
            canHold = true;
            lockTimer = maxLockTimer;
            lockMoveCounter = 15;
            sounds.finesse = 1;
            refresh = 1;
            dropLockTimer = dropLockMax;
            softDrop = false;

            pushDir = 0;

            moveHistory.clear();
            previousKey = 0;

            pawn.y = (int)lengthY / 2;
            pawn.x = (int)lengthX / 2 - 2;
            pawn.rotation = 0;

            return;
        }
    }else{
        finesseStreak++;
    }

    pawn.y = pawn.lowest;

    // profile_start();
    if(rotationSystem != ARS)
        place();
    // log(std::to_string(profile_stop()));

    score += diff * 2;

    return;
}

void Game::update() {
    if(!disappearing)
        timer++;
    else
        eventTimer++;

    if(dropLockTimer)
        dropLockTimer--;

    if(zoneTimer > 1){
        zoneTimer--;
    }

    if (clearLock || lost)
        return;

    if(!disappearing)
        inGameTimer++;

    if (entryDelay){
        entryDelay--;
    }

    if(pawn.current == -1 && !entryDelay){
        next();
    }

    if (!(left || right)){
        if(gameMode != CLASSIC){
            das = 0;
            arrCounter = 0;
        }
    }else if (das < maxDas){
        das++;
    }

    if(entryDelay)
        return;


    if (das == maxDas && !(left && right)) {
        if (arr >= 0 && --arrCounter <= 0 && !(gameMode == CLASSIC && down)) {
            for(int i = 0; i < 1 + (arr == 0); i++){//move piece twice if arr is 0
                if (left)
                    moveLeft();
                else if(right)
                    moveRight();
            }
            arrCounter = arr;
        }else if(arr < 0){
            bool c = true;
            while(c){
                if(left)
                    c = moveLeft();
                else if(right)
                    c = moveRight();
            }
        }
    }

    if(gameMode == BATTLE){
        auto index = garbageQueue.begin();

        while(index != garbageQueue.end()){
            if(index->timer){
                index->timer--;
            }
            ++index;
        }
    }else if(gameMode == SURVIVAL){
        int rate = 60 * (4-goal);
        if(timer % rate == 0){
            generateGarbage(1,0);
            refresh = 1;

            while(!checkRotation(0,0,pawn.rotation))
                pawn.y--;
        }
    }else if(gameMode == MASTER && !disappearing){
        if(level < 500 && timer >= 25200)
            won = 1;

        if(decayTimer && comboCounter < 2){
            decayTimer--;
        }else{
            decayTimer = gradeTable[internalGrade][1];
            if(gradePoints)
                gradePoints--;
        }
    }

    if((linesCleared >= goal && gameMode == SPRINT && goal && subMode == 0) ||
       (linesSent >= goal && gameMode == SPRINT && goal && subMode == 1) ||
       (goal && linesCleared >= goal && gameMode == MARATHON) ||
       (garbageCleared >= goal && gameMode == DIG) ||
       (timer > goal && (gameMode == ULTRA || gameMode == BLITZ)) ||
       (linesCleared >= goal && gameMode == CLASSIC && goal)){
       won = 1;
       if(gameMode == CLASSIC && goal){
           score+= (level + bTypeHeight) * 1000;
       }
    }else if(disappearing && (eventTimer) >= CREDITS_LENGTH){
        won = 1;

        creditGrade += creditPoints[4][disappearing-1];
    }

    if(gracePeriod){
        gracePeriod--;
    }else if(!zoneTimer)
        speedCounter += speed;

    int n = (int)speedCounter;
    for (int i = 0; i < n; i++) {
        if (checkRotation(0, 1, pawn.rotation)){
            pawn.y++;
            if(rotationSystem == ARS)
                lockTimer = maxLockTimer;
        }else if(rotationSystem == NRS){
            place();
        }
    }

    speedCounter -= n;

    pawn.lowest = lowest();

    bool placed = false;

    if(dropping){
        hardDrop();
        dropping = false;
        return;
    }else{
        if (pawn.lowest == pawn.y && !zoneTimer)
            lockTimer--;

        if (lockTimer == 0 && rotationSystem != NRS){
            place();
            placed = true;
        }
    }

    if (down) {
        if(rotationSystem == NRS || gameMode == MASTER)
            softDropCounter = maxDas;
        else if (softDropCounter < maxDas)
            softDropCounter++;
    }else
        softDropCounter = 0;

    if (((!delaySoftDrop && down) || softDropCounter == maxDas) && !placed) {
        if(softDropSpeed >= 0 && ++softDropRepeatTimer >= softDropSpeed){
            int n = 1 + (softDropSpeed == 0); // move piece twice if softDropSpeed is 0

            for(int i = 0; i < n; i++){
                moveDown();
                if (pawn.y != pawn.lowest)
                    score++;
            }
            softDropRepeatTimer = 0;
        }else if (softDropSpeed < 0){
            int n = pawn.lowest - pawn.y;

            pawn.y = pawn.lowest;
            score+=n;
        }
    }

    if(disappearing)
        updateDisappear();

    while((int)moveHistory.size() > 10)
        moveHistory.pop_front();

}

int Game::lowest() {

    if(!pawn.big){
        int start = stackHeight-4;
        if(stackHeight-4 < pawn.y)
            start = pawn.y;

        for(int i = start; i < lengthY; i++){
            for(int j = 0; j < 4; j++){
                if(pawn.boardLowest[pawn.rotation][j] == -1)
                    continue;
                int x = pawn.x+j;
                int y = i+pawn.boardLowest[pawn.rotation][j];
                if(y >= lengthY || board[y][x])
                    return i-1;
            }
        }
    }else{
        for(int i = 0; i < (lengthY - pawn.y)*2; i++){
            for(int j = 0; j < 4; j++){
                if(pawn.boardLowest[pawn.rotation][j] == -1)
                    continue;
                int x = (pawn.x+j)*2;
                int y = (pawn.y+i+pawn.boardLowest[pawn.rotation][j])*2;
                for(int k = 0; k < 2; k++){
                    for(int l = 0; l < 2; l++){
                        if(y+k >= lengthY || board[y+k][x+l])
                            return pawn.y+i-1;
                    }
                }
            }
        }
    }

    return pawn.y;
}

void Game::place() {
    if(pawn.current == -1)
        return;

    lastDrop = calculateDrop();

    for (int j = 0; j < 4; j++) {
        for (int i = pawn.boardLowest[pawn.rotation][j]; i >= 0; i--) {
            if (pawn.board[pawn.rotation][i][j] == 0)
                continue;

            int x = pawn.x + j;
            int y = pawn.y + i;

            if(!pawn.big){
                if (y > lengthY - 1 || x > lengthX)
                    continue;

                board[y][x] = pawn.current + pawn.board[pawn.rotation][i][j];
                if(disappearing == 1)
                    disappearTimers[y][x] = MAX_DISAPPEAR;
                else if(disappearing == 2)
                    disappearTimers[y][x] = 1;
            }else{
                x *=2;
                y *=2;

                for(int k = 0; k < 2; k++){
                    for(int l = 0; l < 2; l++){
                        if (y+k > lengthY - 1 || x+l > lengthX)
                            continue;

                        board[y+k][x+l] = pawn.current + pawn.board[pawn.rotation][i][j];
                        if(disappearing == 1)
                            disappearTimers[y+k][x+l] = MAX_DISAPPEAR;
                        else if(disappearing == 2)
                            disappearTimers[y][x] = 1;
                    }
                }
            }
        }
    }

    int lowestPart = 0;
    for(int i = 0; i < 4; i++)
        if(pawn.boardLowest[pawn.rotation][i] > lowestPart)
            lowestPart = pawn.boardLowest[pawn.rotation][i];

    if ((pawn.y+lowestPart) * (1+pawn.big) < (lengthY / 2 - 2)) {
        lost = 1;
        return;
    }

    int prevLevel = level;

    if (clear(lastDrop)){
        if(rotationSystem != NRS)
            comboCounter++;
        else
            comboCounter = 1;

        if(gameMode == MASTER)
            entryDelay = lineAre;

    } else {
        if(gameMode == COMBO && comboCounter)
            lost = 1;

        comboCounter = 0;

        int targetHeight = 9;
        if(pawn.big)
            targetHeight = 4;

        if(gameMode == DIG && garbageHeight < targetHeight){
            int toAdd = targetHeight-garbageHeight;
            if(garbageCleared+toAdd+garbageHeight <= goal)
                generateGarbage(toAdd,0);
            else if(garbageCleared+toAdd+garbageHeight > goal && garbageCleared+garbageHeight < goal)
                generateGarbage(goal-(garbageCleared+garbageHeight),0);
        }else if(gameMode == BATTLE){
            int sum = 0;
            std::list<Garbage>::iterator index = garbageQueue.begin();
            while(index != garbageQueue.end()){
                if(index->timer == 0){
                    sum+=index->amount;
                    garbageQueue.erase(index++);
                }else{
                    ++index;
                }
            }
            generateGarbage(sum,1);
        }

        if(gameMode == MASTER)
            entryDelay = areMax;
    }

    if(comboCounter > statTracker.maxCombo)
        statTracker.maxCombo = comboCounter;

    if(rotationSystem == NRS){
        int add = 0;
        while(pawn.y + add < lengthY-3)
            add+=4;

        entryDelay = 10 + add;

        down = 0;
    }else if (gameMode == MASTER){

        if(level % 100 < 99 && level < 998)
            level++;

        if (level % 100 >= 70 && prevLevel % 100 < 70){
            if(sectionTimeGoal[(level / 100)][0] > timer-sectionStart && (timer-sectionStart < previousSectionTime+120))
                cool = true;
        }else if(cool && level % 100 >= 82 && prevLevel % 100 < 82){
            sounds.section = 1;
        }else if((level % 100 == 99 && prevLevel % 100 < 99 && level != 999) || (level == 998 && prevLevel != 998)){
            sounds.section = 2;
        }
    }

    if(entryDelay)
        pawn.current = -1;

    canHold = true;

    // profile_start();
    if (!clearLock && !entryDelay && !(zoneTimer == 1 && comboCounter > 0))
        next();
    // log(std::to_string(profile_stop()));

    lockTimer = maxLockTimer;
    lockMoveCounter = 15;
    dropLockTimer = dropLockMax;
    stopLockReset = false;

    sounds.place = 1;
    refresh = 1;

    pushDir = 0;
    previousKey = 0;
    pieceCounter++;

    moveHistory.clear();

    if(zoneTimer == 1)
        endZone();

    if(maxClearDelay == 1 && clearLock){
        removeClearLock();
    }
}

int Game::clear(Drop drop) {

    int clearCount = 0;
    int attack = 0;
    int isTSpin = 0;
    int isPerfectClear = 1;
    int isBackToBack = 0;
    int isDifficult = 0;

    //check for t-spin
    if (pawn.current == 5 && lastMoveRotation && gameMode != CLASSIC) {
        int frontCount = 0;
        int backCount = 0;
        int x = pawn.x * (1+pawn.big);
        int y = pawn.y * (1+pawn.big);
        int offset = 2 * (1+pawn.big);

        switch (pawn.rotation) {
        case 0:
            frontCount += (board[y][x] != 0) + (board[y][x + offset] != 0);
            if (y + offset > lengthY - 1)
                backCount = 2;
            else
                backCount += (board[y + offset][x] != 0) + (board[y + offset][x + offset] != 0);
            break;
        case 1:
            frontCount += (board[y][x + offset] != 0) + (board[y + offset][x + offset] != 0);
            if (x < 0)
                backCount = 2;
            else
                backCount += (board[y][x] != 0) + (board[y + offset][x] != 0);
            break;
        case 2:
            frontCount += (board[y + offset][x] != 0) + (board[y + offset][x + offset] != 0);
            backCount += (board[y][x] != 0) + (board[y][x + offset]);
            break;
        case 3:
            frontCount += (board[y][x] != 0) + (board[y + offset][x] != 0);
            if (x > lengthX - 1)
                backCount = 2;
            else
                backCount += (board[y][x + offset] != 0) + (board[y + offset][x + offset] != 0);
            break;
        }
        if ((frontCount == 2 && backCount > 0) || (frontCount > 0 && backCount == 2 && specialTspin))
            isTSpin = 2;
        else if (frontCount > 0 && backCount == 2)
            isTSpin = 1;
    }

    //check for lines to clear
    int garbageToRemove = 0;
    int zonedBefore = zonedLines;

    std::list<int> linesToMove;

    int start = lastDrop.startY + 20;
    int end = lastDrop.rawEndY + 1;

    if(zonedLines && !zoneTimer){
        start = 0;
        end = lengthY;
    }

    for (int i = start ; i < end; i++) {
        bool toClear = true;
        for (int j = 0; j < lengthX; j++)
            if (board[i][j] == 0){
                toClear = false;
                break;
            }

        if (toClear) {
            if(!zoneTimer){
                linesToClear.push_back(i);
                clearCount++;
                if(gameMode == DIG && i >= lengthY-(garbageHeight*(1+pawn.big)))
                    garbageToRemove++;
            }else{
                linesToMove.push_back(i);
            }
        }
    }

    if(!linesToMove.empty()) {

        fixConnected(linesToMove);

        int offset = 0;
        for(auto const& line : linesToMove){
            int i = line - offset;
            if(i >= lengthY-zonedLines)
                break;

            for(int ii = i; ii < lengthY-1-zonedLines; ii++)
                for (int j = 0; j < lengthX; j++)
                    board[ii][j] = board[ii+1][j];

            zonedLines++;

            for (int j = 0; j < lengthX; j++)
                board[lengthY-zonedLines][j] = 9;

            offset++;
        }

        if (zonedLines > zonedBefore) {
            sounds.clear = 1;
            clearCount = zonedLines - zonedBefore;
        }

        if (!inversion && zonedLines >= 8 && sounds.zone != -1) {
            inversion = true;
            sounds.zone = 2;
        }

        linesToMove.clear();
    }

    if(pawn.big)
        garbageToRemove/=2;

    garbageHeight-=garbageToRemove;
    garbageCleared+=garbageToRemove;

    if (clearCount == 0){
        return 0;
    }

    if(pawn.big)
        clearCount/=2;

    if(!zoneTimer)
        linesCleared+=clearCount;

    for(auto const & line : linesToClear){
        log(std::to_string(line));

    }

    auto index = linesToClear.begin();
    for (int j = 0; j < lengthY-1; j++) {
        if(j == *index){
            ++index;
            continue;
        }

        for (int i = 0; isPerfectClear && i < lengthX; i++) {
            if (board[j][i]) {
                isPerfectClear = 0;
            }
        }
    }

    fixConnected(linesToClear);

    //check for level up
    int prevLevel = level;
    switch(gameMode){
    case MARATHON:
        level = ((int)linesCleared / 10) + 1;
        if(subMode && !zonedLines){
            if(zoneCharge < 32){
                if((zoneCharge + clearCount) >= 32){
                    sounds.meter = 1;
                    zoneCharge = 32;
                } else
                    zoneCharge += clearCount;
            }
        }
        break;
    case BLITZ:
        for(int i = 0; i < 15; i++){
            if(linesCleared < blitzLevels[i]){
                level = i+1;
                break;
            }
        }
        break;
    case CLASSIC:
        if(goal)
            break;

        if(initialLevel){
            int init = min(initialLevel*10+10,max(100,initialLevel*10-50));
            int requirement = linesCleared-init;
            if(requirement >= 0)
                level = (requirement/10) + initialLevel + 1;
        }else{
            level = ((int)linesCleared / 10);
        }
        break;
    case MASTER:
        if(level == 999)
            break;

        level += clearCount + (clearCount > 2) + (clearCount > 3);

        if(level > 999)
            level = 999;

        if(level == 999 && !disappearing){

            disappearing = 1 + (coolCount == 9);

            disappearTimers = new u16*[lengthY];
            for(int i = 0; i < lengthY; i++){
                disappearTimers[i] = new u16[lengthX];
                for(int j = 0; j < lengthX; j++)
                    disappearTimers[i][j] = 0;
            }

            sectionStart = timer;

            eventLock = true;

            for(int i = 0; i < lengthY; i++)
                for(int j = 0; j < lengthX; j++)
                    board[i][j] = 0;
        }

        int currentSectionLevel = level / 100 ;

        if(currentSectionLevel > prevLevel / 100){
            int currentSession = timer-sectionStart;

            if(cool){
                coolCount++;
                cool = false;
            }

            if(currentSession > sectionTimeGoal[currentSectionLevel-1][1]){//regret
                if(grade){
                    int newInternal = 31;
                    while(newInternal != 0 && gradeTable[newInternal][0] >= grade)
                        newInternal--;

                    internalGrade = newInternal;
                    grade--;
                    gradePoints = 0;
                    regretCount++;
                }
                sounds.section = -1;
            }

            previousSectionTime = currentSession;
            sounds.levelUp = 1;
            sectionStart = timer;
            setMasterTuning();
        }

        if(disappearing && clearCount){
            creditGrade += creditPoints[clearCount-1][disappearing-1];
        }
        break;
    }

    if(level < prevLevel)
        level = prevLevel;

    if (prevLevel < level && (gameMode == MARATHON || gameMode == BLITZ || gameMode == CLASSIC)){
        sounds.levelUp = 1;
    }

    //calculate score
    int add = 0;
    if(clearCount <= 4){
        if(isTSpin == 0){
            //if no t-spin
            if(gameMode != CLASSIC){
                add += scoring[clearCount - 1][0] * prevLevel;
                isDifficult = scoring[clearCount - 1][1];
                attack = scoring[clearCount - 1][2];
            } else
                add += classicScoring[clearCount-1] * (level+1);

            statTracker.clears[clearCount-1]++;
        }else if(isTSpin == 1){
            //if t-spin mini
            add += scoring[clearCount + 4][0] * prevLevel;
            isDifficult = scoring[clearCount + 4][1];
            attack = scoring[clearCount + 4][2];
            statTracker.tspins++;
        }else if(isTSpin == 2){
            //if t-spin
            add += scoring[clearCount  + 7][0] * prevLevel;
            attack = scoring[clearCount + 7][2];
            isDifficult = true;
            statTracker.tspins++;
        }
    }

    stackHeight += clearCount;

    if(gameMode == MASTER && !disappearing){
        gradePoints += (int) ((float) gradeTable[internalGrade][clearCount+1] * masterComboMultiplayer[(comboCounter < 10)?comboCounter:9][clearCount-1] + 1) * ((int)(level/250)+1);

        if(gradePoints >= 100){
            gradePoints = 0;
            internalGrade++;
        }

        grade = gradeTable[internalGrade][0];
    }

    isBackToBack = previousClear.isDifficult && (clearCount == 4 || isTSpin == 2) && gameMode != CLASSIC;

    if (isBackToBack){
        add = (int)add * 3 / 2;
        b2bCounter++;
        attack+= 1;
    }else if (!isDifficult){
        b2bCounter = 0;
    }

    if(b2bCounter > statTracker.maxStreak)
        statTracker.maxStreak = b2bCounter;

    //add combo bonus
    add += scoring[16][0] * level * comboCounter;
    attack += comboTable[comboCounter];

    if (isPerfectClear && gameMode != CLASSIC && clearCount <= 4){
        if(gameMode != BLITZ)
            add += scoring[clearCount - 1 + 11][0] * level;
        else
            add += 3500 * level;
        attack = scoring[clearCount - 1 + 11][2];

        statTracker.perfectClears++;
    }

    if(zoneTimer){
        zoneScore += add;
    } else if (zonedLines){
        int n = (1 + fullZone + (zonedLines >= 8));
        zoneScore = zoneScore * n;
        score += zoneScore;
        zoneScore = 0;
    } else
        score += add;

    previousClear = Score(clearCount, add, comboCounter, isTSpin, isPerfectClear, isBackToBack, isDifficult, drop);

    sounds.clear = 1;

    if(!zoneTimer)
        clearLock = 1;
    specialTspin = false;

    if(garbageQueue.size()){
        auto index = garbageQueue.begin();
        while(attack > 0 && index != garbageQueue.end()){
            bool cleared = false;
            if(attack > index->amount){
                attack -= index->amount;
                cleared = true;

                garbageQueue.erase(index++);
            }else{
                index->amount -= attack;
                attack = 0;
            }
            if(!cleared)
                ++index;
        }
    }

    if(attack){
        attackQueue.push_back(Garbage(timer & 0xff,attack));
        linesSent+=attack;
    }

    setSpeed();

    return 1;
}

Color Game::color(int n) {
    Color result;
    result.r = colors[n][0];
    result.g = colors[n][1];
    result.b = colors[n][2];

    return result;
}

void Game::fillBag() {
    if(rotationSystem == SRS){
        for (int i = 0; i < 7; i++)
            bag.push_back(i);
        bagCounter++;
    }else if(rotationSystem == ARS){
        bag.clear();

        //fill bag
        for(int i = 0; i < 35; i++)
            bigBag[i] = i / 7;

        //fill drought history
        for(int i = 0; i < 7; i++)
            pieceDrought[i] = 5;

        //fill piece history
        historyList.push_back(PIECE_Z);
        historyList.push_back(PIECE_Z);
        historyList.push_back(PIECE_S);
        historyList.push_back(PIECE_S);
    }
}

void Game::next() {//~21-29k cycles
    pawn.y = (int)lengthY / 2;
    pawn.x = (int)lengthX / 2 - 2;

    if((fromLock && irs) || rotationSystem == ARS){
        int sum = rotates[0] + (rotates[1] * -1) + (rotates[2] * ((rotationSystem == ARS)? -1 : 2));

        pawn.rotation = sum + 4 * (sum < 0);
    } else
        pawn.rotation = 0;

    speedCounter = 0;

    pawn.current = queue.front();
    queue.pop_front();
    fillQueue(1);

    //for CLASSIC 2 in a row protection
    pieceHistory = pawn.current;

    pawn.setBlock(rotationSystem); //~15k cycles

    if(gameMode == CLASSIC){
        down = 0;
        if(pawn.current != 0 && pawn.current != 3)
            pawn.x++;
        if(pawn.current == 0)
            pawn.y--;
    }

    if(pawn.big){
        pawn.x/=2;
        pawn.y/=2;
    }

    //check if stack has reached top half of board
    bool check = false;
    for(int j = 0; j < 10; j++){
        if(board[30][j] != 0){
            check = true;
            break;
        }
    }

    if (check || !checkRotation(0, 0, pawn.rotation) || gameMode == CLASSIC){
        pawn.y-=1;
        if (!checkRotation(0, 0, pawn.rotation)){
            if(zoneTimer && zonedLines)
                endZone();
            else if(gameMode == TRAINING)
                clearBoard();
            else{
                lost = 1;
            }
        }
    }

    lastMoveDx = 0;
    lastMoveDy = 0;

    arrCounter = 0;

    softDrop = false;

    if(holding && canHold && (ihs || rotationSystem == ARS))
        hold(1);
}

void Game::fillQueue(int count) {
    if(seed != 0){
        sqran(seed);
        seed+= qran();
    }

    for (int i = 0; i < count; i++) {
        if(rotationSystem == SRS){
            int n = qran() % bag.size();

            std::list<int>::iterator index = bag.begin();
            std::advance(index, n);

            queue.push_back(*index);
            bag.erase(index);

            if (bag.size() == 0)
                fillBag();
        }else if (rotationSystem == NRS){
            int n = qran() % 8;

            if(n == 7 || n == pieceHistory)
                n = qran() % 7;

            queue.push_back(n);
        }else if (rotationSystem == ARS){
            int n = -1;
            int record = 0;
            int droughted = 0;
            if(queue.empty()){
                while(n == -1 || n == PIECE_O || n == PIECE_Z || n == PIECE_S){
                    n = qran() % 7;
                }

                queue.push_back(n);
                continue;
            }

            int r = 0;
            for(int i = 0; i < 6; i++){
                r = qran() % 35;

                n = bigBag[r];

                if(std::find(historyList.begin(),historyList.end(),n) == historyList.end())
                    break;

                //find most droughted
                for(int i = 0; i < 7; i++){
                    if(record < pieceDrought[i]){
                        record = pieceDrought[i];
                        droughted = i;
                    }
                }

                //insert droughted into bag
                bigBag[r] = droughted;

                //in case all previous rolls failed
                if(i == 5){
                    r = qran() % 35;
                    n = bigBag[r];
                }
            }

            //update droughted list
            for(int i = 0; i < 7; i++)
                pieceDrought[i]++;
            pieceDrought[n] = 0;

            //find most droughted
            //record is intentionally not reset; this bug was in the original game
            for(int i = 0; i < 7; i++){
                if(record < pieceDrought[i]){
                    record = pieceDrought[i];
                    droughted = i;
                }
            }

            //insert droughted into bag
            bigBag[r] = droughted;

            historyList.push_front(n);
            historyList.pop_back();

            queue.push_back(n);
        }
    }
}

void Game::hold(int dir) {
    holding = dir;
    if((clearLock || entryDelay) && gameMode != CLASSIC){
        // holding = dir;
        return;
    }else if(dir == 0){
        return;
    }

    // if (!canHold || clearLock || entryDelay || gameMode == CLASSIC || pawn.current == -1)
    if (!canHold || gameMode == CLASSIC || pawn.current == -1)
        return;

    moveCounter++;
    canHold = false;

    if (held == -1) {
        held = pawn.current;
        next();
    } else {
        int temp = held;
        held = pawn.current;
        pawn.current = temp;
        pawn.setBlock(rotationSystem);
        pawn.x = (int)lengthX / 2 - 2;
		pawn.y = (int)lengthY / 2 - 1;

        if(pawn.big){
            pawn.x/=2;
            pawn.y/=2;
        }
    }

    if(rotationSystem == ARS || irs){
        int sum = rotates[0] + (rotates[1] * -1) + (rotates[2] * ((rotationSystem == ARS)? -1 : 2));

        pawn.rotation = sum + (sum < 0) * 4;
    } else
        pawn.rotation = 0;

    sounds.hold = 1;
    holdCounter++;

    lockTimer = maxLockTimer;
    lockMoveCounter = 15;
    arrCounter = 0;

    dropLockTimer = 0;
    moveHistory.clear();

    statTracker.holds++;
}

int* Tetris::getShape(int n,int r, int rotationSystem) {
    int* result;
    result = new int[4* 4];

    int * source;

    switch(rotationSystem){
    case SRS:
        source = (int * )tetraminos[n][r];
        break;
    case NRS:
        source = (int * )classic[n][r];
        break;
    case ARS:
        source = (int * )ars[n][r];
        break;
    default:
        source = (int * )tetraminos[n][r];
        break;
    }

    memcpy32(result,source,16);

    // for (int i = 0; i < 4; i++){
    //     for (int j = 0; j < 4; j++){
    //     }
    // }

    // if(bigMode)
    //     return result;

    // for(int iy = 0; iy < 4; iy++){
    //     for(int ix = 0; ix < 4; ix++){
    //         if(!result[iy][ix])
    //             continue;

    //         const int n = getNbr(result, ix, iy, 4 , 0 , 4);

    //         switch(n){
    //         case 1:
    //             if(iy > 0 && result[iy-1][ix])
    //                 result[iy][ix] += 1 << 4;
    //             else if(iy < 3 && result[iy+1][ix])
    //                 result[iy][ix] += 2 << 4;
    //             else if(ix > 0 && result[iy][ix-1])
    //                 result[iy][ix] += 3 << 4;
    //             else if(ix < 3 && result[iy][ix+1])
    //                 result[iy][ix] += 4 << 4;
    //             break;
    //         case 2:
    //             if(iy > 0 && result[iy-1][ix] && iy < 3 && result[iy+1][ix])
    //                 result[iy][ix] += 5 << 4;
    //             else if(ix > 0 && result[iy][ix-1] && ix < 3 && result[iy][ix+1])
    //                 result[iy][ix] += 6 << 4;
    //             else if(iy < 3 && result[iy+1][ix] && ix < 3 && result[iy][ix+1])
    //                 result[iy][ix] += 7 << 4;
    //             else if(iy < 3 && result[iy+1][ix] && ix > 0 && result[iy][ix-1])
    //                 result[iy][ix] += 8 << 4;
    //             else if(iy > 0 && result[iy-1][ix] && ix < 3 && result[iy][ix+1])
    //                 result[iy][ix] += 9 << 4;
    //             else if(iy > 0 && result[iy-1][ix] && ix > 0 && result[iy][ix-1])
    //                 result[iy][ix] += 10 << 4;
    //             break;
    //         case 3:
    //             if(iy < 3 && result[iy+1][ix] && ix < 3 && result[iy][ix+1] && result[iy+1][ix+1])
    //                 result[iy][ix] += 11 << 4;
    //             else if(iy < 3 && result[iy+1][ix] && ix > 0 && result[iy][ix-1] && result[iy+1][ix-1])
    //                 result[iy][ix] += 12 << 4;
    //             else if(iy > 0 && result[iy-1][ix] && ix < 3 && result[iy][ix+1] && result[iy-1][ix+1])
    //                 result[iy][ix] += 13 << 4;
    //             else if(iy > 0 && result[iy-1][ix] && ix > 0 && result[iy][ix-1] && result[iy-1][ix-1])
    //                 result[iy][ix] += 14 << 4;
    //             else if(iy > 0 && result[iy-1][ix] && iy < 3 && result[iy+1][ix] && ix < 3 && result[iy][ix+1])
    //                 result[iy][ix] += 15 << 4;
    //             else if(ix > 0 && result[iy][ix-1] && ix < 3 && result[iy][ix+1] && iy < 3 && result[iy+1][ix])
    //                 result[iy][ix] += 16 << 4;
    //             else if(iy > 0 && result[iy-1][ix] && iy < 3 && result[iy+1][ix] && ix > 0 && result[iy][ix-1])
    //                 result[iy][ix] += 17 << 4;
    //             else if(ix > 0 && result[iy][ix-1] && ix < 3 && result[iy][ix+1] && iy > 0 && result[iy-1][ix])
    //                 result[iy][ix] += 18 << 4;
    //             break;
    //         }
    //         // }else if(n == 5){
    //         //     if(iy < 3 && ix < 3 && ix > 0 && result[iy+1][ix-1] && result[iy+1][ix+1])
    //         //         result[iy][ix] += 19 << 4;
    //         //     else if(iy > 0 && ix < 3 && ix > 0 && result[iy-1][ix-1] && result[iy-1][ix+1])
    //         //         result[iy][ix] += 20 << 4;
    //         //     else if(ix < 3 && iy < 3 && iy > 0 && result[iy-1][ix+1] && result[iy+1][ix+1])
    //         //         result[iy][ix] += 21 << 4;
    //         //     else if(ix > 0 && iy < 3 && iy > 0 && result[iy-1][ix-1] && result[iy+1][ix-1])
    //         //         result[iy][ix] += 22 << 4;
    //         // }else if(n == 8){
    //         //     result[iy][ix] += 23 << 4;
    //     }
    // }

    return result;
}

void Game::lockCheck() {
    if (pawn.lowest == pawn.y && lockMoveCounter > 0 && rotationSystem == SRS) {
        lockTimer = maxLockTimer;
        lockMoveCounter--;
    }
}

void Game::keyLeft(int dir) {
    moveCounter++;
    if (clearLock || eventLock || entryDelay || (gameMode == CLASSIC && down)) {
        left = dir;
        return;
    }

    if (left == 0 && dir)
        moveLeft();
    left = dir;
    
    previousKey = -1;
    dropLockTimer = 0;

    if(gameMode == CLASSIC && dir){
        das = 0;
        arrCounter = 0;
    }

    if(!dir){
        pushDir = 0;

        if(directionCancel && gameMode != CLASSIC){
            das = 0;
            arrCounter = 0;
        }
    }
}

void Game::keyRight(int dir) {
    moveCounter++;
    if (clearLock || eventLock || entryDelay || (gameMode == CLASSIC && down)) {
        right = dir;
        return;
    }

    if (right == 0 && dir)
        moveRight();
    right = dir;
    
    previousKey = 1;
    dropLockTimer = 0;

    if(gameMode == CLASSIC && dir){
        das = 0;
        arrCounter = 0;
    }

    if(!dir){
        pushDir = 0;
        if(directionCancel && gameMode != CLASSIC){
            das = 0;
            arrCounter = 0;
        }
    }
}

void Game::keyDown(int dir) {
    moveCounter++;
    if (clearLock || eventLock || entryDelay) {
        if(!(entryDelay && rotationSystem == NRS))
            down = dir;
        return;
    }

    if (down == 0 && dir){
        moveDown();
        if (pawn.y != pawn.lowest)
            score++;
    }

    down = dir;
    softDrop = true;
    dropLockTimer = 0;
    gracePeriod = 0;

}

void Game::removeClearLock() {
    if(!clearLock)
        return;
    clearLock = 0;

    auto index = linesToClear.begin();

    const int len = (int)linesToClear.size();

    for (int i = 0; i < len; i++) {
        for (int j = *index; j >= stackHeight-len; j--){
            memcpy32(board[j],board[j-1],10);
            if(disappearing)
                memcpy16(disappearTimers[j],disappearTimers[j-1],10);
        }
        ++index;
    }

    if(gameMode == COMBO){
        if(!pawn.big){
            for(int i = lengthY/2-1; i < lengthY; i++){
                if(board[i][0] != 0)
                    break;
                for(int j = 0; j < 10; j++){
                    if(j > 2 && j < 7)
                        continue;
                    board[i][j] = (i+comboCounter) % 7 + 1;
                }
                connectBoard(i, i);
            }
        }else{
            for(int i = (lengthY/2-1)/2; i < lengthY/2; i++){
                if(board[i*2][0] != 0)
                    break;
                board[i*2][0] = (i*2+comboCounter) % 7 + 1;
                board[i*2][1] = (i*2+comboCounter) % 7 + 1;
                board[i*2+1][0] = (i*2+comboCounter) % 7 + 1;
                board[i*2+1][1] = (i*2+comboCounter) % 7 + 1;
            }
        }
    }

    linesToClear = std::list<int>();

    fromLock = true;
    if(!entryDelay)
        next();
    fromLock = false;

    refresh = 2;
    zonedLines = 0;
}

void Game::resetSounds(){
    sounds = SoundFlags();
}

void Game::resetRefresh() {
    refresh = 0;
}

void Game::setLevel(int newLevel){
    initialLevel = level = newLevel;
    setSpeed();
}

void Game::setGoal(int newGoal){
    goal = newGoal;
}

void Game::generateGarbage(int height,int mode){
    if(!pawn.big){
        int hole = qran() % lengthX;
        // shift up
        for(int i = 0; i < lengthY; i++){
            for(int j = 0; j < lengthX; j++){
                if(i < lengthY-height)
                    board[i][j] = board[i+height][j];
                else
                    board[i][j] = 0;
            }
        }

        for(int i = lengthY-height; i < lengthY; i++){
            int prevHole = hole;
            int count = 0;
            if(!mode || rand() % 10 < 3){
                do{
                    hole = rand() % lengthX;
                }while(++count < 11 && ((!board[i-1][hole] && height < garbageHeight) || hole == prevHole));
            }
            for(int j = 0; j < lengthX; j++){
                if(j == hole)
                    continue;
                board[i][j]=8;
            }
        }

        garbageHeight+=height;
    }else{
        int hole = qran() % lengthX/2;
        // shift up
        for(int i = 0; i < lengthY; i++){
            for(int j = 0; j < lengthX; j++){
                if(i < lengthY-height*2)
                    board[i][j] = board[i+height*2][j];
                else
                    board[i][j] = 0;
            }
        }

        for(int i = lengthY/2-height; i < lengthY/2; i++){
            int prevHole = hole;
            if(!mode || qran() % 10 < 3){
                do{
                    hole = qran() % (lengthX/2);
                }while((!board[(i-1)*2][hole*2] && height < garbageHeight) || hole == prevHole);
            }
            for(int j = 0; j < lengthX/2; j++){
                if(j == hole)
                    continue;
                board[i*2][j*2]=8;
                board[i*2][j*2+1]=8;
                board[i*2+1][j*2]=8;
                board[i*2+1][j*2+1]=8;
            }
        }

        garbageHeight+=height;
    }

    stackHeight -= height;

    if(bigMode)
        return;

    for(int i = lengthY-height; i < lengthY; i++){
        connectBoard(i, i);
    }
}

void Game::keyDrop(int dir){
    if(!dir)
        return;

    if(gameMode == CLASSIC || entryDelay)
        return;

    dropping = true;

    moveCounter++;
}

Drop Game::getDrop(){
    Drop result = lastDrop;
    lastDrop = Drop();
    return result;
}

Drop Game::calculateDrop(){
    Drop result;

    result.on = true;

    result.x = pawn.x;
    result.y = pawn.y;

    result.dx = lastMoveDx;
    result.dy = lastMoveDy;

    result.piece = pawn.current;
    result.rotation = pawn.rotation;
    result.rotating = lastMoveRotation;

    result.startX = pawn.x;

    int start = 0;
    int end = 3;
    while(pawn.boardLowest[pawn.rotation][start] == -1)
        start++;
    while(pawn.boardLowest[pawn.rotation][end] == -1)
        end--;

    result.startX += start;
    result.endX = pawn.x + end + 1;

    result.startY = pawn.y + pawn.heighest[pawn.rotation];
    
    int add = 0;
    for(int i = 0; i < 4; i++)
        if(pawn.boardLowest[pawn.rotation][i] > add)
            add = pawn.boardLowest[pawn.rotation][i];

    result.rawEndY = result.endY = pawn.y + add;

    if(rotationSystem != NRS){
        if(((pawn.current == 2 && pawn.rotation == 3) || (pawn.current == 1 && pawn.rotation == 1)))
            result.endY--;
    }else{
        if(((pawn.current == 2 && pawn.rotation == 1) || (pawn.current == 1 && pawn.rotation == 3)))
            result.endY--;
    }

    if(result.startY < stackHeight)
        stackHeight = result.startY;

    if(pawn.big){
        result.startX *=2;
        result.endX *=2;
        result.startY *=2;
        result.endY *=2;
        result.rawEndY *=2;
    }

    result.startY -= 20;
    result.endY -= 20;
    result.y -= 20;

    return result;
}

void Game::setTuning(Tuning newTune){
    if(gameMode == CLASSIC || gameMode == MASTER)
        return;

    maxDas = newTune.das;
    arr = newTune.arr;
    softDropSpeed = newTune.sfr;
    dropLockMax = newTune.dropProtection;
    directionCancel = newTune.directionalDas;
    delaySoftDrop = newTune.delaySoftDrop;
    maxClearDelay = 20;
    ihs = newTune.ihs;
    irs = newTune.irs;
}

void Game::setMasterTuning(){
    int n = 0;
    int speedLevel = level + coolCount * 100;

    if(speedLevel >= 500){
        n = (speedLevel - 400)/100;
    }

    if(n > 8)
        n = 8;

    arr = 1;
    areMax = masterDelays[n][0];
    lineAre = masterDelays[n][1];
    maxDas = masterDelays[n][2];
    maxLockTimer = masterDelays[n][3];
    maxClearDelay = masterDelays[n][4];
    softDropSpeed = 1;
}

void Game::clearAttack(int id){
    std::list<Garbage>::iterator index = attackQueue.begin();

    while(index != attackQueue.end()){
        if(index->id == id){
            attackQueue.erase(index++);
        }else{
            ++index;
        }
    }
}

void Game::setWin(){
    won = true;
}

void Game::addToGarbageQueue(int id, int amount){
    if(amount <= 0)
        return;

    for(const auto& attack : garbageQueue)
        if(attack.id == id)
            return;

    garbageQueue.push_back(Garbage(id,amount));
}

int Game::getIncomingGarbage(){
    int result = 0;

    for(const auto& atck : garbageQueue){
        result+= atck.amount;
    }

    return result;
}

std::list<int> Game::getBestFinesse(int piece,int dx, int rotation){
    std::list<int> result;

    int r = 0;
    switch(piece){
    case 0:
        r = rotation % 2;
        break;
    case 1:
        r = rotation;
        break;
    case 2:
        r = rotation;
        break;
    case 3:
        r = 0;
        break;
    case 4:
        r = rotation % 2;
        break;
    case 5:
        r = rotation;
        break;
    case 6:
        r = rotation % 2;
        break;
    }

    for(int i = 0; i < 4; i++){
        result.push_back(finesse[piece][r][dx][i]);
        if(finesse[piece][r][dx][i] == 7)
            break;
    }

    return result;
}

void Game::setTrainingMode(bool mode){
    if(pawn.big)
        return;
    trainingMode = mode;
}

void Game::demoClear(){
    clearLock = 1;

    for(int i = lengthY-2; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            board[i][j] = 8;
        }
    }

    clear(Drop());
}

void Game::demoFill(){
    clearLock = 0;
    for(int i = lengthY-2; i < lengthY; i++){
        int n = qran() % lengthX;
        board[i][n] = 0;
    }
}

void Game::bType(int height){
    int n = 0;

    if(height)
        n = height * 2 + (height + 1)/2 - (height == 5);

    for(int i = lengthY-n; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            if(qran() % 2 && !(i == lengthY-n && j == 0)){
                board[i][j] = (qran() % 7);
            }
        }
    }
}

void Game::setSubMode(int sm){

    if(gameMode == MASTER && sm){
        setRotationSystem(ARS);

        return;
    }

    subMode = sm;

    if(sm && gameMode == CLASSIC){
        bType(bTypeHeight);
        goal = 25;
    }
}

void Game::setSpeed(){
    if ((gameMode == MARATHON || gameMode == TRAINING) && !(subMode && zoneTimer))
        speed = gravity[(level < 19) ? level - 1 : 18];
    else if(gameMode == BLITZ)
        speed = blitzGravity[(level < 15) ? level - 1 : 14];
    else if(gameMode == CLASSIC)
        speed = classicGravity[(level < 30) ? level: 29];
    else if(gameMode == MASTER){
        int speedLevel = level + coolCount * 100;
        if(speedLevel >= 500)
            speed = masterGravity[29][1];
        else{
            for(int i = 28; i >= 0; i--){
                if(speedLevel >= masterGravity[i][0]){
                    speed = masterGravity[i][1];
                    break;
                }
            }
        }
        setMasterTuning();
    } else
        speed = gravity[0];

}

void Game::setRotationSystem(int rs){
    rotationSystem = rs;

    if(rs == ARS){
        queue.clear();

        fillBag();
        fillQueue(3);
    }
}

int Game::checkITouching(int r, int oriantation){
    int x = pawn.x;
    int y = pawn.y;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if(!pawn.big){
                if(oriantation == 0){
                    if((j == 1 || j == 3) && ((x + j < 0) || (x+j > lengthX - 1) || ((y + i >= 0 && y + i <= lengthY-1) && board[i+y][j+x])))
                        return j;
                }else{
                    if(i == 2 && ((y + i > lengthY-1) || ((x + j >= 0 && x + j <= lengthX-1) && (board[y+i][x+j]))))
                        return i;
                }
            }else{
                for(int k = 0; k < 2; k++){
                    int yoffset = (y+i) * 2 + k;
                    for(int l = 0; l < 2; l++){
                        int xoffset = (x+j) * 2 + l;
                        if(oriantation == 0){
                            if((j == 1 || j == 3) && ((xoffset < 0) || (xoffset > lengthX - 1) || ((yoffset >= 0 && yoffset <= lengthY-1) && board[yoffset][xoffset])))
                                return j;
                        }else{
                            if(i == 2 && ((yoffset > lengthY-1) || ((xoffset >= 0 && yoffset <= lengthX-1) && (board[yoffset][xoffset]))))
                                return i;
                        }
                    }
                }
            }
        }
    }

    return 0;
}

void Game::updateDisappear(){
    if(!disappearing)
        return;

    bool found = false;

    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            if(disappearTimers[i][j] <= 1)
                continue;
            disappearTimers[i][j]--;

            if(disappearTimers[i][j] == 1){
                found = true;
                toDisappear.push_back(std::make_tuple(j,i));
            }
        }
    }

    if(found)
        sounds.disappear = 1;
}

void Game::removeEventLock(){
    eventLock = false;
    inversion = false;
}

void Game::activateZone(int dir){
    if(!dir)
        return;

    if(zoneCharge < 8 && !DIAGNOSE)
        return;

    if(DIAGNOSE)
        zoneCharge = 32;

    fullZone = (zoneCharge == 32);

    previousClear.isTSpin = 0;
    previousClear.isBackToBack = 0;
    previousClear.isPerfectClear = 0;

    zoneTimer = (float) 37.5 * zoneCharge;

    zoneStart = zoneTimer;

    zoneCharge = 0;

    sounds.zone = 1;
}

void Game::endZone(){
    if(zonedLines > statTracker.maxZonedLines)
        statTracker.maxZonedLines = zonedLines;

    zoneTimer = 0;
    sounds.zone = -1;
    clear(Drop());
    eventLock = true;
}

void Game::clearBoard(){
    for(int i = 0; i < lengthY; i++){
        for(int j = 0; j < lengthX; j++){
            board[i][j] = 0;
        }
    }
}

void Game::fixConnected(std::list<int> sourceList){
    std::list<std::tuple<int,int>> fixList;

    for(auto const& line : sourceList){
        bool found = false;
        if(!fixList.empty()){
            int n = std::get<0>(fixList.back());
            if(n == line){
                fixList.pop_back();
                found = true;
            }else if (n == line-1){
                std::get<1>(fixList.back())++;
                found = true;
            }
        }

        if(!found && line > 0){
            fixList.push_back(std::make_tuple(line-1,0));
        }

        if(line < lengthY-1){
            fixList.push_back(std::make_tuple(line+1,1));
        }
    }

    for(auto const & line : fixList){
        int iy = std::get<0>(line);
        int type = std::get<1>(line);
        for(int ix = 0; ix < lengthX; ix++){
            int n = board[iy][ix];
            board[iy][ix] = (n & 0xf) + (GameInfo::connectedFix[type][n >> 4] << 4);
        }
    }
}

void Game::connectBoard(int startY,int endY){
    int ex = lengthX;
    int sy = startY;
    int ey = endY;

    for(int iy = startY; iy < endY+1; iy++){

        for(int ix = 0; ix < lengthX; ix++){
            if(!board[iy][ix] || (board[iy][ix] >> 4) != 0)
                continue;

            int n = getNbr(board, ix, iy, ex, sy, ey);

            if(n == 1){
                if(iy > sy && board[iy-1][ix])
                    board[iy][ix] += 1 << 4;
                else if(iy < ey && board[iy+1][ix])
                    board[iy][ix] += 2 << 4;
                else if(ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 3 << 4;
                else if(ix < ex && board[iy][ix+1])
                    board[iy][ix] += 4 << 4;
            }else if(n == 2){
                if(iy > sy && board[iy-1][ix] && iy < ey && board[iy+1][ix])
                    board[iy][ix] += 5 << 4;
                else if(ix > 0 && board[iy][ix-1] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 6 << 4;
                else if(iy < ey && board[iy+1][ix] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 7 << 4;
                else if(iy < ey && board[iy+1][ix] && ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 8 << 4;
                else if(iy > sy && board[iy-1][ix] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 9 << 4;
                else if(iy > sy && board[iy-1][ix] && ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 10 << 4;
            }else if(n == 3){
                if(iy < ey && board[iy+1][ix] && ix < ex && board[iy][ix+1] && board[iy+1][ix+1])
                    board[iy][ix] += 11 << 4;
                else if(iy < ey && board[iy+1][ix] && ix > 0 && board[iy][ix-1] && board[iy+1][ix-1])
                    board[iy][ix] += 12 << 4;
                else if(iy > sy && board[iy-1][ix] && ix < ex && board[iy][ix+1] && board[iy-1][ix+1])
                    board[iy][ix] += 13 << 4;
                else if(iy > sy && board[iy-1][ix] && ix > 0 && board[iy][ix-1] && board[iy-1][ix-1])
                    board[iy][ix] += 14 << 4;
                else if(iy > sy && board[iy-1][ix] && iy < ey && board[iy+1][ix] && ix < ex && board[iy][ix+1])
                    board[iy][ix] += 15 << 4;
                else if(ix > 0 && board[iy][ix-1] && ix < ex && board[iy][ix+1] && iy < ey && board[iy+1][ix])
                    board[iy][ix] += 16 << 4;
                else if(iy > sy && board[iy-1][ix] && iy < ey && board[iy+1][ix] && ix > 0 && board[iy][ix-1])
                    board[iy][ix] += 17 << 4;
                else if(ix > 0 && board[iy][ix-1] && ix < ex && board[iy][ix+1] && iy > sy && board[iy-1][ix])
                    board[iy][ix] += 18 << 4;
            }else if(n == 5){
                if(iy < ey && ix < ex && ix > 0 && board[iy+1][ix-1] && board[iy+1][ix+1])
                    board[iy][ix] += 19 << 4;
                else if(iy > sy && ix < ex && ix > 0 && board[iy-1][ix-1] && board[iy-1][ix+1])
                    board[iy][ix] += 20 << 4;
                else if(ix < ex && iy < ey && iy > sy && board[iy-1][ix+1] && board[iy+1][ix+1])
                    board[iy][ix] += 21 << 4;
                else if(ix > 0 && iy < ey && iy > sy && board[iy-1][ix-1] && board[iy+1][ix-1])
                    board[iy][ix] += 22 << 4;
            }else if(n == 8){
                board[iy][ix] += 23 << 4;
            }
        }
    }
}

void Game::liftKeys(){
    for(int i = 0; i < 3; i++)
        rotates[0] = false;

    holding = 0;
    left = 0;
    right = 0;
}

void Game::setOptions(Options newOptions){
    goal = newOptions.goal;
    setLevel(newOptions.level);
    setTuning(newOptions.tuning);

    trainingMode = newOptions.trainingMode;

    bTypeHeight = newOptions.bTypeHeight;
    setRotationSystem(newOptions.rotationSystem);
    setSubMode(newOptions.subMode);
}
