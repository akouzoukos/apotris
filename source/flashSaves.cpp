#include "tonc.h"
#include "def.h"
#include "stddef.h"
#include "flashSaves.h"
#include "LinkConnection.h"
#include "logging.h"

#include "text.h"

#if ENABLE_FLASH_SAVE

void flash_write(u8 flash_type, u32 sa);
void save_sram_flash();

// static u32 total_rom_size = 0;
u32 flash_size = 0;
u32 flash_sram_area = 0;
u8 flash_type = 0;
bool escape = false;

__attribute__((section(".iwram"), target("arm")))
static void bytecopy(vu8 *dst, vu8 *src, int count) {
	for (int i = 0; i < count; i++) {
		dst[i] = src[i];
	}
}

__attribute__((section(".ewram")))
static int bytecmp(vu8 *buf1, vu8 *buf2, size_t count) {
	if (count == 0) {
		return 0;
	}
	while (--count && *buf1 == *buf2) {
		buf1++;
		buf2++;
	}
	return *buf1 - *buf2;
}

// This function will auto-detect four common
// types of reproduction flash cartridges.
// Must run in EWRAM because ROM data is
// not visible to the system while checking.
__attribute__((section(".ewram")))
u32 get_flash_type() {

	//test if able to write to SRAM/FRAM
    volatile u8* sf = (volatile u8*)sram_mem;

    savefile = new Save();
    u8* arr = (u8*)savefile;

    for (int i = 0; i < (int)sizeof(Save); i++)
		arr[i] = (u8)sf[i];

	if(savefile->newGame >= 0x4b && savefile->newGame <= SAVE_TAG){
		escape = true;
		return 0;
	}

	u32 rom_data, data;
	u16 ie = REG_IE;
	//stop_dma_interrupts();
	REG_IE = ie & 0xFFFE;

	rom_data = *(u32 *)AGB_ROM;

	// Type 1 or 4
	_FLASH_WRITE(0, 0xFF);
	_FLASH_WRITE(0, 0x90);
	data = *(u32 *)AGB_ROM;
	_FLASH_WRITE(0, 0xFF);
	if (rom_data != data) {
		// Check if the chip is responding to this command
		// which then needs a different write command later
		_FLASH_WRITE(0x59, 0x42);
		data = *(u8 *)(AGB_ROM+0xB2);
		_FLASH_WRITE(0x59, 0x96);
		_FLASH_WRITE(0, 0xFF);
		if (data != 0x96) {
			REG_IE = ie;
			//resume_interrupts();
			return 4;
		}
		REG_IE = ie;
		//resume_interrupts();
		return 1;
	}

	// Type 2
	_FLASH_WRITE(0, 0xF0);
	_FLASH_WRITE(0xAAA, 0xA9);
	_FLASH_WRITE(0x555, 0x56);
	_FLASH_WRITE(0xAAA, 0x90);
	data = *(u32 *)AGB_ROM;
	_FLASH_WRITE(0, 0xF0);
	if (rom_data != data) {
		REG_IE = ie;
		//resume_interrupts();
		return 2;
	}

	// Type 3
	_FLASH_WRITE(0, 0xF0);
	_FLASH_WRITE(0xAAA, 0xAA);
	_FLASH_WRITE(0x555, 0x55);
	_FLASH_WRITE(0xAAA, 0x90);
	data = *(u32 *)AGB_ROM;
	_FLASH_WRITE(0, 0xF0);
	if (rom_data != data) {
		REG_IE = ie;
		//resume_interrupts();
		return 3;
	}

	REG_IE = ie;
	//resume_interrupts();
	return 0;
}

