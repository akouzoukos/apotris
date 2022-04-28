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

#include "LinkConnection.h"

#include "posprintf.h"

#define SHOW_FINESSE 0
#define DIAGNOSE 0
#define SAVE_TAG 0x4b

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
void drawGrid();
void showClearText();
void startScreen();
void fallingBlocks();
void endScreen();
void endAnimation();
void showStats();
void pauseMenu();
void reset();
void countdown();
void addGlow(Tetris::Drop);
void loadSave();
void saveToSram();
void loadFromSram();
void screenShake();
std::string nameInput();
std::string timeToString(int);
bool onRecord();
void diagnose();
void progressBar();
void showBar(int,int,int,int);
void showFrames();
void clearGlow();
void handleMultiplayer();
void startMultiplayerGame(int);
void setSkin();

LinkConnection* linkConnection = new LinkConnection();

int frameCounter = 1;

Game *game;
OBJ_ATTR obj_buffer[128];
OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE*) obj_buffer;

OBJ_ATTR *pawnSprite;
OBJ_ATTR *pawnShadow;
OBJ_ATTR *holdSprite;
OBJ_ATTR *holdFrameSprite;
OBJ_ATTR *queueFrameSprites[3];

OBJ_ATTR *queueSprites[5];

OBJ_ATTR *titleSprites[2];

int backgroundArray[24][30];
int bgSpawnBlock = 0;
int bgSpawnBlockMax = 20;
int gravity = 0;
int gravityMax = 10;

int shakeMax = 5;
int shake = 0;
int shakeTimer = 0;
int shakeTimerMax = 5;

int pushMax = 2;
int push = 0;
int pushTimer = 0;

int gameSeconds;

bool pause = false;
int clearTimer = 0;
int maxClearTimer = 20;

int marathonClearTimer = 20;

std::string clearTypeText = "";
int maxClearTextTimer = 100;
int clearTextHeight = 16;

u32 highscore = 0;
int bestTime = 0;

int canDraw = 0;

int profileResults[10];

void addToResults(int input,int index);

int glow[20][10];
int glowDuration = 12;

Save *savefile;

int mode = 0;

u8* blockSprite;

bool restart = false;
bool onStates = false;

int enemyHeight = 0;
bool playAgain = false;
int nextSeed = 0;

int attackFlashTimer = 0;
int attackFlashMax = 10;

Settings previousSettings;

Tetris::Game *quickSave;
bool saveExists = false;

bool multiplayer = false;

int multiplayerStartTimer = 0;

int attackCheckCooldown = 10;
int attackCheckTimer = 0;

int connected = 0;

int skinSelect = 1;
int framePalette = 0;

class FloatText{
public:
	std::string text;
	int timer = 0;

	FloatText(){}
	FloatText(std::string _t){
		text = _t;
	}
};

std::list<FloatText> floatingList;

class Effect{
public:
	int timer = 0;
	int duration;
	int type;
	int x;
	int y;

	Effect(){}
	Effect(int _type){
		type = _type;
		duration = glowDuration*3;
	}
	Effect(int _type, int _x, int _y){
		type = _type;
		duration = glowDuration*(3/2);
		x = _x;
		y = _y;
	}
};

std::list<Effect> effectList;

void onVBlank(void){

	mmVBlank();
	LINK_ISR_VBLANK();

	if(canDraw){
		canDraw = 0;

		control();

		showShadow();
		showHold();
	
		drawGrid();
		// profile_start();
		showTimer();
		// addToResults(profile_stop(),0);
		showPawn();
		screenShake();

		showClearText();

		if(game->refresh){
			// profile_start();
			showBackground();
			// addToResults(profile_stop(),1);
			update();
			game->resetRefresh();
		}else if(game->clearLock)
			showBackground();

		oam_copy(oam_mem,obj_buffer,20);
		obj_aff_copy((OBJ_AFFINE*)oam_mem,obj_aff_buffer,10);
	}

	mmFrame();
}

int main(void) {
	irq_init(NULL);
	irq_enable(II_VBLANK);
	irq_add(II_VBLANK,onVBlank);
	
	irq_add(II_SERIAL,LINK_ISR_SERIAL);
	irq_add(II_TIMER3,LINK_ISR_TIMER);

	mmInitDefault((mm_addr)soundbank_bin,10);

	REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2);
	REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3);
	REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0);
	REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(2);
	REG_DISPCNT= 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering
// 	// Load bg palette

	memcpy16(pal_bg_mem,palette,paletteLen/2);
// 	// //Load bg tiles
	memcpy16(&tile_mem[0][2],sprite3tiles_bin,sprite3tiles_bin_size/2);
	memcpy16(&tile_mem[0][3],sprite4tiles_bin,sprite4tiles_bin_size/2);
	memcpy16(&tile_mem[0][4],sprite5tiles_bin,sprite3tiles_bin_size/2);
	memcpy16(&tile_mem[0][5],sprite7tiles_bin,sprite7tiles_bin_size/2);
	memcpy16(&tile_mem[0][6],sprite8tiles_bin,sprite8tiles_bin_size/2);
	memcpy16(&tile_mem[0][12],sprite10tiles_bin,sprite10tiles_bin_size/2);
	memcpy16(&tile_mem[0][13],sprite11tiles_bin,sprite11tiles_bin_size/2);
	memcpy16(&tile_mem[0][14],sprite12tiles_bin,sprite12tiles_bin_size/2);
	memcpy16(&tile_mem[0][15],sprite13tiles_bin,sprite13tiles_bin_size/2);
	
	memcpy16(&tile_mem[5][0],sprite6tiles_bin,sprite6tiles_bin_size/2);

	//queue frame Tiles

	memcpy16(&tile_mem[5][16],sprite6tiles_bin,sprite6tiles_bin_size/2*3/4);
	memcpy16(&tile_mem[5][16+12],&sprite6tiles_bin[128],sprite6tiles_bin_size/2*1/4);
	memcpy16(&tile_mem[5][32],&sprite6tiles_bin[128],sprite6tiles_bin_size/2*2/4);
	memcpy16(&tile_mem[5][32+8],&sprite6tiles_bin[128],sprite6tiles_bin_size/2*2/4);
	memcpy16(&tile_mem[5][48],&sprite6tiles_bin[128],sprite6tiles_bin_size/2*1/4);
	memcpy16(&tile_mem[5][48+4],&sprite6tiles_bin[128],sprite6tiles_bin_size/2*3/4);
	
	memcpy16(&tile_mem[5][64],title1tiles_bin,title1tiles_bin_size/2);
	memcpy16(&tile_mem[5][96],title2tiles_bin,title2tiles_bin_size/2);

	memcpy16(&tile_mem[2][0],fontTiles,fontTilesLen/2);
	
	
	//load tetriminoes into tile memory for menu screen animation

	memcpy16((COLOR*)MEM_PAL_OBJ, palette,paletteLen/2);

	memcpy16(&pal_obj_mem[14*16], title_pal_bin,title_pal_bin_size/2);
	
	//load mini sprite tiles
	for(int i = 0; i < 7; i++)
		memcpy16(&tile_mem[4][9*16+i*8],mini[i],16*7);
	
	//initialise background array
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 30; j++)
			backgroundArray[i][j] = 0;

	setSkin();

	loadSave();
	
	//init glow 
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 10; j++)
			glow[i][j] = 0;

	pawnSprite = &obj_buffer[0];
	pawnShadow = &obj_buffer[1];
	holdSprite = &obj_buffer[2];

	for(int i = 0; i < 5; i++)
		queueSprites[i] = &obj_buffer[3+i];

	holdFrameSprite = &obj_buffer[10];
	
	for(int i = 0; i < 3; i++)
		queueFrameSprites[i] = &obj_buffer[11+i];

	for(int i = 0; i < 2; i++)
		titleSprites[i] = &obj_buffer[14+i];

	oam_init(obj_buffer,128);

	// linkConnection->deactivate();

	mmStart(MOD_MENU,MM_PLAY_LOOP);
	mmSetModuleVolume(512*((float)savefile->settings.volume/10));

	for(int i = 0; i < 2; i++){
		obj_unhide(titleSprites[i],0);
		obj_set_attr(titleSprites[i],ATTR0_WIDE,ATTR1_SIZE(3),ATTR2_PALBANK(14));
		titleSprites[i]->attr2 = ATTR2_BUILD(512+64+i*32,14,0);
		obj_set_pos(titleSprites[i],120-64+64*i,24);
	}

	oam_copy(oam_mem,obj_buffer,128);
	
	//start screen animation
	startScreen();

	oam_init(obj_buffer,128);
	drawFrame();

	mmStop();
	
	showFrames();

	oam_copy(oam_mem,obj_buffer,128);

	countdown();

	if(!(game->gameMode == 1 && game->goal == 0)){
		mmStart(MOD_THIRNO,MM_PLAY_LOOP	);
		mmSetModuleVolume(512*((float)savefile->settings.volume/10));
	}

	clearText();
	update();
	
	while (1) {
		diagnose();
		if(!game->lost && !pause)
			game->update();
		checkSounds();
		handleMultiplayer();

		progressBar();

		// gameSeconds = (int)(0.0167f * game->timer);
		gameSeconds = game->timer;

		if(game->clearLock){
			clearTimer++;
		}

		if(clearTimer == maxClearTimer){
			game->removeClearLock();
			shake = -shakeMax;
			clearTimer = 0;
		}

		Tetris::Drop latestDrop = game->getDrop();

		if(latestDrop.on)
			addGlow(latestDrop);

		canDraw = 1;
		VBlankIntrWait();

		if(game->won || game->lost)
			endScreen();

		if(pause)
			pauseMenu();

		if(restart){
			restart = false;
			int oldGoal = game->goal;
			int oldLevel = game->level;

			mmStop();

			delete game;
			game = new Game(game->gameMode);
			game->setGoal(oldGoal);
			game->setLevel(oldLevel);
			game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);

			memset32(&se_mem[25],0x0000,32*10);
			memset32(&se_mem[27],0x0000,32*10);

			VBlankIntrWait();
			oam_init(obj_buffer,128);
			oam_copy(oam_mem,obj_buffer,128);

			clearText();
		
			showFrames();
			oam_copy(oam_mem,obj_buffer,128);
			progressBar();

			clearGlow();
			floatingList.clear();

			countdown();

			if(!(game->gameMode == 1 && game->goal == 0)){
				mmStart(MOD_THIRNO,MM_PLAY_LOOP);
				mmSetModuleVolume(512*((float)savefile->settings.volume/10));
			}
		
			clearText();
			update();
		}

		sqran(qran() % frameCounter++);
		
		// addToResults(profile_stop(),3);
	}
}

