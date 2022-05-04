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

#define SHOW_FINESSE 1
#define DIAGNOSE 0
#define SAVE_TAG 0x4c

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
void graphicTest();
void showTitleSprites();
void drawEnemyBoard(int);
void handleBotGame();
void setLightMode();
void sleep();
void playSong(int,int);
void playSongRandom(int);
void drawUIFrame(int,int,int,int);
void songListMenu();
void settingsText();

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

OBJ_ATTR *enemyBoardSprite;

int backgroundArray[24][30];
int bgSpawnBlock = 0;
int bgSpawnBlockMax = 20;
int gravity = 0;
int gravityMax = 10;

int shakeMax = 5;
int shake = 0;

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
int initialLevel = 0;

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
bool playingBotGame = false;

int multiplayerStartTimer = 0;

int attackCheckCooldown = 10;
int attackCheckTimer = 0;

int connected = 0;

#define TRAINING_MESSAGE_MAX 300

int trainingMessageTimer = 0;


#define MAX_SKINS 6
#define MAX_SHADOWS 4
#define MAX_BACKGROUNDS 3

#define MAX_MENU_SONGS 2
#define MAX_GAME_SONGS 4

int currentScanHeight = 0;

int enemyBoard[20][10];

int timeoutTimer = 0;
int maxTimeout = 10;

std::list<std::string> menuOptions = {"Play","Settings","Credits"};
std::list<std::string> gameOptions = {"Marathon","Sprint","Dig","Ultra","2P Battle","Training"};

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

class WordSprite{
public:
	std::string text = "";
	int startIndex;
	int startTiles;
	OBJ_ATTR * sprites[3];
	
	void show(int x, int y,int palette){
		for(int i = 0; i < 3; i++){
			obj_unhide(sprites[i],0);
			obj_set_attr(sprites[i],ATTR0_WIDE,ATTR1_SIZE(1),palette);
			sprites[i]->attr2 = ATTR2_BUILD(startTiles+i*4,palette,1);
			obj_set_pos(sprites[i],x+i*32,y);
		}
	}

	void hide(){
		for(int i = 0; i < 3; i++)
			obj_hide(sprites[i]);
	}

	void setText(std::string _text){
		if(_text == text)
			return;

		text = _text;

		TILE * font = (TILE*)fontTiles;
		for(int i = 0; i < (int) text.length() && i < 12; i++){
			int c = text[i]-32;

			memcpy32(&tile_mem[4][startTiles+i],&font[c],8);
		}
	}

	WordSprite(int _index, int _tiles){
		startIndex = _index;
		startTiles = _tiles;

		for(int i = 0; i < 3; i++){
			sprites[i] = &obj_buffer[startIndex+i];
		}
	}
};

WordSprite *wordSprites[10];

void onVBlank(void){

	mmVBlank();
	LINK_ISR_VBLANK();

	if(canDraw){
		canDraw = 0;

		control();
		showPawn();
		showShadow();
		showHold();
		showQueue();
		drawGrid();
		showTimer();
		screenShake();

		showClearText();

		if(game->refresh){
			showBackground();
			update();
			game->resetRefresh();
		}else if(game->clearLock)
			showBackground();

		oam_copy(oam_mem,obj_buffer,21);
		obj_aff_copy((OBJ_AFFINE*)oam_mem,obj_aff_buffer,10);
	}
	mmFrame();
}

// #define GRADIENT_COLOR 0x1a9d
#define GRADIENT_COLOR 0x71a6


void onHBlank(){
	if(REG_VCOUNT < 160){
		clr_fade_fast((COLOR*)palette,GRADIENT_COLOR,pal_bg_mem,2,(REG_VCOUNT/20)*2+16);
		memcpy16(&pal_bg_mem[1],&palette[1],1);
	}else{
		clr_fade_fast((COLOR*)palette,GRADIENT_COLOR,pal_bg_mem,2,16);
		memcpy16(&pal_bg_mem[1],&palette[1],1);
	}
}

int main(void) {
	irq_init(NULL);
	irq_enable(II_VBLANK);
	irq_add(II_VBLANK,onVBlank);
	
	irq_add(II_SERIAL,LINK_ISR_SERIAL);
	irq_add(II_TIMER3,LINK_ISR_TIMER);

	irq_add(II_HBLANK,onHBlank);
	irq_disable(II_HBLANK);

	mmInitDefault((mm_addr)soundbank_bin,10);

	REG_BG0CNT = BG_CBB(0) | BG_SBB(25) | BG_SIZE(0) | BG_PRIO(2);
	REG_BG1CNT = BG_CBB(0) | BG_SBB(26) | BG_SIZE(0) | BG_PRIO(3);
	REG_BG2CNT = BG_CBB(2) | BG_SBB(29) | BG_SIZE(0) | BG_PRIO(0);
	REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
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
	memcpy16(&tile_mem[0][26],sprite17tiles_bin,sprite17tiles_bin_size/2);
	memcpy16(&tile_mem[0][27],sprite20tiles_bin,sprite20tiles_bin_size/2);
	
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

	memcpy16(&pal_obj_mem[13*16], title_pal_bin,title_pal_bin_size/2);
	
	// REG_BLDCNT = BLD_BUILD(BLD_BG1,BLD_BACKDROP,BLD_STD);
	REG_BLDCNT = (1<<6)+(1<<13)+(1<<1);
	REG_BLDALPHA = BLD_EVA(31) | BLD_EVB(2);
	
	//initialise background array
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 30; j++)
			backgroundArray[i][j] = 0;

	for(int i = 0; i < 10; i++){
		wordSprites[i] = new WordSprite(64+i*3,256+i*12);
	}

	loadSave();
	
	// //load mini sprite tiles
	// for(int i = 0; i < 7; i++)
	// 	memcpy16(&tile_mem[4][9*16+i*8],mini[1][i],16*7);
	
	setSkin();
	setLightMode();
	
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

	oam_init(obj_buffer,128);

	// linkConnection->deactivate();

	playSongRandom(0);

	showTitleSprites();

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
		playSongRandom(1);
	}

	clearText();
	update();
	
	while (1) {
		diagnose();
		if(!game->lost && !pause){
			game->update();
		}
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

		if(trainingMessageTimer < TRAINING_MESSAGE_MAX){
			if(++trainingMessageTimer == TRAINING_MESSAGE_MAX){
				aprint("     ",1,3);
				aprint("      ",1,5);
				aprint("         ",1,7);
			}
		}

		if(restart){
			restart = false;
			int oldGoal = game->goal;

			mmStop();

			delete game;
			game = new Game(game->gameMode);
			game->setGoal(oldGoal);
			game->setLevel(initialLevel);
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
				playSongRandom(1);
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
	// showQueue();
	showText();

	if(game->goal == 0){

	}

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
				*dest++ = n + (savefile->settings.lightMode*0x1000);
			}else 
				*dest++ = (1+(((u32)(game->board[i][j]-1)) << 12));
			if(game->clearLock && i == *l2c){
				dest--;
				if(!showEdges)
					*dest = 0;
				if(j < 5){
					if(clearTimer < maxClearTimer-10+j*2)
						*dest = 3 + savefile->settings.lightMode*0x1000;
				}else{
					if(clearTimer < maxClearTimer-10+(10-j)*2)
						*dest = 3 + savefile->settings.lightMode*0x1000;
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
				memset16(&tile_mem[4][16*7+i*4+j],0,sprite1tiles_bin_size/2);
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

	u8 *shadowTexture;

	bool bld = false;
	switch(savefile->settings.shadow){
	case 0:	
		shadowTexture = (u8*) sprite2tiles_bin;
		break;
	case 1:	
		shadowTexture = (u8*) sprite15tiles_bin;
		break;
	case 2:	
		shadowTexture = (u8*) sprite16tiles_bin;
		break;
	case 3:
		shadowTexture = blockSprite;
		bld = true;
		break;
	default:
		shadowTexture = (u8*) sprite2tiles_bin;
		break;
	}

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			if(game->pawn.board[game->pawn.rotation][i][j] > 0)
				memcpy16(&tile_mem[4][16*8+i*4+j],shadowTexture,sprite2tiles_bin_size/2);
			else
				memset16(&tile_mem[4][16*8+i*4+j],0,sprite2tiles_bin_size/2);
		}
	}

	int n = game->pawn.current;

	if(!savefile->settings.lightMode)
		clr_fade_fast((COLOR *) &palette[n*16],0x0000,&pal_obj_mem[8*16],16,(10)*bld);
	else
		clr_adj_brightness(&pal_obj_mem[8*16],(COLOR *) &palette[n*16],16,float2fx(0.15));

	obj_set_attr(pawnShadow,ATTR0_SQUARE,ATTR1_SIZE(2),8);

	pawnShadow->attr2 = ATTR2_BUILD(16*8,8,1);
	obj_set_pos(pawnShadow,(10+game->pawn.x)*8+push*savefile->settings.shake,(game->lowest()-20)*8+shake*savefile->settings.shake);
}

