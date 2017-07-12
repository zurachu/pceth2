#define STRICT
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <conio.h>

#define NOPCESPRINTF
#define __PCEKN__
#include "C:/usr/PIECE/include/piece.h"
#include "C:/usr/PIECE/sysdev/pcekn/vector.h"
#include "C:/usr/PIECE/tools/isd/pieceif.h"
#pragma comment(lib,"C:/usr/PIECE/tools/isd/pieceif.lib")

#include "ufe.h"

/****************************************************************************
 *	
 ****************************************************************************/

USBCOMS ucs;
int ufe_addr;
UFE ufe;

/****************************************************************************
 *	
 ****************************************************************************/

int
main()
{
	int retval;
	int c;
	int i;
	//
	int serial = 0;
	int state = -1; /* 初回メッセージのため */

	/* ismExit()のメッセージが邪魔なので、エラー出力を閉じておきます。 */
	close(2);

L_REDO:
	retval = ismInit();
	if(retval != 0) {
		goto L_ERR;
	}

	retval = ismUCOpen(&ucs);
	if((retval != 0) ||
	   !(ucs.mystat) ||
	   (memcmp(ucs.signature, "UFE1", 4) != 0)) {
		goto L_ERR;
	}
	ufe_addr = *(int*)&ucs.signature[4];

	for(;;) {
		if(_kbhit()) {
			ismUCClose();
			ismExit();
			c = _getch();
			if(c == 'p') {
				printf("### 一時的にP/ECEとの接続を解除しました。なにかキーを押すと復帰します。\n");
				for(;;) {
					i = GetTickCount();
					while((int)(GetTickCount() - i) < 100/*調整可*/) {
						if(_kbhit()) {
							c = _getch();
							goto L_ERR;
						}
						Sleep(100/*調整可*/);
					}
				}
			}
			exit(0);
		}

		retval = ismUCGetStat(&ucs);
		if((retval != 0) ||
		   !(ucs.mystat)) {
			goto L_ERR;
		}

		if(state != 1) {
			printf("### P/ECEとの接続を確立しました。なにかキーを押すと終了します。\n");
			state = 1;
		}

		retval = ismReadMem((void*)&ufe.ksno, ufe_addr + 4/*ksno*/, 4/*ksno*/);
		if(retval != 0) {
			goto L_ERR;
		}
		if(!ufe.ksno) {
			Sleep(0);
			continue;
		}

		retval = ismReadMem((void*)&ufe.api, ufe_addr + 8/*api*/, 16/*api*/);
		if(retval != 0) {
			goto L_ERR;
		}

		switch(ufe.ksno) {
		case KSNO_FileFindOpen:
			printf("%5d: FileFindOpen", serial++);
			ufe.result = pceFileFindOpen(
				ufe.api.find_open.pfi);
			break;
		case KSNO_FileFindNext:
			printf("%5d: FileFindNext", serial++);
			ufe.result = pceFileFindNext(
				ufe.api.find_next.pfi);
			break;
		case KSNO_FileFindClose:
			printf("%5d: FileFindClose", serial++);
			ufe.result = pceFileFindClose(
				ufe.api.find_close.pfi);
			break;
		case KSNO_FileOpen:
			printf("%5d: FileOpen", serial++);
			ufe.result = pceFileOpen(
				ufe.api.open.pfa,
				ufe.api.open.fname,
				ufe.api.open.mode);
			break;
		case KSNO_FileReadSct:
			printf("%5d: FileReadSct", serial++);
			ufe.result = pceFileReadSct(
				ufe.api.read_sct.pfa,
				ufe.api.read_sct.ptr,
				ufe.api.read_sct.sct,
				ufe.api.read_sct.len);
			break;
		case KSNO_FileWriteSct:
			printf("%5d: FileWriteSct", serial++);
			ufe.result = pceFileWriteSct(
				ufe.api.write_sct.pfa,
				ufe.api.write_sct.ptr,
				ufe.api.write_sct.sct,
				ufe.api.write_sct.len);
			break;
		case KSNO_FileClose:
			printf("%5d: FileClose", serial++);
			ufe.result = pceFileClose(
				ufe.api.close.pfa);
			break;
		case KSNO_FileCreate:
			printf("%5d: FileCreate", serial++);
			ufe.result = pceFileCreate(
				ufe.api.create.fname,
				ufe.api.create.size);
			break;
		default:
			printf("%5d: Invalid\n", serial++);
			goto L_ERR;
		}
		printf(" -> %d\n", ufe.result);

		ufe.ksno = 0;
		retval = ismWriteMem((void*)&ufe.result, ufe_addr + 0/*result+ksno*/, 8/*result+ksno*/);
		if(retval != 0) {
			goto L_ERR;
		}
	}
L_ERR:
	ismUCClose();
	ismExit();

	if(state != 0) {
		printf("### P/ECEからの接続を待っています。なにかキーを押すと終了します。\n");
		state = 0;
	}
	i = GetTickCount();
	while((int)(GetTickCount() - i) < 100/*調整可*/) {
		if(_kbhit()) {
			c = _getch();
			exit(0);
		}
		Sleep(100/*調整可*/);
	}
	goto L_REDO;

	return 0;
}

/****************************************************************************
 *	
 ****************************************************************************/

