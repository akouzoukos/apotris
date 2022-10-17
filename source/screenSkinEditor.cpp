#include "def.h"
#include "tonc.h"

#include "logging.h"
#include "text.h"
#include "posprintf.h"

#include "sprite29tiles_bin.h"
#include "sprite30tiles_bin.h"
#include "sprite31tiles_bin.h"
#include "sprite32tiles_bin.h"
#include "sprite33tiles_bin.h"
#include "sprite34tiles_bin.h"
#include "sprite35tiles_bin.h"

#include <string>
#include <tuple>
#include <algorithm>

#include "soundbank.h"
#include "tonc_bios.h"
#include "tonc_input.h"
#include "tonc_memdef.h"
#include "tonc_memmap.h"
#include "tonc_oam.h"

const int xoffset = 15-8;
const int yoffset = 10-8;

class Cursor{
public:
    int x = 0;
    int y = 0;
    OBJ_ATTR* sprite;

    void show(bool mini){
        int sx = (x+mini) * 16 + xoffset * 8;
        int sy = (y+mini) * 16 + yoffset * 8;

        obj_set_pos(sprite,sx,sy);
        obj_unhide(sprite,0);
    }

    bool move(int dx, int dy,bool mini){
        bool sound = false;
        bool moved = false;

        if(dx){
            if(x+dx >= 0 && x+dx <= (7-(mini*2))){
                x += dx;
                sfx(SFX_SHIFT2);
                moved = true;
                sound = true;
            }
        }
        if(dy){
            if(y+dy >= 0 && y+dy <= 7-(mini*2)){
                y += dy;
                moved = true;
                if(!sound)
                    sfx(SFX_SHIFT2);
            }
        }

        return moved;
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

        std::list<std::tuple<int,int>> queue;

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
            if(value == previous)
                return;
            if(record)
                history.push_back(Move(cursor.x,cursor.y,previous,value,2));

            int x,y;

            queue.push_back(std::make_tuple(cursor.x,cursor.y));

            do{
                std::tuple<int,int> coords = queue.front();
                x = std::get<0>(coords);
                y = std::get<1>(coords);

                board[y][x] = value;
                int s = (x) * 4;
                customSkin->data[y] = (customSkin->data[y] & ~(0xf << (s))) | ((value + (value == 5)) << (s));

                if(y - 1 >= 0 && board[y-1][x] == previous){
                    auto n = std::make_tuple(x,y-1);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }
                if(y + 1 <= 7 && board[y+1][x] == previous){
                    auto n = std::make_tuple(x,y+1);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }
                if(x - 1 >= 0 && board[y][x-1] == previous){
                    auto n = std::make_tuple(x-1,y);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }
                if(x + 1 <= 7 && board[y][x+1] == previous){
                    auto n = std::make_tuple(x+1,y);
                    if(std::find(queue.begin(),queue.end(),n) == queue.end())
                        queue.push_back(n);
                }

                queue.pop_front();
            }while(!queue.empty());

        }