void update(){
	// showBackground();
	showQueue();
	showText();
	// showTimer();
	// showProgressBar();
}

void checkSounds(){
	if (game->sounds.hold == 1)
		sfx(SFX_HOLD);
	if (game->sounds.shift == 1)
		sfx(SFX_SHIFT);
	if (game->sounds.place == 1){
		sfx(SFX_PLACE); 
		shake = 2;
	}
	if (game->sounds.invalid == 1)
		sfx(SFX_INVALID);
	if (game->sounds.rotate == 1)
		sfx(SFX_ROTATE);
	if (game->sounds.clear == 1) {

		int speed = game->comboCounter-1;
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

		int soundEffect = -1;

		clearTypeText = "";
		if (game->previousClear.isPerfectClear == 1){
			soundEffect = SFX_PERFECTCLEAR;
			clearTypeText = "perfect clear";
			effectList.push_back(Effect(0));
		}else if (game->previousClear.isTSpin == 2) {
			if(game->previousClear.isBackToBack == 1)
				soundEffect = SFX_BACKTOBACKTSPIN;
			else
				soundEffect = SFX_TSPIN;

			if(game->previousClear.linesCleared == 1){
				clearTypeText = "t-spin single";
			}else if(game->previousClear.linesCleared == 2){
				clearTypeText = "t-spin double";
			}else if(game->previousClear.linesCleared == 3){
				clearTypeText = "t-spin triple";
			}
		}else if (game->previousClear.isTSpin == 1) {
			soundEffect = SFX_TSPINMINI;
			clearTypeText = "t-spin mini";
		}else if (game->previousClear.linesCleared == 4) {
			if(game->previousClear.isBackToBack == 1)
				soundEffect = SFX_BACKTOBACKTETRIS;
			else
				soundEffect = SFX_TETRIS;
			clearTypeText = "tetris";
		}else if (game->previousClear.linesCleared == 3) {
			soundEffect = SFX_TRIPLE;
			clearTypeText = "triple";
		}else if (game->previousClear.linesCleared == 2) {
			soundEffect = SFX_DOUBLE;
			clearTypeText = "double";
		}
	
		if(savefile->settings.floatText)
			floatingList.push_back(FloatText(clearTypeText));
		
		if(savefile->settings.announcer && soundEffect != -1)
			sfx(soundEffect);
	}
	if (game->sounds.levelUp == 1) {
		sfx(SFX_LEVELUPSOUND);
	}
	game->resetSounds();
}

void showBackground(){

	bool showEdges = savefile->settings.edges;	

	u16*dest = (u16*)se_mem[25];

	std::list<int>::iterator l2c = game->linesToClear.begin();
	// n  int neib[3][3];
	bool up,down,left,right;
	bool before = false, after = false;
	
	dest+= 10;
	for(int i = 20; i < 40; i++){
		if(game->linesToClear.size() > 0){
			std::list<int>::iterator l2c2 = game->linesToClear.begin();
			before = after = false;
			while(l2c2 != game->linesToClear.end()){
				if(*l2c2 == i-1){
					before = true;

				}
				if(*l2c2 == i+1){
					after = true;
				}
				++l2c2;
			}
		}
		
		for(int j = 0; j < 10; j++){
			// draw white for clear animation
			if(!game->board[i][j] || (game->clearLock && i == *l2c && showEdges)){
				if(!showEdges){
					*dest++ = 0;
					continue;
				}

				// if((game->linesToClear.size() > 0 && clearTimer > 1) || game->currentHeight < 40-(i+2)){
				// 	dest++;
				// 	continue;
				// }
				if(game->currentHeight < 40-(i+2)){
					dest++;
					continue;
				}

				up = (game->board[i-1][j] > 0 && !before);
				left = (j-1 >= 0 && game->board[i][j-1] > 0 && !(i == *l2c && game->clearLock));
				right = (j+1 <= 9 && game->board[i][j+1] > 0 && !(i == *l2c && game->clearLock));
				down = (i+1 <=39 && game->board[i+1][j] > 0 && !after);

				// int count = up + left + right + down;
				int count = up + down + left + right;
				// 
				// count = 0;

				int n = 0;
				if(count == 0){
					n = 0;
				}else if(count == 4){
					// if(up && left && right && down)//full
					n = 0x0019;
				}else if(count == 3){
					if(up && left && !right && down)//1 side missing
						n = 0x0018;
					else if(up && !left && right && down)
						n = 0x0418;
					else if(up && left && right && !down)
						n = 0x0017;
					else if(!up && left && right && down)
						n = 0x0817;
					else
						n = 0x000a;
				}else if(count == 2){
					if(up && left && !right && !down)//2 sides missing
						n = 0x0016;
					else if(up && !left && right && !down)
						n = 0x0416;
					else if(!up && left && !right && down)
						n = 0x0816;
					else if(!up && !left && right && down)
						n = 0x0c16;
					else if(up && !left && !right && down)//
						n = 0x0012;
					else if(!up && left && right && !down)
						n = 0x0011;
					else
						n = 0x000b;
				}else if(count == 1){
					if(!up && left && !right && !down)//4 sides missing
						n = 0x0010;
					else if(!up && !left && right && !down)
						n = 0x0410;
					else if(up && !left && !right && !down)
						n = 0x000f;
					else if(!up && !left && !right && down)
						n = 0x080f;
					else
						n = 0;
				}
				*dest++ = n;
			}else 
				*dest++ = (1+(((u32)(game->board[i][j]-1)) << 12));
			if(game->clearLock && i == *l2c){
				dest--;
				if(!showEdges)
					*dest = 0;
				if(j < 5){
					if(clearTimer < maxClearTimer-10+j*2)
						*dest = 3;
				}else{
					if(clearTimer < maxClearTimer-10+(10-j)*2)
						*dest = 3;
				}
				dest++;
			}
		}
		if(i == *l2c)
			std::advance(l2c,1);
		dest+=22;
	}
}

void showPawn(){
	if(game->clearLock){
		obj_hide(pawnSprite);
		return;
	}
	
	obj_unhide(pawnSprite,0);

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(game->pawn.board[game->pawn.rotation][i][j] > 0)
				memcpy16(&tile_mem[4][16*7+i*4+j],blockSprite,sprite1tiles_bin_size/2);
			else
				memset(&tile_mem[4][16*7+i*4+j],0,sprite1tiles_bin_size);
		}
	}

	int n = game->pawn.current;
	obj_set_attr(pawnSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
	pawnSprite->attr2 = ATTR2_BUILD(16*7,n,1);
	obj_set_pos(pawnSprite,(10+game->pawn.x)*8+push*savefile->settings.shake,(game->pawn.y-20)*8);
}

void showShadow(){
	if(game->clearLock){
		obj_hide(pawnShadow);
		return;
	}

	obj_unhide(pawnShadow,0);

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(game->pawn.board[game->pawn.rotation][i][j] > 0)
				memcpy16(&tile_mem[4][16*8+i*4+j],sprite2tiles_bin,sprite2tiles_bin_size/2);
			else
				memset(&tile_mem[4][16*8+i*4+j],0,sprite2tiles_bin_size);
		}
	}

	int n = game->pawn.current;
	obj_set_attr(pawnShadow,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
	pawnShadow->attr2 = ATTR2_BUILD(16*8,n,1);
	obj_set_pos(pawnShadow,(10+game->pawn.x)*8+push*savefile->settings.shake,(game->lowest()-20)*8+shake*savefile->settings.shake);
}

