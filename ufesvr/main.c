/*
 *	main.c
 *
 *	UfeSvrW - P/ECE USBファイルサーバ
 *
 *	* Sat Jun 25 21:06:00 JST 2005 Naoyuki Sawa
 *	- 新規作成。
 *	* Wed Jul 19 18:18:33 JST 2017 Naoyuki Sawa
 *	- 高性能なPCで使用すると、接続したP/ECEが過負荷でハングアップする事が有る問題を修正しました。
 *	- 原因(推測)は、以下の通りです。
 *	  当ツールはこれまで、ウェイト無しで常時P/ECEに対してUSB通信でポーリングを行って、P/ECEからのファイルアクセスの要求を監視していました。
 *	  当ツールをリリースした2005年頃は、現在と比べてまだPCが遅く、PCが全力でポーリングを行ってもある程度の間隔が空き、P/ECE自身の処理時間が残っていて、問題が生じなかったのだと思います。
 *	  2017現在は、当時と比べてPCが早くなり、PCが全力でポーリングを行うと、P/ECE自身の処理時間が残らないぐらい短い間隔でUSB割り込みがかかり、P/ECE上のアプリがハングアップするようになってしまったのだと思います。
 *	- 対策した方法は、以下の通りです。
 *	  当ツールからP/ECEへのポーリングを行った結果、P/ECEからのファイルアクセスの要求が無かった場合は、次回のポーリングまでの待ち時間を、だんだん長くして行くようにしました。
 *	  10ミリ秒づつ増やしていって、最大、100ミリ秒まで長くなるようにしました。(10ミリ秒,100ミリ秒という数値は意味のある数値ではなく、実際に動かしてみて上手く動いた数値です。)
 *	  一般的にファイルアクセスの要求は連続して発生する事が多いですから、その場合は、短い間隔でポーリングしているのですぐに要求に応えることができ、要求がしばらく無ければポーリング間隔も長くなって負荷が減ります。
 *	- 上記の調整を行って、30分ぐらい実機で動作確認してみて、今のところハングアップせずに動作するようになりました。
 *	  多分、大丈夫かな、と思います。
 */
#define STRICT
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "resource.h"

#define APPNAME		"UFESVRW"
#define APPTITLE	"P/ECE USBファイルサーバ"

#define SEC_HELP	"Help"
#define KEY_DISABLE	"Disable"

#define SEC_WINDOW	"Window"
#define KEY_X		"X"
#define KEY_Y		"Y"

/****************************************************************************
 *	
 ****************************************************************************/

#define NOPCESPRINTF
#define __PCEKN__
#include "C:/usr/PIECE/include/piece.h"
#include "C:/usr/PIECE/sysdev/pcekn/vector.h"
#include "C:/usr/PIECE/tools/isd/pieceif.h"
#pragma comment(lib,"C:/usr/PIECE/tools/isd/pieceif.lib")

#include "ufe.h"

USBCOMS ucs;
int ufe_addr;
UFE ufe;

/****************************************************************************
 *	
 ****************************************************************************/

/* インスタンスハンドル */
HINSTANCE hInst;

/* アプリケーションファイル名 */
char AppPath[_MAX_PATH];	/* Drive+Dir+Fname+Ext */
char AppFolder[_MAX_PATH];	/* Drive+Dir */
char AppDrive[_MAX_DRIVE];
char AppDir[_MAX_DIR];
char AppFname[_MAX_FNAME];
char AppExt[_MAX_EXT];

/* INIファイル名 */
char IniPath[_MAX_PATH];

/* ウインドウ */
HWND hMainDlg;
HWND hState;
HWND hPause;
HWND hExit;

/* アイコン */
HICON hIconMain;

/* ビットマップ */
HBITMAP hBmpActive;
HBITMAP hBmpPaused;
HBITMAP hBmpAppErr;
HBITMAP hBmpUsbErr;

/* 接続状態・ビットマップ */
#define STATE_ACTIVE	0
#define STATE_PAUSED	1
#define STATE_APPERR	2
#define STATE_USBERR	3
int CurrentState = -1;

/* 一時停止/再開要求 */
int Pause;

/* トレース用シリアルナンバー */
int Serial;

