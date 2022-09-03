#pragma once
#include "tonc_types.h"
#include <string>

#define VOLADDR(a, t) (*(t volatile *)(a))
#define REG_MGBA_ENABLE         VOLADDR(0x04FFF780, u16)
#define REG_MGBA_FLAGS          VOLADDR(0x04FFF700, u16)
#define MGBA_LOG_OUT            ((char*)0x04FFF600)

static bool logInitMgba(void)
{
    REG_MGBA_ENABLE = 0xC0DE;

    return REG_MGBA_ENABLE == 0x1DEA;
}

static void logOutputMgba(u8 level, const char *message)
{
    for (int i = 0; message[i] && i < 256; i++)
    {
        MGBA_LOG_OUT[i] = message[i];
    }

    REG_MGBA_FLAGS = (level - 1) | 0x100;
}

static inline void log(std::string str){
    logOutputMgba(4, str.c_str());
}
