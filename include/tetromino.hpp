#pragma once

#include "tonc_types.h"

namespace GameInfo{
    extern const int colors[7][3];

    extern const int tetraminos[7][4][4][4];

    extern const int classic[7][4][4][4];

    extern const int ars[7][4][4][4];

    extern const int kickTwice[4][6][2];

    extern const int kicks[2][2][4][5][2];

    extern const int arikaKicks[];

    extern const float gravity[19];

    extern const float classicGravity[30];

    extern const float blitzGravity[15];

    extern const int blitzLevels[15];

    extern const int scoring[17][3];

    extern const int classicScoring[4];

    extern const int comboTable[20];

    extern const int finesse[7][4][10][4];

    extern const u16 masterDelays[9][5];

    extern const float masterGravity[30][2];

    extern const int sectionTimeGoal[10][2];

    extern const char* masterGrades[36];

    extern const u8 gradeTable[32][6];

    extern const float masterComboMultiplayer[10][4];
}
