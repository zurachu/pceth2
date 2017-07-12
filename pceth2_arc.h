#ifndef __PCETH2_ARC_H_
#define __PCETH2_ARC_H_

#include "libfpk/libfpk.h"

BOOL fpk_InitHandle(char *fName);
BYTE *fpk_getEntryData(char *fName, DWORD *len, BYTE *pDst);
BOOL fpk_getFileInfoS(char *fName, FPKENTRY *fpkEntry);
DWORD fpk_getEntryDataPosEx(char *fName, BYTE *pDst, DWORD offset, DWORD size);
DWORD fpk_getEntryDataPos(FPKENTRY *fpkEntry, BYTE *pDst, DWORD offset, DWORD size);
void fpk_ReleaseHandle(void);

#endif