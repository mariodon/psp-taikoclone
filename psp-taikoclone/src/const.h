#ifndef __CONST_H__
#define __CONST_H__

#define SCREEN_WIDTH    480
#define SCREEN_HEIGHT   272

#define TAIKO_PF    OSL_PF_4444

//speical note
#define BARLINE    9
#define GGT_START  10
#define GGT_END    11 //won't draw when it "passed" screen, stop ggt
#define BRANCH_E   12
#define BRANCH_N   13
#define BRANCH_H   14
#define SCROLL     15
#define BPMCHANGE  16

#define MAX_FILENAME    256
#define MAX_FILENAME_UCS2   (MAX_FILENAME >> 1)
#define MAX_TITLE   100
#define MAX_SUBTITLE    100

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#define MAX_TEXTURE_NAME	23

#define BG_UPPER_WIDTH	128

#define FUMEN_LEVEL_NORMAL	0
#define FUMEN_LEVEL_EXPERT	1
#define FUMEN_LEVEL_MASTER	2

#endif
