#include <tonc.h>
#include <tonc_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>
#include <maxmod.h>

#include "def.h"
#include "tetrisgame.h"
#include "sprites.h"

#include "soundbank.h"
#include "soundbank_bin.h"

using namespace Tetris;

void showBackground();
void control();
void showPawn();
void showShadow();
void showHold();
void showQueue();
void showText();
void showTimer();
void update();
void checkSounds();
void saveHighscore();
void saveBestTime();
void drawFrame();
void showClearText();
void startScreen();
void fallingBlocks();
void endScreen();
void endAnimation();
void showStats();
void pauseMenu();
void reset();
void countdown();
std::string timeToString(int);

Game game(0);
OBJ_ATTR obj_buffer[128];

OBJ_ATTR *pawnSprite;
OBJ_ATTR *pawnShadow;
OBJ_ATTR *holdSprite;

OBJ_ATTR *queueSprites[5];

int backgroundArray[24][30];
int bgSpawnBlock = 0;
int bgSpawnBlockMax = 20;
int gravity = 0;
int gravityMax = 10;

int shakeMax = 5;
int shake = 0;
int shakeTimer = 0;
int shakeTimerMax = 5;

int gameSeconds;

int pause = 0;
int clearTimer = 0;
int maxClearTimer = 20;
std::string clearTypeText = "";
int clearTextTimer = 0;
int maxClearTextTimer = 100;
int clearTextHeight = 0;

u32 highscore = 0;
int bestTime = 0;

Save*savefile = (Save*) sram_mem;

int canDraw = 0;

int profileResults[10];

void addToResults(int input,int index);

void onVBlank(void){
	profile_start();
	mmVBlank();
	if(canDraw){
		canDraw = 0;
		showPawn();
		showShadow();
		showHold();
		if(game.clearLock)
			showBackground();
		if(game.refresh){
			showBackground();
			update();
			game.resetRefresh();
		}
		oam_copy(oam_mem,obj_buffer,10);
	}

	mmFrame();
	addToResults(profile_stop(),2);
}

int main(void) {
	irq_init(NULL);
	irq_enable(II_VBLANK);
	irq_add(II_VBLANK,onVBlank);
	mmInitDefault((mm_addr)soundbank_bin,8);

	REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2);
	REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3);
	REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0);
	REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0);
	REG_DISPCNT= 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2; //Set to Sprite mode, 1d rendering

// 	// Load bg palette
	memcpy16(pal_bg_mem,palette,paletteLen/2);
// 	// //Load bg tiles
	memcpy16(&tile_mem[0][1],sprite1tiles_bin,sprite1tiles_bin_size/2);
	memcpy16(&tile_mem[0][2],sprite3tiles_bin,sprite3tiles_bin_size/2);
	memcpy16(&tile_mem[0][3],sprite4tiles_bin,sprite4tiles_bin_size/2);
	memcpy16(&tile_mem[0][4],sprite5tiles_bin,sprite3tiles_bin_size/2);

	memcpy16(&tile_mem[2][0],fontTiles,fontTilesLen/2);


	//load tetriminoes into tile memory for menu screen animation

	int **board;
	for(int i = 0; i < 7; i++){
		board = game.getShape(i);
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				if(board[j][k]){
					memcpy16(&tile_mem[4][16*i+j*4+k],sprite1tiles_bin,sprite1tiles_bin_size/2);
				}
			}
		}
	}
	for(int i = 0; i < 4; i++)
		delete [] board[i];
	delete [] board;

	memcpy16((COLOR*)MEM_PAL_OBJ, palette,paletteLen/2);


	//initialise background array
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 30; j++)
			backgroundArray[i][j] = 0;

	//stard screen animation
	startScreen();

	pawnSprite = &obj_buffer[0];
	pawnShadow = &obj_buffer[1];
	holdSprite = &obj_buffer[2];

	for(int i = 0; i < 5; i++)
		queueSprites[i] = &obj_buffer[3+i];

// 	//load highscore
	for(int i = 0; i < 4; i++)
		highscore += savefile->highscore[i] << (8*(4-i-1));

	if(highscore == 0xffffffff)
		highscore = 0;

