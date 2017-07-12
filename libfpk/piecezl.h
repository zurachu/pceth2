// yui: kernel 1.20 のソースをそのまま利用

/////////////////////////////////////////////////////////////////////////////
//
//             /
//      -  P  /  E  C  E  -
//           /                 mobile equipment
//
//              System Programs
//
//
// PIECE KERNEL : Ver 1.00
//
// Copyright (C)2001 AUQAPLUS Co., Ltd. / OeRSTED, Inc. all rights reserved.
//
// Coded by MIO.H (OeRSTED)
//
// Comments:
//
// Zlib 展開用のヘッダファイルです。
//
//  v1.00 2001.11.09 MIO.H
//



typedef struct _zlibIO {
	unsigned char *ptr;
	unsigned char *ptre;
	unsigned char *ptr0;
	union {
		int (*fil)(struct _zlibIO *);
		void (*fls)(struct _zlibIO *);
	} fn;
} zlibIO;

#define zlgetc(_stream) (((_stream)->ptr < (_stream)->ptre) ? *((_stream)->ptr++) : (_stream)->fn.fil(_stream))


int pceZlibExpand( zlibIO *zlIn, zlibIO *zlOut, void *works );


