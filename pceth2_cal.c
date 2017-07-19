/*
 *	pceth2 - カレンダー関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/13	開発開始
 *	2005/05/31	やっと実装
 *	2005/06/11	日付を進めるのも初期化で行う
 *				連続丸の処理で、毎日画像を読み込まないようにし、月初めは画像を読み直し、溜まった丸を消す
 *	2005/07/19	pceth2_isCalenderMode()を追加
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"

#include "common.h"
#include "pceth2_cal.h"
#include "pceth2_sys.h"
#include "pceth2_grp.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"

static PIECE_BMP	pcircle;
static BYTE		*circle;
static int			circleAnim;

#define MONTH_INDEX(x)	((x) - START_MONTH)

#define FNAMELEN_CAL	10
#define BG_CALENDER		"CAL_BG.pgx"
#define CIRCLE_CALENDER	"CAL_CIRCLE.pgd"

/*
 *	スクリプト中でカレンダーモードに移行	D4,16
 *
 *	*s		スクリプトデータ
 *
 *	return	0（制御を戻す）
 */
int pceth2_calenderInitEx(SCRIPT_DATA *s)
{
	s->p++;	// D
	MONTH = pceth2_getNum(s);	// 月
	s->p++;	// ,
	DAY = pceth2_getNum(s);	// 日
	pceth2_calenderInit();

	return 0;
}

/*
 *	カレンダー表示を初期化
 */
void pceth2_calenderInit()
{
	static const int day_num[] = {31, 30, 31};	// MONTH_INDEX(x)月の日数
	char buf[FNAMELEN_CAL + 1];

	if (DAY > day_num[MONTH_INDEX(MONTH)]) {
		MONTH++;
		DAY = 1;
	}

	if (!pceth2_isCalenderMode() || DAY == 1) {
		pceth2_loadGraphic(BG_CALENDER, GRP_BG);	// 背景
		// カレンダー画像ファイル名を作成して読み込む
		pcesprintf(buf, "CAL_%02d.pgx", MONTH);
		pceth2_loadGraphic(buf, GRP_C);
		pceth2_DrawGraphic();

		Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
	}
	circle = fpk_getEntryData(CIRCLE_CALENDER, NULL, NULL);	// 丸
	PieceBmp_Construct(&pcircle, circle);
	// ↑毎日解放されてるので、毎日確保しないといけなかった。if{}から出しました。
	// 　速度が遅いからメモリ残量が問題なければ常に置いときたいけど…。
	circleAnim = 0;

	play.gameMode = GM_CALENDER;
}

#define CIRCLE_L	7
#define CIRCLE_T	22
#define CIRCLE_W	16
#define CIRCLE_H	12
#define ANIMATION_PROC	1

static const int zero_date[] = {0, 3, 5};	// MONTH_INDEX(x)月0日の曜日

/*
 *	何曜日か返す
 */
int pceth2_getDate(int month, int day)
{
	return (day + zero_date[MONTH_INDEX(month)]) % 7;
}

/*
 *	カレンダーの丸を描く
 */
void pceth2_calenderDrawCircle()
{
	DRAW_OBJECT d;

	pceLCDSetObject(&d, &pcircle, \
					pceth2_getDate(MONTH, DAY) * CIRCLE_W + CIRCLE_L, \
					(DAY + zero_date[MONTH_INDEX(MONTH)]) / 7 * CIRCLE_H + CIRCLE_T, \
					0, (circleAnim / ANIMATION_PROC) * 16, 16, 16, DRW_CLR(COLOR_GLAY_B, COLOR_MASK));
	pceLCDDrawObject(d);
	Ldirect_Update();

	if (++circleAnim >= 16 * ANIMATION_PROC) {
		pceHeapFree(circle);
		circle = NULL;
		if (play.scData.size) {
			play.gameMode = GM_KEYWAIT;	// SCRIPT中のカレンダー命令に対応
		} else {
			pceth2_loadEVScript();	// 次のEVスクリプトを読む
		}
	}
}
