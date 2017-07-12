#include "PrecisionTimer.h"

/** @file
	高精度タイマー関連の実装.
	@author zurachu
*/

#include <piece.h>

void PrecisionTimer_Construct( PrecisionTimer* p )
{
	p->count = pceTimerGetPrecisionCount();
}

unsigned long PrecisionTimer_Count( PrecisionTimer* p )
{
	unsigned long const now = pceTimerGetPrecisionCount();
	unsigned long const adjusted = pceTimerAdjustPrecisionCount( p->count, now );
	p->count = now;
	return adjusted;
}

