#if !defined( ZURAPCE_FILE_PACK_H )
#define ZURAPCE_FILE_PACK_H

/** @file
	ファイルパック 関連.
	P/ECE 開発環境付属の FilePack.exe で生成したパックデータからファイルを抽出します。
	@author zurachu
*/

#include <piece.h> // FILEACC

/// パックファイル読み込みハンドル
struct FilePackHandle
{
	FILEACC file_acc; ///< ファイル構造体
	int file_amount; ///< ファイル数
	/**
		16bit CRC.
		ファイル探索時、まずファイル名から取得した CRC が同一か比較し、
		同一なら実際にファイル名を比較することで、読み込みの高速化を図る。
		ハンドルを開くと、ファイル数×2バイト確保されます。
		（Autch氏の libfpk に pceth2 で nsawa氏が施した改良を取り込み）
	*/
	unsigned short* crc;
};
typedef struct FilePackHandle FilePackHandle;

/**
	ハンドルを開く.
	@param handle ハンドル
	@param filename パックファイル名
	@retval 0 正常終了
	@retval 1 パックファイルが無い
*/
int FilePackHandle_Open( FilePackHandle* handle, char const* filename );

/**
	ハンドルを閉じる.
	@param handle ハンドル
	@retval 0 常に正常終了
*/
int FilePackHandle_Close( FilePackHandle* handle );

/**
	パックファイルから１ファイルを読み込んでバッファに格納.
	@param dst 出力先バッファ
	@param handle ハンドル
	@param filename ファイル名
	@return 読み込みサイズ（失敗時０）
	@see FileAcc_ReadPosTo()
*/
int FilePackHandle_ReadTo( unsigned char* dst, FilePackHandle* handle, char const* filename );

/**
	ファイルサイズ分のヒープを確保して読み込み.
	@param handle ハンドル
	@param filename ファイル名
	@return 読み込んだヒープ（失敗時 NULL）
	@warning 不要になったヒープは pceHeapAlloc() で解放すること。
	@see FileAcc_ReadPosAlloc()
*/
unsigned char* FilePackHandle_ReadAlloc( FilePackHandle* handle, char const* filename );

/**
	メモリ上のパックデータから、指定ファイルへのポインタを取得.
	デコードsrc の FPK_FindPackData() と等価。
	@param filename ファイル名
	@param source パックデータ
	@return 指定ファイルへのポインタ（失敗時 NULL）
*/
unsigned char* FilePack_Data( char const* filename, unsigned char* source );

#endif // !defined( ZURAPCE_FILE_PACK_H )
