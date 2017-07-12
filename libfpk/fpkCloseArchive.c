/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/
#include "libfpk_impl.h"

VOID fpkCloseArchive(HFPK hFpk)
{
	pceHeapFree(hFpk->crc); /*{{2005/06/12 Naoyuki Sawa}}*/
	pceFileClose(&hFpk->pfa);
	pceHeapFree(hFpk);
}
