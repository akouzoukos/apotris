#include "tetromino.hpp"
#include "tetrisEngine.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include "tonc.h"
#include "logging.h"

using namespace Tetris;
using namespace GameInfo;

int Game::checkRotation(int dx, int dy, int r) {
    // int x = (dx + pawn.x) * ((pawn.big)?2:1);
    // int y = (dy + pawn.y) * ((pawn.big)?2:1);

    int x = dx + pawn.x;
    int y = dy + pawn.y;

    int total = 0;
    for (int i = 0; i < 4; i++) {
        if(total >= 4)
            break;


        for (int j = 0; j < 4; j++) {
            if (pawn.board[r][i][j] == 0)
                continue;
            
            total++;

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

            if(total >= 4)
                break;
        }
    }
    return 1;
}

void Pawn::setBlock(bool alt) {
    for (int i= 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            for (int k = 0; k < 4; k++){
                if(!alt)
                    board[i][j][k] = tetraminos[current][i][j][k];
                else
                    board[i][j][k] = classic[current][i][j][k];
            }
        }
    }
}

void Game::rotateCW() {
    if (clearLock || entryDelay)
        return;

    moveCounter++;
    lastMoveRotation = 1;
    for (int i = 0; i < 5; i++) {
        int dx = kicks[(pawn.current == 0)][0][pawn.rotation][i][0];
        int dy = kicks[(pawn.current == 0)][0][pawn.rotation][i][1];
        int r = (pawn.rotation == 3) ? 0 : pawn.rotation + 1;
        
        if (checkRotation(dx, dy, r)) {
            pawn.rotation = r;

            lockCheck();
            pawn.x += dx;
            pawn.y += dy;

            pawn.setBlock(gameMode == CLASSIC);
            sounds.rotate = 1;

            if(i == 4 && pawn.current == 5)
                specialTspin = true;
            
            moveHistory.push_back(4);

            return;
        }else
			sounds.invalid = 1;

        if(gameMode == CLASSIC)
            break;
    }
}

void Game::rotateCCW() {
    if (clearLock || entryDelay)
        return;

    moveCounter++;
    lastMoveRotation = 1;
    for (int i = 0; i < 5; i++) {
        int dx = kicks[(pawn.current == 0)][1][pawn.rotation][i][0];
        int dy = kicks[(pawn.current == 0)][1][pawn.rotation][i][1];
        int r = (pawn.rotation == 0) ? 3 : pawn.rotation - 1;
        
        if (checkRotation(dx, dy, r)) {
            pawn.rotation = r;

            lockCheck();
            pawn.x += dx;
            pawn.y += dy;

            pawn.setBlock(gameMode == CLASSIC);
            sounds.rotate = 1;
            
            if(i == 4 && pawn.current == 5)
                specialTspin = true;
            
            moveHistory.push_back(5);

            return;
        }else
			sounds.invalid = 1;

        if(gameMode == CLASSIC)
            break;
    }
}

void Game::rotateTwice() {
    if (clearLock || gameMode == CLASSIC)
        return;

    moveCounter++;
    lastMoveRotation = 1;
    for (int i = 0; i < 6; i++) {
        int dx = kickTwice[pawn.rotation][i][0];
        int dy = kickTwice[pawn.rotation][i][1];
        int r = pawn.rotation + 2;
        if(r > 3)
            r -= 4;

        if (checkRotation(dx, dy, r)) {
            pawn.rotation = r;

            lockCheck();
            pawn.x += dx;
            pawn.y += dy;

            pawn.setBlock(gameMode == CLASSIC);
            sounds.rotate = 1;

            moveHistory.push_back(7);

            return;
        }else
			sounds.invalid = 1;
    }
}

void Game::moveLeft() {
    if (checkRotation(-1, 0, pawn.rotation)) {
        pawn.x--;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
    }else{
        pushDir = -1;
    }
}

void Game::moveRight() {
    if (checkRotation(1, 0, pawn.rotation)) {
        pawn.x++;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
    }else{
        pushDir = 1;
    }
}

void Game::moveDown() {
    if (checkRotation(0, 1, pawn.rotation)){
        pawn.y++;
        sounds.shift = 1;
        // lockTimer = maxLockTimer;
    }else if(gameMode == CLASSIC){
        place();
    }
}

