#ifndef __PCETH2_SEL_H_
#define __PCETH2_SEL_H_

void pceth2_drawSelArrow();

int  pceth2_addSelItem(SCRIPT_DATA *s);
int  pceth2_initSelect(SCRIPT_DATA *s);
void pceth2_Select();

int pceth2_addMapItem(SCRIPT_DATA *s);
int pceth2_initMapSelect();
void pceth2_MapSelect();

#endif
