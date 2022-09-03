#pragma once

#include "tonc.h"

#define AGB_ROM  ((u8*)0x8000000)
#define AGB_SRAM ((u8*)0xE000000)
#define AGB_SRAM_SIZE 32*1024
#define _FLASH_WRITE(pa, pd) { *(((u16 *)AGB_ROM)+((pa)/2)) = pd; __asm("nop"); }
#define STATEID 0x57a731d8
#define STATEID2 0x57a731d9

extern u32 flash_size;
extern u32 flash_sram_area;
extern u8 flash_type;

extern u32 get_flash_type();
void save_sram_flash();
void flash_init();
