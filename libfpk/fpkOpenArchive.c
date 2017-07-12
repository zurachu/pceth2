/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/
#include "libfpk_impl.h"

HFPK fpkOpenArchive(LPSTR lpszFileName)
{
	PFPK fpk;

	fpk = pceHeapAlloc(sizeof(FPK));
	if(!fpk)	return NULL;
	/*{{2005/06/12 Naoyuki Sawa*/
	memset(fpk, 0, sizeof(FPK));
	fpk->sct = -1;
	/*}}2005/06/12 Naoyuki Sawa*/

	if(pceFileOpen(&fpk->pfa, lpszFileName, FOMD_RD) != 0)
	{
		pceHeapFree(fpk);
		return NULL;
	}

	//pceFileReadPos(&fpk->pfa, (void *)&fpk->fpkHeader, 0, sizeof(FPKHEADER));
	//«{{2005/06/12 Naoyuki Sawa}}
// yui: 2005.07.03 fpkFileReadPos() ‚Ì‰ü–¼‚É”º‚¤•ÏX
	fpkFileReadPos(fpk, (void*)&fpk->fpkHeader, 0, sizeof(FPKHEADER));

	/*{{2005/06/12 Naoyuki Sawa*/
	{
		FPKENTRY entry;
		int i;

		fpk->crc = pceHeapAlloc(sizeof(unsigned short) * fpk->fpkHeader.dwFilesAmount);
		if(!fpk->crc) {
			pceHeapFree(fpk);
			return NULL;
		}

		for(i = 0; i < fpk->fpkHeader.dwFilesAmount; i++) {
			fpkFileReadPos(fpk, (void*)&entry, sizeof(FPKHEADER) + sizeof(FPKENTRY) * i, sizeof(FPKENTRY));
			fpk->crc[i] = fpkFileNameCRC(entry.szFileName);
		}
	}
	/*}}2005/06/12 Naoyuki Sawa*/

	return fpk;
}

/*{{2005/06/12 Naoyuki Sawa*/
#include <ctype.h>
int fpkStrCaseCmp(const char* s1, const char* s2)
{
	int c1;
	int c2;

	do {
		c1 = tolower((unsigned char)*s1++);
		c2 = tolower((unsigned char)*s2++);
	} while((c1 != '\0') && (c1 == c2));

	return c1 - c2;
}
int fpkFileNameCRC(const char* fname)
{
	int crc = pceCRC32(NULL, 0);
	int c;

	while((c = tolower(*fname++)) != '\0') {
		crc = pceCRC32(&c, 1);
	}

	return (unsigned short)(crc ^ (crc >> 16)); /* ƒƒ‚ƒŠß–ñ‚Ì‚½‚ß */
}
/*}}2005/06/12 Naoyuki Sawa*/
