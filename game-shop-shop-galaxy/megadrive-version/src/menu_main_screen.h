#ifndef MENU_MAIN_SCREEN_H
#define MENU_MAIN_SCREEN_H

#define SPRITE_MAX		5
#define	TABLE_LEN	 	(224 >> 3)
#define PLAN_A_X	 	-200
#define PRESS_START_X	204
#define PRESS_START_Y	192
#define BAR_X			128
#define BAR_Y			210
#define TIME_TO_STORY	1000 //vbl
#define STORY_TEXT01_X	0
#define STORY_TEXT01_Y	0
#define STORY_TEXT02_X	20
#define STORY_TEXT02_Y	20

const u16 palettes_white[64] = {0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,
								0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE,0xEEE};

struct storyTexter_{
	u16 vbl_delta;
	u16 x;
	u16 y;
	Box rect;
	u16 cursor;
};

u16 storyTexter_update(const char *text, struct storyTexter_ *p_storyTexter, u16 len, u16 vbl);
void resetScrolling();
#endif
