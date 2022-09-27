#include "def.h"
#include "tonc.h"
#include "LinkConnection.h"
#include "tetrisEngine.h"
#include "text.h"

using namespace Tetris;

void drawEnemyBoard(int);

int enemyBoard[20][10];
OBJ_ATTR* enemyBoardSprite;

int timeoutTimer = 0;
int maxTimeout = 10;

int multiplayerStartTimer = 0;

int attackCheckCooldown = 10;
int attackCheckTimer = 0;

int connected = 0;

int currentScanHeight = 0;

int enemyHeight = 0;

Game *botGame = new Game(0,bigMode);
int botIncomingHeight = 0;

void handleMultiplayer() {
    if (!multiplayer)
        return;
    if (multiplayerStartTimer) {
        if (--multiplayerStartTimer == 0) {
            // startMultiplay8erGame(nextSeed & 0x1fff);
            playAgain = true;
        } else {
            linkConnection->send((u16)((1 << 13) + (nextSeed & 0x1fff)));
        }
        return;
    }

    auto linkState = linkConnection->linkState.get();

    int receivedAttack = false;

    u16 data[LINK_MAX_PLAYERS];
    for (int i = 0; i < LINK_MAX_PLAYERS; i++)
        data[i] = 0;

    int incomingHeight = 0;

    if (linkState->isConnected()) {
        timeoutTimer = 0;
        for (u32 i = 0; i < linkState->playerCount; i++)
            while (linkState->hasMessage(i))
                data[i] = linkState->readMessage(i);

        for (u32 i = 0; i < linkState->playerCount; i++) {

            int command = data[i] >> 13;
            int value = data[i] & 0x1fff;

            switch (command) {
            case 0:
                continue;
            case 1:
                playAgain = true;
                nextSeed = value;
                break;
            case 2:
                if (value == 0x123)
                    game->setWin();
                break;
            case 3://receive attack
                game->addToGarbageQueue((value >> 4) & 0xff, value & 0xf);
                receivedAttack = (value >> 4) & 0xff;
                break;
            case 4://acknowledge attack
                game->clearAttack(value & 0xff);
                break;
            case 5:
                for (int i = 0; i < 10; i++)
                    enemyBoard[value >> 10][i] = (value >> i) & 0b1;
                incomingHeight = value >> 10;
                break;
            case 6:
                for (int i = 0; i < 10; i++)
                    enemyBoard[(value >> 10) + 8][i] = (value >> i) & 0b1;
                incomingHeight = (value >> 10) + 8;
                break;
            case 7:
                if ((value >> 10) + 16 > 19)
                    break;
                for (int i = 0; i < 10; i++)
                    enemyBoard[(value >> 10) + 16][i] = (value >> i) & 0b1;
                incomingHeight = (value >> 10) + 16;

                break;
            }
        }
        if (game->lost) {
            linkConnection->send((u16)(2 << 13) + 0x123);
        } else {
            if (receivedAttack) {
                linkConnection->send((u16)((4 << 13) + receivedAttack));
            } else if (game->attackQueue.size() == 0) {
                if (currentScanHeight < 19)
                    currentScanHeight++;
                else
                    currentScanHeight = 0;

                u16 row = 0;
                for (int i = 0; i < 10; i++)
                    if (game->board[currentScanHeight + 20][i])
                        row += 1 << i;

                linkConnection->send((u16)(((5) << 13) + ((currentScanHeight & 0x1f) << 10) + (row & 0x3ff)));

            } else {
                Tetris::Garbage atck = game->attackQueue.front();

                linkConnection->send((u16)((3 << 13) + (atck.id << 4) + atck.amount));
            }

            drawEnemyBoard(incomingHeight);

        }
    } else {
        if (timeoutTimer++ == maxTimeout) {
            timeoutTimer = 0;
            game->setWin(); // aprint("no connection.",0,5);
            connected = -1;
            aprint("Connection Lost.", 0, 0);
        }
    }
}

void startMultiplayerGame(int seed) {
    memset32(&tile_mem[5][256], 0, 8 * 8);

    for (int i = 0; i < 20; i++)
        for (int j = 0; j < 10; j++)
            enemyBoard[i][j] = 0;

    delete game;
    game = new Game(4, seed & 0x1fff,bigMode);
    game->setLevel(1);
    game->setTuning(savefile->settings.das, savefile->settings.arr, savefile->settings.sfr, savefile->settings.dropProtectionFrames,savefile->settings.directionalDas);
    game->setGoal(100);
    multiplayer = true;
    linkConnection->send(50);
    nextSeed = 0;
}

void drawEnemyBoard(int height) {

    enemyBoardSprite = &obj_buffer[25];
    obj_unhide(enemyBoardSprite, ATTR0_AFF_DBL);
    obj_set_attr(enemyBoardSprite, ATTR0_TALL | ATTR0_AFF_DBL, ATTR1_SIZE(2) | ATTR1_AFF_ID(6), 0);
    enemyBoardSprite->attr2 = ATTR2_BUILD(768, 0, 1);
    obj_set_pos(enemyBoardSprite, 43, 24);
    obj_aff_identity(&obj_aff_buffer[6]);
    obj_aff_scale(&obj_aff_buffer[6], float2fx(0.5), float2fx(0.5));

    if (height > 19)
        return;

    TILE* dest2;

    for (int j = 0; j < 10; j++) {
        dest2 = (TILE*)&tile_mem[5][256 + ((height) / 8) * 2 + (j) / 8];//12 IS NOT CONFIRMED

        if (enemyBoard[height][j])
            dest2->data[(height) % 8] |= (4 + 2 * savefile->settings.lightMode) << ((j % 8) * 4);
        else
            dest2->data[(height) % 8] &= ~(0xffff << ((j % 8) * 4));
    }
}

void handleBotGame(){

    if(botIncomingHeight++ > 19){
        botIncomingHeight = 0;

    }

	if(botGame->attackQueue.size()){
		Tetris::Garbage atck = botGame->attackQueue.front();
		game->addToGarbageQueue(atck.id,atck.amount);
		botGame->clearAttack(atck.id);
	}

	if(game->attackQueue.size()){
		Tetris::Garbage atck = game->attackQueue.front();
		botGame->addToGarbageQueue(atck.id,atck.amount);
		game->clearAttack(atck.id);
	}

	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 10; j++)
			enemyBoard[i][j] = (botGame->board[i+20][j]);

	drawEnemyBoard(botIncomingHeight);

	if(botGame->clearLock){
		botGame->removeClearLock();
	}
}