void showHold(){
	obj_unhide(holdFrameSprite,0);
	obj_set_attr(holdFrameSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(savefile->settings.palette));
	holdFrameSprite->attr2 = ATTR2_BUILD(512,savefile->settings.palette,1);
	obj_set_pos(holdFrameSprite,4*8+5+(push < 0)*push,9*8-2);

	if(game->held == -1){
		obj_hide(holdSprite);
		return;
	}

	int add = !(game->held == 0 || game->held == 3);

	if(savefile->settings.skin == 0 || savefile->settings.skin == 5){
		obj_unhide(holdSprite,0);
		obj_set_attr(holdSprite,ATTR0_WIDE,ATTR1_SIZE(2),ATTR2_PALBANK(game->held));
		holdSprite->attr2 = ATTR2_BUILD(9*16+8*game->held,game->held,1);
		obj_set_pos(holdSprite,(5)*8+add*3+1+(push<0)*push,(10)*8-3*(game->held == 0));
	}else{
		obj_unhide(holdSprite,ATTR0_AFF);
		obj_set_attr(holdSprite,ATTR0_WIDE | ATTR0_AFF,ATTR1_SIZE(2) | ATTR1_AFF_ID(5),ATTR2_PALBANK(game->held));
		holdSprite->attr2 = ATTR2_BUILD(16*game->held,game->held,1);
		FIXED size = float2fx(1.4);
		obj_aff_scale(&obj_aff_buffer[5],size,size);
		obj_set_pos(holdSprite,(5)*8+add*3+1-4+(push < 0)*push,(10)*8-3*(game->held == 0)-3);
	}
}

