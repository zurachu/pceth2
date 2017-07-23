#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define array_size(a)	(sizeof(a)/sizeof(a[0]))
#define isKanji(x)	(((x) >= 0x81 && (x) <= 0x9f) || ((x) >= 0xe0 && (x) <= 0xfc))

/*
 *	数値を取得
 *	戻り値	ポインタを進める数(6)
 */
int getNumber(const BYTE *ptr, int *num)
{
	if (*ptr != 0x09) return 0;
	*num = (int)(*(float*)(ptr + 2));

	// 020003800.BIN, 070000300.BIN用の対策（場当たり的
	if (*(ptr + 6) == 0x02 && *(ptr + 7) == 0x02 && *(ptr + 8) == 0x09) {
		return 7;
	}

	return 6;
}

/*
 *	レジスタインデックスを取得
 *	戻り値	ポインタを進める数(2)
 */
int getRegIndex(const BYTE *ptr, int *num)
{
	if (*ptr != 0x06)	return 0;
	*num = (int)(*(ptr + 1));

	return 2;
}

/*
 *	演算子を取得
 *	戻り値	ポインタを進める数(1)
 */
int getOperator(const BYTE *ptr, char *ope)
{
	switch (*ptr) {
	case 0x17:	strcpy(ope, "＋");	break;
	case 0x18:	strcpy(ope, "－");	break;
	case 0x19:	strcpy(ope, "×");	break;
	case 0x1A:	strcpy(ope, "÷");	break;
	case 0x2A:	strcpy(ope, "＝");	break;
	case 0x2B:	strcpy(ope, "≠");	break;
	case 0x2C:	strcpy(ope, "＜");	break;
	case 0x2D:	strcpy(ope, "≦");	break;
	case 0x2E:	strcpy(ope, "＞");	break;
	case 0x2F:	strcpy(ope, "≧");	break;
	default:	return 0;
	}

	return 1;
}

/*
 *	逆ポーランド記法の計算式を求める
 *	（レジスタが絡むものは求めようがないので諦める）
 *	戻り値	ポインタを進める数
 */
int calcRevPolish(const BYTE *ptr, int *num)
{
	int buf[8], i = 0, ret = 0;

	while(*(ptr + ret) != 0x02) {
		switch (*(ptr + ret)) {
		case 0x09:
			ret += getNumber(ptr + ret, &buf[i]);	i++;	break;
		case 0x17:
			ret++;	i--;	buf[i-1] += buf[i];	break;
		case 0x18:
			ret++;	i--;
			if (i >= 1) { buf[i-1] -= buf[i]; }
			else { buf[i] = -buf[i]; } break;
		case 0x19:
			ret++;	i--;	buf[i-1] *= buf[i];	break;
		case 0x1A:
			ret++;	i--;	buf[i-1] /= buf[i];	break;
		case 0x2A:
			ret++;	i--;	buf[i-1] = (buf[i-1] == buf[i]);	break;
		case 0x2B:
			ret++;	i--;	buf[i-1] = (buf[i-1] != buf[i]);	break;
		case 0x2C:
			ret++;	i--;	buf[i-1] = (buf[i-1] <  buf[i]);	break;
		case 0x2D:
			ret++;	i--;	buf[i-1] = (buf[i-1] <= buf[i]);	break;
		case 0x2E:
			ret++;	i--;	buf[i-1] = (buf[i-1] >  buf[i]);	break;
		case 0x2F:
			ret++;	i--;	buf[i-1] = (buf[i-1] >= buf[i]);	break;
		default:
			return 0;
		}
		continue;
	}

	if (ret == 0) {
		return 0;
	}
	*num = buf[0];
	return ret;
}

/*
 *	逆ポーランド記法の計算式をそのまま出力する
 *	戻り値	ポインタを進める数
 */
int getRevPolish(const BYTE *ptr, char *str)
{
	int num, len, ret = 0;
	char buf[10];
	*str = '\0';

	while(*(ptr + ret) != 0x02) {
		if (len = getNumber(ptr + ret, &num)) {
			sprintf(buf, " %d", num);
			strcat(str, buf);
			ret += len;
		} else if (len = getRegIndex(ptr + ret, &num)) {
			sprintf(buf, "$%d", num);
			strcat(str, buf);
			ret += len;
		} else if (len = getOperator(ptr + ret, buf)) {
			strcat(str, buf);
			ret += len;
		} else {
			return 0;
		}
	}

	return ret;
}

/*
 *	文字列を取得
 *	戻り値	ポインタを進める数
 */
