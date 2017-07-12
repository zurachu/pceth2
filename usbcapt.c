/*
** PceCapsëŒâûÉRÅ[Éh
**  programed by Ç‹Ç©Ç◊Ç–ÇÎÇµ( http://www.marchen.to/~piece/ )
*/

#include <piece.h>
#include "usbcapt.h"

typedef void (*PCEAPI_LCDTrans)();
typedef void (*PCEAPI_LCDTransDirect)( const unsigned char *lcd_direct );
typedef unsigned char *(*PCEAPI_LCDSetBuffer)( unsigned char *pbuff );

static PCEAPI_LCDTrans			orig_pceLCDTrans;
static PCEAPI_LCDTransDirect	orig_pceLCDTransDirect;
static PCEAPI_LCDSetBuffer		orig_pceLCDSetBuffer;

static unsigned char *pVramBuf;

static unsigned char *my_pceLCDSetBuffer( unsigned char *pbuff ) {
	pVramBuf = pbuff;
	return (*orig_pceLCDSetBuffer)( pbuff );
}

static void my_pceLCDTrans() {
	unsigned long *p = (unsigned long *)( 0x13d000 - 8 );
	p[0] = 0x43425355;
	p[1] = (unsigned long)pVramBuf;
	(*orig_pceLCDTrans)();
}

static void my_pceLCDTransDirect( const unsigned char *lcd_direct ) {
	unsigned long *p = (unsigned long *)( 0x13d000 - 8 );
	p[0] = 0x43425356;
	p[1] = (unsigned long)lcd_direct;
	(*orig_pceLCDTransDirect)( lcd_direct );
}

void usbCaptureInit() {
	orig_pceLCDTrans		= pceVectorSetKs( 18, my_pceLCDTrans );
	orig_pceLCDSetBuffer	= pceVectorSetKs( 19, my_pceLCDSetBuffer );
	orig_pceLCDTransDirect	= pceVectorSetKs( 22, my_pceLCDTransDirect );
}

void usbCaptureRelease() {
	pceVectorSetKs( 18, orig_pceLCDTrans );
	pceVectorSetKs( 19, orig_pceLCDSetBuffer );
	pceVectorSetKs( 22, orig_pceLCDTransDirect );
}

