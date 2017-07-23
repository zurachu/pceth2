/*
 *	pceth2 - セーブ・ロード関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/30	開発開始
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"

#include "common.h"
#include "pceth2_sav.h"
#include "pceth2_grp.h"
#include "pceth2_snd.h"
#include "pceth2_cal.h"
#include "pceth2_msg.h"
#include "pceth2_arc.h"

GLOBAL_SAVE_DATA global;
SAVE_DATA play;

static int const s_debug_mode_flag = 0x100; ///< @see common.h SAVE_DATA::gameMode

//=============================================================================
//	グローバルセーブファイル
//=============================================================================

#define GLOBAL_SAVE_FILE_NAME	"pceth2.sav"
/*
 *	グローバルセーブの読み込み
 */
BOOL pceth2_readGlobalSaveData()
{
	memset(&global, 0, sizeof(GLOBAL_SAVE_DATA));
	global.bright = pceLCDSetBright(INVALIDVAL);
	global.masteratt = pceWaveSetMasterAtt(INVALIDVAL);

	if(File_ReadTo((unsigned char*)&global, GLOBAL_SAVE_FILE_NAME) != sizeof(GLOBAL_SAVE_DATA)
		&& pceFileCreate(GLOBAL_SAVE_FILE_NAME, sizeof(GLOBAL_SAVE_DATA)) != 0) {	// なければ作る
		return FALSE;
	}

	if (global.bright < MIN_BRIGHT) {
		global.bright = MIN_BRIGHT;
	}
	pceLCDSetBright(global.bright);
	pceWaveSetMasterAtt(global.masteratt);
	return TRUE;
}

/*
 *	グローバルセーブの書き込み
 */
BOOL pceth2_writeGlobalSaveData()
{
	FILEACC pfa;

	if (pceFileOpen(&pfa, GLOBAL_SAVE_FILE_NAME, FOMD_WR) == 0) {
		pceFileWriteSct(&pfa, &global, 0, sizeof(GLOBAL_SAVE_DATA));
		pceFileClose(&pfa);
		return TRUE;
	}

	return FALSE;
}


//=============================================================================
//	セーブファイル
//=============================================================================

#define FNAMELEN_SAV	12

/*
 *	セーブファイルの読み込み
 *
 *	num	セーブ番号(0-7)
 */
BOOL pceth2_readSaveData(int num)
{
	char buf[FNAMELEN_SAV + 1];

	sprintf(buf, "pceth2_%d.sav", num);
	if(File_ReadTo((unsigned char*)&play, buf) == sizeof(SAVE_DATA)) {
		debug_mode = play.gameMode & s_debug_mode_flag;
		play.gameMode &= ~s_debug_mode_flag;
		return TRUE;
	}
	return FALSE;
}


/*
 *	セーブファイルの書き込み
 *
 *	num	セーブ番号(0-7)
 */
BOOL pceth2_writeSaveData(int num)
{
	char buf[FNAMELEN_SAV + 1];
	FILEACC pfa;

	sprintf(buf, "pceth2_%d.sav", num);
	if (pceFileCreate(buf, sizeof(SAVE_DATA)) == 0) {
		if (pceFileOpen(&pfa, buf, FOMD_WR) == 0) {
			pceFileWriteSct(&pfa, &play, 0, sizeof(SAVE_DATA));
			pceFileClose(&pfa);
			return TRUE;
		}
	}

	return FALSE;
}

//=============================================================================
//	タイトル画面
//=============================================================================

#define TITLE_BG	"B001000.pgx"
#define TITLE_LOGO	"TH2_LOGO.pgx"

static void draw_object(const PIECE_BMP *pbmp, int dx, int dy)
{
	Ldirect_DrawObject(pbmp, dx, dy, 0, 0, pbmp->header.w, pbmp->header.h);
}

/*
 *	タイトル画像準備
 *	（lbuffに描画してすぐに解放します）
 */
