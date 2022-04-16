#include "tetramino.hpp" 
#include "tetrisgame.h"
#include <stdlib.h>
#include <iostream>
#include "tonc_core.h"


using namespace Tetris;

int Game::checkRotation(int dx, int dy, int r) {
    int i, j;
    int x = dx + pawn.x;
    int y = dy + pawn.y;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (pawn.board[r][i][j] == 0)
                continue;

            if (x + j < 0 || x + j > lengthX - 1 || y + i < 0 || y + i > lengthY - 1 || board[i + y][j + x] != 0) 
                return 0;
        }
    }
    return 1;
}

void Pawn::setBlock() {
    int i, j, k;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            for (k = 0; k < 4; k++)
                Pawn::board[i][j][k] = GameInfo::tetraminos[current][i][j][k];
}

void Game::rotateCW() {
    if (clearLock)
        return;
    lastMoveRotation = 1;
    int len = (pawn.current == 0) ? 7 : 6;
    for (int i = 0; i < len; i++) {
        int dx = GameInfo::kicks[(pawn.current == 0)][0][pawn.rotation][i][0];
        int dy = GameInfo::kicks[(pawn.current == 0)][0][pawn.rotation][i][1];
        int r = (pawn.rotation == 3) ? 0 : pawn.rotation + 1;
        if (checkRotation(dx, dy, r)) {
            pawn.rotation++;
            if (pawn.rotation > 3)
                pawn.rotation = 0;

            pawn.x += dx;
            pawn.y += dy;

            lockCheck();

            pawn.setBlock();
            sounds.rotate = 1;
            return;
        }else
			sounds.invalid = 1;
    }
}

void Game::rotateCCW() {
    if (clearLock)
        return;
    lastMoveRotation = 1;
    int len = (pawn.current == 0) ? 7 : 6;
    for (int i = 0; i < len; i++) {
        int dx = GameInfo::kicks[(pawn.current == 0)][1][pawn.rotation][i][0];
        int dy = GameInfo::kicks[(pawn.current == 0)][1][pawn.rotation][i][1];
        int r = (pawn.rotation == 0) ? 3 : pawn.rotation - 1;
        if (checkRotation(dx, dy, r)) {
            pawn.rotation--;
            if (pawn.rotation < 0)
                pawn.rotation = 3;

            pawn.x += dx;
            pawn.y += dy;

            lockCheck();

            pawn.setBlock();
            sounds.rotate = 1;
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
    }
}

void Game::moveRight() {
    if (checkRotation(1, 0, pawn.rotation)) {
        pawn.x++;
        sounds.shift = 1;
        lastMoveRotation = 0;
        lockCheck();
    }
}

void Game::moveDown() {
    if (checkRotation(0, 1, pawn.rotation))
        pawn.y++;
}

void Game::hardDrop() {
    if (clearLock)
        return;
    int diff = lowest() - pawn.y;
    pawn.y = lowest();
    place();
    score += diff * 2;
}

void Game::update() {
    if (lost)
        return;
    timer++;
    if (clearLock)
        return;
    int prevLevel = level;
    level = ((int)linesCleared / 10) + 1;

    if((linesCleared > 39 && gameMode == 1) || (linesCleared > 199 && gameMode == 2))
        won = 1;

    if (prevLevel != level && (gameMode == 0 || gameMode == 2))
        sounds.levelUp = 1;

    if (gameMode == 1)
        speed = GameInfo::speed[0];
    else
        speed = GameInfo::speed[(level < 19) ? level - 1 : 18];

    speedCounter += speed;

    int n = (int)speedCounter;
    for (int i = 0; i < n; i++) {
        moveDown();
    }
    speedCounter -= n;

    if (lowest() == pawn.y)
        lockTimer--;
    else if (lockMoveCounter > 0)
        lockTimer = maxLockTimer;

    if (lockTimer == 0)
        place();

    if (!(left || right))
        das = 0;
    else if (das < maxDas)
        das++;

    if (das == maxDas && !(left && right)) {
        arrCounter++;
        if (arrCounter == arr) {
            if (left)
                moveLeft();
            else
                moveRight();
            arrCounter = 0;
        }
    }

    if (down) {
        if (softDropCounter < maxSoftDrop)
            softDropCounter++;
    }
    else
        softDropCounter = 0;

    if (softDropCounter == maxSoftDrop) {
        for (int i = 0; i < softDropSpeed; i++) {
            moveDown();
            if (pawn.y != lowest())
                score++;
        }
    }
}

int Game::lowest() {
    for (int i = 0; i < lengthY - pawn.y; i++)
        if (!checkRotation(0, i, pawn.rotation))
            return i + pawn.y - 1;
    return pawn.y;
}

void Game::place() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            if (pawn.board[pawn.rotation][i][j] == 0)
                continue;

            int x = pawn.x + j;
            int y = pawn.y + i;

            if (y > lengthY - 1 || x > lengthX)
                continue;

            board[y][x] = pawn.current + 1;
        }
    }

    if (pawn.y < ((int)lengthY / 2 - 1)) {
        lost = 1;
        return;
    }

    if (clear())
        comboCounter++;
    else
        comboCounter = 0;

    if (!clearLock)
        next();

    canHold = 1;
    lockTimer = maxLockTimer;
    lockMoveCounter = 15;
    sounds.place = 1;
    refresh = 1;
}

