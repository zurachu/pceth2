#ifndef __PCETH2_SAV_H_
#define __PCETH2_SAV_H_

#include "common.h"

BOOL _load();
BOOL _save();


//=============================================================================
//	グローバルセーブ
//=============================================================================

BOOL pceth2_readGlobalSaveData();
BOOL pceth2_writeGlobalSaveData();

void pceth2_drawTitleGraphic();
void pceth2_TitleInit();
void pceth2_Title();
void pceth2_Init();

int pceth2_backTitle(SCRIPT_DATA *s);


void pceth2_SaveInit();
void pceth2_SaveMenu();

#endif