// 	//load bestTime
	bestTime += savefile->bestTime[0] * 3600;
	bestTime += savefile->bestTime[1] * 60;
	bestTime += savefile->bestTime[2];

	if(bestTime == 0)
		bestTime = 0xffffffff;

	oam_init(obj_buffer,128);

	drawFrame();

	if(game.gameMode == 1){
		countdown();
	}

	clearText();
	update();
	
	int frameCounter = 0;

	while (1) {
		profile_start();
		// clearText();
		control();
		if(!game.lost && !pause)
			game.update();
		addToResults(profile_stop(),0);
		profile_start();
		checkSounds();

		showClearText();
		// showText();
		
		gameSeconds = (int)(0.0167f * game.timer);


		if(game.clearLock){
			if(clearTextTimer == 0)
				clearTextTimer++;
			clearTimer++;
		}

		if(clearTextTimer){
			if(++clearTextTimer > maxClearTextTimer){
				clearText();
				showText();
				clearTextTimer = 0;
			}	
		}

		if(clearTimer == maxClearTimer){
			game.removeClearLock();
			shake = -shakeMax;
			clearTimer = 0;
		}

		addToResults(profile_stop(),1);
		canDraw = 1;
		VBlankIntrWait();

		REG_BG0VOFS = -shake;
		shakeTimer++;
		if(shakeTimer == shakeTimerMax)
			shakeTimer = 0;
		if(shake != 0){
			if(shake > 0)
				shake--;
			else
				shake++;
			shake *= -1;
		}
		
		profile_start();

		if(game.won || game.lost)
			endScreen();

		if(pause)
			pauseMenu();

		showTimer();
		sqran(qran() % frameCounter++);
		addToResults(profile_stop(),3);

		// for(int i = 0; i < 4; i++){
		// 	aprint("       ",20,15+i);
		// 	std::string n = std::to_string(profileResults[i]);
		// 	aprint((char*)n.c_str(),20,15+i);
		// }
	}
}

void update(){
	// showBackground();
	showQueue();
	showText();
	showTimer();
}

void checkSounds(){
	if (game.sounds.hold == 1)
		sfx(SFX_HOLD);
	if (game.sounds.shift == 1)
		sfx(SFX_SHIFT);
	if (game.sounds.place == 1){
		sfx(SFX_PLACE); 
		shake = 2;
	}
	if (game.sounds.invalid == 1)
		sfx(SFX_INVALID);
	if (game.sounds.rotate == 1)
		sfx(SFX_ROTATE);
	if (game.sounds.clear == 1) {

		int speed = game.comboCounter-1;
		if(speed > 10)
			speed = 10;
		
		mm_sound_effect clear = {
			{SFX_LEVELUP},
			(mm_hword)((1.0+(float)speed/10) * (1<<10)),
			0,
			255,
			128,
		};

		mmEffectEx(&clear);

		clearTypeText = "";
		if (game.previousClear.isPerfectClear == 1){
			sfx(SFX_PERFECTCLEAR);
			clearTypeText = "perfect clear";
		}else if (game.previousClear.isTSpin == 2) {
			if(game.previousClear.isBackToBack == 1)
				sfx(SFX_BACKTOBACKTSPIN);
			else
				sfx(SFX_TSPIN);
			clearTypeText = "t-spin";
		}else if (game.previousClear.isTSpin == 1) {
			sfx(SFX_TSPINMINI);
			clearTypeText = "t-spin mini";
		}else if (game.previousClear.linesCleared == 4) {
			if(game.previousClear.isBackToBack == 1)
				sfx(SFX_BACKTOBACKTETRIS);
			else
				sfx(SFX_TETRIS);
			clearTypeText = "tetris";
		}else if (game.previousClear.linesCleared == 3) {
			sfx(SFX_TRIPLE);
			clearTypeText = "triple";
		}else if (game.previousClear.linesCleared == 2) {
			sfx(SFX_DOUBLE);
			clearTypeText = "double";
		}
	}
	if (game.sounds.levelUp == 1) {
		sfx(SFX_LEVELUPSOUND);
	}
	game.resetSounds();
}

void showBackground(){
	int i,j;

	u16*dest = (u16*)se_mem[25];

	std::list<int>::iterator l2c = game.linesToClear.begin();

	clearTextHeight = *l2c+((int)game.linesToClear.size()/2)-21;

	dest+= 10;
	for(i = 20; i < 40; i++){
		for(j = 0; j < 10; j++){
			//draw white for clear animation
			if(game.clearLock && i == *l2c){
				if(j < 5){
					if(clearTimer < maxClearTimer-10+j*2)
						*dest++ = 3;
					else
						*dest++ = 0;
				}else{
					if(clearTimer < maxClearTimer-10+(10-j)*2)
						*dest++ = 3;
					else
						*dest++ = 0;
				}

			}else if(!game.board[i][j])
				*dest++ = 0;
			else 
				*dest++ = (1+(((u32)(game.board[i][j]-1)) << 12));	
		}
		if(i == *l2c)
			std::advance(l2c,1);
		dest+=22;
	}
}