void showHold(){
	obj_unhide(holdFrameSprite,0);
	obj_set_attr(holdFrameSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(framePalette));
	holdFrameSprite->attr2 = ATTR2_BUILD(512,framePalette,0);
	obj_set_pos(holdFrameSprite,4*8+5,9*8-2);

	if(game->held == -1){
		obj_hide(holdSprite);
		return;
	}

	int add = !(game->held == 0 || game->held == 3);

	if(skinSelect == 0){
		obj_unhide(holdSprite,0);
		obj_set_attr(holdSprite,ATTR0_WIDE,ATTR1_SIZE(2),ATTR2_PALBANK(game->held));
		holdSprite->attr2 = ATTR2_BUILD(9*16+8*game->held,game->held,1);
		obj_set_pos(holdSprite,(5)*8+add*3+1,(10)*8-3*(game->held == 0));
	}else{
		obj_unhide(holdSprite,ATTR0_AFF);
		obj_set_attr(holdSprite,ATTR0_WIDE | ATTR0_AFF,ATTR1_SIZE(2) | ATTR1_AFF_ID(6),ATTR2_PALBANK(game->held));
		holdSprite->attr2 = ATTR2_BUILD(16*game->held,game->held,1);
		FIXED size = float2fx(1.5);
		obj_aff_scale(&obj_aff_buffer[6],size,size);
		obj_set_pos(holdSprite,(5)*8+add*3+1-4,(10)*8-3*(game->held == 0)-3);
	}
}

