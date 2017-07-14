/*
 *	pceth2 - 画像表示関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/04/27	main.cから分離
 *	2005/05/10	読み込み／解放が他のソースからも扱えるように
 *	2005/05/11	表示位置自動決定を追加
 *	2005/05/27	Win側pceth2bin2に対応（スクリプト形式を多少変更
 *	2005/05/30	立ち絵表示位置AUTOを廃止（pceth2bin2側で対応
 *	2005/06/17	立ち絵表示、消去の即時書き換えフラグを消去
 *				立ち絵表示位置を左中右の3種類に減らす
 *	2005/06/18	立ち絵表示、消去にスライドイン、アウトを追加
 *	2005/06/23	背景画像の桜を時期で書き換え（デバッグ版は無理）
 *	2005/06/30	スライドアウトした後、スライドx座標を戻していなかったので、
 *				3/9マップ選択（このみの「てりゃ！」後）でチップキャラ表示されなかったのを修正
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"

#include "common.h"
#include "pceth2_grp.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"

extern SAVE_DATA play;

static PIECE_BMP	pbmp[GRP_NUM];
static BYTE			pgx[GRP_NUM][DISP_X * DISP_Y * 5 / 8];

// スライド管理用
static int	slide_x[GRP_BG];	// 
static int	slide_goal;			// 目標座標
static int	slide_pos;			// スライドしている画像のセット位置（LCR）

//=============================================================================
//	画像描画
//=============================================================================

#define POS_L	30
#define POS_C	64
#define POS_R	98

static void draw_object(const PIECE_BMP *pbmp, int dx, int dy)
{
	Ldirect_DrawObject(pbmp, dx, dy, 0, 0, pbmp->header.w, pbmp->header.h);
}

/*
 *	lbuffに画像描画
 */
void pceth2_DrawGraphic()
{
//	static int pos_table[] = {30, 64, 98};	// 立ち絵表示中心位置テーブル

	if (*play.pgxname[GRP_BG]) {	// 背景
		draw_object(&pbmp[GRP_BG], 0, 0);
	} else {
		Ldirect_Paint(15, 0, 0, DISP_X, DISP_Y);	// 黒背景
	}

	if (*play.pgxname[GRP_R]) {	// 右
		draw_object(&pbmp[GRP_R], (POS_R - pbmp[GRP_R].header.w / 2) + slide_x[GRP_R], DISP_Y - pbmp[GRP_R].header.h);
	}
	if (*play.pgxname[GRP_L]) {	// 左
		draw_object(&pbmp[GRP_L], (POS_L - pbmp[GRP_L].header.w / 2) + slide_x[GRP_L], DISP_Y - pbmp[GRP_L].header.h);
	}
	if (*play.pgxname[GRP_C]) {	// 中央
		draw_object(&pbmp[GRP_C], (POS_C - pbmp[GRP_C].header.w / 2) + slide_x[GRP_C], DISP_Y - pbmp[GRP_C].header.h);
	}
}

//=============================================================================
//	画像ファイル読み込み
//=============================================================================

/*
 *	特定位置に画像を読み込み
 *
 *	*fName	画像ファイル名
 *	pos		表示位置
 */
void pceth2_loadGraphic(const char *fName, const int pos)
{
	pceth2_clearGraphic(pos);

	strcpy(play.pgxname[pos], fName);
	/*pgx[pos] =*/ fpk_getEntryData(play.pgxname[pos], NULL, pgx[pos]);

	if (pgx[pos] != NULL) {
		PieceBmp_Construct(&pbmp[pos], pgx[pos]);
	} else {
		*play.pgxname[pos] = '\0';
	}
}

/*
 *	特定位置の画像を消去
 *
 *	pos	表示位置
 */
void pceth2_clearGraphic(const int pos)
{
	*play.pgxname[pos] = '\0';
//	pceHeapFree(pgx[pos]);
//	pgx[pos] = NULL;
	memset(pbmp[pos], 0, sizeof(PIECE_BMP));
}

//=============================================================================
//	画像表示・消去
//=============================================================================

