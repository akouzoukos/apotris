#include "def.h"
#include "tonc.h"

#include "logging.h"
#include "text.h"
#include "tonc_input.h"
#include "tonc_memdef.h"
#include "posprintf.h"

#include "sprite29tiles_bin.h"
#include "sprite30tiles_bin.h"
#include "sprite31tiles_bin.h"
#include "sprite32tiles_bin.h"
#include "sprite33tiles_bin.h"
#include "sprite34tiles_bin.h"

const int xoffset = 15-8;
const int yoffset = 10-8;

class Cursor{
public:
    int x = 0;
    int y = 0;
    OBJ_ATTR* sprite;

    void show(){
        int sx = x * 16 + xoffset * 8;
        int sy = y * 16 + yoffset * 8;

        obj_set_pos(sprite,sx,sy);
        obj_unhide(sprite,0);
    }

    void move(int dx, int dy){
        if(x+dx >= 0 && x+dx <= 7)
            x += dx;
        //     x = 7;
        // else if(x > 7)
        //     x = 0;

        if(y+dy >= 0 && y+dy <= 7)
            y += dy;
        //     y = 7;
        // else if(y > 7)
        //     y = 0;
    }

    Cursor(){
        sprite = &obj_buffer[0];
        obj_set_attr(sprite,ATTR0_SQUARE,ATTR1_SIZE(1),ATTR2_BUILD(712, 15, 0));
    }
};

class Move{
    public:
        int x = 0;
        int y = 0;

        int previousValue = 0;
        int currentValue = 0;
        int tool = 0;

        Move(int _x, int _y, int previous, int current, int _tool){
            x = _x;
            y = _y;

            previousValue = previous;
            currentValue = current;

            tool = _tool;
        }
};

class Board{
    public:
        int board[8][8];
        Cursor cursor;

        std::list<Move> history;

        void set(int value, bool record){
            if(record)
                history.push_back(Move(cursor.x,cursor.y,board[cursor.y][cursor.x],value,1));

            if(history.size() > 10)
                history.pop_front();

            board[cursor.y][cursor.x] = value;
            int s = (cursor.x) * 4;
            customSkin->data[cursor.y] = (customSkin->data[cursor.y] & ~(0xf << (s))) | ((value + (value == 5)) << (s));
        }

        void fill(int value, int record){
            int previous = board[cursor.y][cursor.x];
            if(record)
                history.push_back(Move(cursor.x,cursor.y,previous,value,2));

            board[cursor.y][cursor.x] = value;
            int s = (cursor.x) * 4;
            customSkin->data[cursor.y] = (customSkin->data[cursor.y] & ~(0xf << (s))) | (value << (s));

            if(cursor.y - 1 >= 0 && board[cursor.y-1][cursor.x] == previous){
                cursor.y--;
                fill(value,false);
                cursor.y++;
            }
            if(cursor.y + 1 <= 7 && board[cursor.y+1][cursor.x] == previous){
                cursor.y++;
                fill(value,false);
                cursor.y--;
            }
            if(cursor.x - 1 >= 0 && board[cursor.y][cursor.x-1] == previous){
                cursor.x--;
                fill(value,false);
                cursor.x++;
            }
            if(cursor.x + 1 <= 7 && board[cursor.y][cursor.x+1] == previous){
                cursor.x++;
                fill(value,false);
                cursor.x--;
            }
        }

        void show(){
            u16* dest = (u16*) &se_mem[25];

            for(int i = 0; i < 8; i++){
                int y = ((i * 2)+yoffset) * 32;
                for(int j = 0; j < 8; j++){
                    int x = j * 2 + xoffset;
                    int n = board[i][j] + 100;
                    if(board[i][j] != 0){
                        dest[y+x] = n;
                        dest[y+x+1] = n;
                        dest[y+32+x] = n;
                        dest[y+32+x+1] = n;
                    }else{
                        dest[y+x] = 0xf06a;
                        dest[y+x+1] = 0xf06b;
                        dest[y+32+x] = 0xf06c;
                        dest[y+32+x+1] = 0xf06d;
                    }
                }
            }

            cursor.show();
        }

        void undo(){
            if(history.empty())
               return;

            Move previous = history.back();

            cursor.x = previous.x;
            cursor.y = previous.y;

            switch(previous.tool){
            case 1: set(previous.previousValue,false); break;
            case 2: fill(previous.previousValue,false); break;
            }

            history.pop_back();
        }

        Board(){
            for(int i = 0; i < 8; i++)
                for(int j = 0; j < 8; j++)
                    board[i][j] = 0;
        }
};

void generateTiles();
void showColorPalette(int);
void showMinos();
void refreshSkin();
void showTools(int);
void showCursor();

TILE * customSkin;

static int cursorTimer = 0;
static int toolTimer = 0;