void showQueue(){
	for(int i = 0; i < 3; i++){
		obj_unhide(queueFrameSprites[i],0);
		obj_set_attr(queueFrameSprites[i],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(framePalette));
		queueFrameSprites[i]->attr2 = ATTR2_BUILD(512+16+16*i,framePalette,0);
		obj_set_pos(queueFrameSprites[i],173,12+32*i);
	}
	
	std::list<int>::iterator q = game->queue.begin();
	for(int k = 0; k < 5; k++){

		int n = *q;

		int add = !(n == 0 || n == 3);
		if(skinSelect == 0){
			obj_unhide(queueSprites[k],0);
			obj_set_attr(queueSprites[k],ATTR0_WIDE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
			queueSprites[k]->attr2 = ATTR2_BUILD(16*9+8*n,n,1);
			obj_set_pos(queueSprites[k],(22)*8+add*3+1,(3+(k*3))*6-3*(n == 0));
		}else{
			obj_unhide(queueSprites[k],ATTR0_AFF);
			obj_set_attr(queueSprites[k],ATTR0_SQUARE | ATTR0_AFF,ATTR1_SIZE(2) | ATTR1_AFF_ID(k),ATTR2_PALBANK(n));
			queueSprites[k]->attr2 = ATTR2_BUILD(16*n,n,1);
			obj_aff_identity(&obj_aff_buffer[k]);
			FIXED size = float2fx(1.5);
			obj_aff_scale(&obj_aff_buffer[k],size,size);
			obj_set_pos(queueSprites[k],(22)*8+add*3+1-4,(3+(k*3))*6-3*(n == 0)-4);
		}
		

		++q;
	}
}

void control(){
	if(pause)
		return;

	key_poll();
	u16 key = key_hit(KEY_FULL);

	if(KEY_START == (key & KEY_START) && !multiplayer){
		sfx(SFX_MENUCONFIRM);
		pause = true;
		mmPause();
		clearText();
		update();
	}

	if(KEY_UP == (key & KEY_UP))
		game->keyDrop();

	if(KEY_DOWN == (key & KEY_DOWN))
		game->keyDown(1);

	if(KEY_LEFT == (key & KEY_LEFT))
		game->keyLeft(1);

	if(KEY_RIGHT == (key & KEY_RIGHT))
		game->keyRight(1);

	if(KEY_A == (key & KEY_A))
		game->rotateCW();

	if(KEY_B == (key & KEY_B))
		game->rotateCCW();

	if(KEY_L == (key & KEY_L) || KEY_R == (key & KEY_R)){
		game->hold();
		update();
	}

	if(KEY_SELECT == (key & KEY_SELECT) && game->goal == 0 && game->gameMode == 1){
		sfx(SFX_MENUCONFIRM);
		pause = true;
		mmPause();
		clearText();
		update();
		onStates = true;
	}

	key = key_released(KEY_FULL);

	if(KEY_DOWN == (key & KEY_DOWN))
		game->keyDown(0);

	if(KEY_LEFT == (key & KEY_LEFT))
		game->keyLeft(0);

	if(KEY_RIGHT == (key & KEY_RIGHT))
		game->keyRight(0);
}

void showText(){
	if(game->gameMode == 0 || game->gameMode == 2){

		aprint("Score",3,3);

		std::string score = std::to_string(game->score);
		aprint(score,8-score.size(),5);

		aprint("Level",2,14);

		std::string level = std::to_string(game->level);
		aprint(level,4,15);

	}else if(game->gameMode == 1){
		if(SHOW_FINESSE){
			aprint("Finesse",1,14);
			std::string finesse = std::to_string(game->finesse);
			aprint(finesse,4,15);
		}
		if(game->goal == 0){
			aprint("Training",1,1);
			std::string bagCount = std::to_string(game->bagCounter-1);
			aprint(bagCount,26,1);

		}
	}

	aprint("Lines",2,17);
	std::string lines;
	if(game->gameMode == 3)
		lines = std::to_string(game->garbageCleared);
	else
		lines = std::to_string(game->linesCleared);
	aprint(lines,4,18);
}

void showClearText(){
	if(game->comboCounter > 1){
		aprint("Combo x",21,clearTextHeight-1);

		std::string text = std::to_string(game->comboCounter);
		aprint(text,28,clearTextHeight-1);
	}else{
		aprint("          ",20,clearTextHeight-1);
	}
	
	if(game->b2bCounter > 0){
		aprint("Streak",22,clearTextHeight+1);

		std::string text = std::to_string(game->b2bCounter+1);
		aprint("x",24,clearTextHeight+2);
		aprint(text,25,clearTextHeight+2);
	}else{
		aprint("          ",20,clearTextHeight+1);
		aprint("          ",20,clearTextHeight+2);
	}

	std::list<FloatText>::iterator index = floatingList.begin();

	for(int i = 0; i < (int)floatingList.size(); i++){
		std::string text = (*index).text;
		if(index->timer++ > maxClearTextTimer){
			index = floatingList.erase(index);
			aprint("          ",10,0);
		}else{
			int height = 0;
			if(index->timer < 2*maxClearTextTimer/3)
				height = 5*(float)index->timer/(2*maxClearTextTimer/3);
			else
				height = (30*(float)(index->timer)/maxClearTextTimer)-15;
			if(text.size() <= 10){
				aprint(text,15-text.size()/2,15-height);
			}else{
				aprint("          ",10,15-height);

				std::size_t pos = text.find(" ");
				std::string part1 = text.substr(0,pos);
				std::string part2 = text.substr(pos+1);
				
				if(15-height-1 > 0)
					aprint(part1,15-part1.size()/2,15-height-1);
				aprint(part2,15-part2.size()/2,15-height);
			}
			aprint("          ",10,15-height+1);
			std::advance(index,1);
		}
	}
}

void saveToSram(){
	volatile u8 *sf = (volatile u8*) sram_mem;

	u8 * arr = (u8*) savefile;

	for(int i = 0; i < (int)sizeof(Save); i++)
		sf[i] = (u8) arr[i];
}

void loadFromSram(){
	volatile u8 *sf = (volatile u8*) sram_mem;

	u8 * arr = (u8*) savefile;

	for(int i = 0; i < (int)sizeof(Save); i++)
		arr[i] = (u8) sf[i];
}

void showTimer(){
	if(!(game->gameMode == 1 && game->goal == 0)){
		std::string timer = timeToString(gameSeconds);
		aprint(timer,1,1);
	}
}

std::string timeToString(int frames){
	int t = fx2int(fxmul(float2fx(0.0167f),int2fx(frames)));
	int millis = (fx2int(fxmul(float2fx(1.67f),int2fx(frames)))) % 100;
	int seconds = t % 60;
	int minutes = t / 60;
	
	std::string result = "";
	if(std::to_string(minutes).length() < 2)
		result += "0";
	result += std::to_string(minutes) + ":";
	if(std::to_string(seconds).length() < 2)
		result += "0";
	result += std::to_string(seconds) + ":";
	if(std::to_string(millis).length() < 2)
		result += "0";
	result += std::to_string(millis);

	return result;
}


void drawFrame(){
	u16*dest = (u16*)se_mem[26];

	dest+= 9;

	for(int i = 0; i < 20; i++){
		for(int j = 0; j < 2; j++){
			*dest++ = 0x0004 + j*0x400 + framePalette * 0x1000;
			dest+=10;
		}
		dest+=32-22;
	}

	progressBar();

	for(int i = 0; i < 3; i++){
		obj_unhide(queueFrameSprites[i],0);
		obj_set_attr(queueFrameSprites[i],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(framePalette));
		queueFrameSprites[i]->attr2 = ATTR2_BUILD(512+16+16*i,framePalette,0);
		obj_set_pos(queueFrameSprites[i],173,12+32*i);
	}

	drawGrid();
}

void drawGrid(){
	std::list<Effect>::iterator index = effectList.begin();

	for(int i = 0; i < (int) effectList.size() && (savefile->settings.effects); i++){
		if(index->timer < index->duration){
			switch(index->type){
			case 0:
				if(index->timer % glowDuration == 0){
					for(int i = 0; i < 20; i++)
						for(int j = 0; j < 10; j++)
							glow[i][j] = glowDuration;
				}
				break;
			case 1:
				if(index->timer == index->duration-1){
					for(int i = 0; i < 20; i++)
						for(int j = 0; j < 10; j++)
							if(glow[i][j] < glowDuration)
								glow[i][j] = glowDuration+Sqrt(abs(index->x-j)*abs(index->x-j)+abs(index->y-i)*abs(index->y-i));
				}
				break;
			case 2:
				if(index->timer == index->duration-1){
					for(int i = 0; i < 20; i++)
						for(int j = 0; j < 10; j++)
							if(glow[i][j] < glowDuration)
								glow[i][j] = glowDuration+abs(index->x-j)+abs(index->y-i);
				}
				break;
			case 3:
				if(index->timer == 0){
					for(int i = 0; i < 20; i++)
						for(int j = 0; j < 10; j++)
							glow[i][j] = glowDuration+abs(index->x-j)+abs(index->y-i);
				}
				break;
			}

			index->timer++;
		}else{
			index = effectList.erase(index);
		}

		std::advance(index,1);
	}

	u16*dest = (u16*)se_mem[26];
	dest+= 10;

	u32 gridTile;
	
	if(savefile->settings.backgroundGrid)
		gridTile = 0x0002;
	else
		gridTile = 0x000c;

	for(int i = 0; i < 20; i++){
		for(int j = 0; j < 10; j++){
			if(glow[i][j] == 0 || !savefile->settings.effects){
				*dest++ = gridTile;
			}else if(glow[i][j] > glowDuration){
				glow[i][j]--;
				*dest++ = gridTile;
			}else if(glow[i][j] > 0){
				glow[i][j]--;
				int color = 0;
				if(glow[i][j] >= glowDuration * 3/4)
					color = 3;
				else if(glow[i][j] >= glowDuration * 2/4)
					color = 2;
				else if(glow[i][j] >= glowDuration * 1/4)
					color = 1;
				*dest++ = gridTile + color * 0x1000;
			}
		}
		dest+=22;
	}
}

void startText(bool onSettings, int selection, int goalSelection, int level, int toStart){

	if(!onSettings){
		// aprint("APOTRIS",12,4);

		int startX = 12;
		int startY = 9;
		int space = 2;

		aprint("Marathon",startX,startY);
		aprint("Sprint",startX,startY+space);
		aprint("Dig",startX,startY+space*2);
		aprint("Settings",startX,startY+space*3);
		aprint("Credits",startX,startY+space*4);

		aprint("akouzoukos",20,19);

		for(int i = 0; i < 5; i++)
			aprint(" ",startX-2,startY+space*i);
		
		aprint(">",startX-2,startY+space*selection);
	}else{
		if(toStart == 2){//Marathon Options
			int levelHeight = 3;
			int goalHeight = 7;

			aprint("Level: ",12,levelHeight);
			aprint("Lines: ",12,goalHeight);
			aprint("START",12,18);
			
			aprint(" ||||||||||||||||||||    ",2,levelHeight+2);
			aprintColor(" 150   200   300   Endless ",1,goalHeight+2,1);

			std::string levelText = std::to_string(level);
			aprint(levelText,27-levelText.length(),levelHeight+2);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,12+i);
				aprint("                       ",5,12+i);
				if(savefile->marathon[goalSelection].highscores[i].score == 0)
					continue;

				aprint(savefile->marathon[goalSelection].highscores[i].name,6,12+i);
				std::string score = std::to_string(savefile->marathon[goalSelection].highscores[i].score);
				
				aprint(score,25-(int)score.length(),12+i);
			}

			aprint(" ",10,18);
			if(selection == 0){
				aprint("<",2,levelHeight+2);
				aprint(">",23,levelHeight+2);
			}else if(selection == 1){
				switch(goalSelection){
				case 0:
					aprint("[",1,goalHeight+2);
					aprint("]",5,goalHeight+2);
					break;
				case 1:
					aprint("[",7,goalHeight+2);
					aprint("]",11,goalHeight+2);
					break;
				case 2:
					aprint("[",13,goalHeight+2);
					aprint("]",17,goalHeight+2);
					break;
				case 3:
					aprint("[",19,goalHeight+2);
					aprint("]",27,goalHeight+2);
					break;
				}
			}else if(selection == 2){
				aprint(">",10,18);
			}
			
			switch(goalSelection){
			case 0:
				aprint("150",2,goalHeight+2);
				break;
			case 1:
				aprint("200",8,goalHeight+2);
				break;
			case 2:
				aprint("300",14,goalHeight+2);
				break;
			case 3:
				aprint("Endless",20,goalHeight+2);
				break;
			}

			// show level cursor
			u16*dest = (u16*)se_mem[29];
			dest+=(levelHeight+2)*32+2+level;

			*dest = 0x5061;

		}else if(toStart == 1){//Sprint Options
			int goalHeight = 5;
			aprint("START",12,18);
			aprint("Lines: ",12,goalHeight);
			aprintColor(" 20   40   100 ",8,goalHeight+2,1);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,12+i);
				aprint("                       ",5,12+i);
				if(savefile->sprint[goalSelection].times[i].frames == 0)
					continue;

				aprint(savefile->sprint[goalSelection].times[i].name,6,12+i);
				std::string time = timeToString(savefile->sprint[goalSelection].times[i].frames);
				
				aprint(time,25-(int)time.length(),12+i);
			}

			aprint(" ",10,18);
			if(selection == 0){
				switch(goalSelection){
				case 0:
					aprint("[",8,goalHeight+2);
					aprint("]",11,goalHeight+2);
					break;
				case 1:
					aprint("[",13,goalHeight+2);
					aprint("]",16,goalHeight+2);
					break;
				case 2:
					aprint("[",18,goalHeight+2);
					aprint("]",22,goalHeight+2);
					break;
				}
			}else if(selection == 1){
				aprint(">",10,18);
			}

			switch(goalSelection){
			case 0:
				aprint("20",9,goalHeight+2);
				break;
			case 1:
				aprint("40",14,goalHeight+2);
				break;
			case 2:
				aprint("100",19,goalHeight+2);
				break;
			}
		}else if(toStart == 3){//Dig Options
			int goalHeight = 5;
			aprint("START",12,18);
			aprint("Lines: ",12,goalHeight);
			aprintColor(" 10   20   100 ",8,goalHeight+2,1);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,12+i);
				aprint("                       ",5,12+i);
				if(savefile->dig[goalSelection].times[i].frames == 0)
					continue;

				aprint(savefile->dig[goalSelection].times[i].name,6,12+i);
				std::string time = timeToString(savefile->dig[goalSelection].times[i].frames);
				
				aprint(time,25-(int)time.length(),12+i);
			}
			
			aprint(" ",10,18);
			if(selection == 0){
				switch(goalSelection){
				case 0:
					aprint("[",8,goalHeight+2);
					aprint("]",11,goalHeight+2);
					break;
				case 1:
					aprint("[",13,goalHeight+2);
					aprint("]",16,goalHeight+2);
					break;
				case 2:
					aprint("[",18,goalHeight+2);
					aprint("]",22,goalHeight+2);
					break;
				}
			}else if(selection == 1){
				aprint(">",10,18);
			}
			
			switch(goalSelection){
			case 0:
				aprint("10",9,goalHeight+2);
				break;
			case 1:
				aprint("20",14,goalHeight+2);
				break;
			case 2:
				aprint("100",19,goalHeight+2);
				break;
			}
		}else if(toStart == -1){
			int startX = 4;
			int startY = 5;
			int space = 1;
			aprint("Voice",startX,startY);
			aprint("Clear Text",startX,startY+space);
			aprint("Screen Shake",startX,startY+space*2);
			aprint("Effects",startX,startY+space*3);
			aprint("Music",startX,startY+space*4);
			aprint("Auto Repeat Delay",startX,startY+space*5);
			aprint("Auto Repeat Rate",startX,startY+space*6);
			aprint("Soft Drop Speed",startX,startY+space*7);
			aprint("Drop Protection",startX,startY+space*8);
			aprint("Background Grid",startX,startY+space*9);
			aprint("Block Edges",startX,startY+space*10);
			aprint(" SAVE ",12,17);

			int endX = 24;

			for(int i = 0; i < 11; i++)
				aprint("      ",endX-1,startY+space*i);
			
			if(savefile->settings.announcer)
				aprint("ON",endX,startY);
			else
				aprint("OFF",endX,startY);

			if(savefile->settings.floatText)
				aprint("ON",endX,startY+space);
			else
				aprint("OFF",endX,startY+space);

			if(savefile->settings.shake)
				aprint("ON",endX,startY+space*2);
			else
				aprint("OFF",endX,startY+space*2);
			
			if(savefile->settings.effects)
				aprint("ON",endX,startY+space*3);
			else
				aprint("OFF",endX,startY+space*3);

			aprint(std::to_string(savefile->settings.volume),endX,startY+space*4);
			
			if(savefile->settings.das == 8)
				aprint("FAST",endX,startY+space*5);
			else if(savefile->settings.das == 11)
				aprint("MID",endX,startY+space*5);
			else if(savefile->settings.das == 16)
				aprint("SLOW",endX,startY+space*5);

			if(savefile->settings.arr == 1)
				aprint("FAST",endX,startY+space*6);
			else if(savefile->settings.arr == 2)
				aprint("MID",endX,startY+space*6);
			else if(savefile->settings.arr == 3)
				aprint("SLOW",endX,startY+space*6);
			
			if(savefile->settings.sfr == 1)
				aprint("FAST",endX,startY+space*7);
			else if(savefile->settings.sfr == 2)
				aprint("MID",endX,startY+space*7);
			else if(savefile->settings.sfr == 3)
				aprint("SLOW",endX,startY+space*7);
			
			if(savefile->settings.dropProtection)
				aprint("ON",endX,startY+space*8);
			else
				aprint("OFF",endX,startY+space*8);

			if(savefile->settings.backgroundGrid)
				aprint("ON",endX,startY+space*9);
			else
				aprint("OFF",endX,startY+space*9);
			
			if(savefile->settings.edges)
				aprint("ON",endX,startY+space*10);
			else
				aprint("OFF",endX,startY+space*10);
			
			if(selection == 0){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.announcer),startY+space*selection);
			}else if(selection == 1){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.floatText),startY+space*selection);
			}else if(selection == 2){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.shake),startY+space*selection);
			}else if(selection == 3){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.effects),startY+space*selection);
			}else if(selection == 4){
				if(savefile->settings.volume > 0)
					aprint("<",endX-1,startY+space*selection);
				if(savefile->settings.volume < 10)
					aprint(">",endX+1,startY+space*selection);
			}else if(selection == 5){
				if(savefile->settings.das > 8)
					aprint(">",endX+3+(savefile->settings.das != 11),startY+space*selection);
				if(savefile->settings.das < 16)
					aprint("<",endX-1,startY+space*selection);
			}else if(selection == 6){
				if(savefile->settings.arr < 3)
					aprint("<",endX-1,startY+space*selection);
				if(savefile->settings.arr > 1)
					aprint(">",endX+3+(savefile->settings.arr != 2),startY+space*selection);
				
			}else if(selection == 7){
				if(savefile->settings.sfr < 3)
					aprint("<",endX-1,startY+space*selection);
				if(savefile->settings.sfr > 1)
					aprint(">",endX+3+(savefile->settings.sfr != 2),startY+space*selection);

			}else if(selection == 8){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.dropProtection),startY+space*selection);
			}else if(selection == 9){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.backgroundGrid),startY+space*selection);
			}else if(selection == 10){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.edges),startY+space*selection);
			}else if(selection == 11){
				aprint("[",12,17);
				aprint("]",17,17);
			}
		}else if(toStart == -2){
			int startX = 4;
			int startY = 3;
			
			int startY2 = 12;

			aprint("Menu Music:",startX,startY);
			aprint("veryshorty-extended",startX,startY+2);
			aprint("by supernao",startX,startY+3);

			
			aprint("In-Game Music:",startX,startY2);
			aprint("Thirno",startX,startY2+2);
			aprint("by Nikku4211",startX,startY2+3);
		}else if(toStart == -3){
			if(connected < 1){
				aprint("Waiting for",8,9);
				aprint("connection...",10,10);	
			}else{
				aprint("Connected!",10,6);
			}
		}
	}
}