/*
 *	背景画像の時間帯、天候を指定	G0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_setBGOption(SCRIPT_DATA *s)
{
	s->p++;
	if (*(s->data + s->p) < '0' + BG_WEATHER_FLAG) {	// 時間帯
		BG_TIME		= *(s->data + s->p++);
	} else {						// 天候
		BG_WEATHER	= *(s->data + s->p++) - BG_WEATHER_FLAG;
	}

	return 1;
}

/*
 *	背景画像、イベント画像を表示	B000000.pgx,0	V000000.pgx,0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_loadBG(SCRIPT_DATA *s)
{	// 桜背景画像変更テーブル
	// 2005/07/19	回想などで直接画像番号を指定するケースもあるみたいなので、
	//				使われていない画像番号にしてみる
	static const char *cherry[][6] = {
		{"88", "78", "03", "01", "02", "04"},	// 校門（外）
		{"89", "79", "07", "05", "06", "08"},	// 校門（内）
		{"90", "80", "36", "34", "35", "37"},	// 橋
		{"91", "81", "50", "48", "49", "51"},	// 川沿いの道
	};	//	基本, つぼみ, 五分咲き, 満開, 五分散り, 葉桜
	char buf[FNAMELEN_BG + 1];
	int i;

	// ファイル名をバッファにコピー
	pceth2_strcpy(buf, s, FNAMELEN_BG);

	if (*buf == 'B') {	// 背景画像
		for (i = 0; i < array_size(cherry); i++) {	// 桜の咲き具合を変更
			if (!strncmp(buf + 2, cherry[i][0], 2)) {
				if (MONTH == 5 || (MONTH == 4 && DAY >= 28)) {	// 葉桜（4/28～）
					strncpy(buf + 2, cherry[i][5], 2);
				} else if (MONTH == 4 && DAY >= 16) {			// 五分散り（4/16～）
					strncpy(buf + 2, cherry[i][4], 2);
				} else if (MONTH == 4 || DAY >= 29) {			// 満開（3/29～）
					strncpy(buf + 2, cherry[i][3], 2);
				} else if (DAY >= 16) {							// 五分咲き（3/16～）
					strncpy(buf + 2, cherry[i][2], 2);
				} else {										// つぼみ（3/1～）
					strncpy(buf + 2, cherry[i][1], 2);
				}
			}
		}
		strncpy(buf + 4, play.bgopt, 2);	// 時間帯、天候を反映
	}

	s->p++;	// ,
	if (!(*(s->data + s->p++) - '0')) {	// フラグ0の場合全画像消去
		for (i = GRP_L; i <= GRP_R; i++) {
			pceth2_clearGraphic(i);
		}
	}
	pceth2_loadGraphic(buf, GRP_BG);	// 画像を開く
	pceth2_DrawGraphic();				// 必ず画像描画

	return 1;
}

/*
 *	立ち絵画像を表示	C00000000.pgx,0	CLOCK00.pgx,0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_loadChara(SCRIPT_DATA *s)
{
	char buf[FNAMELEN_CH + 1];
	int pos, i = 0;

	// ファイル名をバッファにコピー
	if (*(s->data + s->p + 1) == 'L') {	// 時計
		pceth2_strcpy(buf, s, FNAMELEN_CLOCK);
	} else {
		pceth2_strcpy(buf, s, FNAMELEN_CH);
	}

	s->p++;	// ,
	pos = (int)(*(s->data + s->p++) - '0');	// 表示位置を取得
	for (i = GRP_L; i <= GRP_R; i++) {
		if (!strncmp(buf, play.pgxname[i], 1 + NUMLEN_CH)) {
			pceth2_clearGraphic(i);	// 同じキャラが既に読み込まれていたら消去
			if (pos >= GRP_AUTO) {	// 自動位置決定（既に読み込まれている位置に）
				pos = i;
			}
		}
	}
	if (pos >= GRP_AUTO) {	// 自動位置決定できなかったので既定の位置に
		pos -= GRP_AUTO;
	}

	pceth2_loadGraphic(buf, pos);	// 画像を開く

	slide_x[pos] = 0;
	if (*(s->data + s->p) == ',') {	// スライド用引数があればスライド処理
		s->p++;	// ,
		i = (int)(*(s->data + s->p++) - '0');
		slide_pos = pos;
		slide_x[pos] = ((i == 1)? -128 : 128);
		slide_goal = 0;
		wait = 8;	// フレーム速度は無視
		play.gameMode = GM_SLIDECHR;
		return 0;
	}

	if (*(s->data + s->p) != 'B' && *(s->data + s->p) != 'C' && *(s->data + s->p) != 'c') {
		pceth2_DrawGraphic();	// 次が画像系命令でなければすぐに描く
	}
	return 1;
}

/*
 *	特定キャラの立ち絵画像を消去	c000
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_clearChara(SCRIPT_DATA *s)
{
	int i;
	s->p++;	// c

	// 画像ファイル名とキャラ番号が一致したら消去
	for (i = GRP_L; i <= GRP_R; i++) {
		if (*play.pgxname[i] && !strncmp(s->data + s->p, play.pgxname[i] + 1, NUMLEN_CH)) {
			break;
		}
	}
	s->p += NUMLEN_CH;

	if (i < GRP_BG) {	// 削除対象に一致？
		if (*(s->data + s->p) == ',') {	// スライドアウト処理
			s->p++;
			slide_pos = i;
			// slide_x[slide_pos]は既に1のはず
			i = (int)(*(s->data + s->p++) - '0');
			slide_goal = ((i == 1)? -128 : 128);
			wait = 8;	// とりあえずフレーム速度は無視
			play.gameMode = GM_SLIDECHR;
			return 0;
		}

		pceth2_clearGraphic(i);
	}

	if (*(s->data + s->p) != 'B' && *(s->data + s->p) != 'C' && *(s->data + s->p) != 'c') {
		pceth2_DrawGraphic();	// 次が画像系命令でなければすぐに描く
	}
	return 1;
}

//=============================================================================
//	立ち絵スライド
//=============================================================================

/*
 *	立ち絵のスライド処理
 */
void pceth2_slideChara()
{
	slide_x[slide_pos] += (slide_goal - slide_x[slide_pos]) / wait;

	if (--wait <= 0) {
		slide_x[slide_pos] = slide_goal;
		if (slide_x[slide_pos] != 0) {	// 画面外なら消します
			slide_x[slide_pos] = 0;	// センターに戻しておく（マップ選択対策）
			pceth2_clearGraphic(slide_pos);
		}
		play.gameMode = GM_SCRIPT;
	}

	pceth2_DrawGraphic();
}


/*
 *	今カレンダーモードかどうか返す（度々調べてるので関数化）
 */
int pceth2_isCalenderMode()
{
	return (*(play.pgxname[GRP_BG] + 2) == 'L');
}