void showQueue(){
	for(int i = 0; i < 3; i++){
		obj_unhide(queueFrameSprites[i],0);
		obj_set_attr(queueFrameSprites[i],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(savefile->settings.palette));
		queueFrameSprites[i]->attr2 = ATTR2_BUILD(512+16+16*i,savefile->settings.palette,1);
		obj_set_pos(queueFrameSprites[i],173+(push > 0)*push,12+32*i);
	}
	
	std::list<int>::iterator q = game->queue.begin();
	for(int k = 0; k < 5; k++){

		int n = *q;

		int add = !(n == 0 || n == 3);
		if(savefile->settings.skin == 0 || savefile->settings.skin == 5){
			obj_unhide(queueSprites[k],0);
			obj_set_attr(queueSprites[k],ATTR0_WIDE,ATTR1_SIZE(2),ATTR2_PALBANK(n));
			queueSprites[k]->attr2 = ATTR2_BUILD(16*9+8*n,n,1);
			obj_set_pos(queueSprites[k],(22)*8+add*3+1+(push > 0)*push,(3+(k*3))*6-3*(n == 0));
		}else{
			obj_unhide(queueSprites[k],ATTR0_AFF);
			obj_set_attr(queueSprites[k],ATTR0_SQUARE | ATTR0_AFF,ATTR1_SIZE(2) | ATTR1_AFF_ID(k),ATTR2_PALBANK(n));
			queueSprites[k]->attr2 = ATTR2_BUILD(16*n,n,1);
			obj_aff_identity(&obj_aff_buffer[k]);
			FIXED size = 358;//~1.4
			obj_aff_scale(&obj_aff_buffer[k],size,size);
			obj_set_pos(queueSprites[k],(22)*8+add*3+1-4+(push>0)*push,(3+(k*3))*6-3*(n == 0)-4);
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
	if(game->gameMode == 0 || game->gameMode == 2 || game->gameMode == 5){

		aprint("Score",3,3);

		std::string score = std::to_string(game->score);
		aprint(score,8-score.size(),5);

		if(game->gameMode != 5){
			aprint("Level",2,14);

			std::string level = std::to_string(game->level);
			aprint(level,4,15);
		}

	}else if(game->gameMode == 1){
		if(savefile->settings.finesse){
			aprint("Finesse",1,14);
			std::string finesse = std::to_string(game->finesse);
			aprint(finesse,4,15);
		}
		if(game->goal == 0){
			aprint("Training",1,1);
			// std::string bagCount = std::to_string(game->bagCounter-1)
			// aprint(bagCount,26,1);
		}
	}

	if(game->gameMode!= 4){
		aprint("Lines",2,17);
		std::string lines;
		if(game->gameMode == 3)
			lines = std::to_string(game->garbageCleared);
		else
			lines = std::to_string(game->linesCleared);
		aprint(lines,4,18);
	}else{
		aprint("Attack",2,17);
		std::string lines = std::to_string(game->linesSent);
		aprint(lines,4,18);
	}

	if(game->goal == 0 && trainingMessageTimer < TRAINING_MESSAGE_MAX){
		aprint("Press",1,3);
		aprint("SELECT",1,5);
		aprint("for Saves",1,7);	
	}
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
	int t = (int) frames*0.0167f;
	int millis = (int) (frames*1.67f) % 100;
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
			*dest++ = 0x0004 + j*0x400 + savefile->settings.palette * 0x1000;
			dest+=10;
		}
		dest+=32-22;
	}

	progressBar();

	for(int i = 0; i < 3; i++){
		obj_unhide(queueFrameSprites[i],0);
		obj_set_attr(queueFrameSprites[i],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(savefile->settings.palette));
		queueFrameSprites[i]->attr2 = ATTR2_BUILD(512+16+16*i,savefile->settings.palette,0);
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
	int palOffset = 4;
	
	switch(savefile->settings.backgroundGrid){
	case 0:
		gridTile = 0x0002;
		break;	
	case 1:
		gridTile = 0x000c;
		break;	
	case 2:
		gridTile = 0x001a;
		break;
	default:
		gridTile = 0x0002;
		break;
	}

	if(savefile->settings.lightMode)
		gridTile += palOffset * 0x1000;

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

		aprint("akouzoukos",20,19);

	}else{
		if(toStart == 2){//Marathon Options
			int levelHeight = 2;
			int goalHeight = 6;

			aprint("Level: ",12,levelHeight);
			aprint("Lines: ",12,goalHeight);
			aprint("START",12,17);
			
			aprint(" ||||||||||||||||||||    ",2,levelHeight+2);
			aprintColor(" 150   200   300   Endless ",1,goalHeight+2,1);

			std::string levelText = std::to_string(level);
			aprint(levelText,27-levelText.length(),levelHeight+2);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,11+i);
				aprint("                       ",5,11+i);
				if(savefile->marathon[goalSelection].highscores[i].score == 0)
					continue;

				aprint(savefile->marathon[goalSelection].highscores[i].name,6,11+i);
				std::string score = std::to_string(savefile->marathon[goalSelection].highscores[i].score);
				
				aprint(score,25-(int)score.length(),11+i);
			}

			aprint(" ",10,17);
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
				aprint(">",10,17);
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
			int goalHeight = 4;
			aprint("START",12,17);
			aprint("Lines: ",12,goalHeight);
			aprintColor(" 20   40   100 ",8,goalHeight+2,1);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,11+i);
				aprint("                       ",5,11+i);
				if(savefile->sprint[goalSelection].times[i].frames == 0)
					continue;

				aprint(savefile->sprint[goalSelection].times[i].name,6,11+i);
				std::string time = timeToString(savefile->sprint[goalSelection].times[i].frames);
				
				aprint(time,25-(int)time.length(),11+i);
			}

			aprint(" ",10,17);
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
				aprint(">",10,17);
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
			int goalHeight = 4;
			aprint("START",12,17);
			aprint("Lines: ",12,goalHeight);
			aprintColor(" 10   20   100 ",8,goalHeight+2,1);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,11+i);
				aprint("                       ",5,11+i);
				if(savefile->dig[goalSelection].times[i].frames == 0)
					continue;

				aprint(savefile->dig[goalSelection].times[i].name,6,11+i);
				std::string time = timeToString(savefile->dig[goalSelection].times[i].frames);
				
				aprint(time,25-(int)time.length(),11+i);
			}
			
			aprint(" ",10,17);
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
				aprint(">",10,17);
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
		}else if(toStart == 5){//Ultra Options
			int goalHeight = 4;
			aprint("START",12,17);
			aprint("Minutes: ",12,goalHeight);
			aprintColor(" 3    5    10 ",8,goalHeight+2,1);

			for(int i = 0; i < 5; i++){
				aprint(std::to_string(i+1)+".",3,11+i);
				aprint("                       ",5,11+i);
				if(savefile->ultra[goalSelection].highscores[i].score == 0)
					continue;

				aprint(savefile->ultra[goalSelection].highscores[i].name,6,11+i);
				std::string score = timeToString(savefile->ultra[goalSelection].highscores[i].score);
				
				aprint(score,25-(int)score.length(),11+i);
			}

			aprint(" ",10,17);
			if(selection == 0){
				switch(goalSelection){
				case 0:
					aprint("[",8,goalHeight+2);
					aprint("]",10,goalHeight+2);
					break;
				case 1:
					aprint("[",13,goalHeight+2);
					aprint("]",15,goalHeight+2);
					break;
				case 2:
					aprint("[",18,goalHeight+2);
					aprint("]",21,goalHeight+2);
					break;
				}
			}else if(selection == 1){
				aprint(">",10,17);
			}

			switch(goalSelection){
			case 0:
				aprint("3",9,goalHeight+2);
				break;
			case 1:
				aprint("5",14,goalHeight+2);
				break;
			case 2:
				aprint("10",19,goalHeight+2);
				break;
			}
		}else if(toStart == -1){
			int startY = 4;
			int space = 1;
			aprint(" SAVE ",12,17);

			int endX = 23;

			for(int i = 0; i < 11; i++)
				aprint("       ",endX-1,startY+space*i);
			
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
			
			// if(savefile->settings.effects)
			// 	aprint("ON",endX,startY+space*3);
			// else
			// 	aprint("OFF",endX,startY+space*3);

			aprint(std::to_string(savefile->settings.volume),endX,startY+space*3);
			
			
			if(savefile->settings.das == 8)
				aprint("V.FAST",endX,startY+space*4);
			else if(savefile->settings.das == 9)
				aprint("FAST",endX,startY+space*4);
			else if(savefile->settings.das == 11)
				aprint("MID",endX,startY+space*4);
			else if(savefile->settings.das == 16)
				aprint("SLOW",endX,startY+space*4);

			if(savefile->settings.arr == 1)
				aprint("FAST",endX,startY+space*5);
			else if(savefile->settings.arr == 2)
				aprint("MID",endX,startY+space*5);
			else if(savefile->settings.arr == 3)
				aprint("SLOW",endX,startY+space*5);
			
			if(savefile->settings.sfr == 1)
				aprint("FAST",endX,startY+space*6);
			else if(savefile->settings.sfr == 2)
				aprint("MID",endX,startY+space*6);
			else if(savefile->settings.sfr == 3)
				aprint("SLOW",endX,startY+space*6);
			
			if(savefile->settings.dropProtection)
				aprint("ON",endX,startY+space*7);
			else
				aprint("OFF",endX,startY+space*7);

			if(savefile->settings.finesse)
				aprint("ON",endX,startY+space*8);
			else
				aprint("OFF",endX,startY+space*8);

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
				if(savefile->settings.volume > 0)
					aprint("<",endX-1,startY+space*selection);
				if(savefile->settings.volume < 10)
					aprint(">",endX+1,startY+space*selection);
			}else if(selection == 4){
				if(savefile->settings.das > 8)
					aprint(">",endX+3+(savefile->settings.das != 11),startY+space*selection);
				if(savefile->settings.das < 16)
					aprint("<",endX-1,startY+space*selection);
			}else if(selection == 5){
				if(savefile->settings.arr < 3)
					aprint("<",endX-1,startY+space*selection);
				if(savefile->settings.arr > 1)
					aprint(">",endX+3+(savefile->settings.arr != 2),startY+space*selection);
				
			}else if(selection == 6){
				if(savefile->settings.sfr < 3)
					aprint("<",endX-1,startY+space*selection);
				if(savefile->settings.sfr > 1)
					aprint(">",endX+3+(savefile->settings.sfr != 2),startY+space*selection);
			}else if(selection == 7){

				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.dropProtection),startY+space*selection);
			}else if(selection == 8){

				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2+(!savefile->settings.finesse),startY+space*selection);
			}else if(selection == 9 || selection == 10){
				aprint("[",endX-1,startY+space*selection);
				aprint("]",endX+2,startY+space*selection);
			}else if(selection == 11){
				aprint("[",12,17);
				aprint("]",17,17);
			}
		}else if(toStart == -2){
			int startX = 4;
			int startY = 2;
			
			int startY2 = 9;

			aprint("Menu Music:",startX-1,startY);
			aprint("-veryshorty-extended",startX,startY+1);
			aprint("by supernao",startX+3,startY+2);
			aprint("-optikal innovation",startX,startY+3);
			aprint("by substance",startX+3,startY+4);

			
			aprint("In-Game Music:",startX-1,startY2);
			aprint("-Thirno",startX,startY2+1);
			aprint("by Nikku4211",startX+3,startY2+2);
			aprint("-oh my god!",startX,startY2+3);
			aprint("by kb-zip",startX+3,startY2+4);
			aprint("-unsuspected <h>",startX,startY2+5);
			aprint("by curt cool",startX+3,startY2+6);
			aprint("-Warning Infected!",startX,startY2+7);
			aprint("by Basq",startX+3,startY2+8);
		}else if(toStart == -3){
			if(connected < 1){
				aprint("Waiting for",7,7);
				aprint("Link Cable",9,9);
				aprint("connection...",11,11);
			}else{
				aprint("Connected!",10,6);
			}
		}
	}
}

