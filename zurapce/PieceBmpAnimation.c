#include "PieceBmpAnimation.h"

/** @file
	PIECE_BMP を使ったアニメーション関連の実装.
	@author zurachu
*/

void PieceBmpAnimation_Start( PieceBmpAnimation* p
	, UnitedPieceBmp* bmp, int start, int end, int update_period, BOOL loop )
{
	p->playing = bmp;
	p->current_index = p->start_index = start;
	p->end_index = end;
	p->delta = 0;
	p->update_period = update_period;
	p->loop = loop;
}

void PieceBmpAnimation_StartToEnd( PieceBmpAnimation* p
	, UnitedPieceBmp* bmp, int update_period, BOOL loop )
{
	PieceBmpAnimation_Start( p, bmp, 0, UnitedPieceBmp_NumDivision( bmp )
		, update_period, loop );
}

void PieceBmpAnimation_Update(PieceBmpAnimation* p, int delta )
{
	if( PieceBmpAnimation_Playing( p ) )
	{
		p->delta += delta;
		p->current_index += p->delta / p->update_period;
		p->delta = p->delta % p->update_period;
		if( p->loop )
		{
			int const one_loop = p->end_index - p->start_index + 1;
			while( p->end_index < p->current_index )
			{
				p->current_index -= one_loop;
			}
		}
		else
		{
			if( p->end_index < p->current_index )
			{
				p->current_index = p->end_index;
			}
		}
	}
}

UnitedPieceBmp* PieceBmpAnimation_Playing( PieceBmpAnimation const* p )
{
	return p->playing;
}

BOOL PieceBmpAnimation_IsEnd( PieceBmpAnimation const* p )
{
	return !PieceBmpAnimation_Playing( p )
		|| ( !p->loop && p->current_index == p->end_index );
}

int PieceBmpAnimation_Draw( PieceBmpAnimation const* p
	, int dx, int dy, int param )
{
	UnitedPieceBmp* const playing = PieceBmpAnimation_Playing( p );
	if( playing )
	{
		return UnitedPieceBmp_Draw( playing, dx, dy
			, p->current_index, param );
	}
	return 0;
}

void PieceBmpAnimation_Clear( PieceBmpAnimation* p )
{
	p->playing = NULL;
}
