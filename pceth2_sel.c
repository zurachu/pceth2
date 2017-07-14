/*
 *	pceth2 - 選択肢、マップ選択関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/14	開発開始
 *	2005/06/19	選択肢追加終了時に行頭なら改行しない
 *	2005/06/24	pceth2_map.cと統合
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"

#include "common.h"
#include "pceth2_sel.h"
#include "pceth2_sys.h"
#include "pceth2_msg.h"
#include "pceth2_grp.h"
#include "pceth2_snd.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"

static void pceth2_loadMapChipChara();

//=============================================================================
//	共通部分
//=============================================================================

/*
 *	選択の矢印を描画
 */
void pceth2_drawSelArrow()
{
	Ldirect_VBuffClear(0, 0, MSG_X_MIN + FONT_W / 2 + 1, DISP_Y);
	FontFuchi_Put(MSG_X_MIN, play.selY[play.selIndex], '>');
}

/*
 *	選択の実体
 *
 *	amount	選択肢数
 *
 *	return	-1=未選択／-1以外=選択値
 */
#define NO_SELECT	-1

int pceth2_SelectEx(int amount)
{
	static BOOL LCDUpdate = FALSE;

	if (pcePadGet() & TRG_UP) {	// ↑
		play.selIndex = (play.selIndex + amount - 1) % amount;
		LCDUpdate = TRUE;
	}
	if (pcePadGet() & TRG_DN) {	// ↓
		play.selIndex = (play.selIndex + 1) % amount;
		LCDUpdate = TRUE;
	}

	if (LCDUpdate) {
		if (play.gameMode == GM_MAPSELECT) {	// マップ選択は
			pceth2_loadMapChipChara();			// チップキャラも描き替え
		}
		pceth2_drawSelArrow();	// 矢印
		Ldirect_Update();
		LCDUpdate = FALSE;
	}

	if (pcePadGet() & TRG_A) {	// A
		pceth2_setPageTop();
		pceth2_clearMessage();
		Ldirect_Update();

		return play.selIndex;
	}

	return NO_SELECT;
}


//=============================================================================
//	選択
//=============================================================================

/*
 *	選択肢を追加登録	q[str]（スペースで終端）
 */
int pceth2_addSelItem(SCRIPT_DATA *s)
{
	s->p++;

	FontFuchi_GetPos(NULL, &play.selY[play.selAmount]);	// y座標を記憶

	while (*(s->data + s->p) != ' ') {	// 選択肢を描画
		if (strncmp(s->data + s->p, "　", 2) && strncmp(s->data + s->p, "\\n", 2)) {	// P/ECEではズレてしまうので無視
			if (pceth2_isLineTop()) {	// 行頭なら1文字あける
				pceth2_putKanji("　");
			}
			pceth2_putKanji(s->data + s->p);
		}
		s->p += 2;
	}
	if (!pceth2_isLineTop()) {	// 行頭でなければ改行
		pceth2_putCR();
	}
	s->p++;

	play.selAmount++;
	return 1;
}

/*
 *	選択を初期化	Q0
 */
int pceth2_initSelect(SCRIPT_DATA *s)
{
	s->p++;
	play.selReg = (int)(*(s->data + s->p++) - '0');	// 結果格納レジスタ番号を取得
	play.gameMode = GM_SELECT;

	pceth2_drawSelArrow();	//	矢印

	return 0;
}

/*
 *	選択
 */
void pceth2_Select()
{
	if (pceth2_SelectEx(play.selAmount) != NO_SELECT) {
		reg[play.selReg] = play.selIndex;
		play.selIndex = play.selAmount = 0;
		play.gameMode = GM_SCRIPT;
	}
}

//=============================================================================
//	マップ選択
//=============================================================================

#define CH_NOTHING	0
#define LM_MYHOME	0

/*
 *	チップキャラを読み込んで画面を再描画
 */
#define FNAMELEN_CHIP	13

