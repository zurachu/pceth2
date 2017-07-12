/*
 *	ufe.c - UFE サーバ/クライアント共通ヘッダ
 *
 *	UFE - P/ECE USB File System Emulation
 *
 *	* Wed Jun 08 20:00:00 JST 2005 Naoyuki Sawa
 *	- 新規作成。
 */
#ifndef __UFE_H__
#define __UFE_H__

/*****************************************************************************
 *	サーバ/クライアント共通の定義
 *****************************************************************************/

/* int pceFileFindOpen(FILEINFO* pfi) */
typedef struct _UFEFILEFINDOPEN {
	FILEINFO* pfi;			/* + 0, 4 */
} UFEFILEFINDOPEN;			/* = 4    */

/* int pceFileFindNext(FILEINFO* pfi) */
typedef struct _UFEFILEFINDNEXT {
	FILEINFO* pfi;			/* + 0, 4 */
} UFEFILEFINDNEXT;			/* = 4    */

/* int pceFileFindClose(FILEINFO* pfi) */
typedef struct _UFEFILEFINDCLOSE {
	FILEINFO* pfi;			/* + 0, 4 */
} UFEFILEFINDCLOSE;			/* = 4    */

/* int pceFileOpen(FILEACC* pfa, const char* fname, int mode) */
typedef struct _UFEFILEOPEN {
	FILEACC* pfa;			/* + 0, 4 */
	const char* fname;		/* + 4, 4 */
	int mode;			/* + 8, 4 */
} UFEFILEOPEN;				/* =12    */

/* int pceFileReadSct(FILEACC* pfa, void* ptr, int sct, int len) */
typedef struct _UFEFILEREADSCT {
	FILEACC* pfa;			/* + 0, 4 */
	void* ptr;			/* + 4, 4 */
	int sct;			/* + 8, 4 */
	int len;			/* +12, 4 */
} UFEFILEREADSCT;			/* =16    */

/* int pceFileWriteSct(FILEACC* pfa, const void* ptr, int sct, int len) */
typedef struct _UFEFILEWRITESCT {
	FILEACC* pfa;			/* + 0, 4 */
	const void* ptr;		/* + 4, 4 */
	int sct;			/* + 8, 4 */
	int len;			/* +12, 4 */
} UFEFILEWRITESCT;			/* =16    */

/* int pceFileClose(FILEACC* pfa) */
typedef struct _UFEFILECLOSE {
	FILEACC* pfa;			/* + 0, 4 */
} UFEFILECLOSE;				/* = 4    */

/* int pceFileCreate(const char* fname, unsigned long size) */
typedef struct _UFEFILECREATE {
	const char* fname;		/* + 0, 4 */
	unsigned long size;		/* = 4    */
} UFEFILECREATE;

typedef union _UFEAPI {
	UFEFILEFINDOPEN find_open;	/* + 0, 4 */
	UFEFILEFINDNEXT find_next;	/* + 0, 4 */
	UFEFILEFINDCLOSE find_close;	/* + 0, 4 */
	UFEFILEOPEN open;		/* + 0,12 */
	UFEFILEREADSCT read_sct;	/* + 0,16 */
	UFEFILEWRITESCT write_sct;	/* + 0,16 */
	UFEFILECLOSE close;		/* + 0, 4 */
	UFEFILECREATE create;		/* + 0, 4 */
} UFEAPI;				/* =16    */

typedef struct _UFE {
	int result;			/* + 0,   4 */
	int ksno;			/* + 4,   4 */
	UFEAPI api;			/* + 8,  16 */
	unsigned char sctbuf[4096];	/* +24,4096 */
} UFE;

/* FILEINFO.works[15] */
#define FILEINFO_PFFS	0xe6		/* PFFS検索中 */
#define FILEINFO_UFE	0xe5		/* UFE 検索中 */

/* FILEACC.valid */
#define FILEACC_PFFS	0x9ce6		/* PFFS読み書き中(カーネル定義) */
#define FILEACC_UFE	0x9ce5		/* UFE 読み書き中 */

/*****************************************************************************
 *	クライアント専用の関数
 *****************************************************************************/

int ufe_setup();
void ufe_stop();

#endif /*__UFE_H__*/