// This function will issue a flash sector erase
// operation at the given sector address and then
// write 64 kilobytes of SRAM data to Flash ROM.
// Must run in EWRAM because ROM data is
// not visible to the system while erasing/writing.
__attribute__((section(".ewram")))
void flash_write(u8 flash_type, u32 sa)
{
	if (flash_type == 0) return;
	u16 ie = REG_IE;
	//stop_dma_interrupts();
	REG_IE = ie & 0xFFFE;

	if (flash_type == 1) {
		// Erase flash sector
		_FLASH_WRITE(sa, 0xFF);
		_FLASH_WRITE(sa, 0x60);
		_FLASH_WRITE(sa, 0xD0);
		_FLASH_WRITE(sa, 0x20);
		_FLASH_WRITE(sa, 0xD0);
		while (1) {
			__asm("nop");
			if (*(((u16 *)AGB_ROM)+(sa/2)) == 0x80) {
				break;
			}
		}
		_FLASH_WRITE(sa, 0xFF);

		// Write data
		for (int i=0; i<AGB_SRAM_SIZE; i+=2) {
			_FLASH_WRITE(sa+i, 0x40);
			_FLASH_WRITE(sa+i, (*(u8 *)(AGB_SRAM+i+1)) << 8 | (*(u8 *)(AGB_SRAM+i)));
			while (1) {
				__asm("nop");
				if (*(((u16 *)AGB_ROM)+(sa/2)) == 0x80) {
					break;
				}
			}
		}
		_FLASH_WRITE(sa, 0xFF);

	} else if (flash_type == 2) {
		// Erase flash sector
		_FLASH_WRITE(sa, 0xF0);
		_FLASH_WRITE(0xAAA, 0xA9);
		_FLASH_WRITE(0x555, 0x56);
		_FLASH_WRITE(0xAAA, 0x80);
		_FLASH_WRITE(0xAAA, 0xA9);
		_FLASH_WRITE(0x555, 0x56);
		_FLASH_WRITE(sa, 0x30);
		while (1) {
			__asm("nop");
			if (*(((u16 *)AGB_ROM)+(sa/2)) == 0xFFFF) {
				break;
			}
		}
		_FLASH_WRITE(sa, 0xF0);

		// Write data
		for (int i=0; i<AGB_SRAM_SIZE; i+=2) {
			_FLASH_WRITE(0xAAA, 0xA9);
			_FLASH_WRITE(0x555, 0x56);
			_FLASH_WRITE(0xAAA, 0xA0);
			_FLASH_WRITE(sa+i, (*(u8 *)(AGB_SRAM+i+1)) << 8 | (*(u8 *)(AGB_SRAM+i)));
			while (1) {
				__asm("nop");
				if (*(((u16 *)AGB_ROM)+((sa+i)/2)) == ((*(u8 *)(AGB_SRAM+i+1)) << 8 | (*(u8 *)(AGB_SRAM+i)))) {
					break;
				}
			}
		}
		_FLASH_WRITE(sa, 0xF0);

	} else if (flash_type == 3) {
		// Erase flash sector
		_FLASH_WRITE(sa, 0xF0);
		_FLASH_WRITE(0xAAA, 0xAA);
		_FLASH_WRITE(0x555, 0x55);
		_FLASH_WRITE(0xAAA, 0x80);
		_FLASH_WRITE(0xAAA, 0xAA);
		_FLASH_WRITE(0x555, 0x55);
		_FLASH_WRITE(sa, 0x30);
		while (1) {
			__asm("nop");
			if (*(((u16 *)AGB_ROM)+(sa/2)) == 0xFFFF) {
				break;
			}
		}
		_FLASH_WRITE(sa, 0xF0);

		// Write data
		for (int i=0; i<AGB_SRAM_SIZE; i+=2) {
			_FLASH_WRITE(0xAAA, 0xAA);
			_FLASH_WRITE(0x555, 0x55);
			_FLASH_WRITE(0xAAA, 0xA0);
			_FLASH_WRITE(sa+i, (*(u8 *)(AGB_SRAM+i+1)) << 8 | (*(u8 *)(AGB_SRAM+i)));
			while (1) {
				__asm("nop");
				if (*(((u16 *)AGB_ROM)+((sa+i)/2)) == ((*(u8 *)(AGB_SRAM+i+1)) << 8 | (*(u8 *)(AGB_SRAM+i)))) {
					break;
				}
			}
		}
		_FLASH_WRITE(sa, 0xF0);

	} else if (flash_type == 4) {
		// Erase flash sector
		_FLASH_WRITE(sa, 0xFF);
		_FLASH_WRITE(sa, 0x60);
		_FLASH_WRITE(sa, 0xD0);
		_FLASH_WRITE(sa, 0x20);
		_FLASH_WRITE(sa, 0xD0);
		while (1) {
			__asm("nop");
			if ((*(((u16 *)AGB_ROM)+(sa/2)) & 0x80) == 0x80) {
				break;
			}
		}
		_FLASH_WRITE(sa, 0xFF);

		// Write data
		int c = 0;
		while (c < AGB_SRAM_SIZE) {
			_FLASH_WRITE(sa+c, 0xEA);
			while (1) {
				__asm("nop");
				if ((*(((u16 *)AGB_ROM)+((sa+c)/2)) & 0x80) == 0x80) {
					break;
				}
			}
			_FLASH_WRITE(sa+c, 0x1FF);
			for (int i=0; i<1024; i+=2) {
				_FLASH_WRITE(sa+c+i, (*(u8 *)(AGB_SRAM+c+i+1)) << 8 | (*(u8 *)(AGB_SRAM+c+i)));
			}
			_FLASH_WRITE(sa+c, 0xD0);
			while (1) {
				__asm("nop");
				if ((*(((u16 *)AGB_ROM)+((sa+c)/2)) & 0x80) == 0x80) {
					break;
				}
			}
			_FLASH_WRITE(sa+c, 0xFF);
			c += 1024;
		}
	}

	REG_IE = ie;
	//resume_interrupts();
}