void Game::hardDrop() {
    if (clearLock || dropLockTimer)
        return;

    int diff = pawn.lowest - pawn.y;

    if(diff)
        lastMoveRotation = 0;

    int offset = 0;
    for(int j = 0; j < 4; j++){
        bool escape = false;
        for(int i = 0; i < 4; i++){
            if(pawn.board[pawn.rotation][i][j] == 1){
                offset = j;
                escape = true;
                break;
            }
        }
        if(escape)
            break;
    }

    std::list<int> bestFinesse = getBestFinesse(pawn.current,pawn.x+offset,pawn.rotation);
    previousBest = bestFinesse;//for debugging

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
    }

    pawn.y = pawn.lowest;

    place();

    score += diff * 2;

    return;
}

void Game::update() {
    if (lost)
        return;
    timer++;

    if(pawn.current == -1 && !entryDelay)
        next();

    if(dropLockTimer)
        dropLockTimer--;

    if (clearLock)
        return;

    if (entryDelay){
        entryDelay--;
        return;
    }
    
    if(gameMode == DIG){
        auto index = garbageQueue.begin();

        while(index != garbageQueue.end()){
            if(index->timer){
                index->timer--;
            }
            ++index;
        }
    }

    if(gameMode == SURVIVAL){
        int rate = 60 * (4-goal);
        if(timer % rate == 0){
            generateGarbage(1,0);
            refresh = 1;

            while(!checkRotation(0,0,pawn.rotation))
                pawn.y--;
        }
    }

    if((linesCleared >= goal && gameMode == SPRINT && goal) ||
       (linesCleared >= goal && gameMode == MARATHON) ||
       (garbageCleared >= goal && gameMode == DIG) ||
       (timer > goal && (gameMode == ULTRA || gameMode == BLITZ))){
        won = 1;
    }

    if (gameMode == MARATHON)
        speed = gravity[(level < 19) ? level - 1 : 18];
    else if(gameMode == BLITZ)
        speed = gravity[(level < 15) ? level - 1 : 14];
    else if(gameMode == CLASSIC)
        speed = gravity[(level < 30) ? level: 29];
    else
        speed = gravity[0];

    speedCounter += speed;

    int n = (int)speedCounter;
    for (int i = 0; i < n; i++) {
        if (checkRotation(0, 1, pawn.rotation)){
            pawn.y++;
            // lockTimer = maxLockTimer;
        }else if(gameMode == CLASSIC){
            place();
        }
    }

    speedCounter -= n;

    pawn.lowest = lowest();

    if(dropping){
        hardDrop();
        dropping = false;
        return;
    }else{
        if (pawn.lowest == pawn.y)
            lockTimer--;

        if (lockTimer == 0 && gameMode != CLASSIC)
            place();
    }

    if (!(left || right)){
        das = 0;
    }else if (das < maxDas){
        das++;
    }

    if (das == maxDas && !(left && right)) {
        if (++arrCounter >= arr) {
            for(int i = 0; i < 1 + (arr == 0); i++){//move piece twice if arr is 0
                if (left)
                    moveLeft();
                else
                    moveRight();
            }
            arrCounter = 0;
        }
    }

    if (down) {
        if(gameMode == CLASSIC)
            softDropCounter = maxDas;
        else if (softDropCounter < maxDas)
            softDropCounter++;
    }else
        softDropCounter = 0;

    if (softDropCounter == maxDas) {
        if(++softDropRepeatTimer >= softDropSpeed){
            for(int i = 0; i < 1 + (softDropSpeed ==0); i++) // move piece twice if softDropSpeed is 0
                moveDown();
            if (pawn.y != pawn.lowest)
                score++;
            softDropRepeatTimer = 0;
        }
    }
}