void showPawn(){
	if(game.clearLock){
		obj_hide(pawnSprite);
		return;
	}
	
	obj_unhide(pawnSprite,0);

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(game.pawn.board[game.pawn.rotation][i][j] > 0)
				memcpy16(&tile_mem[4][16*7+i*4+j],sprite1tiles_bin,sprite1tiles_bin_size/2);
			else
				memset(&tile_mem[4][16*7+i*4+j],0,sprite1tiles_bin_size);
		}
	}

	int n = game.pawn.current;
	obj_set_attr(pawnSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
	pawnSprite->attr2 = ATTR2_BUILD(16*7,n,1);
	obj_set_pos(pawnSprite,(10+game.pawn.x)*8,(game.pawn.y-20)*8);
}

void showShadow(){
	if(game.clearLock){
		obj_hide(pawnShadow);
		return;
	}
	
	obj_unhide(pawnShadow,0);

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(game.pawn.board[game.pawn.rotation][i][j] > 0)
				memcpy16(&tile_mem[4][16*8+i*4+j],sprite2tiles_bin,sprite2tiles_bin_size/2);
			else
				memset(&tile_mem[4][16*8+i*4+j],0,sprite2tiles_bin_size);
		}
	}

	int n = game.pawn.current;
	obj_set_attr(pawnShadow,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
	pawnShadow->attr2 = ATTR2_BUILD(16*8,n,1);
	obj_set_pos(pawnShadow,(10+game.pawn.x)*8,(game.lowest()-20)*8);
}

void showHold(){
	if(game.held == -1){
		obj_hide(holdSprite);
		return;
	}

	obj_unhide(holdSprite,0);

	obj_set_attr(holdSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(game.held));
	holdSprite->attr2 = ATTR2_BUILD(16*game.held,game.held,1);

	obj_set_pos(holdSprite,(3)*8,(10)*8);

}