/****************************************************************************
 *	
 ****************************************************************************/

void Trace(const char* fmt, ...);
void SetState(int NewState);
void AppInit();
BOOL CALLBACK HelpDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MainDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int DoEvents();
int GetPause();
void AppMain();

/****************************************************************************
 *	
 ****************************************************************************/

void
Trace(const char* fmt, ...)
{
#ifdef _DEBUG

	char buf[1024];
	va_list ap;

	va_start(ap, fmt);
	_vsnprintf(buf, sizeof buf, fmt, ap);
	va_end(ap);

	OutputDebugString(buf);

#endif _DEBUG
}

void
SetState(int NewState)
{
	if(CurrentState != NewState) {
		CurrentState = NewState;
		switch(CurrentState) {
		case STATE_ACTIVE:
			SendMessage(hState, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpActive);
			SetWindowText(hPause, "一時停止");
			EnableWindow(hPause, TRUE);
			break;
		case STATE_PAUSED:
			SendMessage(hState, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpPaused);
			SetWindowText(hPause, "再開");
			EnableWindow(hPause, TRUE);
			break;
		case STATE_APPERR:
			SendMessage(hState, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpAppErr);
			SetWindowText(hPause, "一時停止");
			EnableWindow(hPause, FALSE);
			break;
		case STATE_USBERR:
			SendMessage(hState, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpUsbErr);
			SetWindowText(hPause, "一時停止");
			EnableWindow(hPause, FALSE);
			break;
		}
	}
}

int PASCAL
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HANDLE hMutex;

	/* 多重起動防止のため、共有ミューテクスを作成、または、参照を取得します。 */
	hMutex = CreateMutex(NULL, TRUE, APPNAME);

	/* 共有ミューテクスのロックを試みます。 */
	if(WaitForSingleObject(hMutex, 0) == WAIT_OBJECT_0) {

		/* インスタンスハンドルを格納します。 */
		hInst = hInstance;

		/* アプリケーションを初期化します。 */
		AppInit();

		/* メインループ処理を行います。 */
		AppMain();

		/* 共有ミューテクスのロックを解除します。 */
		ReleaseMutex(hMutex);
	}

	/* 共有ミューテクスを削除、または、参照を開放します。 */
	CloseHandle(hMutex);

	return 0;
}

void
AppInit()
{
	/* カレントディレクトリを確実にEXTファイルと同じ位置とします。 */
	GetModuleFileName(NULL, AppPath, sizeof AppPath);
	_splitpath(AppPath, AppDrive, AppDir, AppFname, AppExt);
	_makepath(AppFolder, AppDrive, AppDir, NULL, NULL);
	SetCurrentDirectory(AppFolder);

	/* INIファイル名を作成します。 */
	_makepath(IniPath, AppDrive, AppDir, AppFname, "ini");

	/* アイコンをロードします。 */
	hIconMain = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIN));

	/* ビットマップをロードします。 */
	hBmpActive = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_ACTIVE));
	hBmpPaused = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_PAUSED));
	hBmpAppErr = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_APPERR));
	hBmpUsbErr = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_USBERR));

	/* 使用上の注意を表示します。 */
	if(!GetPrivateProfileInt(SEC_HELP, KEY_DISABLE, 0, IniPath)) {
		if(DialogBox(hInst, MAKEINTRESOURCE(IDD_HELP), NULL, HelpDlgProc)) {
			WritePrivateProfileString(SEC_HELP, KEY_DISABLE, "1", IniPath);
		}
	}

	/* ダイアログを作成します。 */
	hMainDlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	ShowWindow(hMainDlg, SW_SHOW);
}

BOOL CALLBACK
HelpDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		SendMessage(hWnd, WM_SETICON, ICON_BIG  , (LPARAM)hIconMain);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconMain);
		return TRUE;

	case WM_COMMAND:
		switch(wParam) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, SendDlgItemMessage(hWnd, IDC_DISABLE, BM_GETCHECK, 0, 0));
			return TRUE;
		}
		break;
	}

	return FALSE;
}

