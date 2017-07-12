/*
 *	16階調表示関連ライブラリ
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/02/04	初版
 *	2005/02/17	ld_DrawObjectで上下左右にはみ出した場合の処理を訂正
 *	2005/02/22	ld_LCDPoint、ld_LCDTransDirectを追加
 *	2005/03/11	16階調画像はlbuffに描いて、lbuff、フィルタ、vbuffの順に
 *				更新フラグに応じて1周期に1画面ずつdbuffに描画するようにしました。
 *				128*88=11,264bytes余計にメモリを食いますが、描き換えが高速です。
 *				ld_Clear→ld_LCDPaint
 *	2005/05/20	ld_DrawObjectでx=8の倍数縛りを解除
 *				ld_DrawObjectでマスク付き描画時にバッファを1ドットずらしてしまっていた不具合を修正
 *				ld_VBuffCopyでvbuffに描いたフォントにグレー線が入る不具合を修正？
 */

#ifndef __LDIRECT_H__
#define __LDIRECT_H__

#include <piece.h>
#include <string.h>

/*
 *	void pceAppInit(void)の先頭で
 *	FRAMCOPY(V_FRAMC, fram_ld_code);
 *	としてflam_ld.cのコードを高速RAMにコピーする。
 *	また、makefileのLDFLAGSに
 *	+codeblock FRAMC {fram_ld.o} \
 *	+bssblock FRAMB {fram_ld.o} \
 *	+addr 0x1000 {@FRAMC FRAMB}
 *	を加えること。		
 */
#define FRAMCOPY(dstsect, srcsect)\
	do {\
		extern unsigned long __START_##dstsect[];\
		extern unsigned long __START_##srcsect[];\
		extern unsigned long __SIZEOF_##srcsect[];\
		memcpy(&__START_##dstsect, &__START_##srcsect, (unsigned)&__SIZEOF_##srcsect);\
	} while(0)

#define PAGE_NUM	5
#define COLOR_MAX	15
#define V_TRANS		0x04
extern BYTE vbuff[DISP_X * DISP_Y];
extern BYTE lbuff[DISP_X * DISP_Y];
extern BYTE dbuff[][DISP_X * DISP_Y >> 2];

/* fram_ld.c */
void ld_LBuffUpdate();
void ld_VBuffClear(int x, int y, int w, int h);
void ld_VBuffUpdate();
//void ld_FilterDark();
void ld_LCDTransDirect();
void ld_DrawObject(const PIECE_BMP *pbmp, int dx, int dy);
void ld_LCDPoint(const BYTE c, const int x, const int y);
void ld_LCDPaint(BYTE c, int x, int y, int w, int h);

/* ld.c */
void Get_PieceBmp(PIECE_BMP *pbmp, BYTE *data);
//void Draw_Object(PIECE_BMP *pbmp, const int dx, const int dy);
//void VBuffReverse();


/*
 *	構造体はPIECE_BMPをそのまま使います。

typedef struct {
	DWORD	head;		//	HEADER			'PBEX'（既存アプリの誤動作防止
	DWORD	fsize;		//	ファイル長
	BYTE	bpp;		//	bit深度			(4)
	BYTE	mask;		//	マスクのbit深度	(1)
	int	w;			//	X幅				8ピクセル単位厳守
	int	h;			//	Y高さ
	DWORD	buf_size;	//	BMPサイズ = (bpp + mask) * w * h / 8
}PBMP_FILEHEADER;

typedef struct{
	PBMP_FILEHEADER	header;
	BYTE			*buf;	//4BIT 1ピクセル
	BYTE			*mask;	//1BIT 1ピクセル
}PIECE_BMP;

 */

#endif	// #ifndef __LDIRECT_H__