int Game::clear() {
    int i, j;
    int clearCount = 0;
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
        if (frontCount == 2 && backCount > 0)
            isTSpin = 2;
        else if (frontCount > 0 && backCount == 2)
            isTSpin = 1;
    }

    for (i = 0; i < lengthY; i++) {
        int toClear = 1;
        for (j = 0; j < lengthX; j++)
            if (board[i][j] == 0)
                toClear = 0;

        if (toClear) {
            linesToClear.push_back(i);
            linesCleared++;
            clearCount++;
        }
    }

    if (linesCleared > 0) {
        std::cout << "perfect clear is : " << isPerfectClear << std::endl;
        for (j = 0; j < lengthY; j++) {
            int skip = 0;
            std::list<int>::iterator index = linesToClear.begin();
            for (i = 0; i < linesToClear.size(); i++) {
                std::cout << "index is: " << *index << std::endl;
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
                    std::cout << "Found at: " << j << std::endl;
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
        break;
    case 1:
        add += GameInfo::scoring[clearCount - 1 + 4][0] * level;
        isDifficult = GameInfo::scoring[clearCount - 1 + 4][1];
        break;
    case 2:
        add += GameInfo::scoring[clearCount - 1 + 7][0] * level;
        isDifficult = 1;
        break;
    }

    isBackToBack = previousClear.isDifficult && (isDifficult || isTSpin > 0);

    if (isBackToBack)
        add = (int)add * 3 / 2;

    add += GameInfo::scoring[16][0] * level * comboCounter;

    if (isPerfectClear)
        add += GameInfo::scoring[clearCount - 1 + 11][0] * level;

    score += add;

    if (clearCount > 0)
        previousClear = Score(clearCount, add, comboCounter, isTSpin, isPerfectClear, isBackToBack, isDifficult);

    sounds.clear = 1;
    clearLock = 1;
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
}

void Game::next() {
    pawn.y = (int)lengthY / 2 - 1;
    pawn.x = (int)lengthX / 2 - 2;
    pawn.rotation = 0;

    pawn.current = queue.front();
    queue.pop_front();

    fillQueue(1);
    pawn.setBlock();

    if (!checkRotation(0, 0, pawn.rotation))
        lost = 1;
}

void Game::fillQueue(int count) {
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
    canHold = 0;
    sounds.hold = 1;
}

int** Game::getShape(int n) {
    int** result = 0;    
    result = new int*[4];

    int i, j;
    for (i = 0; i < 4; i++){
        result[i] = new int[4];
        for (j = 0; j < 4; j++){
            result[i][j] = GameInfo::tetraminos[n][0][i][j]; 
        }
    }

    return result;
}

void Game::lockCheck() {
    if (lowest() == pawn.y) {
        if (lockMoveCounter > 0) {
            lockMoveCounter--;
            lockTimer = maxLockTimer;
        }
    }
}

void Game::keyLeft(int dir) {
    if (clearLock) {
		left = dir;
        return;
    }

    if (left == 0)
        moveLeft();
    left = dir;
}

void Game::keyRight(int dir) {
    if (clearLock) {
        right = dir;
        return;
    }

    if (right == 0)
        moveRight();
    right = dir;
}

void Game::keyDown(int dir) {
    if (clearLock) {
        down = dir;
        return;
    }

    if (down == 0)
        moveDown();
    down = dir;
}

void Game::removeClearLock() {
    clearLock = 0;
    std::list<int>::iterator index = linesToClear.begin();

    for (int i = 0; i < linesToClear.size(); i++) {
        for (int j = *index; j > 0; j--)
            for (int k = 0; k < lengthX; k++)
                board[j][k] = board[j - 1][k];
        std::advance(index, 1);
    }

    linesToClear = std::list<int>();
    next();
    refresh = 1;
}

void Game::resetSounds(){
    sounds = SoundFlags();
}

void Game::resetRefresh() {
    refresh = 0;
}