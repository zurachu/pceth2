#include "PieceBmp.h"

/** @file
	PIECE_BMP ŠÖ˜A‚ÌŽÀ‘•.
	@author zurachu
*/

#include <piece.h>
#include <string.h>

void PieceBmp_Construct( PIECE_BMP* p, unsigned char* source )
{
	memcpy( &p->header, source, sizeof(PBMP_FILEHEADER) );
	p->buf = source + sizeof(PBMP_FILEHEADER);
	p->mask = p->buf + ( p->header.w * p->header.h * p->header.bpp >> 3 );
}

int PieceBmp_Draw( PIECE_BMP* p, int dx, int dy, int sx, int sy
				, int width, int height, int param )
{
	DRAW_OBJECT draw_object;
	pceLCDSetObject( &draw_object, p, dx, dy, sx, sy, width, height, param );
	return pceLCDDrawObject( draw_object );
}

void UnitedPieceBmp_Construct( UnitedPieceBmp* p
	, unsigned char* source, int width, int height )
{
	PieceBmp_Construct( &(p->bmp), source );
	p->width = width;
	p->height = height;
}

void UnitedPieceBmp_ConstructByDivision( UnitedPieceBmp* p
	, unsigned char* source, int divx, int divy )
{
	PIECE_BMP* const bmp = &(p->bmp);
	PieceBmp_Construct( bmp, source );
	p->width = bmp->header.w / divx;
	p->height = bmp->header.h / divy;
}

int UnitedPieceBmp_NumDivision( UnitedPieceBmp const* p )
{
	int x, y;
	UnitedPieceBmp_NumDivisionByXY( p, &x, &y );
	return x * y;
}

void UnitedPieceBmp_NumDivisionByXY( UnitedPieceBmp const* p, int* x, int* y )
{
	*x = p->bmp.header.w / p->width;
	*y = p->bmp.header.h / p->height;
}

int UnitedPieceBmp_Draw( UnitedPieceBmp* p
	, int dx, int dy, int index, int param )
{
	int const divx = p->bmp.header.w / p->width;
	return UnitedPieceBmp_DrawByXY( p, dx, dy
		, index % divx, index / divx, param );
}

int UnitedPieceBmp_DrawByXY( UnitedPieceBmp* p
	, int dx, int dy, int ix, int iy, int param )
{
	return PieceBmp_Draw( &(p->bmp), dx, dy
		, ix * p->width, iy * p->height
		, p->width, p->height, param );
}