void startScreen(){
	int selection = 0;
	int previousSelection = 0;

	int options = (int) menuOptions.size();

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

	bool onPlay = false;
	
	REG_DISPCNT &= ~DCNT_BG1;
	REG_DISPCNT &= ~DCNT_BG3;
	drawUIFrame(0,0,30,20);	

	while(1){
		VBlankIntrWait();
		if(!onSettings){
			irq_disable(II_HBLANK);
		}else{
			irq_enable(II_HBLANK);
		}
		
		if(refreshText){
			refreshText = false;
			if(onSettings){
				REG_DISPCNT |= DCNT_BG1;
				REG_DISPCNT |= DCNT_BG3;
			}else{
				REG_DISPCNT &= ~DCNT_BG1;
				REG_DISPCNT &= ~DCNT_BG3;
			}
			
			startText(onSettings,selection,goalSelection,level,toStart);
		}	

		key_poll();

		u16 key = key_hit(KEY_FULL);


		if(!onSettings){
			
			fallingBlocks();
			for(int i = 0; i < 2; i++)
				obj_unhide(titleSprites[i],0);
			if(savefile->settings.lightMode)
				memset16(pal_bg_mem,0x5ad6,1);//background gray
			else
				memset16(pal_bg_mem,0x0000,1);

			int startX,startY,space = 2;
			startX = 12;
			startY = 11;
			
			std::list<std::string>::iterator index = menuOptions.begin();
			for(int i = 0; i < (int) menuOptions.size(); i++){
				wordSprites[i]->setText(*index);
				wordSprites[i]->show((startX-5*onPlay)*8,(startY+i*space)*8,15-!((onPlay && i == 0)|| (!onPlay && selection == i)));
				++index;
			}

			int offset = (int) menuOptions.size();
			
			index = gameOptions.begin();
			for(int i = 0; i < (int) gameOptions.size(); i++){
				wordSprites[i+offset]->setText(*index);
				int height = i-selection;
				if(onPlay && height >= -2 && height < 4){
					wordSprites[i+offset]->show((startX+5)*8,(startY+height*space)*8,15-(selection!=i));
				} else
					wordSprites[i+offset]->hide();
				++index;
			}
			
			for(int i = 0; i < 5; i++)
				aprint(" ",startX-2,startY+space*i);
			if(!onPlay){
				aprint(">",startX-2,startY+space*selection);
				aprint(" ",15,startY);
			}else{
				aprint(">",15,startY);
			}
			
			if(key == KEY_A || key == KEY_START){
				int n = 0;
				if(!onPlay){
					if(selection == 0){
						onPlay = !onPlay;
						if(onPlay)
							options = (int) gameOptions.size();
						else
							options = (int) menuOptions.size();
					}else if(selection == 1){
						n = -1;
						previousSettings = savefile->settings;
						options = 12;
					}else if(selection == 2){
						n = -2;
					}

				}else{
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
						options = 2;
						n = 5;
					}else if(selection == 4){
						n = -3;
						linkConnection->activate();
						
					}else if(selection == 5){
						sfx(SFX_MENUCONFIRM);
						delete game;
						game = new Game(1);
						game->setLevel(1);
						game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);
						game->setGoal(0);

						memset16(&se_mem[25],0,20*32);
						memset16(&se_mem[26],0,20*32);
						memset16(&se_mem[27],0,20*32);
						
						REG_DISPCNT |= DCNT_BG1;
						REG_DISPCNT |= DCNT_BG3;

						clearText();
						break;
					}
				}

				sfx(SFX_MENUCONFIRM);
				if(n != 0){
					toStart = n;
					onSettings = true;
					
					u16*dest = (u16*)se_mem[25];
					memset16(dest,0,20*32);

					clearText();
					previousSelection = selection;
					selection = 0;
					
					refreshText = true;
					if(n == -1)
						settingsText();
				}
			}

			if(key == KEY_B){
				if(onPlay){
					onPlay = false;
					selection = 0;
					options = (int) menuOptions.size();
					sfx(SFX_MENUCANCEL);
				}
			}

		}else{
			for(int i = 0; i < 10; i++)
				wordSprites[i]->hide();
			
			for(int i = 0; i < 2; i++)
				obj_hide(titleSprites[i]);

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
			}else if(toStart == 1 || toStart == 3 || toStart == 5){
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
						if(key == KEY_RIGHT && savefile->settings.volume < 10)
							savefile->settings.volume++;
						else if(key == KEY_LEFT && savefile->settings.volume > 0)
							savefile->settings.volume--;

						mmSetModuleVolume(512*((float)savefile->settings.volume/10));
					}else if(selection == 4){
						if(key == KEY_RIGHT){
							if(savefile->settings.das == 16)
								savefile->settings.das = 11;
							else if(savefile->settings.das == 11)
								savefile->settings.das = 9;
							else if(savefile->settings.das == 9)
								savefile->settings.das = 8;
						}else if(key == KEY_LEFT){
							if(savefile->settings.das == 8)
								savefile->settings.das = 9;
							else if(savefile->settings.das == 9)
								savefile->settings.das = 11;
							else if(savefile->settings.das == 11)
								savefile->settings.das = 16;
						}
					}else if(selection == 5){
						if(key == KEY_RIGHT && savefile->settings.arr > 1)
							savefile->settings.arr--;
						else if(key == KEY_LEFT && savefile->settings.arr < 3)
							savefile->settings.arr++;
					}else if(selection == 6){
						if(key == KEY_RIGHT && savefile->settings.sfr > 1)
							savefile->settings.sfr--;
						else if(key == KEY_LEFT && savefile->settings.sfr < 3)
							savefile->settings.sfr++;
					}else if(selection == 7){
						savefile->settings.dropProtection = !savefile->settings.dropProtection;
					}else if(selection == 8){
						savefile->settings.finesse = !savefile->settings.finesse;
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

			if(key == KEY_A || (key == KEY_START && toStart != -3)){
				if(onSettings && toStart == -1 && selection == options-3 && key == KEY_A){
					clearText();
					sfx(SFX_MENUCONFIRM);
					graphicTest();
					clearText();
					refreshText = true;
					settingsText();
				}else if(onSettings && toStart == -1 && selection == options-2 && key == KEY_A){
					clearText();
					sfx(SFX_MENUCONFIRM);
					songListMenu();
					clearText();
					refreshText = true;
					settingsText();
				}else if(selection != options-1 && toStart != -2){
					selection = options-1;
					sfx(SFX_MENUCONFIRM);
					refreshText = true;
				}else{
					if(toStart != -1 && toStart != -2){
						if(toStart == 2 && goalSelection == 3)
							toStart = 0;
						
						initialLevel = level;

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
						case 5:
							if(goalSelection == 0)
								goal = 3*3600;
							else if(goalSelection == 1)
								goal = 5*3600;
							else if(goalSelection == 2)
								goal = 10*3600;
							break;
						}

						if(goal)
							game->setGoal(goal);
						
						sfx(SFX_MENUCONFIRM);
						break;
					}else{
						if(toStart == -1)
							saveToSram();
						onSettings = false;
						options = (int) menuOptions.size();
						selection = 0;
						clearText();
						refreshText = true;
						sfx(SFX_MENUCONFIRM);
					}

				}
			}

			if(key == KEY_B){
				if(toStart != -1){
					if(selection == 0 || toStart == -2){
						onSettings = false;
						if(onPlay)
							options = (int) gameOptions.size();
						else
							options = (int) menuOptions.size();
						clearText();
						sfx(SFX_MENUCANCEL);
						refreshText = true;
						selection = previousSelection;

					}else{
						selection = 0;
						sfx(SFX_MENUCANCEL);
						refreshText = true;
					}
				}else{
					onSettings = false;
					options = (int) menuOptions.size();
					clearText();
					sfx(SFX_MENUCANCEL);
					savefile->settings = previousSettings;
					mmSetModuleVolume(512*((float)savefile->settings.volume/10));
					setSkin();
					setLightMode();
					drawUIFrame(0,0,30,20);
					refreshText = true;
					selection = previousSelection;
				}

				goalSelection = 0;
				sfx(SFX_MENUCONFIRM);
			}
		}

		if(!(onSettings && toStart == -2)){
			if(key == KEY_UP){
				if(selection == 0)
					selection = options-1;
				else
					selection--;
				sfx(SFX_MENUMOVE);
				refreshText = true;
			}
			
			if(key == KEY_DOWN || key == KEY_SELECT){
				if(selection == options-1)
					selection = 0;
				else
					selection++;
				sfx(SFX_MENUMOVE);
				refreshText = true;
			}
		}

		sqran(qran() * frameCounter++);
		
		oam_copy(oam_mem,obj_buffer,128);
	}
	VBlankIntrWait();
	clearText();
	onSettings = false;
	irq_disable(II_HBLANK);
	memset16(pal_bg_mem,0x0000,1);

	memset16(&se_mem[26],0x0000,32*20);
	memset16(&se_mem[27],0x0000,32*20);

		
	if(savefile->settings.lightMode)
		memset16(pal_bg_mem,0x5ad6,1);//background gray
	else
		memset16(pal_bg_mem,0x0000,1);
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
				*dest++ = 2*(!savefile->settings.lightMode);
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

	endAnimation();	

	if(game->gameMode != 4 && game->gameMode != 5){
		if(game->won == 1)
			sfx(SFX_CLEAR);
		else if(game->lost == 1)
			sfx(SFX_GAMEOVER);
	}else if(game->gameMode == 5){
		if(game->won == 1)
			sfx(SFX_TIME);
		else if(game->lost == 1)
			sfx(SFX_GAMEOVER);
	}else{
		if(game->won == 1)
			sfx(SFX_YOUWIN);
		else if(game->lost == 1)
			sfx(SFX_YOULOSE);
	}

	playSongRandom(0);

	showStats();
	bool record = onRecord();

	if(multiplayer){
		enemyHeight = 0;
		progressBar();
	}

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
			playSongRandom(1);
			
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
					game->setLevel(initialLevel);
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
						playSongRandom(1);
					}

					update();

					break;

				}else{
					if(connected == 1){
						sfx(SFX_MENUCONFIRM);
						multiplayerStartTimer = 5;
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
	
	// int timer = 0;
	// int maxTimer = 20;

	// while(timer++ < maxTimer)
	// 	VBlankIntrWait();

	sfx(SFX_END);

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
		
	}else if(game->gameMode == 0 || game->gameMode == 2 || game->lost || game->gameMode == 5){
		std::string score = std::to_string(game->score);

		if(game->lost)	
			aprint("GAME  OVER",10,4);
		else{
			if(game->gameMode != 5)
				aprint("CLEAR!",12,4);
			else
				aprint("TIME!",12,4);
		}

		if(game->gameMode == 0 || game->gameMode == 2 || game->gameMode == 5)
			aprint(score,15-((int)score.size()/2),9);
	}else{
		aprint("CLEAR!",12,4);
		
		aprint(timeToString(gameSeconds),11,8);
	}
}