void flash_init() {

	REG_DISPCNT = 0;

	flash_type = get_flash_type();
	if (flash_type > 0) {

		// Determine the size of the flash chip by checking for ROM loops,
		// then set the SRAM storage area 0x40000 bytes before the end.
		// This is due to different sector sizes of different flash chips,
		// and should hopefully cover all cases.
		if (bytecmp(AGB_ROM+4, AGB_ROM+4+0x400000, 0x40) == 0) {
			flash_size = 0x400000;
		} else if (bytecmp(AGB_ROM+4, AGB_ROM+4+0x800000, 0x40) == 0) {
			flash_size = 0x800000;
		} else if (bytecmp(AGB_ROM+4, AGB_ROM+4+0x1000000, 0x40) == 0) {
			flash_size = 0x1000000;
		} else {
			flash_size = 0x2000000;
		}
		flash_sram_area = flash_size - 0x40000;

		// Finally, restore the SRAM data and proceed.
		bytecopy(AGB_SRAM, ((vu8*)AGB_ROM+flash_sram_area), AGB_SRAM_SIZE);

	} else { // Emulator mode?
		if(escape)
			return;

		if ((*(vu32*)(AGB_ROM+0x400000-0x40000) == STATEID) || (*(vu32*)(AGB_ROM+0x400000-0x40000) == STATEID2)) {
			flash_sram_area = 0x400000-0x40000;
		} else if ((*(vu32*)(AGB_ROM+0x800000-0x40000) == STATEID) || (*(vu32*)(AGB_ROM+0x800000-0x40000) == STATEID2)) {
			flash_sram_area = 0x800000-0x40000;
		} else if ((*(vu32*)(AGB_ROM+0x1000000-0x40000) == STATEID) || (*(vu32*)(AGB_ROM+0x1000000-0x40000) == STATEID2)) {
			flash_sram_area = 0x1000000-0x40000;
		} else if ((*(vu32*)(AGB_ROM+0x2000000-0x40000) == STATEID) || (*(vu32*)(AGB_ROM+0x2000000-0x40000) == STATEID2)) {
			flash_sram_area = 0x2000000-0x40000;
		}

		if (flash_sram_area != 0) {
			bytecopy(AGB_SRAM, ((vu8*)AGB_ROM+flash_sram_area), AGB_SRAM_SIZE);
		}
	}
}

void save_sram_flash()
{
	if (flash_type == 0) return;

	aprint("Saving...",21,19);
    irq_disable(II_VBLANK);
    irq_disable(II_HBLANK);
    int stat_value = REG_SNDSTAT;
    int dsc_value = REG_SNDDSCNT;
    int dmg_value = REG_SNDDMGCNT;

    REG_SNDSTAT = 0;
    REG_SNDDSCNT = 0;
    REG_SNDDMGCNT = 0;

    linkConnection->deactivate();
    irq_disable(II_TIMER3);
    irq_disable(II_SERIAL);

	flash_write(flash_type, flash_sram_area);

    REG_SNDSTAT = stat_value;
    REG_SNDDSCNT = dsc_value;
    REG_SNDDMGCNT = dmg_value;

    linkConnection->activate();
    irq_enable(II_TIMER3);
    irq_enable(II_SERIAL);
    irq_delete(II_KEYPAD);
    irq_enable(II_VBLANK);

	aprint("   Saved!",21,19);//spaces necessary to "erase" the previous text
}
#endif
