/*
 *	pceth2 - 制御命令関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/01	main.cから分離
 *	2005/05/09	EV_～スクリプトを開くように
 *				pceth2_jumpScriptをEVならgosub、それ以外ならgotoに
 *	2005/05/27	Win側pceth2bin2に対応（スクリプト形式を多少変更
 *				getNumの仕様変更
 *	2005/06/11	pceth2_backTitleを追加
 */

#include <string.h>
#include <piece.h>
#include "common.h"
#include "pceth2_sys.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"
#include "pceth2_sav.h"

int pceth2_isCalenderMode();

//=============================================================================
//	スクリプトファイル読み込み
//=============================================================================

/*
 *	スクリプトを開く
 *
 *	*s		スクリプトデータへのポインタ
 *	*fName	スクリプトファイル名
 */
void pceth2_loadScript(SCRIPT_DATA *s, const char *fName)
{
	pceth2_closeScript(s);
	strcpy(s->name, fName);
	s->data = fpk_getEntryData(s->name, &s->size, NULL);
}

/*
 *	スクリプトを閉じる
 *
 *	*s		スクリプトデータへのポインタ
 */
void pceth2_closeScript(SCRIPT_DATA *s)
{
	*s->name = '\0';
	pceHeapFree(s->data);
	s->data = NULL;
	s->p = s->size = 0;
}

//=============================================================================
//	スクリプト
//=============================================================================

/*
 *	EVスクリプトを開く
 */
void pceth2_loadEVScript()
{
	char buf[FNAMELEN_EV + 1];

	// EVスクリプトファイル名を作成して読み込む
	pcesprintf(buf, "EV_%02d%02d%1d.scp", MONTH, DAY, TIME);
	pceth2_loadScript(&play.evData, buf);

	JUMP = 0;	// ジャンプフラグをリセット（gosubに）

	// 読み込んだ時点で次の時間帯に進めておく
	TIME++;

	play.gameMode = GM_EVSCRIPT;
}

/*
 *	指定スクリプトにジャンプ	J000000000.scp
 *
 *	*s		スクリプトデータ
 *
 *	return	0（制御を戻す）
 */
int pceth2_jumpScript(SCRIPT_DATA *s)
{
	char buf[FNAMELEN_SCP + 1];
	s->p++;

	// ファイル名をバッファにコピー
	pceth2_strcpy(buf, s, FNAMELEN_SCP);

	// 2005/06/20 まるしすさん案：1e, 4, 1が来たらgoto
	if (JUMP == 1) {	// GM_SCRIPTだったら以下で上書きされるから関係ないんだけど
		s->p = s->size;	// 最後まで読んだことにする（戻ってきたら即終了）
	}
	pceth2_loadScript(&play.scData, buf);
	play.gameMode = (pceth2_isCalenderMode())? GM_KEYWAIT : GM_SCRIPT;

	return 0;
}

//=============================================================================
//	ラベルジャンプ
//=============================================================================

#define NUMLEN_LABEL	3

