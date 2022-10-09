#include "def.h"
#include "tonc.h"

#include "sprite29tiles_bin.h"
#include "logging.h"
#include "text.h"
#include "tonc_input.h"
#include "tonc_memdef.h"

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
        x += dx;
        if(x < 0)
            x = 7;
        else if(x > 7)
            x = 0;

        y += dy;
        if(y < 0)
            y = 7;
        else if(y > 7)
            y = 0;
    }

    Cursor(){
        sprite = &obj_buffer[0];
        obj_set_attr(sprite,ATTR0_SQUARE,ATTR1_SIZE(1),ATTR2_BUILD(712, 15, 0));
    }
};


class Board{
    public:
        int board[8][8];
        Cursor cursor;

        void set(int value){
            board[cursor.y][cursor.x] = value;
            int s = (cursor.x) * 4;
            customSkin->data[cursor.y] = (customSkin->data[cursor.y] & ~(0xf << (s))) | (value << (s));
        }

        void show(){
            u16* dest = (u16*) &se_mem[25];

            for(int i = 0; i < 8; i++){
                int y = ((i * 2)+yoffset) * 32;
                for(int j = 0; j < 8; j++){
                    int x = j * 2 + xoffset;
                    int n = board[i][j] + 100;
                    dest[y+x] = n;
                    dest[y+x+1] = n;
                    dest[y+32+x] = n;
                    dest[y+32+x+1] = n;
                }
            }

            cursor.show();
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

TILE * customSkin;

void skinEditor(){

    u16* dest = (u16*) &se_mem[26];

    for(int i = 0; i < 32; i++)
        for(int j = 0; j < 20; j++)
            *dest++ = 2;

    memcpy16(&tile_mem[5][200],sprite29tiles_bin,sprite29tiles_bin_size/2);

    generateTiles();

    Board board;

    int currentColor = 0;

    while(true){
        VBlankIntrWait();
        key_poll();

        int dx = 0;
        int dy = 0;

        if(key_is_down(KEY_R)){
            if(key_hit(KEY_LEFT)){
                currentColor--;
                if(currentColor < 0)
                    currentColor = 4;
            }

            if(key_hit(KEY_RIGHT)){
                currentColor++;
                if(currentColor > 4)
                    currentColor = 0;
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

            board.cursor.move(dx,dy);
            if((dx || dy) && key_is_down(KEY_A)){
                board.set(currentColor);
                refreshSkin();
            }
        }

        if(key_hit(KEY_A)){
            board.set(currentColor);

            refreshSkin();
        }

        if(key_hit(KEY_B)){
            board.set(0);

            refreshSkin();
        }

        if(key_held(KEY_START | KEY_SELECT))
            break;

        board.show();
        showMinos();

        showColorPalette(currentColor);

        oam_copy(oam_mem, obj_buffer, 32);
    }
}

void generateTiles(){

    TILE *t = new TILE();

    for(int i = 0; i < 5; i++){
        int n = 0;
        for(int j = 0; j < 8; j++)
            n += i << (j*4);

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
    for(int i = 0; i < 5; i++)
        *dest++ = i + 100;

    aprint("     ",0,1);
    aprint("^",c,1);
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