int
pceFileFindOpen(FILEINFO* pfi)
{
	int retval;
	FILEINFO fi;
	//
	HANDLE hfind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA* pfd = NULL;

	pfd = malloc(sizeof(WIN32_FIND_DATA));
	if(!pfd) {
		goto L_ERR;
	}

	hfind = FindFirstFile("*.*", pfd);
	//if(hfind == INVALID_HANDLE_VALUE) {
	//	goto L_ERR;
	//}

	memset(&fi, 0, sizeof fi);
	*(int*)&fi.works[0] = (int)hfind;
	*(int*)&fi.works[4] = (int)pfd;

	retval = ismWriteMem((void*)&fi, (int)pfi, sizeof(FILEINFO));
	if(retval != 0) {
		goto L_ERR;
	}

	return 0;
L_ERR:
	if(hfind != INVALID_HANDLE_VALUE) {
		FindClose(hfind);
	}
	if(pfd) {
		free(pfd);
	}
	return -1;
}

int
pceFileFindNext(FILEINFO* pfi)
{
	int retval;
	FILEINFO fi;
	HANDLE hfind;
	WIN32_FIND_DATA* pfd;

	retval = ismReadMem((void*)&fi, (int)pfi, sizeof(FILEINFO));
	if(retval != 0) {
		return 0;
	}

	hfind = (HANDLE)          *(int*)&fi.works[0];
	pfd   = (WIN32_FIND_DATA*)*(int*)&fi.works[4];

	if(hfind == INVALID_HANDLE_VALUE) {
		return 0;
	}

	while(pfd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		if(!FindNextFile(hfind, pfd)) {
			return 0;
		}
	}

	GetShortPathName(pfd->cFileName, fi.filename, sizeof fi.filename);
	strlwr(fi.filename);
	fi.length = pfd->nFileSizeLow;

	if(!FindNextFile(hfind, pfd)) {
		FindClose(hfind);
		hfind = INVALID_HANDLE_VALUE;
		*(int*)&fi.works[0] = (int)hfind;
	}

	retval = ismWriteMem((void*)&fi, (int)pfi, sizeof(FILEINFO));
	if(retval != 0) {
		return 0;
	}

	printf(" %s(%d)", fi.filename, fi.length);

	return 1;
}

int
pceFileFindClose(FILEINFO* pfi)
{
	int retval;
	FILEINFO fi;
	HANDLE hfind;
	WIN32_FIND_DATA* pfd;

	retval = ismReadMem((void*)&fi, (int)pfi, sizeof(FILEINFO));
	if(retval != 0) {
		return -1;
	}

	hfind = (HANDLE)          *(int*)&fi.works[0];
	pfd   = (WIN32_FIND_DATA*)*(int*)&fi.works[4];

	if(hfind != INVALID_HANDLE_VALUE) {
		FindClose(hfind);
	}
	if(pfd) {
		free(pfd);
	}

	return 0;
}

int
pceFileOpen(FILEACC* pfa, const char* fname, int mode)
{
	int retval;
	FILEACC fa;
	char path[MAXFILENAME + 1];
	//
	FILE* fp = NULL;

	retval = ismReadMem(path, (int)fname, MAXFILENAME);
	if(retval != 0) {
		goto L_ERR;
	}
	path[MAXFILENAME] = '\0';

	printf(" %s(%d)", path, mode);

	switch(mode) {
	case FOMD_RD:
		fp = fopen(path, "rb");
		break;
	case FOMD_WR:
		fp = fopen(path, "r+b");
		break;
	}
	if(!fp) {
		goto L_ERR;
	}

	fseek(fp, 0, SEEK_END);

	memset(&fa, 0, sizeof fa);
	fa.valid         = FILEACC_UFE;
	*(int*)&fa.chain = (int)fp;
	fa.fsize         = ftell(fp);

	retval = ismWriteMem((void*)&fa, (int)pfa, sizeof(FILEACC));
	if(retval != 0) {
		goto L_ERR;
	}

	return 0;

L_ERR:
	if(fp) {
		fclose(fp);
	}
	return -1;
}

int
pceFileReadSct(FILEACC* pfa, void* ptr, int sct, int len)
{
	int retval;
	FILEACC fa;
	FILE* fp;

	printf(" %d", sct);

	retval = ismReadMem((void*)&fa, (int)pfa, sizeof(FILEACC));
	if(retval != 0) {
		return -1;
	}
	if(fa.valid != FILEACC_UFE) {
		return -1;
	}
	fp = (FILE*)*(int*)&fa.chain;

	if(!ptr || (len > 4096)) {
		len = 4096;
	}
	fseek(fp, sct * 4096, SEEK_SET);
	len = fread(ufe.sctbuf, 1, len, fp);

	if(ptr) {
		retval = ismWriteMem(ufe.sctbuf, (int)ptr, len);
		if(retval != 0) {
			return -1;
		}
	} else {
		fa.aptr = (void*)(ufe_addr + 24/*sctbuf*/);
		retval = ismWriteMem((void*)ufe.sctbuf, (int)fa.aptr, len);
		if(retval != 0) {
			return -1;
		}
		retval = ismWriteMem((void*)&fa, (int)pfa, sizeof(FILEACC));
		if(retval != 0) {
			return -1;
		}
	}

	return len;
}

int
pceFileWriteSct(FILEACC* pfa, const void* ptr, int sct, int len)
{
	/* ※TODO: */
	return -1;
}

int
pceFileClose(FILEACC* pfa)
{
	int retval;
	FILEACC fa;
	FILE* fp;

	retval = ismReadMem((void*)&fa, (int)pfa, sizeof(FILEACC));
	if(retval != 0) {
		return -1;
	}
	if(fa.valid != FILEACC_UFE) {
		return -1;
	}
	fp = (FILE*)*(int*)&fa.chain;

	fclose(fp);

	return 0;
}

int
pceFileCreate(const char* fname, unsigned long size)
{
	/* ※TODO: */
	return -1;
}