void startScreen(){
	int selection = 0;

	int options = 5;

	int toStart = 0;
	bool onSettings = false;
	int level = 1;

	int goalSelection = 0;

	bool refreshText = true; 
	
	int maxDas = 16;
	int dasLeft = 0;
	int dasRight = 0;

	int maxArr = 3;
	int arr = 0;

	while(1){
		VBlankIntrWait();
		if(!onSettings){
			fallingBlocks();
			for(int i = 0; i < 2; i++)
				obj_unhide(titleSprites[i],0);
		}else{
			for(int i = 0; i < 2; i++)
				obj_hide(titleSprites[i]);
		}

		if(refreshText){
			startText(onSettings,selection,goalSelection,level,toStart);
			refreshText = false;
		}

		key_poll();

		u16 key = key_hit(KEY_FULL);

		if(!onSettings){
			if(key_is_down(KEY_SELECT) && key_is_down(KEY_A)){
				delete game;
				game = new Game(1);
				game->setLevel(1);
				game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);
				game->setGoal(0);

				u16*dest = (u16*)se_mem[25];
				memset16(dest,0,20*32);
				break;
			}

			if(key_is_down(KEY_SELECT) && key_is_down(KEY_B)){
				toStart = -3;
				sfx(SFX_MENUCONFIRM);
				clearText();
				onSettings = 3;
				refreshText = true;

				u16*dest = (u16*)se_mem[25];
				memset16(dest,0,20*32);
				
				linkConnection->activate();
			}
			
			if(key == KEY_A){
				int n = 0;
				if(selection == 0){
					n = 2;
					options = 3;
					// maxClearTimer = marathonClearTimer;
				}else if(selection == 1){
					n = 1;
					options = 2;
					goalSelection = 1;// set default goal to 40 lines for sprint
					// maxClearTimer = 1;
				}else if(selection == 2){
					n = 3;
					options = 2;
				}else if(selection == 3){
					n = -1;
					previousSettings = savefile->settings;
					options = 12;
				}else if(selection == 4){
					n = -2;
				}

				sfx(SFX_MENUCONFIRM);
				toStart = n;
				onSettings = true;
				
				u16*dest = (u16*)se_mem[25];
				memset16(dest,0,20*32);

				clearText();
				selection = 0;
				
				refreshText = true;
			}

		}else{
			if(toStart == 2){
				if(selection == 0){
					if(key == KEY_RIGHT && level < 20){
						level++;
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}

					if(key == KEY_LEFT && level > 1){
						level--;
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}

					if(key_is_down(KEY_LEFT)){
						if(dasLeft < maxDas){
							dasLeft++;
						}else if(level > 1){
							if(arr++ > maxArr){
								arr = 0;
								level--;
								sfx(SFX_MENUMOVE);
								refreshText = true;
							}
						}
					}else if(key_is_down(KEY_RIGHT)){
						if(dasRight < maxDas){
							dasRight++;
						}else if(level < 20){
							if(arr++ > maxArr){
								arr = 0;
								level++;
								sfx(SFX_MENUMOVE);
								refreshText = true;
							}
						}
					}else{
						dasLeft = 0;
						dasRight = 0;
					}
				}else if(selection == 1){
					if(key == KEY_RIGHT && goalSelection < 3){
						goalSelection++;
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}

					if(key == KEY_LEFT && goalSelection > 0){
						goalSelection--;
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}
				}
			}else if(toStart == 1 || toStart == 3){
				if(selection == 0){
					if(key == KEY_RIGHT && goalSelection < 2){
						goalSelection++;
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}

					if(key == KEY_LEFT && goalSelection > 0){
						goalSelection--;
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}
				}
			}else if(toStart == -1){
				if(key == KEY_RIGHT || key == KEY_LEFT){
					if(selection == 0){
						savefile->settings.announcer = !savefile->settings.announcer;
					}else if(selection == 1){
						savefile->settings.floatText = !savefile->settings.floatText;
					}else if(selection == 2){
						savefile->settings.shake = !savefile->settings.shake;
					}else if(selection == 3){
						savefile->settings.effects = !savefile->settings.effects;
					}else if(selection == 4){
						if(key == KEY_RIGHT && savefile->settings.volume < 10)
							savefile->settings.volume++;
						else if(key == KEY_LEFT && savefile->settings.volume > 0)
							savefile->settings.volume--;

						mmSetModuleVolume(512*((float)savefile->settings.volume/10));
					}else if(selection == 5){
						if(savefile->settings.das == 11){
							if(key == KEY_LEFT)
								savefile->settings.das = 16; 
							if(key == KEY_RIGHT)
								savefile->settings.das = 8; 
						}else if((key == KEY_RIGHT && savefile->settings.das == 16) || (key == KEY_LEFT && savefile->settings.das == 8)){
							savefile->settings.das = 11; 
						}
					}else if(selection == 6){
						if(key == KEY_RIGHT && savefile->settings.arr > 1)
							savefile->settings.arr--;
						else if(key == KEY_LEFT && savefile->settings.arr < 3)
							savefile->settings.arr++;
					}else if(selection == 7){
						if(key == KEY_RIGHT && savefile->settings.sfr > 1)
							savefile->settings.sfr--;
						else if(key == KEY_LEFT && savefile->settings.sfr < 3)
							savefile->settings.sfr++;
					}else if(selection == 8){
						savefile->settings.dropProtection = !savefile->settings.dropProtection;
					}else if(selection == 9){
						savefile->settings.backgroundGrid = !savefile->settings.backgroundGrid;
					}else if(selection == 10){
						savefile->settings.edges = !savefile->settings.edges;
					}
					if(selection != options-1){
						sfx(SFX_MENUMOVE);
						refreshText = true;
					}
				}
			}else if(toStart == -3){
				if(multiplayerStartTimer){
					if(--multiplayerStartTimer == 0){
						startMultiplayerGame(nextSeed);
						break;
					}else{
						linkConnection->send((u16)nextSeed+100);
					}
				}else{
					auto linkState = linkConnection->linkState.get();

					if(linkState->isConnected()){
						u16 data[LINK_MAX_PLAYERS];
						for (u32 i = 0; i < LINK_MAX_PLAYERS; i++)
							data[i] = 0;
						for (u32 i = 0; i < linkState->playerCount; i++) {
							while (linkState->hasMessage(i))
								data[i] = linkState->readMessage(i);
						}

						for (u32 i = 0; i < LINK_MAX_PLAYERS; i++){
							if(data[i] == 2){
								if(connected < 1){
									refreshText = true;	
									clearText();
								}
								connected = 1;
							} else if(data[i] > 100)
								nextSeed = data[i]-100;
						}

						if(linkState->playerCount == 2){
							if(linkState->currentPlayerId != 0){
								if(nextSeed > 100){
									startMultiplayerGame(nextSeed);
									break;
								}
								aprint("Waiting",12,15);
								aprint("for host...",12,16);
							}else{
								aprint("Press Start",10,15);

							}
						}else if(linkState->playerCount == 1){
							if(connected > -1){
								refreshText = true;	
								clearText();
							}
							connected = -1;
						}

						if(key == KEY_START && linkState->currentPlayerId == 0){
							nextSeed = (u16) qran() & 0x1fff;
							multiplayerStartTimer = 3;
						}else{
							linkConnection->send(2);
						}

						if(key == KEY_SELECT){
							linkConnection->send(5);
						}
						aprint("             ",0,19);
					}else{
						if(connected > -1){
							refreshText = true;	
							clearText();
						}
						connected = -1;
					}
					
				}

			}

			if(key == KEY_A){
				if(selection != options-1){
					selection = options-1;
					sfx(SFX_MENUCONFIRM);
					refreshText = true;
				}else{
					if(toStart != -1){
						if(toStart == 2 && goalSelection == 3)
							toStart = 0;
						
						delete game;
						game = new Game(toStart);
						game->setLevel(level);
						game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);
						mode = goalSelection;

						int goal = 0;

						switch(toStart){
						case 1:
							if(goalSelection == 0)
								goal = 20;
							else if(goalSelection == 1)
								goal = 40;
							else if(goalSelection == 2)
								goal = 100;
							break;
						case 2:
							if(goalSelection == 0)
								goal = 150;
							else if(goalSelection == 1)
								goal = 200;
							else if(goalSelection == 2)
								goal = 300;
							break;
						case 3:
							if(goalSelection == 0)
								goal = 10;
							else if(goalSelection == 1)
								goal = 20;
							else if(goalSelection == 2)
								goal = 100;
							break;
						}

						if(goal)
							game->setGoal(goal);
						
						sfx(SFX_MENUCONFIRM);
						break;
					}else{
						saveToSram();
						onSettings = false;
						options = 5;
						selection = 0;
						clearText();
						refreshText = true;
						sfx(SFX_MENUCONFIRM);
					}

				}
			}

			if(key == KEY_B){
				if(toStart != -1){
					if(selection == 0){
						onSettings = false;
						options = 5;
						clearText();
						sfx(SFX_MENUCANCEL);
						refreshText = true;

					}else{
						selection = 0;
						sfx(SFX_MENUCANCEL);
						refreshText = true;
					}
				}else{
					onSettings = false;
					options = 5;
					clearText();
					sfx(SFX_MENUCANCEL);
					savefile->settings = previousSettings;
					refreshText = true;
				}

				goalSelection = 0;
				selection = 0;

				// if(toStart == -3)
				// 	linkConnection->deactivate();

				sfx(SFX_MENUCONFIRM);
			}
		}
		
		if(key == KEY_UP){
			if(selection == 0)
				selection = options-1;
			else
				selection--;
			sfx(SFX_MENUMOVE);
			refreshText = true;
		}
		
		if(key == KEY_DOWN){
			if(selection == options-1)
				selection = 0;
			else
				selection++;
			sfx(SFX_MENUMOVE);
			refreshText = true;
		}
		
		sqran(qran() * frameCounter++);
		
		oam_copy(oam_mem,obj_buffer,128);
	}
	clearText();
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
		int **p = game->getShape(n,qran()%4);

		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				if(backgroundArray[i][j+x])
					return;

		for(i = 0; i < 4; i++)
			for(j = 0; j < 4; j++)
				if(p[i][j])
					backgroundArray[i][j+x] = n+1; 
	}
}