int Game::lowest() {

    int blocks[4];

    for(int i = 0; i < 4; i++)//initialize height array
        blocks[i] = -1;

    for(int i = 3; i >= 0; i--)//find height of lowest block in column
        for(int j = 0; j < 4; j++)
            if(pawn.board[pawn.rotation][i][j] && blocks[j] == -1)
                blocks[j] = i;

    if(!pawn.big){
        for(int i = 0; i < lengthY - pawn.y; i++){
            for(int j = 0; j < 4; j++){
                if(blocks[j] == -1)
                    continue;
                int x = pawn.x+j;
                int y = pawn.y+i+blocks[j];
                if(y >= lengthY || board[y][x])
                    return pawn.y+i-1;
            }
        }
    }else{
        for(int i = 0; i < (lengthY - pawn.y)*2; i++){
            for(int j = 0; j < 4; j++){
                if(blocks[j] == -1)
                    continue;
                int x = (pawn.x+j)*2;
                int y = (pawn.y+i+blocks[j])*2;
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

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (pawn.board[pawn.rotation][i][j] == 0)
                continue;

            int x = pawn.x + j;
            int y = pawn.y + i;

            if(!pawn.big){
                if (y > lengthY - 1 || x > lengthX)
                    continue;

                board[y][x] = pawn.current + 1;
            }else{
                x *=2;
                y *=2;

                for(int k = 0; k < 2; k++){
                    for(int l = 0; l < 2; l++){
                        if (y+k > lengthY - 1 || x+l > lengthX)
                            continue;

                        board[y+k][x+l] = pawn.current + 1;

                    }
                }
            }
        }
    }

    int lowestPart = 0;

    for(int i = 3; i >= 0; i--)//find height of lowest block in column
        for(int j = 0; j < 4; j++)
            if(pawn.board[pawn.rotation][i][j]){
                lowestPart = i;
                break;
            }

    if ((pawn.y+lowestPart) * (1+pawn.big) < (lengthY / 2 - 2)) {
        lost = 1;
        return;
    }

    lastDrop = calculateDrop();

    if (clear(lastDrop)){
        if(gameMode != CLASSIC)
            comboCounter++;
        else
            comboCounter = 1;

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

    } else{
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
    }


    if(comboCounter > statTracker[7])
        statTracker[7] = comboCounter;

    if(gameMode == CLASSIC){
        int add = 0;
        while(pawn.y + add < lengthY-2)
            add+=4;

        entryDelay = 10 + add;

        pawn.current = -1;

        down = 0;
        left = 0;
        right = 0;
    }

    if (!clearLock && !entryDelay)
        next();

    canHold = true;
    lockTimer = maxLockTimer;
    lockMoveCounter = 15;
    sounds.place = 1;
    refresh = 1;
    dropLockTimer = dropLockMax;

    pushDir = 0;
    previousKey = 0;
    pieceCounter++;

    moveHistory.clear();

    if(das == maxDas)
        moveHistory.push_back(2+(right));
}

int Game::clear(Drop drop) {
    int i, j;
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
    for (i = 0; i < lengthY; i++) {
        int toClear = 1;
        for (j = 0; j < lengthX; j++)
            if (board[i][j] == 0)
                toClear = 0;

        if (toClear) {
            linesToClear.push_back(i);
            clearCount++;
            if(gameMode == DIG && i >= lengthY-(garbageHeight*(1+pawn.big)))
                garbageToRemove++;
        }
    }

    if(pawn.big)
        garbageToRemove/=2;

    garbageHeight-=garbageToRemove;
    garbageCleared+=garbageToRemove;

    if (clearCount > 0) {
        if(pawn.big)
            clearCount/=2;

        linesCleared+=clearCount;

        for (j = 0; j < lengthY; j++) {
            bool skip = false;
            auto index = linesToClear.begin();
            for (i = 0; i < (int)linesToClear.size(); i++) {
                if (j == *index) {
                    skip = true;
                    break;
                }
                ++index;
            }
            if (skip)
                continue;
            for (i = 0; i < lengthX; i++) {
                if (board[j][i]) {
                    isPerfectClear = 0;
                }
            }
        }
    }

    if (clearCount == 0)
        return 0;

    //check for level up
    int prevLevel = level;
    if(gameMode == MARATHON){
        level = ((int)linesCleared / 10) + 1;
        if(level < prevLevel)
            level = prevLevel;
    }else if(gameMode == BLITZ){
        for(int i = 0; i < 15; i++){
            if(linesCleared < blitzLevels[i]){
                level = i+1;
                break;
            }
        }
        if(level < prevLevel)
            level = prevLevel;
    }else if(gameMode == CLASSIC){
        level = ((int)linesCleared / 10);
        if(level < prevLevel)
            level = prevLevel;
    }

    if (prevLevel != level && (gameMode == MARATHON || gameMode == BLITZ || gameMode == CLASSIC))
        sounds.levelUp = 1;

    //calculate score
    int add = 0;
    switch (isTSpin) {
    case 0:
        //if no t-spin
        if(gameMode != CLASSIC){
            add += scoring[clearCount - 1][0] * prevLevel;
            isDifficult = scoring[clearCount - 1][1];
            attack = scoring[clearCount - 1][2];
        } else
            add += classicScoring[clearCount-1] * (level+1);

        statTracker[clearCount-1]++;
        break;
    case 1:
        //if t-spin mini
        add += scoring[clearCount + 4][0] * prevLevel;
        isDifficult = scoring[clearCount + 4][1];
        attack = scoring[clearCount + 4][2];
        statTracker[4]++;
        break;
    case 2:
        //if t-spin
        add += scoring[clearCount  + 7][0] * prevLevel;
        attack = scoring[clearCount + 7][2];
        isDifficult = true;
        statTracker[4]++;
        break;
    }

    isBackToBack = previousClear.isDifficult && (clearCount == 4 || isTSpin == 2) && gameMode != CLASSIC;

    if (isBackToBack){
        add = (int)add * 3 / 2;
        b2bCounter++;
        attack+= 1;
    }else if (!isDifficult){
        b2bCounter = 0;
    }

    if(b2bCounter > statTracker[6])
        statTracker[6] = b2bCounter;

    //add combo bonus
    add += scoring[16][0] * level * comboCounter;
    attack += comboTable[comboCounter];

    if (isPerfectClear && gameMode != CLASSIC){
        if(gameMode != BLITZ)
            add += scoring[clearCount - 1 + 11][0] * level;
        else
            add += 3500 * level;
        attack = scoring[clearCount - 1 + 11][2];

        statTracker[5]++;
    }

    score += add;

    previousClear = Score(clearCount, add, comboCounter, isTSpin, isPerfectClear, isBackToBack, isDifficult, drop);

    sounds.clear = 1;
    clearLock = 1;
    specialTspin = false;

    if(garbageQueue.size()){
        auto index = garbageQueue.begin();
        while(attack > 0 && index != garbageQueue.end()){
            if(index->timer == 0){
                if(attack > index->amount){
                    attack -= index->amount;
                }else{
                    index->amount -= attack;
                    attack = 0;
                }
            }
            ++index;
        }
    }

    if(attack){
        attackQueue.push_back(Garbage(timer & 0xff,attack));
        linesSent+=attack;
    }
    
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
    for (int i = 0; i < 7; i++)
        bag.push_back(i);
    bagCounter++;
}

void Game::next() {
    pawn.y = (int)lengthY / 2;
    pawn.x = (int)lengthX / 2 - 2;

    if(pawn.big){
        pawn.x/=2;
        pawn.y/=2;
    }

    pawn.rotation = 0;

    pawn.current = queue.front();
    queue.pop_front();

    pieceHistory = pawn.current;

    fillQueue(1);

    pawn.setBlock(gameMode == CLASSIC);

    if(gameMode == CLASSIC)
        down = 0;

    //check if stack has reached top 3 lines
    bool check = false;
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 10; j++){
            if(board[21+i][j] != 0){
                check = true;
                break;
            }
        }
        if(check)
            break;
    }

    if (check || !checkRotation(0, 0, pawn.rotation) || gameMode == CLASSIC){
        pawn.y-=1;
        if (!checkRotation(0, 0, pawn.rotation))
            lost = 1;
    }

    softDrop = false;
}