void skinEditor(){

    u16* dest = (u16*) &se_mem[26];

    for(int i = 0; i < 32; i++)
        for(int j = 0; j < 20; j++)
            *dest++ = 2;

    memcpy16(&tile_mem[0][106],sprite34tiles_bin,sprite34tiles_bin_size/2);

    memcpy16(&tile_mem[5][200],sprite29tiles_bin,sprite29tiles_bin_size/2);

    memcpy16(&tile_mem[5][204],sprite31tiles_bin,sprite31tiles_bin_size/2);
    memcpy16(&tile_mem[5][208],sprite32tiles_bin,sprite32tiles_bin_size/2);
    memcpy16(&tile_mem[5][212],sprite33tiles_bin,sprite33tiles_bin_size/2);

    generateTiles();

    Board board;

    int currentColor = 0;
    int currentTool = 1;

    int das = 0;
    int dasMax = 20;
    int arr = 4;

    while(true){
        VBlankIntrWait();
        key_poll();

        int dx = 0;
        int dy = 0;

        if(key_is_down(KEY_R)){
            if(key_hit(KEY_LEFT)){
                currentColor--;
                if(currentColor < 0)
                    currentColor = 5;
            }

            if(key_hit(KEY_RIGHT)){
                currentColor++;
                if(currentColor > 5)
                    currentColor = 0;
            }

            if(key_hit(KEY_UP)){
                currentTool--;
                if(currentTool < 0)
                    currentTool = 2;
            }

            if(key_hit(KEY_DOWN)){
                currentTool++;
                if(currentTool > 2)
                    currentTool = 0;
            }
        }else if(key_is_down(KEY_L)){
            if(key_hit(KEY_LEFT)){
                savefile->settings.colors--;
                if(savefile->settings.colors < 0)
                    savefile->settings.colors = MAX_COLORS-1;
            }

            if(key_hit(KEY_RIGHT)){
                savefile->settings.colors++;
                if(savefile->settings.colors > MAX_COLORS-1)
                    savefile->settings.colors = 0;
            }

            if(key_hit(KEY_LEFT | KEY_RIGHT))
                setPalette();

            // if(key_hit(KEY_UP)){
            //     savefile->settings.skin--;
            //     if(savefile->settings.skin < 0)
            //         savefile->settings.skin = MAX_SKINS-1;
            // }

            // if(key_hit(KEY_DOWN)){
            //     savefile->settings.skin++;
            //     if(savefile->settings.skin > MAX_SKINS-1)
            //         savefile->settings.skin = 0;
            // }

            // if(key_hit(KEY_UP | KEY_DOWN))
            //     setPalette();
        }else{
            if(key_hit(KEY_LEFT | KEY_RIGHT)){
                dx = ((key_hit(KEY_RIGHT) != 0) - (key_hit(KEY_LEFT) != 0));
            }

            if(key_hit(KEY_UP | KEY_DOWN)){
                dy = ((key_hit(KEY_DOWN) != 0) - (key_hit(KEY_UP) != 0));
            }

            if(key_is_down(KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN))
                das++;
            else
                das = 0;

            if(das >= dasMax){

                if(key_is_down(KEY_LEFT))
                    dx = -1;
                else if(key_is_down(KEY_RIGHT))
                    dx = 1;

                if(key_is_down(KEY_UP))
                    dy = -1;
                else if(key_is_down(KEY_DOWN))
                    dy = 1;

                das -= arr;
            }

            board.cursor.move(dx,dy);
            if((dx || dy) && key_is_down(KEY_A)){
                switch(currentTool){
                case 0: board.set(0,true); break;
                case 1: board.set(currentColor,true); break;
                }
                refreshSkin();
            }
        }

        if(key_hit(KEY_A)){
            switch(currentTool){
            case 0: board.set(0,true); break;
            case 1: board.set(currentColor,true); break;
            case 2: board.fill(currentColor,true); break;
            }

            refreshSkin();
        }

        if(key_hit(KEY_B)){
            board.undo();

            refreshSkin();
        }

        if(key_hit(KEY_START))
            break;

        board.show();
        showMinos();
        showCursor();

        showColorPalette(currentColor);
        showTools(currentTool);

        oam_copy(oam_mem, obj_buffer, 32);
    }
}

void generateTiles(){

    TILE *t = new TILE();

    for(int i = 0; i < 6; i++){
        int n = 0;
        for(int j = 0; j < 8; j++)
            n += (i + (i == 5)) << (j*4);

        for(int j = 0; j < 8; j++)
            t->data[j] = n;

        memcpy16(&tile_mem[0][100+i],t,16);
    }

    delete t;

    customSkin = new TILE();
    for (int j = 0; j < 8; j++) {
        customSkin->data[j] = 0;
    }
}

void showColorPalette(int c){
    u16* dest = (u16*) &se_mem[25];
    for(int i = 0; i < 6; i++)
        *dest++ = 100 + i;

    aprint("     ",0,1);
    aprint("^",c,1);
}

void showTools(int c){
    OBJ_ATTR * sprites[3];

    for(int i = 0; i < 3; i++){
        sprites[i] = &obj_buffer[i+8];
        obj_set_attr(sprites[i], ATTR0_SQUARE, ATTR1_SIZE(1), ATTR2_BUILD(716 + i * 4, 15, 2));

        int offset = 0;

        if(i == c){
            aprint("<",3,4+i*2);

            offset = ((sin_lut[toolTimer]*2)>>12);

        } else
            aprint(" ",3,4+i*2);

        obj_set_pos(sprites[i],4,32+(i*16) - 4 + offset);
        obj_unhide(sprites[i],0);
    }

    toolTimer+=6;
    if(toolTimer >= 512)
        toolTimer = 0;

}

void showMinos(){
    OBJ_ATTR * sprites[7];

    for(int i = 0; i < 7; i++){
        sprites[i] = &obj_buffer[i+1];
        obj_set_attr(sprites[i], ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_BUILD(16 * i, i, 2));
        obj_set_pos(sprites[i],208 - 4 * (i == 0 || i == 3),i * 24 - 4);
        obj_unhide(sprites[i],0);
    }
}

void refreshSkin(){
    savefile->settings.skin = -1;
    setSkin();
}

void showCursor(){
    cursorTimer++;

    const int animationLength = 32;

    if(cursorTimer < animationLength/2){
        memcpy16(&tile_mem[5][200],sprite29tiles_bin,sprite29tiles_bin_size/2);
    }else{
        memcpy16(&tile_mem[5][200],sprite30tiles_bin,sprite30tiles_bin_size/2);
        if(cursorTimer >= animationLength-1)
            cursorTimer = 0 ;
    }
}