BOOL CALLBACK
MainDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int x;
	int y;
	int w;
	int h;
	RECT rect1;
	RECT rect2;
	RECT* pRect;
	char buf[256];

	switch(msg) {
	case WM_INITDIALOG:
		/* コントロールハンドルを取得します。 */
		hState = GetDlgItem(hWnd, IDC_STATE);
		hPause = GetDlgItem(hWnd, ID_PAUSE );
		hExit  = GetDlgItem(hWnd, ID_EXIT  );
		/* タイトルを設定します。 */
		SetWindowText(hWnd, APPTITLE);
		/* アイコンを設定します。 */
		SendMessage(hWnd, WM_SETICON, ICON_BIG  , (LPARAM)hIconMain);
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconMain);
		/* 初期位置を設定します。 */
		GetWindowRect(hWnd, &rect1);
		x = GetPrivateProfileInt(SEC_WINDOW, KEY_X, rect1.left, IniPath);
		y = GetPrivateProfileInt(SEC_WINDOW, KEY_Y, rect1.top , IniPath);
		w = rect1.right - rect1.left;
		h = rect1.bottom - rect1.top;
		SetRect(&rect1, x, y, x + w, y + h);
		SendMessage(hWnd, WM_MOVING, 0, (LPARAM)&rect1);
		SetWindowPos(hWnd, NULL, rect1.left, rect1.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		return TRUE;

	case WM_DESTROY:
		/* ウインドウ位置を記憶します。 */
		GetWindowRect(hWnd, &rect1);
		x = rect1.left;
		y = rect1.top;
		WritePrivateProfileString(SEC_WINDOW, KEY_X, itoa(x, buf, 10), IniPath);
		WritePrivateProfileString(SEC_WINDOW, KEY_Y, itoa(y, buf, 10), IniPath);
		return TRUE;

	/*{{画面外に移動しない。 */
	case WM_MOVING:
		pRect = (RECT*)lParam;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rect2, 0);
		if(pRect->right  > rect2.right ) { pRect->left   -= pRect->right  - rect2.right ; pRect->right  = rect2.right ; }
		if(pRect->bottom > rect2.bottom) { pRect->top    -= pRect->bottom - rect2.bottom; pRect->bottom = rect2.bottom; }
		if(pRect->left   < rect2.left  ) { pRect->right  -= pRect->left   - rect2.left  ; pRect->left   = rect2.left  ; }
		if(pRect->top    < rect2.top   ) { pRect->bottom -= pRect->top    - rect2.top   ; pRect->top    = rect2.top   ; }
		return TRUE;
	/*}}画面外に移動しない。 */

	/*{{クライアント領域をつかんでウインドウを移動する方法。どちらか一方でOK。 */
	case WM_NCHITTEST:
		SetWindowLong(hWnd, DWL_MSGRESULT, HTCAPTION);
		return TRUE;
	//case WM_LBUTTONDOWN:
	//	SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE | 2/*マウスによる移動*/, 0);
	//	return TRUE;
	/*}}クライアント領域をつかんでウインドウを移動する方法。どちらか一方でOK。 */

	case WM_COMMAND:
		switch(wParam) {
		case ID_PAUSE:
			Pause = 1;
			return TRUE;
		case ID_EXIT:
			DestroyWindow(hWnd);
			PostQuitMessage(0);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

int
DoEvents()
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if(msg.message == WM_QUIT) {
			return 0;
		}
		if(!IsDialogMessage(hMainDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 1;
}

int
GetPause()
{
	if(!Pause) {
		return 0;
	}
	Pause = 0;
	return 1;
}

void
AppMain()
{
	int retval;
	int sleep_time = 0; /* ポーリング間隔。P/ECEからのファイルアクセス要求が無ければ、10ミリ秒づつ増やして行って、最長100ミリ秒間隔まで増やす。 */

L_REDO:
	retval = ismInit();
	if(retval != 0) {
		SetState(STATE_USBERR);
		goto L_ERR;
	}

	retval = ismUCOpen(&ucs);
	if((retval != 0) ||
	   !(ucs.mystat) ||
	   (memcmp(ucs.signature, "UFE1", 4) != 0)) {
		SetState(STATE_APPERR);
		goto L_ERR;
	}
	ufe_addr = *(int*)&ucs.signature[4];

	SetState(STATE_ACTIVE);

	for(;;) {
		if(!DoEvents()) {
			goto L_EXIT;
		}
		if(GetPause()) {
			ismUCClose();
			ismExit();
			SetState(STATE_PAUSED);
			do {
				if(!DoEvents()) {
					goto L_EXIT;
				}
				Sleep(100); /* ポーズ中のCPU使用率低減 */
			} while(!GetPause());
			goto L_ERR;
		}

		retval = ismUCGetStat(&ucs);
		if((retval != 0) ||
		   !(ucs.mystat)) {
			goto L_ERR;
		}

		retval = ismReadMem((void*)&ufe.ksno, ufe_addr + 4/*ksno*/, 4/*ksno*/);
		if(retval != 0) {
			goto L_ERR;
		}
		if(!ufe.ksno) {
			if((sleep_time += 10/*調整可*/) > 100/*調整可*/) { sleep_time = 100/*調整可*/; } /* P/ECEからのファイルアクセス要求が無ければ、P/ECE側の負荷低減のために、ポーリング間隔を増やして行く。 */
			Sleep(sleep_time);
			continue;
		}
		sleep_time = 0; /* P/ECEからのファイルアクセス要求が発生したら、ポーリング間隔をリセットする。P/ECEからのファイルアクセス要求は連続的に発生する事が多いと想定して、次回の要求にもなるべく素早く応答するためです。 */

		retval = ismReadMem((void*)&ufe.api, ufe_addr + 8/*api*/, 16/*api*/);
		if(retval != 0) {
			goto L_ERR;
		}

		switch(ufe.ksno) {
		case KSNO_FileFindOpen:
			Trace("%5d: FileFindOpen", Serial++);
			ufe.result = pceFileFindOpen(
				ufe.api.find_open.pfi);
			break;
		case KSNO_FileFindNext:
			Trace("%5d: FileFindNext", Serial++);
			ufe.result = pceFileFindNext(
				ufe.api.find_next.pfi);
			break;
		case KSNO_FileFindClose:
			Trace("%5d: FileFindClose", Serial++);
			ufe.result = pceFileFindClose(
				ufe.api.find_close.pfi);
			break;
		case KSNO_FileOpen:
			Trace("%5d: FileOpen", Serial++);
			ufe.result = pceFileOpen(
				ufe.api.open.pfa,
				ufe.api.open.fname,
				ufe.api.open.mode);
			break;
		case KSNO_FileReadSct:
			Trace("%5d: FileReadSct", Serial++);
			ufe.result = pceFileReadSct(
				ufe.api.read_sct.pfa,
				ufe.api.read_sct.ptr,
				ufe.api.read_sct.sct,
				ufe.api.read_sct.len);
			break;
		case KSNO_FileWriteSct:
			Trace("%5d: FileWriteSct", Serial++);
			ufe.result = pceFileWriteSct(
				ufe.api.write_sct.pfa,
				ufe.api.write_sct.ptr,
				ufe.api.write_sct.sct,
				ufe.api.write_sct.len);
			break;
		case KSNO_FileClose:
			Trace("%5d: FileClose", Serial++);
			ufe.result = pceFileClose(
				ufe.api.close.pfa);
			break;
		case KSNO_FileCreate:
			Trace("%5d: FileCreate", Serial++);
			ufe.result = pceFileCreate(
				ufe.api.create.fname,
				ufe.api.create.size);
			break;
		default:
			Trace("%5d: Invalid\n", Serial++);
			goto L_ERR;
		}
		Trace(" -> %d\n", ufe.result);

		ufe.ksno = 0;
		retval = ismWriteMem((void*)&ufe.result, ufe_addr + 0/*result+ksno*/, 8/*result+ksno*/);
		if(retval != 0) {
			goto L_ERR;
		}
	}

L_ERR:
	ismUCClose();
	ismExit();

	if(!DoEvents()) {
		goto L_EXIT;
	}
	Sleep(100); /* 切断中のCPU使用率低減 */

	goto L_REDO;

L_EXIT:
	ismUCClose();
	ismExit();
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

	Trace(" %s(%d)", fi.filename, fi.length);

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

	Trace(" %s(%d)", path, mode);

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

	Trace(" %d", sct);

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

