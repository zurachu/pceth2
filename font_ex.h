#ifndef __FONT_EX_H_
#define __FONT_EX_H_

typedef struct SFONT {
	int x;							//表示位置横座標
	int y;							//表示位置縦座標
	int xMin;						//表示範囲横軸最小値
	int xMax;						//表示範囲横軸最大値
	int yMin;						//表示範囲縦軸最小値
	int yMax;						//表示範囲縦軸最大値
	int spr;						//文字色/背景色/縁取色/透過
									/*
									0:黒    /白    /なし  /なし
									1:黒    /白    /なし  /あり
									2:白    /黒    /なし  /なし
									3:白    /黒    /なし  /あり
									5:黒    /白    /白    /あり
									7:白    /黒    /黒    /あり
									*/
} SFONT;

extern SFONT sFontStatus;

void hook_FontGetAdrs();
void unhook_FontGetAdrs();

unsigned short sFontPut( int x, int y, unsigned short code );
char *sFontPutStr(const char *p);
int sFontPrintf(const char* fmt, ...);

#endif