/*
 *	ラベルをスキップするだけ	@000
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_skipLabel(SCRIPT_DATA *s)
{
	s->p += NUMLEN_LABEL + 1;
	return 1;
}

/*
 *	指定ラベルにジャンプ		j000
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_jumpLabel(SCRIPT_DATA *s)
{
	DWORD ptr;
	s->p++;

	for (ptr = 0; ptr < s->size; ptr++) {
		if (*(s->data + ptr) == '@') {	// スクリプト走査
			ptr++;
			if (!strncmp(s->data + s->p, s->data + ptr, NUMLEN_LABEL)) {	// ラベル照合
				s->p = ptr + NUMLEN_LABEL;
				break;
			}
		}
		if (pceth2_isKanji(s)) { ptr++; }	// 漢字の場合2bytes進める
	}

	return 1;
}

/*
 *	条件付きで指定ラベルにジャンプ	b[exp],000
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_branchLabel(SCRIPT_DATA *s)
{
	s->p++;
	if (pceth2_calcExpression(s)) {
		pceth2_jumpLabel(s);
	} else {
		s->p += NUMLEN_LABEL + 1;
	}

	return 1;
}

//=============================================================================
//	フラグ・レジスタ操作
//=============================================================================

unsigned short reg[REG_NUM];

/*
 *	フラグをレジスタにロード	l?,0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_loadFlag(SCRIPT_DATA *s)
{
	int i, j;
	
	s->p++;
	i = pceth2_getNum(s);
	s->p++;
	j = *(s->data + s->p++) - '0';
	reg[j] = play.flag[i];

	return 1;
}

/*
 *	フラグに即値をストア	s?,[exp]
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_saveFlag(SCRIPT_DATA *s)
{
	int i, j;
	s->p++;
	i = pceth2_getNum(s);
	s->p++;
	j = pceth2_calcExpression(s);
	if (j != -1) {	// -1（デフォルト値）の場合変更しない
		play.flag[i] = j;
		if (i >= 81 && i <= 91) {	// クリアフラグの場合
			play.flag[80] = 0;
			for (i = 81; i <= 91; i++) {	// flag[80]=クリアシナリオ数
				play.flag[80] += play.flag[i];
			}
			memcpy(&global.flag[0], &play.flag[80], sizeof(unsigned short) * GLOBAL_FLAG_NUM);
			pceth2_writeGlobalSaveData();	// グローバルセーブに書き込む
		}
	}

	return 1;
}

/*
 *	レジスタに即値をセット	=0,[exp]
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_setReg(SCRIPT_DATA *s)
{
	int i;
	s->p++;
	i = *(s->data + s->p++) - '0';
	s->p++;
	reg[i] = pceth2_calcExpression(s);

	return 1;
}

/*
 *	レジスタをインクリメント	+0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_incReg(SCRIPT_DATA *s)
{
	int i;
	s->p++;
	i = *(s->data + s->p++) - '0';
	reg[i]++;

	return 1;
}

/*
 *	レジスタをデクリメント	-0
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_decReg(SCRIPT_DATA *s)
{
	int i;
	s->p++;
	i = *(s->data + s->p++) - '0';
	reg[i]--;

	return 1;
}

//=============================================================================
//	その他
//=============================================================================

/*
 *	時間ウェイトを指定	w?
 *
 *	*s		スクリプトデータ
 *
 *	return	0（制御を戻す）
 */
int pceth2_wait(SCRIPT_DATA *s)
{
    s->p++;
	wait = pceth2_getNum(s) * 30 / 100;
	play.gameMode = GM_TIMEWAIT;

	return 0;
}

/*
 *	エピローグに入る（日時不明にする）	z
 *
 *	*s		スクリプトデータ
 *
 *	return	1（引き続き実行）
 */
int pceth2_goEpilogue(SCRIPT_DATA *s)
{
	s->p++;
	MONTH = DAY = 0;

	return 1;
}

//=============================================================================
//	数値計算
//=============================================================================

/*
 *	オーバーフローチェックをしていない駄目なスタック
 */
#define STACK_NUM	8
static int stack[STACK_NUM];
static int stack_index;

// プッシュ
static void _push(int num)
{
	stack[stack_index] = num;
	stack_index++;
}

// ポップ（スタックが空のときは0を返す）
static int _pop(void)
{
	if (stack_index > 0) {
		stack_index--;
		return stack[stack_index];
	}

	return 0;	// これで負記号が上手く処理できるはず
}

/*
 *	逆ポーランド記法の計算式を解く
 *	*s	スクリプトデータ
 *	return	計算結果
 */
int pceth2_calcExpression(SCRIPT_DATA *s)
{
	static const char * const operator_table[] = {
		"＋", "－", "×", "÷", "＝", "≠", "＜", "＞", "≦", "≧",
	};
	int val1, val2, i;

	stack_index = 0;

	while (1) {
		switch (*(s->data + s->p))
		{
			case ' ':	// 即値
				s->p++;
				_push(pceth2_getNum(s));
				break;
			case '$':	// レジスタ
				s->p++;
				_push(reg[pceth2_getNum(s)]);
				break;
			default:	// 演算子
				val1 = _pop();
				val2 = _pop();
				for (i = 0; i < array_size(operator_table); i++) {
					if (!strncmp(s->data + s->p, operator_table[i], 2)) {
						s->p += 2;
						break;
					}
				}
				switch (i) {
					case 0:		_push(val2 +  val1);	break;	// ＋
					case 1:		_push(val2 -  val1);	break;	// －
					case 2:		_push(val2 *  val1);	break;	// ×
					case 3:		_push(val2 /  val1);	break;	// ÷
					case 4:		_push(val2 == val1);	break;	// ＝
					case 5:		_push(val2 != val1);	break;	// ≠
					case 6:		_push(val2 <  val1);	break;	// ＜
					case 7:		_push(val2 >  val1);	break;	// ＞
					case 8:		_push(val2 <= val1);	break;	// ≦
					case 9:		_push(val2 >= val1);	break;	// ≧
					default:	return val1;
				}
		}
	}
}
