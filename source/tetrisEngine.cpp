#include "tetromino.hpp"
#include "tetrisEngine.h"
#include <stdlib.h>
#include <iostream>
#include "tonc.h"


using namespace Tetris;

int Game::checkRotation(int dx, int dy, int r) {
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

            if (x + j < 0 || x + j > lengthX - 1 || y + i < 0 || y + i > lengthY - 1 || board[i + y][j + x] != 0) 
                return 0;
            
            if(total >= 4)
                break;
        }
    }
    return 1;
}

void Pawn::setBlock() {
    for (int i= 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                Pawn::board[i][j][k] = GameInfo::tetraminos[current][i][j][k];
}

void Game::rotateCW() {
    if (clearLock)
        return;

    moveCounter++;
    lastMoveRotation = 1;
    for (int i = 0; i < 5; i++) {
        int dx = GameInfo::kicks[(pawn.current == 0)][0][pawn.rotation][i][0];
        int dy = GameInfo::kicks[(pawn.current == 0)][0][pawn.rotation][i][1];
        int r = (pawn.rotation == 3) ? 0 : pawn.rotation + 1;
        
        if (checkRotation(dx, dy, r)) {
            pawn.rotation = r;

            lockCheck();
            pawn.x += dx;
            pawn.y += dy;

            pawn.setBlock();
            sounds.rotate = 1;

            if(i == 4 && pawn.current == 5)
                specialTspin = true;
            
            moveHistory.push_back(4);

            return;
        }else
			sounds.invalid = 1;
    }
}

void Game::rotateCCW() {
    if (clearLock)
        return;

    moveCounter++;
    lastMoveRotation = 1;
    for (int i = 0; i < 5; i++) {
        int dx = GameInfo::kicks[(pawn.current == 0)][1][pawn.rotation][i][0];
        int dy = GameInfo::kicks[(pawn.current == 0)][1][pawn.rotation][i][1];
        int r = (pawn.rotation == 0) ? 3 : pawn.rotation - 1;
        
        if (checkRotation(dx, dy, r)) {
            pawn.rotation = r;

            lockCheck();
            pawn.x += dx;
            pawn.y += dy;

            pawn.setBlock();
            sounds.rotate = 1;
            
            if(i == 4 && pawn.current == 5)
                specialTspin = true;
            
            moveHistory.push_back(5);

            return;
        }else
			sounds.invalid = 1;
    }
}

void Game::rotateTwice() {
    if (clearLock)
        return;

    moveCounter++;
    lastMoveRotation = 1;
    for (int i = 0; i < 6; i++) {
        int dx = GameInfo::kickTwice[pawn.rotation][i][0];
        int dy = GameInfo::kickTwice[pawn.rotation][i][1];
        int r = pawn.rotation + 2;
        if(r > 3)
            r -= 4;

        if (checkRotation(dx, dy, r)) {
            pawn.rotation = r;

            lockCheck();
            pawn.x += dx;
            pawn.y += dy;

            pawn.setBlock();
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
        finesse += difference;

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

    if(pawn.current == -1)
        next();

    if(dropLockTimer)
        dropLockTimer--;

    if (clearLock)
        return;
    
    if(gameMode == 4){
        auto index = garbageQueue.begin();

        while(index != garbageQueue.end()){
            if(index->timer){
                index->timer--;
            }
            ++index;
        }
    }

    if(gameMode == 8){
        int rate = 60 * (4-goal);
        if(timer % rate == 0){
            generateGarbage(1,0);
            refresh = 1;

            while(!checkRotation(0,0,pawn.rotation))
                pawn.y--;
        }
    }

    int prevLevel = level;
    if(gameMode == 0 || gameMode == 2){
        level = ((int)linesCleared / 10) + 1;
        if(level < prevLevel)
            level = prevLevel;
    }else if(gameMode == 6){
        for(int i = 0; i < 15; i++){
            if(linesCleared < GameInfo::blitzLevels[i]){
                level = i+1;
                break;
            }
        }
        if(level < prevLevel)
            level = prevLevel;
    }

    if((linesCleared >= goal && gameMode == 1 && goal) ||
       (linesCleared >= goal && gameMode == 2) ||
       (garbageCleared >= goal && gameMode == 3) ||
       (timer > goal && (gameMode == 5 || gameMode == 6))){
        won = 1;
    }

    if (prevLevel != level && (gameMode == 0 || gameMode == 2 || gameMode == 6))
        sounds.levelUp = 1;

    if (gameMode == 0 || gameMode == 2)
        speed = GameInfo::speed[(level < 19) ? level - 1 : 18];
    else if(gameMode == 6)
        speed = GameInfo::speed[(level < 15)? level-1 : 14];
    else
        speed = GameInfo::speed[0];

    speedCounter += speed;

    int n = (int)speedCounter;
    for (int i = 0; i < n; i++) {
        if (checkRotation(0, 1, pawn.rotation)){
            pawn.y++;
            // lockTimer = maxLockTimer;
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

        if (lockTimer == 0)
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
        if (softDropCounter < maxDas)
            softDropCounter++;
    }
    else
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

    for(int i = 0; i < lengthY - pawn.y; i++){
        for(int j = 0; j < 4; j++){
            if(blocks[j] == -1)
                continue;
            int x = pawn.x+j;
            int y = pawn.y+i;
            if(y+blocks[j] >= lengthY || board[y+blocks[j]][x])
                return y-1;
        }
    }
    
    return pawn.y;
}

void Game::place() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (pawn.board[pawn.rotation][i][j] == 0)
                continue;

            int x = pawn.x + j;
            int y = pawn.y + i;

            if (y > lengthY - 1 || x > lengthX)
                continue;

            board[y][x] = pawn.current + 1;
        }
    }

    int lowestPart = 0;

    for(int i = 3; i >= 0; i--)//find height of lowest block in column
        for(int j = 0; j < 4; j++)
            if(pawn.board[pawn.rotation][i][j]){
                lowestPart = i;
                break;
            }

    // if (pawn.y < ((int)lengthY / 2 - 2)) {
    if (pawn.y+lowestPart < ((int) lengthY / 2 - 2)) {
        lost = 1;
        return;
    }

    lastDrop = calculateDrop();

    if (clear(lastDrop)){
        comboCounter++;

        if(gameMode == 7){
            for(int i = lengthY/2-1; i < lengthY; i++){
                if(board[i][0] != 0)
                    break;
                for(int j = 0; j < 10; j++){
                    if(j > 2 && j < 7)
                        continue;
                    board[i][j] = (i+comboCounter) % 7 + 1;
                }
            }
        }

    } else{
        if(gameMode == 7 && comboCounter)
            lost = 1;
        comboCounter = 0;

        if(gameMode == 3 && garbageHeight < 9){
            int toAdd = 9-garbageHeight;
            if(garbageCleared+toAdd+garbageHeight <= goal)
                generateGarbage(9-garbageHeight,0);
            else if(garbageCleared+toAdd+garbageHeight > goal && garbageCleared+garbageHeight < goal)
                generateGarbage(goal-(garbageCleared+garbageHeight),0);
        }else if(gameMode == 4){
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

    if (!clearLock)
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

    if (pawn.current == 5 && lastMoveRotation) {
        int frontCount = 0;
        int backCount = 0;
        int x = pawn.x;
        int y = pawn.y;

        switch (pawn.rotation) {
        case 0:
            frontCount += (board[y][x] != 0) + (board[y][x + 2] != 0);
            if (y + 2 > lengthY - 1)
                backCount = 2;
            else
                backCount += (board[y + 2][x] != 0) + (board[y + 2][x + 2] != 0);
            break;
        case 1:
            frontCount += (board[y][x + 2] != 0) + (board[y + 2][x + 2] != 0);
            if (x < 0)
                backCount = 2;
            else
                backCount += (board[y][x] != 0) + (board[y + 2][x] != 0);
            break;
        case 2:
            frontCount += (board[y + 2][x] != 0) + (board[y + 2][x + 2] != 0);
            backCount += (board[y][x] != 0) + (board[y][x + 2]);
            break;
        case 3:
            frontCount += (board[y][x] != 0) + (board[y + 2][x] != 0);
            if (x > lengthX - 1)
                backCount = 2;
            else
                backCount += (board[y][x + 2] != 0) + (board[y + 2][x + 2] != 0);
            break;
        }
        if ((frontCount == 2 && backCount > 0) || (frontCount > 0 && backCount == 2 && specialTspin))
            isTSpin = 2;
        else if (frontCount > 0 && backCount == 2)
            isTSpin = 1;
    }

    int garbageToRemove = 0;

    for (i = 0; i < lengthY; i++) {
        int toClear = 1;
        for (j = 0; j < lengthX; j++)
            if (board[i][j] == 0)
                toClear = 0;

        if (toClear) {
            linesToClear.push_back(i);
            linesCleared++;
            clearCount++;
            if(gameMode == 3 && i >= lengthY-garbageHeight)
                garbageToRemove++;
        }
    }

    garbageHeight-=garbageToRemove;
    garbageCleared+=garbageToRemove;

    if (linesCleared > 0) {
        for (j = 0; j < lengthY; j++) {
            int skip = 0;
            std::list<int>::iterator index = linesToClear.begin();
            for (i = 0; i < (int)linesToClear.size(); i++) {
                if (j == *index) {
                    skip = 1;
                    break;
                }
                std::advance(index, 1);
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
    
    int add = 0;

    switch (isTSpin) {
    case 0:
        add += GameInfo::scoring[clearCount - 1][0] * level;
        isDifficult = GameInfo::scoring[clearCount - 1][1];
        attack = GameInfo::scoring[clearCount - 1][2];

        statTracker[clearCount-1]++;
        break;
    case 1:
        add += GameInfo::scoring[clearCount + 4][0] * level;
        isDifficult = GameInfo::scoring[clearCount + 4][1];
        attack = GameInfo::scoring[clearCount + 4][2];
        statTracker[4]++;
        break;
    case 2:
        add += GameInfo::scoring[clearCount  + 7][0] * level;
        attack = GameInfo::scoring[clearCount + 7][2];
        isDifficult = true;
        statTracker[4]++;
        break;
    }

    isBackToBack = previousClear.isDifficult && (clearCount == 4 || isTSpin == 2);

    if (isBackToBack){
        add = (int)add * 3 / 2;
        b2bCounter++;
        attack+= 1;
    }else if (!isDifficult){
        b2bCounter = 0;
    }

    if(b2bCounter > statTracker[6])
        statTracker[6] = b2bCounter;

    add += GameInfo::scoring[16][0] * level * comboCounter;

    attack += GameInfo::comboTable[comboCounter];

    if (isPerfectClear){
        if(gameMode != 6)
            add += GameInfo::scoring[clearCount - 1 + 11][0] * level;
        else
            add += 3500 * level;
        attack = GameInfo::scoring[clearCount - 1 + 11][2];

        statTracker[5]++;
    }

    score += add;

    if (clearCount > 0)
        previousClear = Score(clearCount, add, comboCounter, isTSpin, isPerfectClear, isBackToBack, isDifficult, drop);

    sounds.clear = 1;
    clearLock = 1;
    specialTspin = false;

    if(garbageQueue.size()){
        std::list<Garbage>::iterator index = garbageQueue.begin();
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
    result.r = GameInfo::colors[n][0];
    result.g = GameInfo::colors[n][1];
    result.b = GameInfo::colors[n][2];

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
    pawn.rotation = 0;

    pawn.current = queue.front();
    queue.pop_front();

    fillQueue(1);

    pawn.setBlock();

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

    if (check || !checkRotation(0, 0, pawn.rotation)){
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

    int i;
    for (i = 0; i < count; i++) {
        int n = qran() % bag.size();

        std::list<int>::iterator index = bag.begin();
        std::advance(index, n);

        queue.push_back(*index);
        bag.erase(index);

        if (bag.size() == 0)
            fillBag();
    }
}

void Game::hold() {
    if (!canHold || clearLock)
        return;

    moveCounter++;

    if (held == -1) {
        held = pawn.current;
        next();
    }
    else {
        int temp = held;
        held = pawn.current;
        pawn.current = temp;
        pawn.setBlock();
        pawn.x = (int)lengthX / 2 - 2;
		pawn.y = (int)lengthY / 2 - 1;
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

    int i, j;
    for (i = 0; i < 4; i++){
        result[i] = new int[4];
        for (j = 0; j < 4; j++){
            result[i][j] = GameInfo::tetraminos[n][r][i][j]; 
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
    if (clearLock) {
		left = dir;
        return;
    }

    if (left == 0)
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
    if (clearLock) {
        right = dir;
        return;
    }

    if (right == 0)
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
    if (clearLock) {
        down = dir;
        return;
    }

    if (down == 0){
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
}

void Game::keyDrop(){
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

    result.startY = pawn.y - 20;
    
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
    
    result.endY = pawn.y - 20 + add;

    if((pawn.current == 2 && pawn.rotation == 3) || (pawn.current == 1 && pawn.rotation == 1))
        result.endY--;

    return result;
}

void Game::setTuning(int newDas, int newArr, int newSfr, int newDropProtection, bool directionalDas){
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
        result.push_back(GameInfo::finesse[piece][r][dx][i]);
        if(GameInfo::finesse[piece][r][dx][i] == 7)
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