        void show(bool mini){
            u16* dest = (u16*) &se_mem[25];

            for(int i = 0; i < 8; i++){
                int y = ((i * 2)+yoffset) * 32;
                for(int j = 0; j < 8; j++){
                    int x = j * 2 + xoffset;
                    if(!mini){
                        if(board[i][j] != 0){
                            int n = board[i][j] + 100 - (board[i][j] == 6);
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
                    }else{
                        if(i == 0 || j == 0 || i > 6 || j > 6){
                            dest[y+x] = 0;
                            dest[y+x+1] = 0;
                            dest[y+32+x] = 0;
                            dest[y+32+x+1] = 0;
                        }else if(board[i-1][j-1] != 0){
                            int n = board[i-1][j-1] + 100 - (board[i-1][j-1] == 6);
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
            }

            cursor.show(mini);
        }

        bool undo(){
            if(history.empty())
               return false;

            Move previous = history.back();

            cursor.x = previous.x;
            cursor.y = previous.y;


            switch(previous.tool){
            case 1: set(previous.previousValue,false); break;
            case 2: fill(previous.previousValue,false); break;
            }

            history.pop_back();

            return true;
        }

        void setBoard(TILE * t,int type){
            if(type == 0){
                for(int i = 0; i < 8; i++){
                    for(int j = 0; j < 8; j++){
                        board[i][j] = (t->data[i] >> (j * 4)) & 0xf;
                    }
                }
            }else{
                for(int i = 0; i < 8; i++){
                    for(int j = 0; j < 8; j++){
                        if(i < 6 && j < 6)
                            board[i][j] = (t->data[i] >> ((j) * 4)) & 0xf;
                        else
                            board[i][j] = 0;
                    }
                }
            }
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
void showMini();
int selector();
void helpScreen();
int optionsMenu();

TILE * customSkin;

static int cursorTimer = 0;
static int toolTimer = 0;

static int skinIndex = 0;
static bool onMini = false;

void skinEditor(){
    onMini = false;
    irq_disable(II_HBLANK);
    memset16(&pal_bg_mem[0],0,1);
    memset16(&se_mem[27], 0x0000, 32 * 20);
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(2);

    u16* dest = (u16*) &se_mem[26];

    for(int i = 0; i < 32; i++)
        for(int j = 0; j < 20; j++)
            *dest++ = 2;

    while(true){

        memcpy16(&tile_mem[5][216],sprite35tiles_bin,sprite35tiles_bin_size/2);

        if(selector())
            break;

        REG_DISPCNT |= DCNT_BG3;
        clearText();

        memcpy16(&tile_mem[0][106],sprite34tiles_bin,sprite34tiles_bin_size/2);

        memcpy16(&tile_mem[5][200],sprite29tiles_bin,sprite29tiles_bin_size/2);
        memcpy16(&tile_mem[5][204],sprite31tiles_bin,sprite31tiles_bin_size/2);
        memcpy16(&tile_mem[5][208],sprite32tiles_bin,sprite32tiles_bin_size/2);
        memcpy16(&tile_mem[5][212],sprite33tiles_bin,sprite33tiles_bin_size/2);

        generateTiles();

        Board board;

        for(int i = 0; i < 8; i++)
            customSkin->data[i] = savefile->customSkins[skinIndex].board.data[i];

        board.setBoard(customSkin,0);
        refreshSkin();

        setSmallTextArea(100, 0, 10, 10, 20);
        aprints("Press SELECT",2,64,2);
        aprints("for help",2,72,2);

        int currentColor = 2;
        int currentTool = 1;

        int das = 0;
        const int arr = 4;
        const int dasMax = 20;

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
                    sfx(SFX_MENUMOVE);
                }

                if(key_hit(KEY_RIGHT)){
                    currentColor++;
                    if(currentColor > 5)
                        currentColor = 0;
                    sfx(SFX_MENUMOVE);
                }

                if(key_hit(KEY_UP)){
                    currentTool--;
                    if(currentTool < 0)
                        currentTool = 2;
                    sfx(SFX_MENUMOVE);
                }

                if(key_hit(KEY_DOWN)){
                    currentTool++;
                    if(currentTool > 2)
                        currentTool = 0;
                    sfx(SFX_MENUMOVE);
                }
            }else if(key_is_down(KEY_L)){
                if(key_hit(KEY_UP)){
                    savefile->settings.colors--;
                    if(savefile->settings.colors < 0)
                        savefile->settings.colors = MAX_COLORS-1;
                    sfx(SFX_MENUMOVE);
                }

                if(key_hit(KEY_DOWN)){
                    savefile->settings.colors++;
                    if(savefile->settings.colors > MAX_COLORS-1)
                        savefile->settings.colors = 0;
                    sfx(SFX_MENUMOVE);
                }

                if(key_hit(KEY_UP) || key_hit(KEY_DOWN))
                    setPalette();

                if(key_hit(KEY_LEFT) || key_hit(KEY_RIGHT)){
                    refreshSkin();
                    onMini = !onMini;

                    if(onMini){
                        for(int i = 0; i < 8; i++)
                            customSkin->data[i] = savefile->customSkins[skinIndex].smallBoard.data[i];

                        if(board.cursor.x > 0)
                            board.cursor.x--;
                        else if(board.cursor.x > 5)
                            board.cursor.x = 5;

                        if(board.cursor.y > 0)
                            board.cursor.y--;
                        else if(board.cursor.y > 5)
                            board.cursor.y = 5;
                    }else{
                        for(int i = 0; i < 8; i++)
                            customSkin->data[i] = savefile->customSkins[skinIndex].board.data[i];
                        board.cursor.x++;
                        board.cursor.y++;
                    }

                    board.setBoard(customSkin,onMini);
                }

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

                bool moved = board.cursor.move(dx,dy,onMini);
                if((dx || dy) && key_is_down(KEY_A)){
                    switch(currentTool){
                    case 0: board.set(0,true); break;
                    case 1: board.set(currentColor,true); break;
                    }
                    if(moved)
                        sfx(SFX_PLACE);
                    refreshSkin();
                }
            }

            if(key_hit(KEY_A)){
                switch(currentTool){
                case 0: board.set(0,true); break;
                case 1: board.set(currentColor,true); break;
                case 2: board.fill(currentColor,true); break;
                }

                sfx(SFX_PLACE);
                refreshSkin();
            }

            if(key_hit(KEY_B)){
                if(board.undo())
                    sfx(SFX_ROTATE);
                else
                    sfx(SFX_MENUCANCEL);

                refreshSkin();
            }

            if(key_hit(KEY_START)){
                sfx(SFX_MENUCONFIRM);
                break;
            }

            if(key_hit(KEY_SELECT)){
                helpScreen();
            }

            board.show(onMini);
            showMinos();
            showCursor();
            // showMini();

            showColorPalette(currentColor);
            showTools(currentTool);

            oam_copy(oam_mem, obj_buffer, 32);
        }

        // for(int i = 0; i < 8; i++)
        //     savefile->customSkins[skinIndex].board.data[i] = customSkin->data[i];
        refreshSkin();

        memset16(&tile_mem[5][200],0,64*4);

        oam_init(obj_buffer,128);
        oam_copy(oam_mem, obj_buffer, 128);
        memset16(&se_mem[25], 0x0000, 32 * 20);
        clearText();
        REG_DISPCNT &= ~DCNT_BG3;
    }

    //clean up
    oam_init(obj_buffer,128);
    oam_copy(oam_mem, obj_buffer, 128);
    memset16(&tile_mem[5][200],0,64*4);

    irq_enable(II_HBLANK);
    memset16(&se_mem[25], 0x0000, 32 * 20);
    memset16(&se_mem[26], 0x0000, 32 * 20);
    memset16(&se_mem[27], 0x0000, 32 * 20);
    drawUIFrame(0, 0, 30, 20);
    REG_BG3CNT = BG_CBB(0) | BG_SBB(27) | BG_SIZE(0) | BG_PRIO(3);
    REG_BG3VOFS = 0;
    REG_BG3HOFS = 0;
}

int selector(){
    int das = 0;
    const int arr = 4;
    const int dasMax = 20;

    int selection = 0;
    const int options = 2;

    const int slotHeight = 9;

    aprint("Select a slot to edit:",4,6);

    for(int i = 0; i < 5; i++){
        OBJ_ATTR * sprite = &obj_buffer[i+1];

        memcpy16(&tile_mem[5][200+i],&savefile->customSkins[i].board,16);

        obj_set_attr(sprite, ATTR0_SQUARE, ATTR1_SIZE(0), ATTR2_BUILD(712+i, 5, 0));
        obj_set_pos(sprite,(4+i*5)*8 + 4,(slotHeight * 8) + 12);
        obj_unhide(sprite,0);
    }

    oam_copy(oam_mem, obj_buffer, 32);

    while(1){
        VBlankIntrWait();
        key_poll();

        aprintColor(" C1   C2   C3   C4   C5 ",3, slotHeight, 1);
        aprint("C" + std::to_string(skinIndex+1),4 + skinIndex*5,slotHeight);

        aprint(" BACK ",12,16);

        if(selection == 0){
            int x = 3 + skinIndex*5;
            aprint("[",x,slotHeight);
            aprint("]",x+3,slotHeight);

        }else if(selection == options-1){
            aprint("[",12,16);
            aprint("]",17,16);
        }


        if(key_hit(KEY_A) || key_hit(KEY_START)){
            if(selection == 0){
                sfx(SFX_MENUCONFIRM);
                break;
            }else if(selection == 1){
                sfx(SFX_MENUCANCEL);
                return 1;
            }
        }

        if(key_hit(KEY_UP)){
            selection--;
            if(selection < 0)
                selection = options-1;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_DOWN) || key_hit(KEY_SELECT)){
            selection++;
            if(selection > options-1)
                selection = 0;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_LEFT) && skinIndex > 0 && !selection){
            skinIndex--;
            sfx(SFX_MENUMOVE);
        }

        if(key_hit(KEY_RIGHT) && skinIndex < 4 && !selection){
            skinIndex++;
            sfx(SFX_MENUMOVE);
        }

        if(key_is_down(KEY_LEFT) || key_is_down(KEY_RIGHT)){
            das++;

            if(das == dasMax){
                das-=arr;

                if(key_is_down(KEY_LEFT) && skinIndex > 0 && !selection){
                    skinIndex--;
                    sfx(SFX_MENUMOVE);
                }else if(key_is_down(KEY_RIGHT) && skinIndex < 4 && !selection){
                    skinIndex++;
                    sfx(SFX_MENUMOVE);
                }
            }
        }else {
            das = 0;
        }

        if(key_hit(KEY_B)){
            sfx(SFX_MENUCANCEL);
            return 1;
        }
    }

    return 0;
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
    u16* dest = (u16*) &se_mem[27];

    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 8; j++){
            if(i == 1){
                if(j > 0 && j < 7)
                    *dest++ = 100 + j - 1;
                else
                    *dest++ = 27 + 0x400 * (j == 7) + 0x6000;
            }else{
                if(j > 0 && j < 7)
                    *dest++ = 28 + 0x800 * (i == 2) + 0x6000;
                else
                    *dest++ = 29 + 0x400 * (j == 7) + 0x800 * (i == 2) + 0x6000;

            }
        }

