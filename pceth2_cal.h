#ifndef __PCETH2_CAL_H_
#define __PCETH2_CAL_H_

#define START_MONTH	3
#define START_DAY	1

int pceth2_calenderInitEx(SCRIPT_DATA *s);

void pceth2_calenderInit();
void pceth2_calenderDrawCircle();

int pceth2_getDate(int month, int day);

#endif
