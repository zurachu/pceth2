#ifndef __PCETH2_STR_H_
#define __PCETH2_STR_H_

#include "common.h"

BOOL pceth2_isKanji(SCRIPT_DATA *s);
int pceth2_getNum(SCRIPT_DATA *s);
void pceth2_strcpy(char *dst, SCRIPT_DATA *s, DWORD len);

#endif