static void pceth2_loadMapChipChara()
{
	char buf[FNAMELEN_CHIP + 1];

	if (play.lm[play.selIndex].chip == CH_NOTHING) {	// チップキャラなし
		pceth2_clearGraphic(GRP_R);
	} else {
		pcesprintf(buf, "CHIP%03d01.pgx", play.lm[play.selIndex].chip);
		pceth2_loadGraphic(buf, GRP_R);
	}

	pceth2_DrawGraphic();	// 描き直す
}

/*
 *	選択肢を描画
 */
static void pceth2_putMapItem()
{
	static const char *landName[] = {
		"自宅",		"商店街",	"ゲームセンター",	"公園",		"中学校",	"坂道",		"校門前",
		"校庭",		"駐輪場",	"裏庭",				"下駄箱",	"体育館",	"書庫",		"図書室",
		"視聴覚室",	"１階廊下",	"２階廊下",			"１階教室",	"２階教室",	"３階廊下",	"３階教室",
	};
	int i;

	pceth2_setPageTop();
	pceth2_clearMessage();
	for (i = 0; i < play.lmAmount; i++) {
		pceth2_putKanji("　");
		FontFuchi_GetPos(NULL, &play.selY[i]);	// y座標を記憶
		FontFuchi_Printf("%s\n", landName[play.lm[i].land]);
		play.msglen += pcesprintf(play.msg + play.msglen, "%s\n", landName[play.lm[i].land]);
	}
	Ldirect_Update();
}

/*
 *	ランドマーク追加登録の実体
 *
 *	land	ランドマーク番号
 *	chip	チップキャラ番号
 *	*fName	ジャンプ先スクリプトファイル名
 */
void pceth2_addMapItemEx(const int land, const int chip, const char *fName)
{
	play.lm[play.lmAmount].land = land;
	play.lm[play.lmAmount].chip = chip;
	strcpy(play.lm[play.lmAmount].scp, fName);
	play.lmAmount++;
}

/*
 *	ランドマークを追加登録	m?,?,000000000.scp
 */
int pceth2_addMapItem(SCRIPT_DATA *s)
{
	int l, c;
	char buf[FNAMELEN_SCP + 1];

	s->p++;	// m
	l = pceth2_getNum(s);	// ランドマーク番号
	s->p++;	// ,
	c = pceth2_getNum(s);	// チップキャラ番号
	s->p++;	// ,
	pceth2_strcpy(buf, s, FNAMELEN_SCP);	// ジャンプ先

	pceth2_addMapItemEx(l, c, buf);
	return 1;
}

#define BG_MAPSELECT	"MAP_BG.pgx"
#define BGM_MAPSELECT	"MAP.pmd"

/*
 *	マップ選択を初期化
 *	EV_????AFTER_SCHOOLを読み終わった時に呼び出されます
 *	return ランドマーク登録数（0なら失敗）
 */
int pceth2_initMapSelect()
{
	if (play.lmAmount > 0) {	// ランドマークが登録されている
		if (pceth2_isCalenderMode()) {	// カレンダーモード時
			pceth2_clearGraphic(GRP_C);	// カレンダー画像消去
//			calFlag = 0;	// カレンダーモード終了
		}

		// 自宅を追加
		pceth2_addMapItemEx(LM_MYHOME, CH_NOTHING, "");

		pceth2_loadGraphic(BG_MAPSELECT, GRP_BG);	// 背景
		pceth2_loadMapChipChara();					// チップキャラ
		pceth2_putMapItem();						// 選択肢
		pceth2_drawSelArrow();						// 矢印
		Play_PieceMML(BGM_MAPSELECT);				// BGM

		play.gameMode = GM_MAPSELECT;
	}

	return play.lmAmount;
}




/*
 *	マップ選択
 */
void pceth2_MapSelect()
{
	if (pceth2_SelectEx(play.lmAmount) != NO_SELECT) {
		if (play.lm[play.selIndex].land == LM_MYHOME) {	// 自宅
			pceth2_loadEVScript();
		} else {
			pceth2_loadScript(&play.scData, play.lm[play.selIndex].scp);
			play.gameMode = GM_SCRIPT;
		}
		pceth2_clearGraphic(GRP_R);
		Stop_PieceMML();
		play.selIndex = play.lmAmount = 0;
	}
}
