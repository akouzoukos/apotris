

#include "small1tiles_bin.h"
#include "small2tiles_bin.h"
#include "small3tiles_bin.h"
#include "small4tiles_bin.h"
#include "small5tiles_bin.h"
#include "small6tiles_bin.h"
#include "small7tiles_bin.h"
#include "small8tiles_bin.h"
#include "small9tiles_bin.h"
#include "small10tiles_bin.h"
#include "small11tiles_bin.h"
#include "small12tiles_bin.h"
#include "small13tiles_bin.h"
#include "small14tiles_bin.h"

const uint8_t * mini[2][7] = {
    {
        small1tiles_bin,
        small2tiles_bin,
        small3tiles_bin,
        small4tiles_bin,
        small5tiles_bin,
        small6tiles_bin,
        small7tiles_bin,
    },{
        small8tiles_bin,
        small9tiles_bin,
        small10tiles_bin,
        small11tiles_bin,
        small12tiles_bin,
        small13tiles_bin,
        small14tiles_bin,
    }   
};