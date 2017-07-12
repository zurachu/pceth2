/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/

/*
	* reference: 「おでかけマルチ」のソースコードより piece_ex.c
	* original copyright: AQUA (Leaf)  since 2001 - 
	* original coder: Programd by.  Miyakusa Masakazu
*/

//#include "pceFileReadPos.h"
//↓{{2005/06/12 Naoyuki Sawa}}
// yui: 2005.07.19
//#include "libfpk.h"

#include "libfpk_impl.h"

//#define min(x, y)		((x < y) ? x : y)

//int pceFileReadPos(FILEACC *pfa, unsigned char *buf, int pos, int size)
//{
//	int work = pos;
//	int size2 = 0;
//
//	if(pos % 4096)
//	{
//		pceFileReadSct(pfa, NULL, work / 4096, 4096);
//		size2 += memcpy(buf, pfa->aptr + pos % 4096, min(4096 - pos % 4096, size));
//		work += min(4096 - pos % 4096, size);
//	}
//	while( pos+size > work )
//	{
//		pceFileReadSct(pfa, NULL, work / 4096, min(pos + size - work, 4096));
//		size2 += memcpy(buf + (work - pos), pfa->aptr, min(pos + size - work, 4096));
//		work += 4096;
//	}
//
//	return size2;
//}
//↓{{2005/06/12 Naoyuki Sawa}}
// yui: 2005.07.03 改名：シグナチャがもはや P/ECE 標準のそれではない
int fpkFileReadPos(HFPK hFpk, unsigned char* buf, int pos, int size)
{
	FILEACC* pfa = &hFpk->pfa;
	int rem = size;
	int sct;
	int len;

	sct = pos / 4096;
	pos %= 4096;
	while(rem > 0) {
		if(hFpk->sct != sct) {
			pceFileReadSct(pfa, hFpk->cache, sct, 4096);
			hFpk->sct = sct;
		}
		len = 4096 - pos;
		if(len > rem) {
			len = rem;
		}
		memcpy(buf, hFpk->cache + pos, len);
		buf += len;
		rem -= len;
		sct++;
		pos = 0;
	}

	return size;
}
//#undef min

