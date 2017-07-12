/*
 *	pceth2 - 文字列処理関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/07/23	pceth2_sys.c, common.hから分離
 */

#include <string.h>
#include <piece.h>

#include "common.h"
#include "pceth2_str.h"

/*
 *	2バイト文字かどうか
 *	*s		スクリプトデータ
 *	return	TRUE/FALSE
 */
#define isKanji(x)	(((x) >= 0x81 && (x) <= 0x9f) || ((x) >= 0xe0 && (x) <= 0xfc))

BOOL pceth2_isKanji(SCRIPT_DATA *s)
{
	return(isKanji(*(s->data + s->p)));
}

/*
 *	数値を取得
 *	*s		スクリプトデータ
 *	return	取得した数値
 */
#define isDigit(x)	((x) >= '0' && (x) <= '9')

int pceth2_getNum(SCRIPT_DATA *s)
{
	int ret = 0;

	for(; isDigit(*(s->data + s->p)); s->p++) {
		ret *= 10;
		ret += *(s->data + s->p) - '0';
	}

	return ret;
}

/*
 *	スクリプトデータから文字列をコピー
 *
 *	*dst	コピー先のポインタ
 *	*s		スクリプトデータ
 *	len		コピー文字列長
 */
void pceth2_strcpy(char *dst, SCRIPT_DATA *s, DWORD len)
{
	strncpy(dst, s->data + s->p, len);
	*(dst + len) = '\0';
	s->p += len;
}
