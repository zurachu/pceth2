/*
 *	unpack000
 *	ToHeart2のTH2DATA.000／TH2DATA.001からデータ切り出し
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/02/20	初版
 *	2005/02/26	コマンドラインオプションに-bm2,-binを追加
 *	2005/04/17	unpack000.iniにToHeart2のフォルダを保存
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct tagPACK000 {
	DWORD	header;		// "PACK"
	WORD	famount;
	WORD	reserved;	// 01 00 圧縮フラグか？
} PACK000;

typedef struct tagPACK000_FILEINFO {
	char	fname[24];
	DWORD	offset;
	DWORD	size;
} PACK000_FILEINFO;

void unpack000(const char *fName, FILE *fp000, FILE *fp001, int famount);
int unpack001(PACK000_FILEINFO *finfo, FILE *fp001);

int main(int argc, char *argv[])
{
	FILE	*fp000 = NULL, *fp001 = NULL, *fpout = NULL, *fpini = NULL, *fplst = NULL;
	char	th2dir[_MAX_PATH];
	char	path[_MAX_PATH];
	char	drive[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	PACK000				pinfo;
	PACK000_FILEINFO	finfo;

	if(argc == 1) {	// usage
		fprintf(stderr, "ToHeart2 TH2DATA.000／TH2DATA.001 展開\n");
		fprintf(stderr, "unpack000 { option | filename .. }\n");
		fprintf(stderr, "-all 全ファイル出力\n");
		fprintf(stderr, "-bm2 全BM2ファイル出力\n");
		fprintf(stderr, "-bin 全BINファイル出力\n");
		return 1;
	}

	if((fpini = fopen("unpack000.ini", "r")) != NULL) {
		fscanf(fpini, "%s", th2dir);
		fclose(fpini);
	} else {
		printf("ToHeart2のデータがあるフォルダを指定して下さい。【例】D:\\／C:\\ToHeart2\n");
		scanf("%s", th2dir);
	}

	if(*(th2dir + strlen(th2dir) - 1) != '\\')	strcat(th2dir, "\\");
	_splitpath(th2dir, drive, dir, NULL, NULL);
	_makepath(path, drive, dir, "TH2DATA", ".000");
	if((fp000 = fopen(path, "rb")) == NULL) goto ERR;
	fread(&pinfo, 1, sizeof(PACK000), fp000);
	if(strncmp((char*)&pinfo, "PACK", 4))	goto ERR;
	_makepath(path, drive, dir, "TH2DATA", ".001");
	if((fp001 = fopen(path, "rb")) == NULL) goto ERR;

	if((fpini = fopen("unpack000.ini", "w")) != NULL) {
		fprintf(fpini, "%s", th2dir);
		fclose(fpini);
	}

	if(!strcmp(argv[1], "-all")) {			// 全ファイル
		for(int j = 0; j < pinfo.famount; j++) {
			fread(&finfo, 1, sizeof(PACK000_FILEINFO), fp000);
			if(unpack001(&finfo, fp001)) {
				printf("%s - 成功しました。\n", finfo.fname);
			} else {
				fprintf(stderr, "%s - 失敗しました。\n", finfo.fname);
			}
		}
	} else if(!strcmp(argv[1], "-bm2")) {	// BM2ファイル
		for(int j = 0; j < pinfo.famount; j++) {
			fread(&finfo, 1, sizeof(PACK000_FILEINFO), fp000);
			if(strstr(finfo.fname, ".BM2")) {
				if(unpack001(&finfo, fp001)) {
					printf("%s - 成功しました。\n", finfo.fname);
				} else {
					fprintf(stderr, "%s - 失敗しました。\n", finfo.fname);
				}
			}
		}
	} else if(!strcmp(argv[1], "-bin")) {	// BINファイル
		for(int j = 0; j < pinfo.famount; j++) {
			fread(&finfo, 1, sizeof(PACK000_FILEINFO), fp000);
			if(strstr(finfo.fname, ".BIN")) {
				if(unpack001(&finfo, fp001)) {
					printf("%s - 成功しました。\n", finfo.fname);
				} else {
					fprintf(stderr, "%s - 失敗しました。\n", finfo.fname);
				}
			}
		}
	} else if(!strcmp(argv[1], "-list")) {	// リスト入力
		char fName[32];
		for(int i = 2; i < argc; i++) {
			if((fplst = fopen(argv[i], "rt")) == NULL)	goto ERR_LST;
			while(1) {
				fscanf(fplst, "%s", fName);
				if(!strcmp(fName, "0"))	break;
				unpack000(fName, fp000, fp001, pinfo.famount);
			}
			fclose(fplst);
			continue;
ERR_LST:
			fprintf(stderr, "%s - 失敗しました。\n", argv[i]);
		}
	} else {								// 個別ファイル
		for(int i = 1; i < argc; i++) {
			unpack000(argv[i], fp000, fp001, pinfo.famount);
		}
	}

	fclose(fp001);
	fclose(fp000);
	return 0;

ERR:
	fprintf(stderr, "%s - 失敗しました。\n", path);
	return 1;
}

void unpack000(const char *fName, FILE *fp000, FILE *fp001, int famount)
{
	PACK000_FILEINFO finfo;

	fseek(fp000, sizeof(PACK000), SEEK_SET);
	int i = 0;
	for(; i < famount; i++) {
		fread(&finfo, 1, sizeof(PACK000_FILEINFO), fp000);
		if(!strcmp(fName, finfo.fname))	break;
	}
	if(i >= famount)	goto ERR;
	if(unpack001(&finfo, fp001)) {
		printf("%s - 成功しました。\n", fName);
		return;
	}
ERR:
	fprintf(stderr, "%s - 失敗しました。\n", fName);
}

int unpack001(PACK000_FILEINFO *finfo, FILE *fp001)
{
	BYTE buf;

	FILE *fpout = NULL;
	if((fpout = fopen(finfo->fname, "wb")) == NULL)	return 0;

	fseek(fp001, finfo->offset, SEEK_SET);
	for(int i = 0; i < (long)finfo->size; i++) {
		buf = fgetc(fp001);
		fputc(buf, fpout);
	}
	fclose(fpout);
	return 1;
}