void showQueue(){
	std::list<int>::iterator q = game.queue.begin();
	for(int k = 0; k < 5; k++){
		obj_unhide(queueSprites[k],0);

		int n = *q;
		obj_set_attr(queueSprites[k],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
		queueSprites[k]->attr2 = ATTR2_BUILD(16*n,n,1);

		obj_set_pos(queueSprites[k],(22)*8,(3+(k*3))*8);

		std::advance(q,1);
	}
}


void control(){
	key_poll();
	u16 key = key_hit(KEY_FULL);
	
	if(KEY_START == (key & KEY_START)){
		sfx(SFX_MENUCONFIRM);
		pause = !pause;
		clearText();
		update();
	}

	if(pause)
		return;

	if(KEY_UP == (key & KEY_UP))
		game.hardDrop();

	if(KEY_DOWN == (key & KEY_DOWN))
		game.keyDown(1);

	if(KEY_LEFT == (key & KEY_LEFT))
		game.keyLeft(1);

	if(KEY_RIGHT == (key & KEY_RIGHT))
		game.keyRight(1);

	if(KEY_A == (key & KEY_A))
		game.rotateCW();

	if(KEY_B == (key & KEY_B))
		game.rotateCCW();

	if(KEY_L == (key & KEY_L) || KEY_R == (key & KEY_R)){
		game.hold();
		update();
	}

	if(KEY_SELECT == (key & KEY_SELECT)){
		game = Game(game.gameMode);
		memset32(&se_mem[25],0x0000,32*10);
		oam_init(oam_mem,128);
		clearText();
		if(game.gameMode == 1)
			countdown();
		clearText();
		update();
	}

	key = key_released(KEY_FULL);

	if(KEY_DOWN == (key & KEY_DOWN))
		game.keyDown(0);

	if(KEY_LEFT == (key & KEY_LEFT))
		game.keyLeft(0);
	
	if(KEY_RIGHT == (key & KEY_RIGHT))
		game.keyRight(0);
}

void showText(){

	if(game.gameMode == 0 || game.gameMode == 2){
		// aprint("Highscore",0,0);
		char text[7];
		// sprintf(text,"%d",highscore);

		// for(int i = strlen(text); i < 9; i++){
		// 	memmove(text+1,text,strlen(text)+1);
		// 	*text = '0';
		// }

		// aprint(text,0,1);
		
		aprint("Score",3,3);
		sprintf(text,"%d",game.score);

		for(int i = strlen(text); i < 7; i++){
			memmove(text+1,text,strlen(text)+1);
			*text = ' ';
		}
		aprint(text,1,5);

		aprint("Level",2,14);
		// aprintf(game.level,7,15);

		std::string level = std::to_string(game.level);
		aprint((char*)level.c_str(),4,15);

	}else{
		// aprint("Best Time",0,1);
		// int t = bestTime;
		// if(t > 10000)
		// 	t = 0;
		// aprint((char*) timeToString(t).c_str(),0,2);		
	}

	aprint("Hold",3,8);
	aprint("Next",22,1);

	aprint("Lines",2,17);
	std::string lines = std::to_string(game.linesCleared);
	aprint((char*)lines.c_str(),4,18);
}

void showClearText(){
	if(clearTextTimer){
		char str[13];
		strcpy(str,clearTypeText.c_str());

		if(clearTextTimer){
			int x = 15-(clearTypeText.length()/2);
			aprint(str,x,clearTextHeight);
			if(game.comboCounter > 1){
				aprint("Combo x",11,clearTextHeight-1);

				std::string text = std::to_string(game.comboCounter);
				aprint((char*)text.c_str(),19,clearTextHeight-1);
			}
		}
	}
}

void saveHighscore(){
	for(int i = 0; i < 4; i++){
		savefile->highscore[i] = (u8)(game.score >> (8*(4-i-1)));
	}
	highscore = game.score;
}

void saveBestTime(){
	savefile->bestTime[0] = (int) gameSeconds / 3600;
	savefile->bestTime[1] = (int) gameSeconds / 60;
	savefile->bestTime[2] = gameSeconds % 60;

	bestTime = gameSeconds;
}

void showTimer(){
	std::string timer = timeToString(gameSeconds);
	aprint((char*)timer.c_str(),0,1);
}


std::string timeToString(int t){
	int seconds = t % 60;
	int minutes = t / 60;
	int hours = t / 3600;

	std::ostringstream stream;

	if(hours < 10)
		stream << "0";
	stream << hours << ":";

	if(minutes < 10)
		stream << "0";
	stream << minutes << ":";

	if(seconds < 10)
		stream << "0";
	stream << seconds;

	return stream.str();
}


void drawFrame(){
	u16*dest = (u16*)se_mem[26];

	dest+= 9;

	int i,j;
	for(i = 0; i < 20; i++){
		for(j = 0; j < 2; j++){
			*dest++ = 0x6004 + j*0x400;
			dest+=10;
		}
		dest+=32-22;
	}

	dest = (u16*)se_mem[26];
	dest+= 10;
	for(i = 0; i < 20; i++){
		for(j = 0; j < 10; j++)
			*dest++ = 0x0002;
		dest+=22;
	}
}

void startScreen(){
	// REG_BG0VOFS = 0;
	int selection = 0;
	while(1){
		VBlankIntrWait();
		fallingBlocks();

		aprint("APOTRIS",12,4);

		aprint("Marathon",12,10);
		aprint("Endless",12,13);
		aprint("Sprint",12,16);

		aprint("akouzoukos",20,19);


		aprint(" ",10,10);
		aprint(" ",10,13);
		aprint(" ",10,16);
		
		if(selection == 0){
			aprint(">",10,10);
		}else if(selection == 1){
			aprint(">",10,13);
		}else if(selection == 2){
			aprint(">",10,16);
		}

		key_poll();

		u16 key = key_hit(KEY_FULL);

		if(key == KEY_A){
			int n = 0;
			if(selection == 0)
				n = 2;
			else if(selection == 2)
				n = 1;

			game = Game(n);
			sfx(SFX_MENUCONFIRM);
			break;
		}

		if(key == KEY_UP){
			if(selection == 0)
				selection = 2;
			else
				selection--;
			sfx(SFX_MENUMOVE);
		}
		
		if(key == KEY_DOWN){
			if(selection == 2)
				selection = 0;
			else
				selection++;
			sfx(SFX_MENUMOVE);
		}
	}
	clearText();

	u16*dest = (u16*)se_mem[25];
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 32; j++)
			*dest++ = 0;
}

void fallingBlocks(){
	gravity++;
	bgSpawnBlock++;

	int i,j;
	if(gravity > gravityMax){
		gravity = 0;

		for(i = 23; i >= 0; i--){
			for(j = 0; j < 30; j++){
				if(i == 0)
					backgroundArray[i][j] = 0;
				else
					backgroundArray[i][j] = backgroundArray[i-1][j];
			}
		}
	}

	u16*dest = (u16*)se_mem[25];
	
	for(i = 4; i < 24; i++){
		for(j = 0; j < 30; j++){
			if(!backgroundArray[i][j])
				*dest++ = 2;
			else 
				*dest++ = (1+(((u32)(backgroundArray[i][j]-1)) << 12));	
		}
		dest+=2;
	}

	if(bgSpawnBlock > bgSpawnBlockMax){
		bgSpawnBlock = 0;
		int x = qran() % 27;
		int n = qran() % 7;
		int **p = game.getShape(n);

		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				if(p[i][j])
					backgroundArray[i][j+x] = n+1; 
	}
}