void endScreen(){
	mmStop();

	int selection = 0;

	sfx(SFX_END);

	endAnimation();
	

	if(game->gameMode != 4){
		if(game->won == 1)
			sfx(SFX_CLEAR);
		else if(game->lost == 1)
			sfx(SFX_GAMEOVER);
	}else{
		if(game->won == 1)
			sfx(SFX_YOUWIN);
		else if(game->lost == 1)
			sfx(SFX_YOULOSE);
	}

	mmStart(MOD_MENU,MM_PLAY_LOOP);
	mmSetModuleVolume(512*((float)savefile->settings.volume/10));

	showStats();
	bool record = onRecord();

	while(1){
		handleMultiplayer();
		VBlankIntrWait();
		key_poll();
		
		showStats();

		if(record && game->won && game->gameMode != 4)
			aprint("New Record",10,7);

		aprint("Play",12,14);
		aprint("Again",14,15);

		aprint("Main",12,17);
		aprint("Menu",14,18);

		if(selection == 0){
			aprint(">",10,14);
			aprint(" ",10,17);
		}else if(selection == 1){
			aprint(" ",10,14);
			aprint(">",10,17);
		}

		u16 key = key_hit(KEY_FULL);

		if(playAgain){
			playAgain = false;
			startMultiplayerGame(nextSeed);
			
			mmStop();
			mmStart(MOD_THIRNO,MM_PLAY_LOOP);
			mmSetModuleVolume(512*((float)savefile->settings.volume/10));
			
			floatingList.clear();

			drawFrame();
			clearText();

			oam_init(obj_buffer,128);
			showFrames();
			oam_copy(oam_mem,obj_buffer,128);
			countdown();
			update();
			break;
		}

		if(key == KEY_A){
			if(!selection){
				shake = 0;
				
				if(!multiplayer){
					sfx(SFX_MENUCONFIRM);
					int goal = game->goal;
					delete game;
					game = new Game(game->gameMode);
					game->setGoal(goal);
					game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);

					mmStop();
					
					floatingList.clear();

					drawFrame();
					clearText();

					oam_init(obj_buffer,128);
					showFrames();
					oam_copy(oam_mem,obj_buffer,128);
					countdown();
					
					if(!(game->gameMode == 1 && game->goal == 0)){
						mmStart(MOD_THIRNO,MM_PLAY_LOOP);
						mmSetModuleVolume(512*((float)savefile->settings.volume/10));
					}

					update();
					break;

				}else{
					if(connected == 1){
						sfx(SFX_MENUCONFIRM);
						multiplayerStartTimer = 3;
						nextSeed = (u16) qran();
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}

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
	showHold();
	showQueue();
	oam_copy(oam_mem,obj_buffer,128);

	REG_BG0HOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG0VOFS = 0;
	REG_BG1VOFS = 0;

	for(int i = 0; i < 41; i++){
		VBlankIntrWait();
		drawGrid();
		showClearText();
		REG_BG0VOFS = -1*i*4;
		u16*dest = (u16*)se_mem[25];

		if(i % 2 != 0)
			continue;

		dest+= 32*(20-i/2)+10;
			
		for(int j = 0; j < 10; j++){
			*dest++ = 0;
		}
	}

	while(!floatingList.empty()){
		VBlankIntrWait();
		showClearText();
	}
	clearText();

	// u16*dest,*dest2;
	// for(int i = 19; i >= 0; i--){
	// 	dest = (u16*)se_mem[25];
	// 	dest += i * 32;

	// 	dest2 = (u16*)se_mem[26];
	// 	dest2 += i * 32;
		
	// 	// int timer = 0;
	// 	VBlankIntrWait();

	// 	for(int j = 0; j < 32; j++)
	// 		*dest++ = *dest2++ = 0;
	// }

	REG_BG0HOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG0VOFS = 0;
	REG_BG1VOFS = 0;
}

void showStats(){
	if(game->gameMode == 4){
		if(game->lost)
			aprint("YOU LOSE",11,4);
		else
			aprint("YOU WIN!",11,4);

		aprint("Lines Sent",10,8);
		aprint(std::to_string(game->linesSent),14,10);
		
	}else if(game->gameMode == 0 || game->gameMode == 2 || game->lost){
		std::string score = std::to_string(game->score);

		if(game->lost)	
			aprint("GAME  OVER",10,4);
		else
			aprint("CLEAR!",12,4);

		if(game->gameMode == 0 || game->gameMode == 2)
			aprint(score,15-((int)score.size()/2),9);
	}else{
		aprint("CLEAR!",12,4);
		
		aprint(timeToString(gameSeconds),11,8);
	}
}

void pauseMenu(){

	int selection = 0;
	int maxSelection = 3;
	
	for(int i = 0; i < 20; i++)
		aprint("          ",10,i);

	aprint("PAUSE!",12,6);
	
	while(1){
		VBlankIntrWait();
		key_poll();

		if(selection == 0){
			aprint(">",10,11);
			aprint(" ",10,13);
			aprint(" ",10,15);
		}else if(selection == 1){
			aprint(" ",10,11);
			aprint(">",10,13);
			aprint(" ",10,15);
		}else if(selection == 2){
			aprint(" ",10,11);
			aprint(" ",10,13);
			aprint(">",10,15);
		}

		u16 key = key_hit(KEY_FULL);

		if(!onStates){
			aprint("Resume",12,11);
			aprint("Restart",12,13);
			aprint("Quit",12,15);

			if(key == KEY_A){
				if(selection == 0){
					sfx(SFX_MENUCONFIRM);
					clearText();
					showText();
					pause = false;
					mmResume();
					break;
				}else if(selection == 1){
					restart = true;
					pause = false;
					sfx(SFX_MENUCONFIRM);
					break;
				}else if(selection == 2){
					sfx(SFX_MENUCONFIRM);
					reset();
				}
			}
		}else{
			aprint("Resume",12,11);
			aprintColor("Load",12,13,!(saveExists));
			aprint("Save",12,15);

			if(key == KEY_A){
				if(selection == 0){
					sfx(SFX_MENUCONFIRM);
					clearText();
					showText();
					pause = false;
					onStates = false;
					mmResume();
					break;
				}else if(selection == 1){
					if(saveExists){
						delete game;
						game = new Game(*quickSave);
						update();
						showBackground();
						floatingList.clear();
						clearGlow();
						
						sfx(SFX_MENUCONFIRM);
						break;
					}else{
						
						sfx(SFX_MENUCANCEL);
					}
				}else if(selection == 2){
					delete quickSave;
					quickSave = new Game(*game);
					saveExists = true;

					// Save *test = new Save();
					// test->savedGame = Game(game);
					// savefile->savedGame = Game(*game);

					sfx(SFX_MENUCONFIRM);
				}
			}
		}

		if(key == KEY_START){
			sfx(SFX_MENUCONFIRM);
			clearText();
			showText();
			pause = false;
			onStates = false;
			mmResume();
			break;
		}

		if(key == KEY_B){
			sfx(SFX_MENUCONFIRM);
			clearText();
			showText();
			pause = false;
			onStates = false;
			mmResume();
			break;
		}
		
		if(key == KEY_UP){
			if(selection == 0)
				selection = maxSelection-1;
			else
				selection--;
			sfx(SFX_MENUMOVE);
		}
		
		if(key == KEY_DOWN){
			if(selection == maxSelection-1)
				selection = 0;
			else
				selection++;
			sfx(SFX_MENUMOVE);
		}

		oam_copy(oam_mem,obj_buffer,128);
	}
}

void addToResults(int input, int index){
	profileResults[index] = input;
}

void reset(){
	oam_init(obj_buffer,128);
	clearText();
	REG_BG0HOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG0VOFS = 0;
	REG_BG1VOFS = 0;

	memset32(&se_mem[25],0x0000,32*20);
	memset32(&se_mem[26],0x0000,32*20);
	memset32(&se_mem[27],0x0000,32*20);

	SoftReset();
	RegisterRamReset(0xff);	
}

void countdown(){
	int timer = 0;
	int timerMax = 120;
	while(timer++ < timerMax-1){
		VBlankIntrWait();
		if(timer < timerMax/3){
			aprint("READY?",12,10);
			if(timer == 1)
				sfx(SFX_READY);
		}else if(timer < 2*timerMax/3)
			// aprint("2",15,10);
			aprint("READY?",12,10);
		else{
			aprint("  GO  ",12,10);
			if(timer == 2*timerMax/3)
				sfx(SFX_GO);
		}
	}
	showBackground();
	clearText();
}

void addGlow(Tetris::Drop location){
	for(int i = 0; i < location.endY && i < 20; i++)
		for(int j = location.startX; j < location.endX; j++)
			glow[i][j] = glowDuration;

	if(game->comboCounter > 0){
		int xCenter = (location.endX-location.startX)/2+location.startX;
		if(game->previousClear.isTSpin){
			for(int i = 0; i < 20; i++)
				for(int j = 0; j < 10; j++)
					glow[i][j] = glowDuration+abs(xCenter-j)+abs(location.endY-i);
			
			if(game->previousClear.isBackToBack == 1){
				effectList.push_back(Effect(2,xCenter,location.endY));
			}
		}else{
			for(int i = 0; i < 20; i++)
				for(int j = 0; j < 10; j++)
					glow[i][j] = glowDuration+Sqrt(abs(xCenter-j)*abs(xCenter-j)+abs(location.endY-i)*abs(location.endY-i));

			if(game->previousClear.isBackToBack == 1){
				effectList.push_back(Effect(1,xCenter,location.endY));
			}

		}
	}

}

void loadSave(){
	savefile = new Save();
	loadFromSram();

	if(savefile->newGame == SAVE_TAG-1){
		savefile->settings.backgroundGrid = true;
		savefile->newGame = SAVE_TAG;
		
	}else if(savefile->newGame != SAVE_TAG){
		// delete savefile;
		savefile = new Save();
		savefile->newGame = SAVE_TAG;

		for(int i = 0; i < 8; i++)
			savefile->latestName[i] = ' ';
		savefile->latestName[8] = '\0';
		
		for(int i = 0; i < 4; i++)
			for(int j = 0; j < 5; j++)
				savefile->marathon[i].highscores[j].score = 0;

		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 5; j++)
				savefile->sprint[i].times[j].frames = 0;

		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 5; j++)
				savefile->dig[i].times[j].frames = 0;

		savefile->settings.announcer = true;
		savefile->settings.finesse = false;
		savefile->settings.floatText = true;
		savefile->settings.shake = true;
		savefile->settings.effects = true;
		savefile->settings.das = 11;
		savefile->settings.arr = 2;
		savefile->settings.sfr = 2;
		savefile->settings.dropProtection = true;

		savefile->settings.volume = 10;

		// savefile->savedGame = Game(0);
		// savefile->canLoad = false;
	}

	saveToSram();
}

void screenShake(){
	if(!savefile->settings.shake)
		return;
	
	REG_BG0VOFS = -shake;
	REG_BG1VOFS = -shake;
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
	
	REG_BG0HOFS = -push;
	REG_BG1HOFS = -push;
	if(game->pushDir != 0){
		if(abs(push) < pushMax)
			push+= game->pushDir;
	}else{
		if(push > 0)
			push--;
		else if(push < 0)
			push++;	
	}
}

std::string nameInput(bool first){
	// std::string result = "        ";

	std::string result = savefile->latestName;
	int cursor = 0;

	for(int i = 0; i < (int) result.size(); i++){
		if(result[i] == ' '){
			cursor = i;
			break;
		}
	}

	const static int nameHeight = 14;

	int timer = 0;

	bool onDone = false;
	while(1){
		VBlankIntrWait();

		if(first)
			aprint("New Record",10,7);
		aprint("Name: ",11,nameHeight-2);

		aprint("DONE",14,16);

		key_poll();

		u16 key = key_hit(KEY_FULL);
		
		aprint(result,13,nameHeight);

		if(!onDone){
			if(key == KEY_A || key == KEY_RIGHT){
				cursor++;
				if(cursor > 7){
					onDone = true;
					sfx(SFX_MENUCONFIRM);
				}else{
					sfx(SFX_MENUMOVE);
				}
			}

			if(key == KEY_B){
				result[cursor] = ' ';
				if(cursor > 0)
					cursor--;
				sfx(SFX_MENUCANCEL);
			}

			if(key == KEY_LEFT){
				if(cursor > 0)
					cursor--;
				sfx(SFX_MENUMOVE);
			}

			if(key == KEY_START){
				onDone = true;
				sfx(SFX_MENUCONFIRM);
			}

			if(key == KEY_UP){
				char curr = result.at(cursor);

				if(curr == 'A')
					result[cursor] = ' ';
				else if(curr == ' ')
					result[cursor] = 'Z';
				else if(curr > 'A')
					result[cursor] = curr-1;

				sfx(SFX_MENUMOVE);
			}
			
			if(key == KEY_DOWN){
				char curr = result.at(cursor);

				if(curr == 'Z')
					result[cursor] = ' ';
				else if(curr == ' ')
					result[cursor] = 'A';
				else if(curr < 'Z')
					result[cursor] = curr+1;
				sfx(SFX_MENUMOVE);
			}

			if(timer++ > 19)
				timer = 0;

			if(timer < 10)
				aprint("_",13+cursor,nameHeight);

			aprint(" ",12,16);
		}else{
			aprint(">",12,16);

			if(key == KEY_A || key == KEY_START){
				sfx(SFX_MENUCONFIRM);
				break;
			}
			
			if(key == KEY_B){
				onDone = false;
				sfx(SFX_MENUCANCEL);
			}
		}
	}

	clearText();

	if(result.size() >= 8)
		result = result.substr(0,8);

	for(int i = 0; i < 8; i++)
		savefile->latestName[i] = result.at(i);

	return result;
}

bool onRecord(){
	bool first = false;

	for(int i = 0; i < 5; i++){
		if(game->gameMode == 0 || game->gameMode == 2){
			if(game->score < savefile->marathon[mode].highscores[i].score)
				continue;

			for(int j = 3; j >= i; j--)
				savefile->marathon[mode].highscores[j+1] = savefile->marathon[mode].highscores[j]; 

			std::string name = nameInput((i==0));

			savefile->marathon[mode].highscores[i].score = game->score;
			strncpy(savefile->marathon[mode].highscores[i].name,name.c_str(),9);
			
		}else if(game->gameMode == 1 && game->won == 1){
			if(gameSeconds > savefile->sprint[mode].times[i].frames && savefile->sprint[mode].times[i].frames)
				continue;

			for(int j = 3; j >= i; j--)
				savefile->sprint[mode].times[j+1] = savefile->sprint[mode].times[j]; 

			std::string name = nameInput((i==0));

			savefile->sprint[mode].times[i].frames = gameSeconds;
			strncpy(savefile->sprint[mode].times[i].name,name.c_str(),9);

		}else if(game->gameMode == 3 && game->won == 1){
			if(gameSeconds > savefile->dig[mode].times[i].frames && savefile->dig[mode].times[i].frames)
				continue;

			for(int j = 3; j >= i; j--)
				savefile->dig[mode].times[j+1] = savefile->dig[mode].times[j]; 

			std::string name = nameInput((i==0));
			
			savefile->dig[mode].times[i].frames = gameSeconds;
			strncpy(savefile->dig[mode].times[i].name,name.c_str(),9);
		}

		first = (i == 0);

		saveToSram();
		break;
	}
	
	return first;
}

void diagnose(){
	
	if(!DIAGNOSE)
		return;

	int statHeight = 4;

	// aprint(std::to_string(game->b2bCounter),0,0);	
	// aprint(std::to_string(game->garbageCleared),0,2);	
	for(int i = 0; i < 2; i++){
		aprint("       ",20,statHeight+i);
		std::string n = std::to_string(profileResults[i]);
		aprint(n,20,statHeight+i);
	}

	// for(int i = 0; i < 5; i ++)
	// 	aprint("     ",0,11+i);

	// std::list<Tetris::Garbage>::iterator index = game->garbageQueue.begin();

	// for(int i = 0; i < (int) game->garbageQueue.size();i++){
	// 	aprint(std::to_string(index->amount),0,11+i);
	// 	aprint(std::to_string(index->timer),2,11+i);
	// 	std::advance(index,1);
	// }

	// aprint(std::to_string(game->currentHeight),0,19);
}

void progressBar(){
	if(game->goal == 0)
		return;
	
	int current;
	int max = game->goal;

	if(game->gameMode != 3)
		current = game->linesCleared;
	else
		current = game->garbageCleared;

	showBar(current,max,20,framePalette);

	if(game->gameMode == 4){
		if(++attackFlashTimer > attackFlashMax)
			attackFlashTimer = 0;

		if(attackFlashTimer < attackFlashMax/2){
			memset32(&pal_bg_mem[8*16+5],0x421f,1);
		}else{
			memset32(&pal_bg_mem[8*16+5],0x7fff,1);
		}
		//attack bar
		showBar(game->getIncomingGarbage(),20,9,8);

		//enemy height

		u16*dest = (u16*)se_mem[27];

		for(int i = 19; i >= 0; i--){
			if(i <= (19-enemyHeight))
				dest[32*i+29] = 0x000d + framePalette + 0x1000;
			else
				dest[32*i+29] = 0x000e + framePalette * 0x1000;
		}
	}
}

void showBar(int current, int max, int x, int palette){
	palette *= 0x1000;

	int pixels = fx2int(fxmul(fxdiv(int2fx(current),int2fx(max)),int2fx(158)));
	int segments = fx2int(fxdiv(int2fx(current),fxdiv(int2fx(max),int2fx(20))));
	int edge = pixels - segments * 8+1*(segments != 0);

	u16*dest = (u16*)se_mem[26];

	dest+=x;
	for(int i = 0; i < 20; i++){

		if(i == 0){
			if(segments == 19){
				if(edge == 0)
					*dest = 0x0006+palette;
				else{
					*dest = 0x000a+palette;
					for(int j = 0; j < 7; j++){
						TILE *tile = &tile_mem[0][10];
						if(j == 0){
							if(6-j > edge)
								tile->data[j+1] = 0x13300332;
							else
								tile->data[j+1] = 0x13344332;
						}else{
							if(6-j > edge)
								tile->data[j+1] = 0x13000032;
							else
								tile->data[j+1] = 0x13444432;
						}
					}
				}
			}else if(segments >= 20)
				*dest = 0x0008+palette;
			else
				*dest = 0x0006+palette;

		}else if(i == 19){
			if(segments == 0){
				if(edge == 0)
					*dest = 0x0806+palette;
				else{
					*dest = 0x080a+palette;
					for(int j = 0; j < 7; j++){
						TILE *tile = &tile_mem[0][10];
						if(j == 0){
							if(j >= edge)
								tile->data[j+1] = 0x13300332;
							else
								tile->data[j+1] = 0x13344332;
						}else{
							if(j >= edge)
								tile->data[j+1] = 0x13000032;
							else
								tile->data[j+1] = 0x13444432;
						}
					}
				}
			}else
				*dest = 0x0808+palette;
		}else{
			if(19-i > segments){
				*dest = 0x0007+palette;
			}else if(19-i == segments){
				*dest = 0x000b+palette;
				for(int j = 0; j < 8; j++){
					TILE *tile = &tile_mem[0][11];
					if(7-j > edge){
						tile->data[j] = 0x13000032;
					}else{
						tile->data[j] = 0x13444432;
					}
				}
			}else{
				*dest = 0x0009+palette;
			}
		}
		
		dest+=32;
	}
}

void showFrames(){
	obj_unhide(holdFrameSprite,0);
	obj_set_attr(holdFrameSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(framePalette));
	holdFrameSprite->attr2 = ATTR2_BUILD(512,framePalette,0);
	obj_set_pos(holdFrameSprite,4*8+5,9*8-2);

	for(int i = 0; i < 3; i++){
		obj_unhide(queueFrameSprites[i],0);
		obj_set_attr(queueFrameSprites[i],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(framePalette));
		queueFrameSprites[i]->attr2 = ATTR2_BUILD(512+16+16*i,framePalette,0);
		obj_set_pos(queueFrameSprites[i],173,12+32*i);
	}
}

void clearGlow(){
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 10; j++)
			glow[i][j] = 0;
	drawGrid();
}

