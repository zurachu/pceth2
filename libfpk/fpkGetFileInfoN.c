/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/

#include "libfpk_impl.h"

BOOL fpkGetFileInfoN(HFPK hFpk, DWORD dwFileIndex, FPKENTRY *lpFileEntry)
{
	DWORD dwCount = 0, dwBytesRead = 0;
	FPKENTRY fpkEntry;

	if(hFpk == NULL || lpFileEntry == NULL)
		return FALSE;

	if(dwFileIndex >= hFpk->fpkHeader.dwFilesAmount)
		return FALSE;

	//pceFileReadPos(&hFpk->pfa, (void *)&fpkEntry, sizeof(FPKHEADER) + sizeof(FPKENTRY) * dwFileIndex, sizeof(FPKENTRY));
	//Å´{{2005/06/12 Naoyuki Sawa}}
	// yui: 2005.07.03 fpkFileReadPos() ÇÃâ¸ñºÇ…î∫Ç§ïœçX
	fpkFileReadPos(hFpk, (void *)&fpkEntry, sizeof(FPKHEADER) + sizeof(FPKENTRY) * dwFileIndex, sizeof(FPKENTRY));
	*lpFileEntry = fpkEntry;
	return TRUE;
}
