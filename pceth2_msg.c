/*
 *	pceth2 - メッセージ関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/08	main.cから分離
 *	2005/05/27	getNum仕様変更に対応
 *	2005/06/15	名前置換文字を追加（これで全部だといいけど→抜けてたっていうかコピペでミス
 *				!!、!?の禁則処理を修正。手動禁則目的の\nのみの改行は回避
 *	2005/06/16	禁則処理対象に）を追加。命令直前の\nも改行対象に
 *	2005/06/25	名前置換処理をコンバータ側に移動
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"
#include "ld.h"

#include "common.h"
#include "pceth2_msg.h"
#include "pceth2_sys.h"
#include "pceth2_str.h"

//=============================================================================
//	文字描画
//=============================================================================

/*
 *	行頭ならTRUEを返す
 */
BOOL pceth2_isLineTop()
{
	int x;
	FontFuchi_GetPos(&x, NULL);
	return (x <= MSG_X_MIN);
}

/*
 *	ページ先頭ならTRUEを返す
 */
BOOL pceth2_isPageTop()
{
	int y;
	FontFuchi_GetPos(NULL, &y);
	return (pceth2_isLineTop() && (y <= MSG_Y_MIN));
}

/*
 *	ページ先頭に移動する
 */
void pceth2_setPageTop()
{
	while (!pceth2_isPageTop()) {
		pceth2_putCR();
	}
}

/*
 *	メッセージをクリア、バッファもクリア
 */
void pceth2_clearMessage(void)
{
	ld_VBuffClear(0, 0, DISP_X, DISP_Y);
	*play.msg = '\0';
	play.msglen = 0;
}

/*
 *	2バイト文字を描いて、メッセージバッファにも入れる
 */
void pceth2_putKanji(const char *str)
{
	FontFuchi_Printf("%c%c", *str, *(str + 1));
	*(play.msg + play.msglen++)	= *str;
	*(play.msg + play.msglen++)	= *(str + 1);
	*(play.msg + play.msglen)	= '\0';
}

/*
 *	改行して、メッセージバッファにも入れる
 */
void pceth2_putCR(void)
{
	FontFuchi_PutStr("\n");
	*(play.msg + play.msglen++)	= '\n';
	*(play.msg + play.msglen)	= '\0';
}






/*
 *	メッセージ制御処理
 */
int pceth2_procControl(SCRIPT_DATA *s)
{
	s->p++;	// <
	switch (*(s->data + s->p++))
	{
		case 'S':	// オート制御
		case 'F':
		{
			speed = pceth2_getNum(s);
			s->p++;	// >
			return 1;
		}
		break;
		case 'W':	// ウェイト
		{
			wait = pceth2_getNum(s);
			s->p++;	// >
			play.gameMode = GM_TIMEWAIT;
		}
		break;
	}

	return 0;
}

/*
 *	エスケープシーケンス処理
 */
int pceth2_procEscape(SCRIPT_DATA *s)
{
	static const char *lfWords[] = {"　", "「", "『", "（"};
	int i;

	s->p++;
	switch (*(s->data + s->p++))	// 絶対に1個進めるのでここで++しとく
	{
		case 'k':	// キー待ち
		{
			// 次が\nなら改ページにより連続でキー待ちになる可能性があるので先に改行しておく
            if (!strncmp(s->data + s->p, "\\n", 2)) {
				s->p += 2;
				if (!pceth2_isLineTop()) {	// 行頭でなければ改行する
					pceth2_putCR();
					if (pceth2_isPageTop()) {
						play.gameMode = GM_KEYWAIT;
						ld_VBuffUpdate();
						return 0;
					}
				}
			}
			if (!pceth2_isPageTop()) {
				play.gameMode = GM_KEYWAIT;
				ld_VBuffUpdate();
				return 0;
			}
		}
		break;
		case 'n':	// 改行
		{
			if (!pceth2_isPageTop()) {	// 行頭でなければ改行する
				if (pceth2_isKanji(s)) {	// 命令直前なら改行
					for (i = 0; i < array_size(lfWords); i++) {	// 手動禁則処理の改行は無視したい
						if (!strncmp(s->data + s->p, lfWords[i], 2)) {
							pceth2_putCR();
							if (pceth2_isPageTop()) {
								play.gameMode = GM_KEYWAIT;
								ld_VBuffUpdate();
								return 0;
							}
							break;
						}
					}
				}
			}
		}
		break;
		case 'p':	// キー待ちページ送り
		{
			s->p += 4;	// ページ管理IDを飛ばす
			if (!pceth2_isPageTop()) {				// ページ先頭でなければキー待ち改ページ
				pceth2_setPageTop();
				play.gameMode = GM_KEYWAIT;
				ld_VBuffUpdate();
				return 0;
			}
		}
		break;
	}

	return 1;
}


/*
 *	禁則処理（行う必要がある場合1を返す）
 */
int pceth2_jpnHyphenation(const char *str)
{
	static const char *hypWords[] = {"。", "、", "」", "』", "）"};
	int i, x;

	FontFuchi_GetPos(&x, NULL);
	if (x > MSG_X_MAX - (FONT_W + 1)) {
		for (i = 0; i < array_size(hypWords); i++) {
			if (!strncmp(str, hypWords[i], 2)) {
				return 1;
			}
		}
	}

	return 0;
}

/*
 *	P/ECE版独特の改行処理
 */
int pceth2_lineFeed(const char *str)
{
	static const char *start_word[] = {"　", "「", "『"};
	static const char *end_word[] = {"。", "」", "』"};
	int i, j;

	if (play.msglen >= 2 && !pceth2_isLineTop()) {
		for (i = 0; i < array_size(start_word); i++) {
			if (!strncmp(str, start_word[i], 2)) {
				for (j = 0; j < array_size(start_word); j++) {
					if (!strncmp(play.msg + play.msglen - 2, end_word[j], 2)) {
						return 1;
					}
				}
			}
		}
	}

	return 0;
}
