#include "def.h"
#include "tonc_types.h"
#include "tonc_irq.h"
#include "maxmod.h"
#include "soundbank_bin.h"

#define audioChannels 12

u8 myMixingBuffer[ MM_MIXLEN_16KHZ ] __attribute((aligned(4)));

void maxModInit(){

    u8* myData;
    mm_gba_system mySystem;

    myData = (u8*)malloc( audioChannels * (MM_SIZEOF_MODCH
                               +MM_SIZEOF_ACTCH
                               +MM_SIZEOF_MIXCH)
                               +MM_MIXLEN_16KHZ );

    mySystem.mixing_mode       = MM_MIX_16KHZ;

    mySystem.mod_channel_count = audioChannels;
    mySystem.mix_channel_count = audioChannels;

    mySystem.module_channels   = (mm_addr)(myData+0);
    mySystem.active_channels   = (mm_addr)(myData+(audioChannels*MM_SIZEOF_MODCH));
    mySystem.mixing_channels   = (mm_addr)(myData+(audioChannels*(MM_SIZEOF_MODCH
	                                             +MM_SIZEOF_ACTCH)));
    mySystem.mixing_memory     = (mm_addr)myMixingBuffer;
    mySystem.wave_memory       = (mm_addr)(myData+(audioChannels*(MM_SIZEOF_MODCH
                                                     +MM_SIZEOF_ACTCH
                                                     +MM_SIZEOF_MIXCH)));
    mySystem.soundbank         = (mm_addr)soundbank_bin;

    mmInit( &mySystem );
}