void pauseMenu(){

	int selection = 0;
	int maxSelection;

	if(!onStates)
		maxSelection = 4;
	else
		maxSelection = 3;
	
	for(int i = 0; i < 20; i++)
		aprint("          ",10,i);

	
	while(1){
		VBlankIntrWait();
		key_poll();

		aprint("PAUSE!",12,6);

		for(int i = 0; i < maxSelection; i++)
			aprint(" ",10,11+2*i);

		aprint(">",10,11+2*selection);

		u16 key = key_hit(KEY_FULL);

		if(!onStates){
			aprint("Resume",12,11);
			aprint("Restart",12,13);
			aprint("Sleep",12,15);
			aprint("Quit",12,17);

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
					sleep();
				}else if(selection == 3){
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
						showPawn();
						showShadow();
						showHold();
						showText();
						floatingList.clear();
						clearGlow();
						
						aprint("Loaded!",22,18);
						sfx(SFX_MENUCONFIRM);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}else if(selection == 2){
					delete quickSave;
					quickSave = new Game(*game);
					saveExists = true;

					aprint("Saved!",23,18);
					sfx(SFX_MENUCONFIRM);
				}
			}
		}

		if(key == KEY_START || key == KEY_SELECT){
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

	REG_DISPCNT &= ~(DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3);

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
		Save *temp = new Save();
		int oldSize = sizeof(Test)+sizeof(u8);

		// memcpy32(temp,savefile,oldSize/4);

		u8 * tmp = (u8*) temp;

		u8 * sf = (u8*) savefile;

		for(int i = 0; i < oldSize; i++)
			tmp[i] = sf[i];

		// for(int i = 0; i < (int) sizeof(Save)-oldSize; i++)
		// 	tmp[i+sizeof(Settings)+sizeof(u8)-4] = sf[i+oldSize];
		memcpy16(&tmp[sizeof(Settings)+sizeof(u8)+sizeof(int)-4],&sf[oldSize],(sizeof(Save)-oldSize)/2);

		temp->newGame = SAVE_TAG;
		temp->settings.edges = false;
		temp->settings.backgroundGrid = 0;
		temp->settings.skin = 0;
		temp->settings.palette = 6;
		temp->settings.shadow = 0;
		temp->settings.lightMode = false;
		
		for(int i = 0; i < 10; i++)
			temp->settings.songList[i] = true;
		
		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 5; j++)
				temp->ultra[i].highscores[j].score = 0;

		memcpy32(savefile,temp,sizeof(Save)/4);

		delete temp;
		
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

		for(int i = 0; i < 3; i++)
			for(int j = 0; j < 5; j++)
				savefile->ultra[i].highscores[j].score = 0;

		savefile->settings.announcer = true;
		savefile->settings.finesse = false;
		savefile->settings.floatText = true;
		savefile->settings.shake = true;
		savefile->settings.effects = true;
		savefile->settings.das = 11;
		savefile->settings.arr = 2;
		savefile->settings.sfr = 2;
		savefile->settings.dropProtection = true;
		savefile->settings.edges = false;
		savefile->settings.backgroundGrid = 0;
		savefile->settings.skin = 0;
		savefile->settings.palette = 6;
		savefile->settings.shadow = 0;
		savefile->settings.lightMode = false;

		savefile->settings.volume = 10;
		
		for(int i = 0; i < 10; i++)
			savefile->settings.songList[i] = true;

		// savefile->savedGame = Game(0);
		// savefile->canLoad = false;
	}
	
	if((savefile->settings.lightMode != 0) && (savefile->settings.lightMode != 1))
		savefile->settings.lightMode = false;
	
	u8* dump = (u8*) savefile;

	int sum = 0;

	for(int i = 0; i < (int) sizeof(Save); i++){
		sum += dump[i];
	}

	sqran(sum);

	savefile->seed = qran();
	saveToSram();
}

void screenShake(){
	if(!savefile->settings.shake)
		return;
	
	REG_BG0VOFS = -shake;
	REG_BG1VOFS = -shake;

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

	int das = 0;
	int maxDas = 12;

	int arr = 0;
	int maxArr = 5;

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

			char curr = result.at(cursor);
			if(key == KEY_UP){
				if(curr == 'A')
					result[cursor] = ' ';
				else if(curr == ' ')
					result[cursor] = 'Z';
				else if(curr > 'A')
					result[cursor] = curr-1;
				
				sfx(SFX_MENUMOVE);
			}else if(key == KEY_DOWN){
				if(curr == 'Z')
					result[cursor] = ' ';
				else if(curr == ' ')
					result[cursor] = 'A';
				else if(curr < 'Z')
					result[cursor] = curr+1;
				sfx(SFX_MENUMOVE);
			}else if(key_is_down(KEY_UP) || key_is_down(KEY_DOWN)){
				if(das < maxDas)
					das++;
				else{
					if(arr++ > maxArr){
						arr = 0;
						if(key_is_down(KEY_UP)){
							if(curr == 'A')
								result[cursor] = ' ';
							else if(curr == ' ')
								result[cursor] = 'Z';
							else if(curr > 'A')
								result[cursor] = curr-1;
						}else{
							if(curr == 'Z')
								result[cursor] = ' ';
							else if(curr == ' ')
								result[cursor] = 'A';
							else if(curr < 'Z')
								result[cursor] = curr+1;
						}
						sfx(SFX_MENUMOVE);
					}
				}
			}else{
				das = 0;
				if(timer++ > 19)
					timer = 0;

				if(timer < 10)
					aprint("_",13+cursor,nameHeight);
			}

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
		}else if(game->gameMode == 5){
			if(game->score < savefile->ultra[mode].highscores[i].score)
				continue;

			for(int j = 3; j >= i; j--)
				savefile->ultra[mode].highscores[j+1] = savefile->ultra[mode].highscores[j]; 

			std::string name = nameInput((i==0));

			savefile->ultra[mode].highscores[i].score = game->score;
			strncpy(savefile->ultra[mode].highscores[i].name,name.c_str(),9);
			
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

	aprint(std::to_string(game->linesSent),0,0);

	// int statHeight = 4;

	// for(int i = 0; i < 5; i++){
	// 	aprint("       ",20,statHeight+i);
	// 	std::string n = std::to_string(profileResults[i]);
	// 	aprint(n,20,statHeight+i);
	// }
}

void progressBar(){
	if(game->goal == 0)
		return;
	
	int current;
	int max = game->goal;

	if(game->gameMode == 3)
		current = game->garbageCleared;
	else if(game->gameMode == 5)
		current = game->timer;
	else
		current = game->linesCleared;

	showBar(current,max,20,savefile->settings.palette);

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
		// u16*dest = (u16*)se_mem[27];

		// for(int i = 19; i >= 0; i--){
		// 	if(i <= (19-enemyHeight))
		// 		dest[32*i+29] = 0x000d + savefile->settings.palette * 0x1000;
		// 	else
		// 		dest[32*i+29] = 0x000e + savefile->settings.palette * 0x1000;
		// }
	}
}

