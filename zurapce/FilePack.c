#include "FilePack.h"

/** @file
	ファイルパック関連の実装.
	@author zurachu
*/

#include <piece.h>
#include <string.h>
#include <ctype.h>
#include "File.h"

/* 以下、デコーダsrc/filepack.c から引用 */

/// パックファイル本体の情報
struct FILE_PAC_INFO
{
	unsigned long	head;		///< パックヘッダ
	long			famount;	///< ファイル数
};
typedef struct FILE_PAC_INFO FILE_PAC_INFO;

/// パックされた各ファイルの情報
struct FILE_INFO
{
	char			fname[16];	///< ファイル名（拡張子、'\0'含みで16文字まで）
	unsigned long	offset;		///< オフセットアドレス
	unsigned long	size;		///< ファイルサイズ
};
typedef struct FILE_INFO FILE_INFO;

/**
	大文字小文字を無視した文字列比較.
*/
int strcmpi( char const* str1, char const* str2 )
{
	int c1, c2;
	do {
		c1 = tolower( *str1++ );
		c2 = tolower( *str2++ );
	} while( ( c1 != '\0' ) && ( c1 == c2 ) );
	return c1 - c2;
}

/**
	大文字小文字を無視した 32bit CRC をとり、16bit に非可逆圧縮.
*/
static int crc16i( char const* str )
{
	int crc = pceCRC32( NULL, 0 );
	int c;
	while( ( c = tolower( *str++ ) ) != '\0' )
	{
		crc = pceCRC32( &c, 1 );
	}
	return (unsigned short)( crc ^ (crc >> 16) );
}

/**
	パックデータのヘッダ部分を解析し、ファイル数を返す.
	@param file_pac_info パックデータ先頭
	@return ファイル数（失敗時 -1）
*/
static int get_file_amount( FILE_PAC_INFO const* file_pac_info )
{
	if( !strncmp( &file_pac_info->head, "KAPF", 4 ) )
	{
		return file_pac_info->famount;
	}
	return -1;
}

/**
	パックデータからファイル情報を読み込み.
	@param dst 格納先
	@param handle ハンドル
	@param index インデックス番号
*/
static int get_file_info_to( FILE_INFO* dst, FilePackHandle* handle, int index )
{
	return FileAcc_ReadPosTo( (unsigned char*)dst, &handle->file_acc
							, sizeof(FILE_PAC_INFO) + index * sizeof(FILE_INFO)
							, sizeof(FILE_INFO) );
}

/**
	パックデータから指定ファイルを検索し、情報を取得.
	@param dst 格納先
	@param handle ハンドル
	@param filename ファイル名
	@return インデックス番号（失敗時 -1）
*/
static int find_file_info_to( FILE_INFO* dst, FilePackHandle* handle, char const* filename )
{
	int crc = crc16i( filename );
	int i;
	for( i = 0; i < handle->file_amount; i++ )
	{
		if( crc == handle->crc[ i ] )
		{
			FILE_INFO file_info;
			get_file_info_to( &file_info, handle, i );
			if( !strcmpi( file_info.fname, filename ) )
			{
				memcpy( dst, &file_info, sizeof(FILE_INFO) );
				return i;
			}
		}
	}
	return -1;
}

int FilePackHandle_Open( FilePackHandle* handle, char const* filename )
{
	if( pceFileOpen( &handle->file_acc, filename, FOMD_RD ) == 0 )
	{
		pceFileReadSct( &handle->file_acc, NULL, 0, sizeof(FILE_PAC_INFO) );
		handle->file_amount = get_file_amount( (FILE_PAC_INFO*)(handle->file_acc.aptr) );
		if( handle->file_amount > 0 )
		{
			int i;
			handle->crc = (unsigned short*)pceHeapAlloc( handle->file_amount * sizeof(unsigned short) );
			if( handle->crc )
			{
				for( i = 0; i < handle->file_amount; i++ )
				{
					FILE_INFO file_info;
					get_file_info_to( &file_info, handle, i );
					handle->crc[ i ] = crc16i( file_info.fname );
				}
				return 0;
			}
		}
		pceFileClose( &handle->file_acc );
	}
	return 1;
}

int FilePackHandle_Close( FilePackHandle* handle )
{
	pceHeapFree( handle->crc );
	return pceFileClose( &handle->file_acc );
}

int FilePackHandle_ReadTo( unsigned char* dst, FilePackHandle* handle, char const* filename )
{
	FILE_INFO file_info;
	if( find_file_info_to( &file_info, handle, filename ) >= 0 )
	{
		return FileAcc_ReadPosTo( dst, &handle->file_acc, file_info.offset, file_info.size );
	}
	return 0;
}

unsigned char* FilePackHandle_ReadAlloc( FilePackHandle* handle, char const* filename )
{
	FILE_INFO file_info;
	if( find_file_info_to( &file_info, handle, filename ) >= 0 )
	{
		return FileAcc_ReadPosAlloc( &handle->file_acc, file_info.offset, file_info.size );
	}
	return 0;
}

unsigned char* FilePack_Data( char const* filename, unsigned char* source )
{
	int const file_amount = get_file_amount( (FILE_PAC_INFO*)source );
	FILE_INFO* file_info = (FILE_INFO*)( source + sizeof(FILE_PAC_INFO) );
	int i;
	for( i = 0; i < file_amount; i++ )
	{
		if( !strcmpi( file_info->fname, filename ) )
		{
			return source + file_info->offset;
		}
		file_info++;
	}
	return NULL;
}