void Game::fillQueue(int count) {
    if(seed != 0){
        sqran(seed);
        seed+= qran();
    }

    for (int i = 0; i < count; i++) {
        if(gameMode != CLASSIC){
            int n = qran() % bag.size();

            std::list<int>::iterator index = bag.begin();
            std::advance(index, n);

            queue.push_back(*index);
            bag.erase(index);

            if (bag.size() == 0)
                fillBag();
        }else{
            int n = qran() % 8;

            if(n == 7 || n == pieceHistory)
                n = qran() % 7;

            queue.push_back(n);
        }
    }
}

void Game::hold() {
    if (!canHold || clearLock || gameMode == CLASSIC)
        return;

    moveCounter++;

    if (held == -1) {
        held = pawn.current;
        next();
    } else {
        int temp = held;
        held = pawn.current;
        pawn.current = temp;
        pawn.setBlock(gameMode == CLASSIC);
        pawn.x = (int)lengthX / 2 - 2;
		pawn.y = (int)lengthY / 2 - 1;

        if(pawn.big){
            pawn.x/=2;
            pawn.y/=2;
        }
    }

    pawn.rotation = 0;
    canHold = false;
    sounds.hold = 1;
    holdCounter++;

    lockTimer = maxLockTimer;
    lockMoveCounter = 15;

    moveHistory.clear();

    if(das == maxDas)
        moveHistory.push_back(2+(right));
}