void endScreen(){
	endAnimation();
	int selection = 0;
	while(1){
		VBlankIntrWait();
		key_poll();

		aprint("Play Again",12,14);
		aprint("Main Menu",12,16);

		if(selection == 0){
			aprint(">",10,14);
			aprint(" ",10,16);
		}else if(selection == 1){
			aprint(" ",10,14);
			aprint(">",10,16);
		}

		showStats();

		u16 key = key_hit(KEY_FULL);

		if(key == KEY_A){
			if(!selection){
				sfx(SFX_MENUCONFIRM);
				shake = 0;
				game = Game(game.gameMode);
				drawFrame();
				clearText();
				update();
				break;
			}else{
				sfx(SFX_MENUCONFIRM);
				reset();
			}
		}
		
		if(key == KEY_UP){
			if(selection == 0)
				selection = 1;
			else
				selection--;
			sfx(SFX_MENUMOVE);
		}
		
		if(key == KEY_DOWN){
			if(selection == 1)
				selection = 0;
			else
				selection++;
			sfx(SFX_MENUMOVE);
		}
	}
}

void endAnimation(){
	clearText();
	oam_init(obj_buffer,128);

	int i,j;
	u16*dest,*dest2;
	for(i = 19; i >= 0; i--){
		dest = (u16*)se_mem[25];
		dest += i * 32;

		dest2 = (u16*)se_mem[26];
		dest2 += i * 32;
		
		int timer = 0;
		while(1){
			VBlankIntrWait();
			if(++timer > 1)
				break;
		}

		for(j = 0; j < 32; j++)
			*dest++ = *dest2++ = 0;
	}
}

void showStats(){
	if(game.gameMode == 0 || game.gameMode == 2 || game.lost){
		std::string score = std::to_string(game.score);

		if(game.score > (int) highscore && game.gameMode == 0){
			aprint("New Highscore!",8,7);
			saveHighscore();
		}
		
		aprint("GAME  OVER",10,4);

		if(game.gameMode == 0 || game.gameMode == 2)
			aprint((char*)score.c_str(),15-((int)score.size()/2),9);

	}else{
		aprint("CLEARED!",11,4);
		
		if(gameSeconds < bestTime){
			aprint("New Best Time!",8,7);
			saveBestTime();
		}

		aprint((char *) timeToString(gameSeconds).c_str(),11,8);

	}
}

void pauseMenu(){
	aprint("PAUSE!",12,6);
	aprint("Resume",12,12);
	aprint("Main Menu",12,14);

	int selection = 0;
	while(1){
		VBlankIntrWait();
		key_poll();
		
		if(selection == 0){
			aprint(">",10,12);
			aprint(" ",10,14);
		}else if(selection == 1){
			aprint(" ",10,12);
			aprint(">",10,14);
		}

		u16 key = key_hit(KEY_FULL);

		if(key == KEY_START){
			sfx(SFX_MENUCONFIRM);
			clearText();
			showText();
			pause = 0;
			break;
		}
		
		if(key == KEY_A){
			if(!selection){
				sfx(SFX_MENUCONFIRM);
				clearText();
				showText();
				pause = 0;
				break;
			}else{
				sfx(SFX_MENUCONFIRM);
				reset();
			}
		}
		
		if(key == KEY_UP){
			if(selection == 0)
				selection = 1;
			else
				selection--;
			sfx(SFX_MENUMOVE);
		}
		
		if(key == KEY_DOWN){
			if(selection == 1)
				selection = 0;
			else
				selection++;
			sfx(SFX_MENUMOVE);
		}
	}
}

void addToResults(int input, int index){
	profileResults[index] = input;
}

void reset(){
	oam_init(obj_buffer,128);
	clearText();
	REG_BG0HOFS = 0;

	memset32(&se_mem[26],0x0000,32*20);

	SoftReset();
	RegisterRamReset(0xff);	
}

void countdown(){
	int timer = 0;
	int timerMax = 120;
	while(timer++ < timerMax-1){
		VBlankIntrWait();
		if(timer < timerMax/3)
			aprint("3",15,10);
		else if(timer < 2*timerMax/3)
			aprint("2",15,10);
		else
			aprint("1",15,10);
	}
}