void handleMultiplayer(){
	if(!multiplayer)
		return;
	// if(game->attack)
	if(multiplayerStartTimer){
		if(--multiplayerStartTimer == 0){
			// startMultiplay8erGame(nextSeed & 0x1fff);
			playAgain = true;
		}else{
			linkConnection->send((u16)((5<<13)+(nextSeed&0x1fff)));
			aprint("here",0,0);
		}
		return;
	}
	// 	aprint(std::to_string(game->attack),0,0);
	// // auto linkState = linkConnection.s
	// if()	
	
	auto linkState = linkConnection->linkState.get();
	
	int receivedAttack = false;
	
	u16 data[LINK_MAX_PLAYERS];
	for (int i = 0; i < LINK_MAX_PLAYERS; i++)
		data[i] = 0;

	if(linkState->isConnected()){
		for (u32 i = 0; i < linkState->playerCount; i++)
			while (linkState->hasMessage(i))
				data[i] = linkState->readMessage(i);

		for (u32 i = 0; i < linkState->playerCount; i++){
		
			int command = data[i] >> 13;
			int value = data[i] & 0x1fff;

			switch(command){
			case 0:
				continue;
			case 1:
				enemyHeight = value;
				break;
			case 2:
				game->setWin();
				break;
			case 3://receive attack
				game->addToGarbageQueue(value>>4,value & 0xf);
				receivedAttack = value>>4;
				break;
			case 4://acknowledge attack
				game->clearAttack(value);
				break;
			case 5:
				playAgain = true;
				nextSeed = value;
				break;
			}

		}
			// if(data[i])
			// 	aprint(std::to_string(data[i]),0,7+i);
		if(game->lost){
			linkConnection->send((u16)2<<13);
		}else{
			if(receivedAttack){
				linkConnection->send((u16)((4<<13) + receivedAttack));

			}else if(game->attackQueue.size() == 0){
				// linkConnection->send(linkState->currentPlayerId);
				linkConnection->send((u16)((1<<13) + game->currentHeight));
			}else{
				Tetris::Garbage atck = game->attackQueue.front();

				linkConnection->send((u16)((3<<13) + (atck.id<<4)+atck.amount));
			}
		}
	}else{
		// aprint("0",0,0);
		// multiplayer = false;
		game->setWin();
		// aprint("no connection.",0,5);
		connected = -1;
		aprint("Connection Lost.",0,0);
	}
}

