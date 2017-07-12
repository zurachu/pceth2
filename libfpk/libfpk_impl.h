/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/

// yui: 2005.07.19: 実装用宣言はこちらへ移動

#ifndef LIBFPK_IMPL_H
#define LIBFPK_IMPL_H

#include "libfpk.h"


//int pceFileReadPos(FILEACC *pfa, unsigned char *buf, int pos, int size);
//↓{{2005/06/12 Naoyuki Sawa}}
// int pceFileReadPos(HFPK hFpk, unsigned char *buf, int pos, int size);
// yui: 2005.07.03 もはや P/ECE 標準 pceFileReadPos() ではないので改名
int fpkFileReadPos(HFPK hFpk, unsigned char *buf, int pos, int size);

/*{{2005/06/12 Naoyuki Sawa*/
int fpkStrCaseCmp(const char* s1, const char* s2);
int fpkFileNameCRC(const char* fname); /* 16bit */
/*}}2005/06/12 Naoyuki Sawa*/

// ############################################################################
// ## 圧縮メソッド
// ############################################################################

#ifndef LIBFPK_NO_LZSS
void hitodeLZSSDecode(HFPK hFpk, int ofs, int size, BYTE* dst);
#endif

#ifndef LIBFPK_NO_LZF
DWORD lzfDecompress(HFPK hFpk, DWORD dwOffset, int dwSourceSize, BYTE* pDst, int dwDestSize);
#endif

#ifndef LIBFPK_NO_ZLIB
DWORD zlibDecode(HFPK hFpk, DWORD dwOffset, DWORD dwSourceSize, BYTE *pOutBuff);
#endif

// inflate 関係はややこしいので .h を残す

#endif /* !LIBFPK_IMPL_H */