static void pceth2_drawTitleGraphic()
{
	PIECE_BMP	p_title;
	BYTE		*_title;

	_title = fpk_getEntryData(TITLE_BG, NULL, NULL);	// 背景
	PieceBmp_Construct(&p_title, _title);
	draw_object(&p_title, 0, 0);
	pceHeapFree(_title);
	_title = NULL;

	_title = fpk_getEntryData(TITLE_LOGO, NULL, NULL);	// タイトル
	PieceBmp_Construct(&p_title, _title);
	draw_object(&p_title, 28, 4);
	pceHeapFree(_title);
	_title = NULL;
}

static int index = 0;

/*
 *	タイトル画面初期化
 */
void pceth2_TitleInit()
{
	pceWaveAbort(SND_CH);
	Stop_PieceMML();

	pceth2_drawTitleGraphic();

	pceth2_writeGlobalSaveData();	// ここでグローバルセーブ保存したら回避できるかにゃ？

	Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);	// 選択肢
	FontFuchi_SetPos(38, 56);
	FontFuchi_PutStr("はじめから");
	FontFuchi_SetPos(38, 68);
	FontFuchi_PutStr("つづきから");
	FontFuchi_Put(28, 56 + index * 12, '>');
	FontFuchi_Put(28 + 65, 56 + index * 12, '<');
	Ldirect_Update();

	play.gameMode = GM_TITLE;
}

/*
 *	タイトルに戻る	Z
 *
 *	*s		スクリプトデータ
 *
 *	return	0（制御を戻す）
 */
int pceth2_backTitle(SCRIPT_DATA *s)
{
	s->p++;
	if(!debug_mode) {	// デバッグ版は何もしない
		pceth2_TitleInit();
	}

	return 0;
}

void pceth2_Title()
{
	BOOL LCDUpdate = FALSE;

	if (pcePadGet() & (TRG_UP | TRG_DN)) {
		index ^= 1;	// 0と1切り替え
		LCDUpdate = TRUE;
	}

	if (LCDUpdate) {
		Ldirect_VBuffClear(28, 56, 8, 24);
		Ldirect_VBuffClear(92, 56, 8, 24);
		FontFuchi_Put(28, 56 + index * 12, '>');
		FontFuchi_Put(28 + 65, 56 + index * 12, '<');
		Ldirect_Update();
	}

	if (pcePadGet() & TRG_A) {	// A
		if (index == 0) {	// はじめから
			pceth2_Init();
		} else {			// つづきから
			pceth2_SaveInit();
		}
	}
}

//=============================================================================
//	セーブロード画面
//=============================================================================

static void pceth2_comeBack(int musplay_flag);

#define SAVE_FILE_NUM	7
int last_gameMode;


static void pceth2_drawSaveMenu()
{
	static char * const date[] = {"日", "月", "火", "水", "木", "金", "土"};
	char buf[FNAMELEN_SAV + 1];
	FILEACC pfa;
	int month, day, i;

	Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
	FontFuchi_SetPos(MSG_X_MIN + 5 * 5, MSG_Y_MIN);
	FontFuchi_PutStr("セーブ・ロード\n");

	for (i = 0; i < SAVE_FILE_NUM; i++) {
		FontFuchi_Printf("%c %d. ",((i == global.save_index)? '>' : ' '), i);
		sprintf(buf, "pceth2_%d.sav", i);
		if (pceFileOpen(&pfa, buf, FOMD_RD) == 0) {
			pceFileReadSct(&pfa, NULL, 0, sizeof(SAVE_DATA));
			if(((SAVE_DATA*)pfa.aptr)->gameMode & s_debug_mode_flag) {
				FontFuchi_PutStr(((SAVE_DATA*)pfa.aptr)->scData.name);
			} else {
				month = ((SAVE_DATA*)pfa.aptr)->flag[0];
				day = ((SAVE_DATA*)pfa.aptr)->flag[1];
				FontFuchi_Printf("%1d月%2d日 %s曜日", month, day, date[pceth2_getDate(month, day)]);
			}
			pceFileClose(&pfa);
		}
		FontFuchi_PutStr("\n");
	}
	Ldirect_Update();
}

void pceth2_SaveInit()
{
	pceWaveAbort(SND_CH);

	last_gameMode = play.gameMode;
	play.gameMode = GM_SAVE;

	pceth2_drawTitleGraphic();
	pceth2_drawSaveMenu();
}

#define LOAD	0
#define SAVE	1