int getString(const BYTE *ptr, char *str)
{
	if (*ptr != 0x0A) return 0;

	short len = *((short*)(ptr + 1));

	int i;
	for (i = 0; i < len; i++) {
		if (*(ptr + 3 + i) == 0xF0) {	// 特殊文字
			switch (*(ptr + 3 + i + 1)) {
			case 0x40:	memcpy(str + i, "!?", 2);	break;
			case 0x41:	memcpy(str + i, "!!", 2);	break;
			case 0x42:	// ハートマーク：スクリプタで対応するのでそのまま出力
				memcpy(str + i, ptr + 3 + i, 2);
				*(str + i + 2) = '\0';
				break;
			case 0x46:
			case 0x47:
                memcpy(str + i, "～", 2);
				break;
			}
			i++;
		} else if (isKanji(*(ptr + 3 + i))) {
			memcpy(str + i, ptr + 3 + i, 2);
			i++;
		} else {
			*(str + i) = *(ptr + 3 + i);
		}
	}
	*(str + i) = '\0';

	return len + 3;	// 進めるバイト数
}

/*
 *	ラベルを取得
 *	戻り値	ポインタを進める数
 */
int getLabel(const BYTE *ptr, char *str)
{
	if (*ptr != 0x05) return 0;

	char len = *(ptr + 1);
	memcpy(str, ptr + 2, len);
	*(str + len) = '\0';

	return len + 2;
}

/*
 *	必要ない（訳じゃないけど）引数をスキップ
 */
int skipScript(const BYTE *ptr)
{
	int ret = 0, len, num;
	char str[100];

	while(*(ptr + ret) != 0x02) {	// コンマ
		if (len = getString(ptr + ret, str)) {
			ret += len + 1;
		} else if (len = getLabel(ptr + ret, str)) {
			ret += len + 1;
		} else if (len = calcRevPolish(ptr + ret, &num)) {
			ret += len + 1;
		} else if (len = getRevPolish(ptr + ret, str)) {
			ret += len + 1;
		}
	}

//	if (*(ptr + ret + 1) == 0x02) { ret++; }

	return ret;
}

/*
 *	BGの桜の咲き具合を統一（P/ECE側で再変換する）
 */
int convertBGNum(int num)
{
	switch (num) {
	case  1:	case  2:	case  3:	case  4:	case 78:	num = 88;	break;	// 校門前
	case  5:	case  6:	case  7:	case  8:	case 79:	num = 89;	break;	// 校門内
	case 34:	case 35:	case 36:	case 37:	case 80:	num = 90;	break;	// 橋の上
	case 48:	case 49:	case 50:	case 51:	case 81:	num = 91;	break;	// 川沿い
	}

	return num;
}

int manaka_count; // 「小牧」「愛佳」読み替え数

/*
 *	名前置換文字を名前に置換
 */
void replaceName(char *str)
{
	/* 置換テーブル（前方一致なので置換元が長いものから書くこと） */
	static const struct {
		char *src;	// 置換元
		char *dst;	// 置換先
	} name_table[] = {
						{"*nlk1",	"こ"},			// 姓（ひらがな）1文字目
						{"*nlk",	"こうの"},		// 姓（ひらがな）
						{"*nl1",	"河"},			// 姓1文字目
						{"*nl",		"河野"},		// 姓
						{"*nfk1",	"た"},			// 名（ひらがな）1文字目
						{"*nfk2",	"か"},			// 名（ひらがな）2文字目
						{"*nfk3",	"あ"},			// 名（ひらがな）3文字目
						{"*nfk4",	"き"},			// 名（ひらがな）4文字目
						{"*nfk",	"たかあき"},	// 名（ひらがな）
						{"*nf",		"貴明"},		// 名
						{"*nnk1",	"タ"},			// ニックネーム（カタカナ）1文字目
						{"*nnk",	"タカ"},		// ニックネーム（カタカナ）
						{"*nn1",	"た"},			// 　　　　　　（ひらがな）1文字目
						{"*nn",		"たか"},		// 　　　　　　（ひらがな）
	};
	char tmp[1000];
	*tmp = '\0';

	// 1文字だけゴミが入ってたら除去
	if (*(str+1) == '\0') {
		if (*str != '<' && *str != '>') {
			*str = '\0';
		}
		return;
	}

	int len = 0;
	while (*(str+len) != '\0') {
		if (*(str+len) == '*') {
			if (!strncmp(str+len, "*h2", 3)) {		// 愛佳の呼び方
				// ToHeart2 XRATED ソースコードより、スクリプト側で強引に実装。
				// レジスタ、ジャンプラベルは普段使っていないと思われるものを使用。
				// メッセージ表示中にレジスタに有効な値が入っていることはないはず。
				char buf[38];
				sprintf(buf, "l213,5b$5 0＝,8%02d愛佳j9%02d@8%02d小牧@9%02d", manaka_count, manaka_count, manaka_count, manaka_count);
				manaka_count++;
				strcat(tmp, buf);
				len += 3;
				continue;
			}
			for (int i = 0; i < array_size(name_table); i++) {
				if (!strncmp(str+len, name_table[i].src, strlen(name_table[i].src))) {
					strcat(tmp, name_table[i].dst);
					len += (int)strlen(name_table[i].src);
					break;
				}
			}
			continue;
		}
		if (isKanji(*(str+len))) {
			sprintf(tmp + strlen(tmp), "%c", *(str+len));
			len++;
		}
		sprintf(tmp + strlen(tmp), "%c", *(str+len));
		len++;
	}

	strcpy(str, tmp);
}









