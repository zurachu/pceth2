/*
 *	pgd16cmv
 *	4／8bit無圧縮BMP→16階調拡張PIECE_BMPコンバータ
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/02/16	初版
 *	2005/02/20	引数のワイルドカード対応（setargv.objをリンク）
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

typedef struct tagPBMP_FILEHEADER{
	DWORD	head;		//	HEADER   'PBEX'
	DWORD	fsize;		//	ファイル長 （BYTE単位）
	BYTE	bpp;		//	bit深度  （4）
	BYTE	mask;		//	マスクのbit深度  （0/1）
	short	w;			//	X幅		8ピクセル単位厳守
	short	h;			//	Y高さ		
	DWORD	buf_size;	//	BMPサイズ	（BYTE単位）
}PBMP_FILEHEADER;

int main(int argc, char *argv[])
{
	FILE	*fp = NULL;
	BITMAPFILEHEADER	bf;
	BITMAPINFOHEADER	bi;
	PBMP_FILEHEADER		pb;
	BYTE	*bmp = NULL, *buf = NULL, *mask = NULL;
	char	output_flag = 't';
	char	path[_MAX_PATH];
	char	drive[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	char	fname[_MAX_FNAME];
	char	ext[_MAX_EXT];

	if(argc == 1) {	// usage
		fprintf(stderr, "4／8bit無圧縮BMP→16階調拡張PIECE_BMPコンバータ\n");
		fprintf(stderr, "pgd16cmv [option] filename ..\n");
		fprintf(stderr, "-b バイナリ出力(.pgx)\n");
		fprintf(stderr, "-t テキスト出力(.c)デフォルト\n");
		return 1;
	}

	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-t")) {	// 以後のファイルはテキスト出力
			output_flag = 't';
			continue;
		}
		if(!strcmp(argv[i], "-b")) {	// 以後のファイルはバイナリ出力
			output_flag = 'b';
			continue;
		}

		_splitpath(argv[i], drive, dir, fname, ext);
		if((fp = fopen(argv[i], "rb")) == NULL)	goto ERR;
		// BITMAPFILEHEADER
		if(fread(&bf, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER)) goto ERR;
		if(strncmp((char*)&bf.bfType, "BM", 2)) goto ERR;
		// BITMAPINFOHEADER
		if(fread(&bi, 1, sizeof(BITMAPINFOHEADER), fp) != sizeof(BITMAPINFOHEADER)) goto ERR;
		if(bi.biSize != sizeof(BITMAPINFOHEADER)) goto ERR;
		if(bi.biWidth % 8)	goto ERR;	// 幅は8ピクセル単位
		if(bi.biBitCount != 8 && bi.biBitCount != 4)	goto ERR; // 4/8bit
		if(bi.biCompression != BI_RGB)	goto ERR;	// 無圧縮
		if(bi.biClrUsed == 0)	bi.biClrUsed = 1 << bi.biBitCount;
		// RGBQUAD[bi.biClrUsed]（読み飛ばし）
		if(fseek(fp, sizeof(RGBQUAD) * bi.biClrUsed, SEEK_CUR))	goto ERR;
		// BITMAP本体
		if((bmp = (BYTE*)malloc(bi.biWidth * bi.biHeight * bi.biBitCount / 8)) == NULL)	goto ERR;
		if(bi.biHeight < 0) {	// トップダウン
			fread(bmp, 1, bi.biWidth * bi.biHeight * bi.biBitCount / 8, fp);
			bi.biHeight = -bi.biHeight;
		} else {				// ボトムアップ
			for(int y = bi.biHeight - 1; y >= 0; y--) {
				fread(bmp + bi.biWidth * y * bi.biBitCount / 8, 1, bi.biWidth * bi.biBitCount / 8, fp);
			}
		}
		fclose(fp);

		// PBMP_FILEHEADER
		strncpy((char*)&pb.head, "XEBP", sizeof(DWORD));
		pb.bpp	= 4;
		pb.mask	= 0;
		pb.w	= (short)bi.biWidth;
		pb.h	= (short)bi.biHeight;
		// PIECE_BMP本体
		if((buf  = (BYTE*)malloc(pb.w * pb.h / 2)) == NULL)	goto ERR;
		if((mask = (BYTE*)malloc(pb.w * pb.h / 8)) == NULL)	goto ERR;
		if(bi.biBitCount == 8) {	// 8bit→マスク有/無16階調
			BYTE *b = buf, *m = mask;
			for(int j = 0; j < pb.w * pb.h; j += 8) {	// マスク
				for(int k = 0; k < 8; k++) {
					*m <<= 1;
					if(bmp[j+k] & 0xF0) {
						bmp[j+k] = 0;
						*m |= 1;
						pb.mask = 1;
					}
				}
				m++;
			}
			for(int j = 0; j < pb.w * pb.h; j += 2) {	// 画素
				*b++ = bmp[j] << 4 | bmp[j+1];
			}
		} else {					// 4bit→マスク無16階調
			memcpy(buf, bmp, pb.w * pb.h / 2);
		}
		// PBMP_FILEHEADER
		pb.buf_size = (pb.bpp + pb.mask) * pb.w * pb.h / 8;
		pb.fsize = sizeof(PBMP_FILEHEADER) + pb.buf_size;
		free(bmp);
		bmp = NULL;

		if(output_flag == 't') {	// テキスト出力
			_makepath(path, drive, dir, fname, ".c");
			if((fp = fopen(path, "wt")) == NULL)	goto ERR;
			fprintf(fp, "unsigned char %s[] = {\n", fname);
			BYTE *p = (BYTE*)&pb;
			for(int j = 0; j < sizeof(PBMP_FILEHEADER); j++)	fprintf(fp, "0x%02x, ",*p++);
			fprintf(fp, "\n");
			p = buf;
			for(int y = 0; y < pb.h; y++) {
				for(int x = 0; x < pb.w; x += 2) {
					fprintf(fp, "0x%02x, ", *p++);
				}
				fprintf(fp, "\n");
			}
			if(pb.mask) {
				p = mask;
				for(int y = 0; y < pb.h; y++) {
					for(int x = 0; x < pb.w; x += 8) {
						fprintf(fp, "0x%02x, ", *p++);
					}
					fprintf(fp, "\n");
				}
			}
			fprintf(fp, "\n};\n");
		} else {					// バイナリ出力
			_makepath(path, drive, dir, fname, ".pgx");
			if((fp = fopen(path, "wb")) == NULL)	goto ERR;
			fwrite(&pb, 1, sizeof(PBMP_FILEHEADER), fp);
			fwrite(buf, 1, pb.w * pb.h / 2, fp);
			if(pb.mask)	fwrite(mask, 1, pb.w * pb.h / 8, fp);
		}
		printf("%s - 成功しました。\n", path);
		goto FREE;

ERR:
		fprintf(stderr, "%s - 失敗しました。\n", argv[i]);
FREE:
		if(fp	!= NULL)	fclose(fp);
		if(bmp	!= NULL)	free(bmp);
		if(buf	!= NULL)	free(buf);
		if(mask	!= NULL)	free(mask);
	}

	return 0;
}