int** Game::getShape(int n,int r) {
    int** result = 0;    
    result = new int*[4];

    for (int i = 0; i < 4; i++){
        result[i] = new int[4];
        for (int j = 0; j < 4; j++){
            if(gameMode != CLASSIC)
                result[i][j] = tetraminos[n][r][i][j];
            else
                result[i][j] = classic[n][r][i][j];
        }
    }

    return result;
}

void Game::lockCheck() {
    if (pawn.lowest == pawn.y && lockMoveCounter > 0) {
        lockMoveCounter--;
        lockTimer = maxLockTimer;
    }
}

void Game::keyLeft(int dir) {
    moveCounter++;
    if (clearLock || entryDelay || (gameMode == CLASSIC && down)) {
        if(!entryDelay)
            left = dir;
        return;
    }

    if (left == 0 && dir)
        moveLeft();
    left = dir;
    
    previousKey = -1;

    if(!dir){
        pushDir = 0;

        if(das == maxDas)
            moveHistory.push_back(2);
        else
            moveHistory.push_back(0);

        if(directionCancel)
            das = 0;
    }
}

void Game::keyRight(int dir) {
    moveCounter++;
    if (clearLock || entryDelay || (gameMode == CLASSIC && down)) {
        if(!entryDelay)
            right = dir;
        return;
    }

    if (right == 0 && dir)
        moveRight();
    right = dir;
    
    previousKey = 1;

    if(!dir){
        pushDir = 0;

        if(das == maxDas)
            moveHistory.push_back(3);
        else
            moveHistory.push_back(1);

        if(directionCancel)
            das = 0;
    }
}

void Game::keyDown(int dir) {
    moveCounter++;
    if (clearLock || entryDelay) {
        if(!entryDelay)
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
}

void Game::removeClearLock() {
    if(!clearLock)
        return;
    clearLock = 0;
    std::list<int>::iterator index = linesToClear.begin();

    for (int i = 0; i < (int)linesToClear.size(); i++) {
        for (int j = *index; j > 0; j--)
            for (int k = 0; k < lengthX; k++)
                board[j][k] = board[j - 1][k];
        std::advance(index, 1);
    }

    linesToClear = std::list<int>();

    if(!entryDelay)
        next();
    refresh = 1;

    // setCurrentHeight(); 
}

void Game::resetSounds(){
    sounds = SoundFlags();
}

void Game::resetRefresh() {
    refresh = 0;
}

void Game::setLevel(int newLevel){
    level = newLevel;
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
            if(!mode || qran() % 10 < 3){
                do{
                    hole = qran() % lengthX;
                }while((!board[i-1][hole] && height < garbageHeight) || hole == prevHole);
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
}

void Game::keyDrop(){
    if(gameMode == CLASSIC)
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
    result.startX = pawn.x;

    int start = -1;
    int end = 4;
    for(int i = 0; i < 4; i++){
        bool found = false;
        for(int j = 0; j < 4; j++){
            if(pawn.board[pawn.rotation][j][i] == 1){
                found = true;
                break;
            }
        }

        if(found && start == -1)
            start = i;
        else if(!found && start != -1 && end == 4)
            end = i;
    }

    result.startX += start;
    result.endX = result.startX + end - start;

    result.startY = pawn.y;
    
    int add = 0;
    for(int i = 3; i >=0; i--){
        bool found = false;
        for(int j = 0; j < 4; j++){
            if(pawn.board[pawn.rotation][i][j] == 1){
                found = true;
                break;
            }
        }
        if(found){
            add = i;
            break;
        }
    }
    
    result.endY = pawn.y + add;

    if(gameMode != CLASSIC){
        if(((pawn.current == 2 && pawn.rotation == 3) || (pawn.current == 1 && pawn.rotation == 1)))
            result.endY--;
    }else{
        if(((pawn.current == 2 && pawn.rotation == 1) || (pawn.current == 1 && pawn.rotation == 3)))
            result.endY--;
    }


    if(pawn.big){
        result.startX *=2;
        result.endX *=2;
        result.startY *=2;
        result.endY *=2;
    }

    result.startY -= 20;
    result.endY -= 20;

    return result;
}

void Game::setTuning(int newDas, int newArr, int newSfr, int newDropProtection, bool directionalDas){
    if(gameMode == CLASSIC)
        return;

    maxDas = newDas;
    arr = newArr;
    softDropSpeed = newSfr;
    dropLockMax = newDropProtection;
    directionCancel = directionalDas;
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