void showBar(int current, int max, int x, int palette){
	palette *= 0x1000;

	if(max > 10000){
		current /=10;
		max/=10;
	}

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
	obj_set_attr(holdFrameSprite,ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(savefile->settings.palette));
	holdFrameSprite->attr2 = ATTR2_BUILD(512,savefile->settings.palette,1);
	obj_set_pos(holdFrameSprite,4*8+5,9*8-2);

	for(int i = 0; i < 3; i++){
		obj_unhide(queueFrameSprites[i],0);
		obj_set_attr(queueFrameSprites[i],ATTR0_SQUARE,ATTR1_SIZE(2),ATTR2_PALBANK(savefile->settings.palette));
		queueFrameSprites[i]->attr2 = ATTR2_BUILD(512+16+16*i,savefile->settings.palette,1);
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
			linkConnection->send((u16)((1<<13)+(nextSeed&0x1fff)));
		}
		return;
	}
	
	auto linkState = linkConnection->linkState.get();
	
	int receivedAttack = false;
	
	u16 data[LINK_MAX_PLAYERS];
	for (int i = 0; i < LINK_MAX_PLAYERS; i++)
		data[i] = 0;

	int incomingHeight = 0;

	if(linkState->isConnected()){
		timeoutTimer = 0;
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
				playAgain = true;
				nextSeed = value;
				break;
			case 2:
				if(value == 0x123)
					game->setWin();
				break;
			case 3://receive attack
				game->addToGarbageQueue((value>>4)&0xff,value & 0xf);
				receivedAttack = (value>>4)&0xff;
				break;
			case 4://acknowledge attack
				game->clearAttack(value&0xff);
				break;
			case 5:
				for(int i = 0; i < 10; i++)
					enemyBoard[value>>10][i] = (value>>i)&0b1;
				incomingHeight = value>>10;
				break;
			case 6:
				for(int i = 0; i < 10; i++)
					enemyBoard[(value>>10)+8][i] = (value>>i)&0b1;
				incomingHeight = (value>>10)+8;
				break;
			case 7:
				if((value>>10)+16 > 19)
					break;
				for(int i = 0; i < 10; i++)
					enemyBoard[(value>>10)+16][i] = (value>>i)&0b1;
				incomingHeight = (value>>10)+16;
				
				break;
			}
		}
			// if(data[i])
			// 	aprint(std::to_string(data[i]),0,7+i);
		if(game->lost){
			linkConnection->send((u16)(2<<13)+0x123);
		}else{
			if(receivedAttack){
				linkConnection->send((u16)((4<<13) + receivedAttack));
			}else if(game->attackQueue.size() == 0){
				if(currentScanHeight < 19)
					currentScanHeight++;
				else
					currentScanHeight = 0;
				
				u16 row = 0;
				for(int i = 0; i < 10; i++)
					if(game->board[currentScanHeight+20][i])
						row+= 1<<i;

				linkConnection->send((u16)(((5)<<13) + ((currentScanHeight&0x1f)<<10)+(row&0x3ff)));
				
			}else{
				Tetris::Garbage atck = game->attackQueue.front();

				linkConnection->send((u16)((3<<13) + (atck.id<<4)+atck.amount));
			}
			
			drawEnemyBoard(incomingHeight);

		}
	}else{
		if(timeoutTimer++ == maxTimeout){
			timeoutTimer = 0;
			game->setWin(); // aprint("no connection.",0,5);
			connected = -1;
			aprint("Connection Lost.",0,0);
		}
		// aprint("0",0,0);
		// multiplayer = false;
	}
}

void startMultiplayerGame(int seed){
	memset32(&tile_mem[5][256],0,8*8);
	
	for(int i = 0; i < 20; i++)
		for(int j = 0; j < 10; j++)
			enemyBoard[i][j] = 0;

	delete game;
	game = new Game(4,seed&0x1fff);
	game->setLevel(1);
	game->setTuning(savefile->settings.das,savefile->settings.arr,savefile->settings.sfr,savefile->settings.dropProtection);
	game->setGoal(100);
	multiplayer = true;
	linkConnection->send(50);
	nextSeed = 0;
}