void pceth2_SaveMenu()
{
	static int phase = 0;
	static int mode = LOAD;
	BOOL LCDUpdate = FALSE;

	if (phase == 0)
	{
		if (pcePadGet() & (TRG_UP)) {
			global.save_index = (global.save_index - 1 + SAVE_FILE_NUM) % SAVE_FILE_NUM;
			LCDUpdate = TRUE;
		}
		if (pcePadGet() & (TRG_DN)) {
			global.save_index = (global.save_index + 1) % SAVE_FILE_NUM;
			LCDUpdate = TRUE;
		}
	} else {
		if (pcePadGet() & (TRG_UP | TRG_DN)) {
			mode ^= 1;	// 0と1切り替え
			LCDUpdate = TRUE;
		}
	}

	if (LCDUpdate) {
		Ldirect_VBuffClear(0, 0, MSG_X_MIN + FONT_W / 2 + 1, DISP_Y);
		FontFuchi_Put(MSG_X_MIN, MSG_Y_MIN + global.save_index * FONT_H + FONT_H, '>');
		if (phase == 1) {
			pceLCDPaint(3, 42, 32, 12, 24);
			pceFontSetType(0);
			pceFontSetTxColor(0);
			pceFontSetBkColor(FC_SPRITE);
			pceFontPut(44, 34 + mode * 10, '>');
		}
		Ldirect_Update();
	}

	if (pcePadGet() & TRG_A) {	// A
		if ((phase == 0 && last_gameMode == GM_TITLE) || (phase == 1 && mode == LOAD)) {
			if (pceth2_readSaveData(global.save_index)) {	// ロード
				pceth2_comeBack(1);
				phase = 0;
			}
		} else if (phase == 1) {	// セーブ
			play.gameMode = last_gameMode;	// セーブ用に一瞬戻す
			if(debug_mode) {
				play.gameMode |= s_debug_mode_flag;
			}
			pceth2_writeSaveData(global.save_index);
			play.gameMode = GM_SAVE;
			pceth2_drawSaveMenu();
			phase = 0;
		} else {	// ロード・セーブ選択へ
/* ウィンドウを書く */
			pceLCDPaint(3, 42, 32, 44, 24);
			pceFontSetType(0);
			pceFontSetTxColor(0);
			pceFontSetBkColor(FC_SPRITE);
			pceFontSetPos(54, 34);
			pceFontPutStr("ロード");
			pceFontSetPos(54, 44);
			pceFontPutStr("セーブ");
			pceFontPut(44, 34 + mode * 10, '>');
			Ldirect_Update();
			phase = 1;
		}

	} else if (pcePadGet() & TRG_B) {	// B
		if (phase == 0) {
			if (last_gameMode == GM_TITLE) {
				pceth2_TitleInit();
			} else {
			play.gameMode = last_gameMode;
			pceth2_comeBack(0);
			}
		} else {
			pceth2_drawSaveMenu();
			phase = 0;
		}
	}
}

/*
 *	セーブデータから各種状態復帰
 *
 *	replay_flag	0なら画像、音楽を再生し直さない（セーブメニューから復帰しただけの時）
 */
static void pceth2_comeBack(int replay_flag)
{
	char	buf[16];	// ファイル名退避用
	int		i;

	// クリアフラグをグローバルと同期する
	play.flag[80] = 0;
	for (i = 1; i < GLOBAL_FLAG_NUM; i++) {
		play.flag[80 + i] |= global.flag[i];
		play.flag[80] += play.flag[80 + i];
	}

	if (replay_flag) {
		for (i = 0; i < GRP_NUM; i++) {
			strcpy(buf, play.pgxname[i]);
			pceth2_clearGraphic(i);
			pceth2_loadGraphic(buf, i);
		}

		strcpy(buf, play.pmdname);
		Stop_PieceMML();
		Play_PieceMML(buf);	// BGM再生
	}
	pceth2_DrawGraphic();	// 画像描画

	play.evData.data = fpk_getEntryData(play.evData.name, &play.evData.size, NULL);	// EV
	play.scData.data = fpk_getEntryData(play.scData.name, &play.scData.size, NULL);	// スクリプト

	pceth2_comeBackMessage();
}

