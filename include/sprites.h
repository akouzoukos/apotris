#include "../build/sprite_pal_bin.h"
#include "../build/sprite1tiles_bin.h"
#include "../build/sprite2tiles_bin.h"
#include "../build/sprite3tiles_bin.h"
#include "../build/sprite4tiles_bin.h"
#include "../build/sprite5tiles_bin.h"
#include "../build/sprite6tiles_bin.h"
#include "../build/sprite7tiles_bin.h"
#include "../build/sprite8tiles_bin.h"
#include "../build/sprite9tiles_bin.h"
#include "../build/sprite10tiles_bin.h"
#include "../build/sprite11tiles_bin.h"
#include "../build/sprite12tiles_bin.h"
#include "../build/sprite13tiles_bin.h"
#include "../build/sprite14tiles_bin.h"

#include "../build/title1tiles_bin.h"
#include "../build/title2tiles_bin.h"
#include "../build/title_pal_bin.h"

#define paletteLen 512
const unsigned short palette[256] __attribute__((aligned(4)))=
{
	0x0000,0x45c3,0x6ea5,0x7f26,0x7FFF,0x0c63,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//0

	0x0000,0x48a4,0x5ce5,0x7506,0x7FFF,0x1ce7,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//1

	0x0000,0x0174,0x01fa,0x025f,0x7FFF,0x318c,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//2

	0x0000,0x0276,0x033c,0x037f,0x7FFF,0x5294,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//3

	0x0000,0x0287,0x02e8,0x0368,0x7FFF,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//4

	0x0000,0x588f,0x6cb2,0x7cd5,0x7FFF,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//5

	0x0000,0x0015,0x0018,0x001a,0x7FFF,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//6

	0x0000,0x1ce7,0x318c,0x5294,0x7FFF,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//7

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//8

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//9

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//10

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//11

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//12

	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//13

	0x0000,0x4A52,0x318c,0x0421,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//14

	0x0000,0x4A52,0x7FFF,0x294a,0x0000,0x0000,0x0000,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,//15

};

extern const uint8_t * mini[7];