void startMultiplayerGame(int seed){
	delete game;
	game = new Game(4,seed);
	game->setLevel(1);
	game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);
	game->setGoal(100);
	multiplayer = true;
	linkConnection->send(50);
	nextSeed = 0;
}

void setSkin(){
	switch(skinSelect){
	case 0:
		blockSprite = (u8*)sprite1tiles_bin;
		break;
	case 1:
		blockSprite = (u8*)sprite7tiles_bin;
		break;
	case 2:
		blockSprite = (u8*)sprite9tiles_bin;
		break;
	case 3:
		blockSprite = (u8*)sprite14tiles_bin;
		break;
	}
	
	memcpy16(&tile_mem[0][1],blockSprite,sprite1tiles_bin_size/2);
	memcpy16(&tile_mem[2][97],blockSprite,sprite1tiles_bin_size/2);

	int **board;
	for(int i = 0; i < 7; i++){
		board = game->getShape(i,0);
		for(int j = 0; j < 4; j++){
			for(int k = 0; k < 4; k++){
				if(board[j][k]){
					memcpy16(&tile_mem[4][16*i+j*4+k],blockSprite,sprite1tiles_bin_size/2);
				}
			}
		}
	}
	for(int i = 0; i < 4; i++)
		delete [] board[i];
	delete [] board;
	
}