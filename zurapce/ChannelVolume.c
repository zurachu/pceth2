#include "ChannelVolume.h"

/** @file
	各 ch の音量フェード処理関連.
	@author zurachu
*/

#include <piece.h>
#include "LinearParameter.h"

#define VOL_MAX (0)
#define VOL_MIN (127)

#define CH_NUM (4)
static LinearParameter s_volume[ CH_NUM ];

void ChannelVolume_Init( void )
{
	int ch;
	for( ch = 0; ch < CH_NUM; ch++ )
	{
		pceWaveSetChAtt( ch, VOL_MAX );
		LinearParameter_Set( &s_volume[ ch ], VOL_MAX, VOL_MAX, 0 );
	}
}

void ChannelVolume_Exit( void )
{
	int ch;
	for( ch = 0; ch < CH_NUM; ch++ )
	{
		pceWaveSetChAtt( ch, VOL_MAX );
	}
}

void ChannelVolume_Update( void )
{
	int ch;
	for( ch = 0; ch < CH_NUM; ch++ )
	{
		LinearParameter_Update( &s_volume[ ch ] );
		pceWaveSetChAtt( ch, LinearParameter_Get( &s_volume[ ch ] ) );
	}
}

void ChannelVolume_Fade( int ch, int vol, int fade_time )
{
	if( 0 <= ch && ch < CH_NUM )
	{
		int const current_att = pceWaveSetChAtt( ch, INVALIDVAL );
		LinearParameter_Set( &s_volume[ ch ], current_att, vol, fade_time );
	}
}

void ChannelVolume_FadeIn( int ch, int fade_time )
{
	pceWaveSetChAtt( ch, VOL_MIN );
	ChannelVolume_Fade( ch, VOL_MAX, fade_time );
}

void ChannelVolume_FadeOut( int ch, int fade_time )
{
	ChannelVolume_Fade( ch, VOL_MIN, fade_time );
}

