/*
 *	th2bm2cmv
 *	ToHeart2から抜き出した8／32bitBM2ファイルをBMPファイルに変換
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/02/20	初版
 *				引数のワイルドカード対応（setargv.objをリンク）
 *	2005/02/25	関数プロトタイプ宣言を削除、usageを修正（他のプログラムからのコピペがばれるがな）
 *				拡張子を".BM2"に制限（間違えて".bmp"も再変換してしまっていたため）
 *				8bit画像のビットマップ出力部のbitを入れ替えるよう修正
 *
 *	MEMO : BM2形式について
 *	基本的にWindows DIB。
 *	上下反転。
 *	8bitはパレットの、32bitはビットマップのRとBが入れ替わっている。
 *	予約ビットをα値に使用（0x00～0x80）
 *	8bitはビットマップの第4bitと第5bitが入れ替わっている。
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[])
{
	FILE	*fpin = NULL, *fpout = NULL;
	BITMAPFILEHEADER	bf;
	BITMAPINFOHEADER	bi;
	char	path[_MAX_PATH];
	char	drive[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	char	fname[_MAX_FNAME];
	char	ext[_MAX_EXT];
	BYTE	buf, a, r, g, b;

	if(argc == 1) {	// usage
		fprintf(stderr, "ToHeart2 BM2ファイル→8／32bitBMPコンバータ\n");
		fprintf(stderr, "th2bm2cmv filename ..\n");
		return 1;
	}

	for(int i = 1; i < argc; i++) {
		_splitpath(argv[i], drive, dir, fname, ext);
		if(strcmp(ext, ".BM2"))	goto ERR;
		if((fpin = fopen(argv[i], "rb")) == NULL)	goto ERR;
		// BITMAPFILEHEADER
		if(fread(&bf, 1, sizeof(BITMAPFILEHEADER), fpin) != sizeof(BITMAPFILEHEADER))	goto ERR;
		if(strncmp((char*)&bf.bfType, "BM", 2))	goto ERR;
		// BITMAPINFOHEADER
		if(fread(&bi, 1, sizeof(BITMAPINFOHEADER), fpin) != sizeof(BITMAPINFOHEADER))	goto ERR;
//		if(bi.biSize != sizeof(BITMAPINFOHEADER))	goto ERR;
		if(bi.biBitCount != 32 && bi.biBitCount != 8)	goto ERR; // 8／32bit
		fseek(fpin, 0x40, SEEK_SET);

		bi.biSize = sizeof(BITMAPINFOHEADER);
		bi.biHeight= -bi.biHeight;	// 上下反転
		bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		if(bi.biBitCount == 8)	bf.bfOffBits += sizeof(RGBQUAD) * 256;
		bf.bfSize = bf.bfOffBits + bi.biWidth * abs(bi.biHeight) * bi.biBitCount / 8;
//		bf.bfReserved1 = bf.bfReserved2 = 0;	// 描画オフセットを定める重要数値なのでリセットしない
		_makepath(path, drive, dir, fname, ".bmp");
		if((fpout = fopen(path, "wb")) == NULL)	goto ERR;
		fwrite(&bf, 1, sizeof(BITMAPFILEHEADER), fpout);
		fwrite(&bi, 1, sizeof(BITMAPINFOHEADER), fpout);
		
		if(bi.biBitCount == 32) {	// 32bit
			for(int j = 0; j < bi.biWidth * abs(bi.biHeight); j++) {
				b = fgetc(fpin);	g = fgetc(fpin);	r = fgetc(fpin);	a = fgetc(fpin);
				if(a & 0x80) {	// α値補正
					a = 0xFF;
				} else {
					a <<= 1;
				}
				fputc(r, fpout);	fputc(g, fpout);	fputc(b, fpout);	fputc(a, fpout);	// RB入れ替え
			}
		} else {					// 8bit
			for(int j = 0; j < 256; j++) {
				b = fgetc(fpin);	g = fgetc(fpin);	r = fgetc(fpin);	a = fgetc(fpin);
				if(a & 0x80) {	// α値補正
					a = 0xFF;
				} else {
					a <<= 1;
				}
				fputc(r, fpout);	fputc(g, fpout);	fputc(b, fpout);	fputc(a, fpout);	// RB入れ替え
			}
			for(int j = 0; j < bi.biWidth * abs(bi.biHeight); j++) {
				buf = fgetc(fpin);
				if((buf & 0x10) ^ ((buf & 0x08) << 1))	buf ^= 0x18;	// 第4bitと第5bitを入れ替え
				fputc(buf, fpout);
			}
		}
		printf("%s - 成功しました。\n", argv[i]);
		goto FREE;
ERR:
		fprintf(stderr, "%s - 失敗しました。\n", argv[i]);
FREE:
		if(fpout != NULL)	fclose(fpout);
		if(fpin  != NULL)	fclose(fpin);
	}

	return 0;
}
