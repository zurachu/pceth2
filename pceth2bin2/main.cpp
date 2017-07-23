/*
 *	pceth2bin2
 *	ToHeart2から抜き出したBINファイルを変換
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/05/24	新規開発。オプションで解析と変換を切り替えられるようにします。
 *	2005/05/25	解析モード完成
 *	2005/05/26	スクリプト変換モードをpceth2binに数式を追加したレベルまで完成
 *	2005/05/26	0x03～0x06を実装
 *	2005/05/30	立ち絵の前回表示位置を記憶しておき、引数指定がない場合それを使用
 *	2005/06/01	立ち絵が消去されたときに、前回表示位置を中央にリセット
 *	2005/06/08	0x4Dの引数読み込みミスを修正
 *	2005/06/10	スクリプト変換モードで0x62の処理が空だったのでコメントアウト；
 *				一部スクリプトでコンマ用0x02の数が多いのを場当たり的に修正
 *	2005/06/12	変換後のラベル長を2→3桁に
 *	2005/06/13	不等号を間違っていたのを修正
 *	2005/06/14	解析モードでは逆ポーランド計算式を先計算してしまわないように
 *				06/10の場当たり的な修正が手抜きだったのを補完
 *				背景画像の時間帯、天候による変化を実装
 *	2005/06/15	ウェイト実装
 *	2005/06/17	0x32実装。立ち絵表示、消去の即時書き換えフラグを消去
 *				立ち絵が消去されたときの前回表示位置を、BG書き換えで消去されたときも行う
 *	2005/06/18	立ち絵表示、消去にスライドイン、アウトを追加
 *	2005/06/20	逆ポーランド計算で-1の場合デフォルト値を返す扱いに
 *	2005/06/23	BGの桜の咲き具合を統一（P/ECE側で再変換する）
 *	2005/06/29	立ち絵表示位置の自動設定の条件を見直し
 *	2005/06/30	0x4FもSE停止ぽいので追加
 *				一部スクリプトで命令終端→次の命令開始の0x02の数が多いのを修正
 *	2005/07/19	BG表示命令で桜を日付に合わせないケース（回想など）に対応（第2引数が10000多い）
 *				0x79（カレンダー）命令追加
 *	2005/07/20	立ち絵表示位置の自動設定、今度こそ合ってるかな
 *				表示文字列のゴミ（8,&）を除去
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "func.h"

#define	DUMMY_SIZE	239

#define array_size(a)	(sizeof(a)/sizeof(a[0]))

typedef struct tagBIN_HEADER {
	WORD	head;		// "VE"
	WORD	page_start;	// 開始ページ番号
	long	size;		// ヘッダ・ラベルを除くサイズ
	long	label_num;	// ラベル数
	WORD	page_end;	// 終了ページ番号
	WORD	reserved;	// 00 00
} BIN_HEADER;

typedef struct tagBIN_LABEL {
	char	name[32];	// ラベル（@で開始）
	DWORD	addr;		// ジャンプ先（DWORD）ヘッダを除いた位置ですか
} BIN_LABEL;

int main(int argc, char *argv[])
{
	FILE	*fpin = NULL, *fpout = NULL;
	char	output_flag = 'a';
	char	path[_MAX_PATH];
	char	drive[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	char	fname[_MAX_FNAME];
	char	ext[_MAX_EXT];
	BIN_HEADER	bh;
	BIN_LABEL	*bl;
	BYTE	*buf;

	BYTE	code;
	int		num[10];
	char	str[100];
	int		len;
	int		pos[28];	// 立ち絵表示位置（位置省略のものは前回と同じ位置とみなす）

	if (argc == 1) {	// usage
		fprintf(stderr, "ToHeart2 BINファイルコンバータ\n");
		fprintf(stderr, "pceth2bin2 [option] filename ..\n");
		fprintf(stderr, "-s スクリプト出力(.scp)\n");
		fprintf(stderr, "-a 解析出力(.txt)デフォルト\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-a")) {	// 以後のファイルは解析出力
			output_flag = 'a';
			continue;
		}
		if (!strcmp(argv[i], "-s")) {	// 以後のファイルはスクリプト出力
			output_flag = 's';
			continue;
		}
		if (!strcmp(argv[i], "-d")) {	// 以後のファイルはスクリプト出力（デバッグモード）
			output_flag = 'd';
			continue;
		}

		if ((fpin = fopen(argv[i], "rb")) == NULL)	goto ERR;

		_splitpath(argv[i], drive, dir, fname, ext);
		if (output_flag == 'a') {
			_makepath(path, drive, dir, fname, ".txt");
		} else {
			_makepath(path, drive, dir, fname, ".scp");
		}
		if((fpout = fopen(path, "wt")) == NULL)	goto ERR;

		// BIN_HEADER
		fread(&bh, 1, sizeof(BIN_HEADER), fpin);
		if (strncmp((char*)&bh.head, "VE", 2)) goto ERR;
		// BIN_LABEL
		if ((bl = (BIN_LABEL *)malloc(sizeof(BIN_LABEL) * bh.label_num)) == NULL)	goto ERR;
		for (int j = 0; j < bh.label_num; j++) {
			fread(&bl[j], 1, sizeof(BIN_LABEL), fpin);
		}
		// 謎のデータ
		fseek(fpin, DUMMY_SIZE, SEEK_CUR);
		bh.size -= DUMMY_SIZE;
		// 読み込む
		if ((buf = (BYTE*)malloc(bh.size)) == NULL)	goto ERR;
		fread(buf, 1, bh.size, fpin);
		fclose(fpin);

		for (int j = 0; j < array_size(pos); j++) { pos[j] = 1; }	// 立ち絵表示位置初期化
		manaka_count = 0;

		for (int j = 0; j < bh.size; ) {
			if (*(buf + j + 1) == 0x02)	j++;		// 0x02が1つ多い場合1つスキップ
			if (*(buf + j++) != 0x02)	goto ERR;	// 0x02

			for (int k = 0; k < bh.label_num; k++) {	// ラベル挿入
				if (j + DUMMY_SIZE == bl[k].addr) {
					if (output_flag == 'a') {
						fprintf(fpout, "%s\n", bl[k].name);	// 解析モード
					} else {
						fprintf(fpout, "@%03d",k);			// スクリプト
					}
				}
			}

			switch(*(buf + j++)) {
			case 0x08:	// 命令
			{
				code = *(buf + j++);
				for (int k = 0; k < 10; k++) { num[k] = 0; }

				if (output_flag == 'a') {	// 解析モードではそのまま書き出します
					fprintf(fpout, "%02x", code);
					if (code != 0x20) {	// この命令のみ末尾に0x02が無い
						do {
							fprintf(fpout, ",");
							while(*(buf + j) != 0x02) {	// コンマ
								if (len = getString(buf + j, str)) {
									fprintf(fpout, " %s", str);
									j += len;
								} else if (len = getLabel(buf + j, str)) {
									fprintf(fpout, " %s", str);
									j += len;
/*								} else if (len = calcRevPolish(buf + j, &num[0])) {
									fprintf(fpout, " %d", num[0]);
									j += len;
*/								} else if (len = getRevPolish(buf + j, str)) {
									fprintf(fpout, "%s", str);
									j += len;
								} else {
									goto ERR;
								}
							}
							j++;
						} while (*(buf + j) != 0x02);	// 次の制御の起点に到達するまで
					}
					fprintf(fpout, "\n");
				} else {
					if (output_flag == 'd') {	// デバッグ出力
						printf("%02x\n", code);
					}
					switch (code) {

					case 0x03:	// reg[a]にbをロード
//					case 0x04:	// reg[a]に数式bをロード（現状使っても重いだけなので省略）
						j += getNumber(buf + j, &num[0]) + 1;
						j += getRevPolish(buf + j, str) + 1;
                        fprintf(fpout, "=%d,%s", num[0], str);
						break;
					case 0x05:	// reg[a]をインクリメント
						j += getNumber(buf + j, &num[0]) + 1;
                        fprintf(fpout, "+%d", num[0]);
						break;
					case 0x06:	// reg[a]をデクリメント
						j += getNumber(buf + j, &num[0]) + 1;
                        fprintf(fpout, "-%d", num[0]);
						break;
					case 0x19:	// ウェイト（33ms単位→10ms単位に変換）
						j += calcRevPolish(buf + j, &num[0]) + 1;
						fprintf(fpout, "w%03d", num[0] * 1000 / 30 / 10);
						break;
					case 0x1D:	// flag[a]をreg[b]にロード(num a, num b)
						for (int k = 0; k < 2; k++) { j += calcRevPolish(buf + j, &num[k]) + 1; }
						fprintf(fpout, "l%d,%d", num[0], num[1]);
						break;
					case 0x1E:	// flag[a]に数式bをストア(num a, exp b)
						j += calcRevPolish(buf + j, &num[0]) + 1;
						j += getRevPolish(buf + j, str) + 1;
						fprintf(fpout, "s%d,%s", num[0], str);
						break;
					case 0x1F:	// スクリプトa.scpにジャンプ(str a)
						j += getString(buf + j, str) + 1;
						fprintf(fpout, "J%s.scp", str);
						break;
					case 0x20:	// 既読ページの管理？
						break;	// この命令のみ末尾に0x02が無い

					case 0x22:	// 選択肢
						j += getString(buf + j, str) + 1;
						fprintf(fpout, "q%s ", str);
						break;
					case 0x23:	// reg[a]に結果を代入する選択スタート
						j += getRegIndex(buf + j, &num[0]) + 1;
						fprintf(fpout, "Q%d", num[0]);
						break;

					case 0x27:	// 背景画像B yyy zzz .pgxの表示（立ち絵も消去）
					case 0x28:	// （立ち絵はそのまま）
					case 0x29:	// 0x27と一緒？
					case 0x2A:	// 0x28と一緒？
						for (int k = 0; k < 3; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						if (num[1] >= 0) { // 0x6A の後に出てくることがある -1 は無視する
							num[1] = convertBGNum(num[1]);	// 桜統一
//**					if (!(code == 0x27 && num[0] == 0)) {	// これはレイヤアニメ無視するんだったら表示しない方が？
/*							if (code & 1) {	// 立ち絵消去するなら位置初期化
								for (int q = 0; q < array_size(pos); q++) { pos[q] = 1; }
							}
							// 10000足されてると桜背景も日付無視して表示するらしい（回想など）
*/							fprintf(fpout, "B%03d%03d.pgx,%1d", num[1] % 1000, num[2], (code - 0x27) % 2);
						}
						break;
/*					case 0x2E:	// 立ち絵画像C xxx yyyyy .pgxの表示（位置z）（すぐに描く）
					case 0x2F:	// （すぐには描かない）→P/ECEでは変わらないので共通にしました
								// メモ 4:（1左,2右）からスライドイン（0,3は無視）
								//		5:優先度（常にR→L→Cの順で描きます、無視）
								//		8:スライドインのフレーム速度
						num[2] = 1;
						for (int k = 0; k < 8; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						if (num[2] > 2) { num[2] = ((num[2] % 2)? 0 : 2); }	// 3以上は奇数左、偶数右（本当は位置違うけど無視）
//						if (num[2] == 1) { num[2] = pos[num[0]]; }	// 前回の表示位置にする
//						pos[num[0]] = num[2];	// 表示位置を記憶
						if (code == 0x2E && num[3] == 0) {
							num[2] += 5;	// 立ち絵表示位置自動設定の条件？
						}
						fprintf(fpout, "C%03d%05d.pgx,%1d", num[0], num[1], num[2]);
						if (code == 0x2E && (num[3] == 1 || num[3] == 2)) {
							fprintf(fpout, ",%d", num[3]);	// スライド
						}
						break;
*/
					case 0x2E:
						num[2] = 1;	// デフォルト値：中央
						for (int k = 0; k < 8; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						if (num[2] > 2) { num[2] = ((num[2] % 2)? 0 : 2); }	// 3以上は奇数左、偶数右（本当は位置違うけど無視）
						if (num[3] == 0) {
							num[2] += 5;	// 立ち絵表示位置自動設定の条件？
						}
						fprintf(fpout, "C%03d%05d.pgx,%1d", num[0], num[1], num[2]);
						if (num[3] == 1 || num[3] == 2) {
							fprintf(fpout, ",%d", num[3]);	// スライド
						}
						break;

					case 0x2F:
						num[2] = -1;
						for (int k = 0; k < 8; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						if (num[2] > 2) { num[2] = ((num[2] % 2)? 0 : 2); }	// 3以上は奇数左、偶数右（本当は位置違うけど無視）
						if (num[2] < 0) { num[2] = 6; }	// デフォルト値：中央＆自動位置設定
						fprintf(fpout, "C%03d%05d.pgx,%1d", num[0], num[1], num[2]);
						break;

					case 0x30:	// キャラ番号（立ち絵画像のxxx）xの立ち絵を消去（すぐに消す）
					case 0x31:	// （すぐには消さない）→P/ECEでは変わらないので共通にしました
								// メモ 2:（1左,2右）にスライドアウト
								//		3:スライドアウトのフレーム速度 
						for (int k = 0; k < 3; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						pos[num[0]] = 1;	// 表示位置をリセット
						fprintf(fpout, "c%03d", num[0]);
						if (num[1] == 1 || num[1] == 2) {
							fprintf(fpout, ",%d", num[1]);	// スライド
						}
						break;
					case 0x32:	// 既に表示されているキャラの画像を差し替え（画像表示で対応）
						for (int k = 0; k < 2; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						fprintf(fpout, "C%03d%05d.pgx,6", num[0], num[1]);	// 基本中央の立ち位置自動設定
						break;
					case 0x38:	// イベント画像V yyyyy z .pgxの表示
					case 0x39:
						for(int k = 0; k < 3; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						for (int q = 0; q < array_size(pos); q++) { pos[q] = 1; }	// 立ち絵消去するので位置初期化
						fprintf(fpout, "V%05d%01d.pgx,0", num[1], num[2]);
						break;
					case 0x46:	// BGM演奏
						j += calcRevPolish(buf + j, &num[0]) + 1;
						fprintf(fpout, "M%02d.pmd", num[0]);
						break;
					case 0x47:	// BGM停止
						fprintf(fpout, "Mxx.pmd");
						break;
					case 0x4C:	// SE（単発）
						j += getNumber(buf + j, &num[0]) + 1;
						fprintf(fpout, "SE_%04d.ppd,0", num[0]);
						break;
					case 0x4D:	// SE（リピートあり）
						for(int k = 0; k < 4; k++) {
							if (len = calcRevPolish(buf + j, &num[k])) { j += len + 1; }
						}
						fprintf(fpout, "SE_%04d.ppd,%1d", num[1], num[3]);
						break;
					case 0x4E:	// SE停止
					case 0x4F:
						fprintf(fpout, "SE_xxxx.ppd,0");	// 要は読み込み失敗でSEバッファを空にする
						break;
					case 0x5F:	// 背景画像の時間帯指定（0～3）
						j += calcRevPolish(buf + j, &num[0]) + 1;
						fprintf(fpout, "G%d", num[0]);
						break;
					case 0x60:	// 背景画像の天候指定（0～1）
						j += calcRevPolish(buf + j, &num[0]) + 1;
						fprintf(fpout, "G%d", num[0] + 5);	// 同じ命令でやりくりするため（5以上なら天候変化）
						break;
					case 0x61:	// ラベル@$にブランチ
						j += getRevPolish(buf + j, str) + 1;
						fprintf(fpout, "b%s", str);
						j += getLabel(buf + j, str) + 1;
						for (int k = 0; k < bh.label_num; k++) {
							if (!strcmp(str, bl[k].name)) { fprintf(fpout, ",%03d", k); }
						}
						break;
//					case 0x62:	// ブランチだけど、引数が分からない上シナリオ分岐には使われていないのでスルー（ぉ
//						break;
					case 0x63:	// ラベル@$にジャンプ
						j += getLabel(buf + j, str) + 1;
						for (int k = 0; k < bh.label_num; k++) {
							if (!strcmp(str, bl[k].name)) { fprintf(fpout, "j%03d", k); }
						}
						break;
					case 0x6A:	// アニメーション（座標に使っている演算処理 0x04 をスキップしているので、単純に表示するだけ）
						for (int k = 0; k < 2; k++) { j += getNumber(buf + j, &num[k]) + 1; }
						j += getString(buf + j, str) + 1;
						*str = toupper(*str);
						if (*str != 'F') { // 効果は使用しない
							*strchr(str, '.') = '\0';
							fprintf(fpout, "%s.pgx,0", str);
						}
						break;
					case 0x74:	// マップ移動選択肢を追加
						for(int k = 0; k < 3; k++) { j += getNumber(buf + j, &num[k]) + 1; }
						j += getString(buf + j, str) + 1;
						fprintf(fpout, "m%d,%d%d,%s.scp", num[1], num[0], num[2], str);
						break;
					case 0x76:	// ムービー再生（なんだけど、エピローグに行く（○月○日）制御に使う）
						fprintf(fpout, "z");
						break;
					case 0x77:	// タイトルに戻る
						j++;	// 無引数
						fprintf(fpout, "Z");
						break;
					case 0x78:	// 時計を表示（とりあえず立ち絵と同じ手法でいきますよー）
						j += getNumber(buf + j, &num[0]) + 1;
						fprintf(fpout, "CLOCK%02d.pgx,1", num[0]);	// スクリプト使用で強引に消させるｗ
						fprintf(fpout, "w100cLOC");
						break;
					case 0x79:	// スクリプト中でカレンダー（るーこシナリオで使用
						for (int k = 0; k < 2; k++) {
							j += getNumber(buf + j, &num[k]) + 1;
						}
						fprintf(fpout, "D%d,%d", num[0], num[1]);
						break;
					}
					j += skipScript(buf + j);
				}
				break;
			}
			case 0x0B:	// メッセージ
			{
				char msg[1000];
				if(*(buf + j++) != 0x00)	goto ERR;	// 0x00
				j += getString(buf + j, msg);
				replaceName(msg);
				fprintf(fpout, "%s", msg);
				if (output_flag == 'a') {
					fprintf(fpout, "\n");
				}
				break;
			}
			case 0x00:	// 終端
				break;
			default:
				goto ERR;
			}
		}

		printf("%s - 成功しました。\n", argv[i]);
		goto FREE;
ERR:
		fprintf(stderr, "%s - 失敗しました。\n", argv[i]);
FREE:
		if(fpin  != NULL)	fclose(fpin);
		if(fpout != NULL)	fclose(fpout);
		if(buf   != NULL)	free(buf);
	}

	return 0;
}