void setSkin(){
	for(int i = 0; i < 7; i++)
		memcpy16(&tile_mem[4][9*16+i*8],mini[0][i],16*7);

	switch(savefile->settings.skin){
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
	case 4:
		blockSprite = (u8*)sprite18tiles_bin;
		break;
	case 5:
		blockSprite = (u8*)sprite19tiles_bin;
		//load mini sprite tiles
		for(int i = 0; i < 7; i++)
			memcpy16(&tile_mem[4][9*16+i*8],mini[1][i],16*7);
		break;
	}
	
	memcpy16(&tile_mem[0][1],blockSprite,sprite1tiles_bin_size/2);
	memcpy16(&tile_mem[2][97],blockSprite,sprite1tiles_bin_size/2);
	memcpy16(&pal_bg_mem[8*16],&palette[savefile->settings.palette*16],16);

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

void graphicTest(){
	irq_disable(II_HBLANK);

	if(savefile->settings.lightMode)
		memset16(pal_bg_mem,0x5ad6,1);//background gray
	else
		memset16(pal_bg_mem,0x0000,1);

	memset16(&se_mem[26],0x0000,32*20);

	delete game;
	game = new Game(3);
	game->pawn.y++;

	bool showOptions = true;
	bool showGame = true;

	int startX = 5;
	int endX = 24;

	int startY = 5;
	int options = 8;
	int selection = 0;

	REG_DISPCNT= 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3; //Set to Sprite mode, 1d rendering
	
	REG_DISPCNT &= ~DCNT_BG3;

	showBackground();
	while(1){
		VBlankIntrWait();
		drawGrid();
		drawFrame();

		key_poll();

		aprint("L: Toggle Game",0,18);
		aprint("R: Toggle Options",0,19);

		if(key_hit(KEY_START) || key_hit(KEY_A)){
			sfx(SFX_MENUCONFIRM);
			if(selection != options-1){
				selection = options-1;
			}else{
				break;
			}
			setSkin();
			setLightMode();
		}

		if(key_hit(KEY_B)){
			sfx(SFX_MENUCANCEL);
			if(selection == options-1)
				selection = 0;
			else
				break;
		}

		if(key_hit(KEY_L)){
			showOptions = !showOptions;

			if(!showOptions){
				clearText();
				aprint("L: Toggle Game",0,18);
				aprint("R: Toggle Options",0,19);
			}
		}

		if(key_hit(KEY_R)){
			showGame = !showGame;
			if(showGame){
				REG_DISPCNT= 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2; //Set to Sprite mode, 1d rendering
			}else{
				REG_DISPCNT= 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG2; //Set to Sprite mode, 1d rendering
			}
		}

		if(key_hit(KEY_UP)){
			if(selection > 0)
				selection--;
			else
				selection = options - 1;
			sfx(SFX_MENUMOVE);
		}
		
		if(key_hit(KEY_DOWN)){
			if(selection < options-1)
				selection++;
			else
				selection = 0;
			sfx(SFX_MENUMOVE);
		}

		if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
			switch(selection){
			case 0:
				savefile->settings.effects = !savefile->settings.effects;
				if(savefile->settings.effects){
					effectList.push_back(Effect(1,4,5));
				}
				sfx(SFX_MENUMOVE);
				break;
			case 1:
				savefile->settings.edges = !savefile->settings.edges;
				sfx(SFX_MENUMOVE);
				break;
			case 2:
				if(key_hit(KEY_LEFT)){
					if(savefile->settings.backgroundGrid > 0){
						savefile->settings.backgroundGrid--;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}else{
					if(savefile->settings.backgroundGrid < MAX_BACKGROUNDS-1){
						savefile->settings.backgroundGrid++;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}

				drawGrid();
				break;
			case 3:
				if(key_hit(KEY_LEFT)){
					if(savefile->settings.skin > 0){
						savefile->settings.skin--;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}else{
					if(savefile->settings.skin < MAX_SKINS-1){
						savefile->settings.skin++;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}

				setSkin();
				setLightMode();
				break;
			case 4:
				if(key_hit(KEY_LEFT)){
					if(savefile->settings.palette > 0){
						savefile->settings.palette--;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}else{
					if(savefile->settings.palette < 6){
						savefile->settings.palette++;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}
				break;
			case 5:
				if(key_hit(KEY_LEFT)){
					if(savefile->settings.shadow > 0){
						savefile->settings.shadow--;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}else{
					if(savefile->settings.shadow < MAX_SHADOWS-1){
						savefile->settings.shadow++;
						sfx(SFX_MENUMOVE);
					}else{
						sfx(SFX_MENUCANCEL);
					}
				}
				break;
			case 6:
				savefile->settings.lightMode = !savefile->settings.lightMode;
				setLightMode();
				sfx(SFX_MENUMOVE);
				break;
			}
		}

		if(showOptions){

			aprint(" DONE ",12,15);
			
			aprint("Effects",startX,startY);
			aprint("Block Edges",startX,startY+1);
			aprint("Background",startX,startY+2);
			aprint("Skin",startX,startY+3);
			aprint("Frame Color",startX,startY+4);
			aprint("Ghost Piece",startX,startY+5);
			aprint("Light Mode",startX,startY+6);

			for(int i = 0; i < options; i++)
				aprint("      ",endX-1,startY+i);

			if(savefile->settings.effects)
				aprint("ON",endX,startY);
			else
				aprint("OFF",endX,startY);
			
			if(savefile->settings.edges)
				aprint("ON",endX,startY+1);
			else
				aprint("OFF",endX,startY+1);

			aprint(std::to_string(savefile->settings.backgroundGrid+1),endX,startY+2);
			
			aprint(std::to_string(savefile->settings.skin+1),endX,startY+3);
			
			aprint(std::to_string(savefile->settings.palette+1),endX,startY+4);
			
			aprint(std::to_string(savefile->settings.shadow+1),endX,startY+5);
			
			if(savefile->settings.lightMode)
				aprint("ON",endX,startY+6);
			else
				aprint("OFF",endX,startY+6);

			switch(selection){
			case 0:
				aprint("[",endX-1,startY+selection);
				aprint("]",endX+2+!(savefile->settings.effects),startY+selection);
				break;
			case 1:
				aprint("[",endX-1,startY+selection);
				aprint("]",endX+2+!(savefile->settings.edges),startY+selection);
				break;
			case 2:
				if(savefile->settings.backgroundGrid > 0)
					aprint("<",endX-1,startY+selection);
				if(savefile->settings.backgroundGrid < MAX_BACKGROUNDS-1)
					aprint(">",endX+1,startY+selection);
				break;
			case 3:
				if(savefile->settings.skin > 0)
					aprint("<",endX-1,startY+selection);
				if(savefile->settings.skin < MAX_SKINS-1)
					aprint(">",endX+1,startY+selection);
				break;
			case 4:
				if(savefile->settings.palette > 0)
					aprint("<",endX-1,startY+selection);
				if(savefile->settings.palette < 6)
					aprint(">",endX+1,startY+selection);
				break;
			case 5:
				if(savefile->settings.shadow > 0)
					aprint("<",endX-1,startY+selection);
				if(savefile->settings.shadow < MAX_SHADOWS)
					aprint(">",endX+1,startY+selection);
				break;
			case 6:
				aprint("[",endX-1,startY+selection);
				aprint("]",endX+2+!(savefile->settings.lightMode),startY+selection);
				break;
			case 7:
				aprint("[",12,15);
				aprint("]",17,15);
				break;
			}
		}

		if(showGame){
			showBackground();
			showQueue();
			showPawn();
			showShadow();
			showBackground();
			showHold();
		}else{
			oam_init(obj_buffer,128);
		}

		oam_copy(oam_mem,obj_buffer,128);
	}
	REG_DISPCNT |= DCNT_BG3;
	irq_enable(II_HBLANK);
	memset32(&se_mem[25],0x0000,32*10);
	memset32(&se_mem[26],0x0000,32*10);
	memset32(&se_mem[27],0x0000,32*10);
	drawUIFrame(0,0,30,20);
	oam_init(obj_buffer,128);
	showTitleSprites();
	for(int i = 0; i < 2; i++)
		obj_hide(titleSprites[i]);
	oam_copy(oam_mem,obj_buffer,128);
	drawUIFrame(0,0,30,20);
}

void showTitleSprites(){
	for(int i = 0; i < 2; i++)
		titleSprites[i] = &obj_buffer[14+i];
	for(int i = 0; i < 2; i++){
		obj_unhide(titleSprites[i],0);
		obj_set_attr(titleSprites[i],ATTR0_WIDE,ATTR1_SIZE(3),ATTR2_PALBANK(13));
		titleSprites[i]->attr2 = ATTR2_BUILD(512+64+i*32,13,0);
		obj_set_pos(titleSprites[i],120-64+64*i,24);
	}
}

void drawEnemyBoard(int height){

	enemyBoardSprite = &obj_buffer[20];
	obj_unhide(enemyBoardSprite,ATTR0_AFF_DBL);
	obj_set_attr(enemyBoardSprite,ATTR0_TALL | ATTR0_AFF_DBL,ATTR1_SIZE(2) | ATTR1_AFF_ID(6),0);
	enemyBoardSprite->attr2 = ATTR2_BUILD(768,0,1);
	obj_set_pos(enemyBoardSprite,43,24);
	obj_aff_identity(&obj_aff_buffer[6]);
	obj_aff_scale(&obj_aff_buffer[6],float2fx(0.5),float2fx(0.5));

	if(height> 19)
		return;

	TILE * dest2;

	for(int j = 0; j < 10; j++){
		dest2 = (TILE *) &tile_mem[5][256+((height)/8)*2+(j)/8];//12 IS NOT CONFIRMED

		if(enemyBoard[height][j])
			dest2->data[(height)%8] |= (4+2*savefile->settings.lightMode) << ((j%8)*4);
		else
			dest2->data[(height)%8] &= ~(0xffff << ((j%8)*4));
	}
}

void setLightMode(){

	if(savefile->settings.lightMode){
		memset16(pal_bg_mem,0x5ad6,1);//background gray
		memset16(&pal_bg_mem[8*16+4],0x0421,1);//progressbar

		//activated font
		memset16(&pal_bg_mem[15*16+2],0x0000,1);//font main
		memset16(&pal_bg_mem[15*16+3],0x5ad6,1);//font shadow
		memset16(&pal_obj_mem[15*16+2],0x0000,1);//obj font main
		memset16(&pal_obj_mem[15*16+3],0x5ad6,1);//boj font shadow
		
		//unactivated font
		memset16(&pal_bg_mem[14*16+2],0x7fff,1);//font main
		memset16(&pal_bg_mem[14*16+3],0x5ad6,1);//font shadow
		memset16(&pal_obj_mem[14*16+2],0x7fff,1);//obj font main
		memset16(&pal_obj_mem[14*16+3],0x5ad6,1);//boj font shadow
	}else{
		memset16(pal_bg_mem,0x0000,1);
		memset16(&pal_bg_mem[8*16+4],0x7fff,1);
		
		//activated font
		memset16(&pal_bg_mem[15*16+2],0x7fff,1);
		memset16(&pal_bg_mem[15*16+3],0x294a,1);
		memset16(&pal_obj_mem[15*16+2],0x7fff,1);
		memset16(&pal_obj_mem[15*16+3],0x294a,1);
		
		//unactivated font
		memset16(&pal_bg_mem[14*16+2],0x318c,1);//font main
		memset16(&pal_bg_mem[14*16+3],0x0421,1);//font shadow
		memset16(&pal_obj_mem[14*16+2],0x318c,1);//obj font main
		memset16(&pal_obj_mem[14*16+3],0x0421,1);//boj font shadow
	}
}

// void handleBotGame(){
// 	if(botGame->attackQueue.size()){
// 		Tetris::Garbage atck = botGame->attackQueue.front();
// 		game->addToGarbageQueue(atck.id,atck.amount);
// 		botGame->clearAttack(atck.id);
// 	}
	
// 	if(game->attackQueue.size()){
// 		Tetris::Garbage atck = game->attackQueue.front();
// 		botGame->addToGarbageQueue(atck.id,atck.amount);
// 		game->clearAttack(atck.id);
// 	}
	
// 	for(int i = 0; i < 20; i++)
// 		for(int j = 0; j < 10; j++)
// 			enemyBoard[i][j] = (botGame->board[i+20][j]);
// 	drawEnemyBoard();

// 	if(botGame->clearLock){
// 		botGame->removeClearLock();
// 	}
// }

void sleep(){
	int display_value = REG_DISPCNT;
	REG_DISPCNT= 0x1000 | 0x0040 | DCNT_MODE0 | DCNT_BG2; //Disable all backgrounds except text
	clearText();

	oam_init(obj_buffer,128);
	oam_copy(oam_mem,obj_buffer,128);

	while(1){
		aprint("Entering sleep...",7,4);
		aprint("Press",13,8);
		aprint("L + R + SELECT",9,10);
		aprint("to leave sleep",8,12);

		aprint("A: Sleep      B: Cancel",4,17);

		VBlankIntrWait();

		key_poll();

		if(key_hit(KEY_A)){
			clearText();
			break;
		}

		if(key_hit(KEY_B)){
			clearText();
			REG_DISPCNT = display_value;
			update();
			showPawn();
			showHold();
			showShadow();
			showQueue();
			showTimer();
			return;
		}

	}

	irq_disable(II_VBLANK);
	int stat_value = REG_SNDSTAT;
	int dsc_value = REG_SNDDSCNT;
	int dmg_value = REG_SNDDMGCNT;

	REG_DISPCNT |= 0x0080;
	REG_SNDSTAT = 0;
	REG_SNDDSCNT = 0;
	REG_SNDDMGCNT = 0;

	linkConnection->deactivate();
	irq_disable(II_TIMER3);
	irq_disable(II_SERIAL);

	REG_P1CNT = 0b1100001100000100;
	irq_add(II_KEYPAD,nullptr);

	Stop();

	REG_DISPCNT = display_value;
	REG_SNDSTAT = stat_value;
	REG_SNDDSCNT = dsc_value;
	REG_SNDDMGCNT = dmg_value;
	
	linkConnection->activate();
	irq_enable(II_TIMER3);
	irq_enable(II_SERIAL);
	irq_delete(II_KEYPAD);
	irq_enable(II_VBLANK);
	
	update();
	showPawn();
	showHold();
	showShadow();
	showQueue();
	showTimer();
}

void playSong(int menuId, int songId){
	
	int song = 0;

	if(menuId== 0){
		switch(songId){
		case 0:
			song = MOD_MENU;
			break;
		case 1:
			song = MOD_OPTIKAL_INNOVATION;
			break;
		default:
			return;
		}
	}else if(menuId == 1){
		switch(songId){
		case 0:
			song = MOD_THIRNO;
			break;
		// case 1:
		// 	song = MOD_ALDEBARAN_SHORT;
		// 	break;
		case 1:
			song = MOD_OH_MY_GOD;
			break;
		case 2:
			song = MOD_UNSUSPECTED_H;
			break;
		case 3:
			song = MOD_WARNING_INFECTED;
			break;
		default:
			return;
		}
	}

	mmStart(song,MM_PLAY_LOOP);
	mmSetModuleVolume(512*((float)savefile->settings.volume/10));
}

void playSongRandom(int menuId){

	int songId = -1;
	int max = 0;


	if(menuId == 0){
		for(int i = 0; i < MAX_MENU_SONGS; i++)
			max+=(savefile->settings.songList[i]);
	}else if(menuId == 1){
		for(int i = 0; i < MAX_GAME_SONGS; i++)
			max+=(savefile->settings.songList[i+MAX_MENU_SONGS]);
	}

	int index = 0;
	if(max > 0){
		index = qran() % max;
	}else{
		return;
	}

	int start = 0;
	if(menuId == 1)
		start = MAX_MENU_SONGS;

	int counter = 0;
	for(int i = start; i < 10;i++){
		if(counter == index && savefile->settings.songList[i]){
			songId = i;
			if(menuId == 1)
				songId -= MAX_MENU_SONGS;
			break;
		}
		if(savefile->settings.songList[i])
			counter++;
	}

	if(songId == -1)
		return;

	playSong(menuId,songId);
}

void drawUIFrame(int x, int y, int w, int h){
	u16* dest = (u16*) &se_mem[26];
	u16* dest2 = (u16*) &se_mem[27];

	dest2+=y*32+x;

	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			int tile = 0;
			if((i == 0 && (j == 0 || j == w-1)) || (i == h-1 && (j == 0 || j == w-1))){
				tile = 29 + (i > 0) * 0x800 + (j > 0) * 0x400;
			}else if(i == 0 || i == h-1){
				tile = 28 + (i > 0) * 0x800;
			}else if(j == 0 || j == w-1){
				tile = 4 + (j > 0) * 0x400;
			}
			if(tile)
				*dest2++ = tile + savefile->settings.palette * 0x1000 * (tile != 12);
			else
				dest2++;
		}
		dest2+=32-w;
	}
	
	dest+=(y+1)*32+x+1;
	for(int i = 1; i < h-1; i++){
		for(int j = 1; j < w-1; j++){
			*dest++ = 12 + 4*0x1000 * (savefile->settings.lightMode);
		}
		dest+=32-w+2;
	}
}

void songListMenu(){
	int startX = 3;
	int endX = 24;

	int startY = 3;

	int selection = 0;
	int options = 7;

	mmStop();
	playSong(0,0);

	while(1){
		VBlankIntrWait();

		key_poll();

		if(key_hit(KEY_START)){
			sfx(SFX_MENUCONFIRM);
			if(selection != options-1){
				selection = options-1;
			}else{
				break;
			}
		}

		if(key_hit(KEY_B)){
			sfx(SFX_MENUCANCEL);
			if(selection == options-1)
				selection = 0;
			else
				break;
		}

		if(key_hit(KEY_UP)){
			if(selection > 0)
				selection--;
			else
				selection = options - 1;
			sfx(SFX_MENUMOVE);

			mmStop();
			if(selection < 2)
				playSong(0,selection);
			else if(selection < 6)
				playSong(1,selection-2);
		}
		
		if(key_hit(KEY_DOWN)){
			if(selection < options-1)
				selection++;
			else
				selection = 0;
			sfx(SFX_MENUMOVE);
			if(selection < 2)
				playSong(0,selection);
			else if(selection < 6)
				playSong(1,selection-2);
		}

		if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT) || key_hit(KEY_A)){
			sfx(SFX_MENUCONFIRM);
			if(selection == options-1)
				break;
			else{
				savefile->settings.songList[selection] = !savefile->settings.songList[selection];
			}
		}

		aprint(" DONE ",12,17);
		
		aprint("Menu:",startX,startY);
		aprint("Track 1",startX,startY+2);
		aprint("Track 2",startX,startY+3);

		aprint("Game:",startX,startY+6);
		aprint("Track 1",startX,startY+8);
		aprint("Track 2",startX,startY+9);
		aprint("Track 3",startX,startY+10);
		aprint("Track 4",startX,startY+11);

		for(int i = 0; i < 2; i++)
			aprint("   ",endX-1,startY+2+i);
		 
		for(int i = 0; i < 6; i++)
			aprint("   ",endX-1,startY+7+i);

		if(savefile->settings.songList[0])
			aprint("x",endX,startY+2);
		if(savefile->settings.songList[1])
			aprint("x",endX,startY+3);
		
		if(savefile->settings.songList[2])
			aprint("x",endX,startY+8);
		if(savefile->settings.songList[3])
			aprint("x",endX,startY+9);
		if(savefile->settings.songList[4])
			aprint("x",endX,startY+10);
		if(savefile->settings.songList[5])
			aprint("x",endX,startY+11);
		
		if(selection == options -1){
			aprint("[",12,17);
			aprint("]",17,17);
		}else{
			aprint("[",endX-1,startY+2+selection+(selection>1)*4);
			aprint("]",endX+1,startY+2+selection+(selection>1)*4);
		}

		oam_copy(oam_mem,obj_buffer,128);
	}

	mmStop();
	playSongRandom(0);
}

void settingsText(){
	int startX = 3;
	int startY = 4;
	int space = 1;
	aprint("Voice",startX,startY);
	aprint("Clear Text",startX,startY+space);
	aprint("Screen Shake",startX,startY+space*2);
	aprint("Music",startX,startY+space*3);
	aprint("Auto Repeat Delay",startX,startY+space*4);
	aprint("Auto Repeat Rate",startX,startY+space*5);
	aprint("Soft Drop Speed",startX,startY+space*6);
	aprint("Drop Protection",startX,startY+space*7);
	aprint("Show Finesse",startX,startY+space*8);
	aprint("Graphics...",startX,startY+space*9);
	aprint("Song List...",startX,startY+space*10);
}