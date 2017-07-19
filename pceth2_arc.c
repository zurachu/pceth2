/*
 *	pceth2 - アーカイブ関連
 *
 *	(c)2005 てとら★ぽっと
 *
 *	2005/07/23	main.cから分離
 *	2005/08/20	あらかじめ確保しておいたバッファに展開できるよう変更
 */

//	2005/07/23
//	アーカイブとの折衝は全てこのソースに任せます。

#include "libfpk/libfpk.h"

HFPK	hFpk;	// ハンドル

/*
 *	ハンドルの初期化
 *
 *	*fName	アーカイブファイル名
 *
 *	return	TRUE / FALSE
 */
BOOL fpk_InitHandle(char *fName)
{
	if ((hFpk = fpkOpenArchive(fName)) == NULL) {
		return FALSE;	// 失敗
	}

	return TRUE;	// 成功
}

/*
 *	par形式のハンドルからエントリを開く
 *
 *	*fName	エントリのファイル名
 *	*len	エントリのサイズを返すポインタ（NULLなら返さない）
 *	*pDst	出力先（NULLなら新しくヒープを確保して返す）
 *
 *	return	エントリのデータ（pceHeapAlloc()で確保）
 */
BYTE *fpk_getEntryData(char *fName, DWORD *len, BYTE *pDst)
{
	BYTE		*ret = NULL;
	FPKENTRY	fpkEntry;

	if (fpkGetFileInfoS(hFpk, fName, &fpkEntry)) {
		if (len != NULL) {
			if (fpkEntry.szFileName[15] & 0x80) {	// 圧縮エントリか？
				//pceFileReadPos(&hFpk->pfa, (BYTE *)len, fpkEntry.dwOffset, 4);
				//↓{{2005/06/12 Naoyuki Sawa}}
				// yui: 2005.07.03 pceFileReadPos() から改名
				fpkFileReadPos(hFpk, (BYTE *)len, fpkEntry.dwOffset, 4);
			} else {
				*len = fpkEntry.dwSize;
			}
		}
		ret = fpkExtractToBuffer(hFpk, &fpkEntry, pDst);
	}

	return ret;
}

BOOL fpk_getFileInfoS(char *fName, FPKENTRY *fpkEntry)
{
	return fpkGetFileInfoS(hFpk, fName, fpkEntry);
}

DWORD fpk_getEntryDataPos(const FPKENTRY *fpkEntry, BYTE *pDst, DWORD offset, DWORD size)
{
	DWORD	ret = 0;

	if (offset < fpkEntry->dwSize) {
		ret = (offset + size <= fpkEntry->dwSize)? size : (fpkEntry->dwSize - offset);
		fpkFileReadPos(hFpk, pDst, fpkEntry->dwOffset + offset, ret);
	}
	return ret;
}

/*
 *	par形式のハンドルからエントリの一部分を開く
 *
 *	*fName	エントリのファイル名
 *	*pDst	出力先
 *	offset
 *	size
 *
 *	return 読み込んだ長さ
 */
DWORD fpk_getEntryDataPosEx(char *fName, BYTE *pDst, DWORD offset, DWORD size)
{
	FPKENTRY	fpkEntry;

	if (fpkGetFileInfoS(hFpk, fName, &fpkEntry)) {
		return fpk_getEntryDataPos(&fpkEntry, pDst, offset, size);
	}
	return 0;
}

/*
 *	par形式のハンドルを解放する
 */
void fpk_ReleaseHandle(void)
{
	fpkCloseArchive(hFpk);
}
