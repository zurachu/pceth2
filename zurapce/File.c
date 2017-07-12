#include "File.h"

/** @file
	ファイル関連の実装.
*/

#include <piece.h>
#include <string.h>

#define SECTOR_SIZE (4096)

int FileAcc_ReadPosTo( unsigned char* dst, FILEACC* file_acc, int pos, int size )
{
	int total_size = 0;
	int sct = pos / SECTOR_SIZE;
	int const top_sct_offset = pos % SECTOR_SIZE;
	if( size == INVALIDVAL ) { size = file_acc->fsize - pos; }
	
	if( top_sct_offset > 0 )
	{
		int top_sct_size = SECTOR_SIZE - top_sct_offset;
		if( size < top_sct_size ) { top_sct_size = size; }
		pceFileReadSct( file_acc, NULL, sct++, top_sct_size );
		memcpy( dst, file_acc->aptr + top_sct_offset, top_sct_size );
		total_size += top_sct_size;
		dst += top_sct_size;
	}
	while( total_size < size )
	{
		int read_size = ( total_size + SECTOR_SIZE <= size )
						? SECTOR_SIZE
						: size - total_size;
		read_size = pceFileReadSct( file_acc, dst, sct++, read_size );
		if( !read_size ) { break; } // 念の為
		total_size += read_size;
		dst += SECTOR_SIZE;
	}
	return total_size;
}

unsigned char* FileAcc_ReadPosAlloc( FILEACC* file_acc, int pos, int size )
{
	unsigned char* const p = pceHeapAlloc( ( size != INVALIDVAL )
											? size
											: file_acc->fsize - pos );
	if( p )
	{
		if( FileAcc_ReadPosTo( p, file_acc, pos, size ) > 0 )
		{
			return p;
		}
		pceHeapFree( p );
	}
	return NULL;
}

int FileAcc_ReadTo( unsigned char* dst, FILEACC* file_acc )
{
	return FileAcc_ReadPosTo( dst, file_acc, 0, INVALIDVAL );
}

unsigned char* FileAcc_ReadAlloc( FILEACC* file_acc )
{
	return FileAcc_ReadPosAlloc( file_acc, 0, INVALIDVAL );
}

int File_ReadTo( unsigned char* dst, char const* filename )
{
	FILEACC file_acc;
	if( pceFileOpen( &file_acc, filename, FOMD_RD ) == 0 )
	{
		int const size = FileAcc_ReadTo( dst, &file_acc );
		pceFileClose( &file_acc );
		return size;
	}
	return 0;
}

unsigned char* File_ReadAlloc( char const* filename )
{
	FILEACC file_acc;
	if( pceFileOpen( &file_acc, filename, FOMD_RD ) == 0 )
	{
		unsigned char* const p = FileAcc_ReadAlloc( &file_acc );
		pceFileClose( &file_acc );
		return p;
	}
	return NULL;
}