        dest+=32-8;
    }

    aprint("      ",0,1);

    REG_BG3VOFS = 4;
    REG_BG3HOFS = 4;

    OBJ_ATTR * sprite = &obj_buffer[11];

    obj_set_attr(sprite,ATTR0_SQUARE,ATTR1_SIZE(1),ATTR2_BUILD(512 + 216, 15, 0));
    obj_set_pos(sprite,3+c*8,3);
    obj_unhide(sprite,0);
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

    for(int i = 0; i < 7; i++){
    OBJ_ATTR *sprite = &obj_buffer[i+1];

        if(!onMini){
            obj_set_attr(sprite, ATTR0_SQUARE, ATTR1_SIZE(2), ATTR2_BUILD(16 * i, i, 2));
            obj_unhide(sprite,0);
        }else{
            obj_unhide(sprite, 0);
            obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_BUILD(9 * 16 + 8 * i, i, 3));
            // obj_set_pos(sprite, 192, i * 24 + 4);
        }
        obj_set_pos(sprite,208 - (4-onMini) * (i == 0 || i == 3),i * 24 - 4);
    }
}

void refreshSkin(){
    if(!onMini){
        savefile->settings.skin = -(skinIndex+1);
        for(int i = 0; i < 8; i++){
            savefile->customSkins[skinIndex].board.data[i] = customSkin->data[i];
        }
        setSkin();
    }else{
        for(int i = 0; i < 8; i++){
            savefile->customSkins[skinIndex].smallBoard.data[i] = customSkin->data[i];
        }
        buildMini(customSkin);
    }
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

void showMini(){
    for(int i = 0; i < 7; i++){
        OBJ_ATTR * sprite = &obj_buffer[12+i];

        // int n = savefile->customSkins[skinIndex].previewStyle;

        obj_unhide(sprite, 0);
        obj_set_attr(sprite, ATTR0_WIDE, ATTR1_SIZE(2), ATTR2_BUILD(9 * 16 + 8 * i, i, 3));
        obj_set_pos(sprite, 192, i * 24 + 4);
    }
}

void helpScreen(){
    sfx(SFX_MENUCONFIRM);
    setSmallTextArea(100, 8, 1, 24, 21);
    clearText();

    const int startX = 2;
    const int startY = 2;
    u8 count = 0;

    aprints("-Move using the D-Pad",startX, (startY+count)*4,2);
    count+= 3;

    aprints("-Use tools by pressing A",startX, (startY+count)*4,2);
    count+= 3;

    aprints("-Undo by pressing B",startX, (startY+count)*4,2);
    count+= 3;

    aprints("-Change tools by holding R",startX, (startY+count)*4,2);
    aprints(" and pressing Up/Down",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Change colors by holding R",startX, (startY+count)*4,2);
    aprints(" and pressing Left/Right",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Cycle palettes by holding L",startX, (startY+count)*4,2);
    aprints(" and pressing Up/Down",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Edit preview/hold by holding R",startX, (startY+count)*4,2);
    aprints(" and pressing Left/Right",startX, (startY+count+2)*4,2);
    count+= 5;

    aprints("-Exit by pressing Start",startX, (startY+count)*4,2);
    count+= 3;

    oam_init(obj_mem, 128);

    REG_DISPCNT &= ~DCNT_BG0;
    REG_DISPCNT &= ~DCNT_BG3;

    while(true){
        VBlankIntrWait();
        key_poll();

        if(key_hit(KEY_FULL))
            break;
    }

    REG_DISPCNT |= DCNT_BG0;
    REG_DISPCNT |= DCNT_BG3;
    setSmallTextArea(100, 0, 10, 10, 20);
    clearText();
    aprints("Press SELECT",2,64,2);
    aprints("for help",2,72,2);
    sfx(SFX_MENUCANCEL);
}
