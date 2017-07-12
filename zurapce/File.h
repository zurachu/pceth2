#if !defined( ZURAPCE_FILE_H )
#define ZURAPCE_FILE_H

/** @file
	ファイル 関連.
	@author zurachu
*/

#include <piece.h> // FILEACC

/**
	ファイルの一部分を読み込んでバッファに格納.
	@param dst 出力先バッファ
	@param file_acc 開かれたファイル
	@param pos 読み込み開始位置
	@param size 読み込みサイズ（INVALIDVAL で最後まで読み込む）
	@return 読み込みサイズ（失敗時０）
*/
int FileAcc_ReadPosTo( unsigned char* dst, FILEACC* file_acc, int pos, int size );

/**
	ファイルの一部分をサイズ分のヒープを確保して読み込み.
	@param file_acc 開かれたファイル
	@param pos 読み込み開始位置
	@param size 読み込みサイズ（INVALIDVAL で最後まで読み込む）
	@return 読み込んだヒープ（失敗時 NULL）
	@warning 不要になったヒープは pceHeapAlloc() で解放すること。
*/
unsigned char* FileAcc_ReadPosAlloc( FILEACC* file_acc, int pos, int size );

/**
	ファイルを読み込んでバッファに格納.
	FileAcc_ReadPosTo( dst, file_acc, 0, INVALIDVAL ) と等価。
	@param dst 出力先バッファ
	@param file_acc 開かれたファイル
	@return 読み込みサイズ（失敗時０）
	@see FileAcc_ReadPosTo()
*/
int FileAcc_ReadTo( unsigned char* dst, FILEACC* file_acc );

/**
	ファイルサイズ分のヒープを確保して読み込み.
	FileAcc_ReadPosAlloc( file_acc, 0, INVALIDVAL ) と等価。
	@param file_acc 開かれたファイル
	@return 読み込んだヒープ（失敗時 NULL）
	@warning 不要になったヒープは pceHeapAlloc() で解放すること。
	@see FileAcc_ReadPosAlloc()
*/
unsigned char* FileAcc_ReadAlloc( FILEACC* file_acc );

/**
	ファイルを読み込んでバッファに格納.
	@param dst 出力先バッファ
	@param filename ファイル名
	@return 読み込みサイズ（失敗時０）
	@see FileAcc_ReadTo()
*/
int File_ReadTo( unsigned char* dst, char const* filename );

/**
	ファイルサイズ分のヒープを確保して読み込み.
	@param filename ファイル名
	@return 読み込んだヒープ（失敗時 NULL）
	@warning 不要になったヒープは pceHeapAlloc() で解放すること。
	@see FileAcc_ReadAlloc()
*/
unsigned char* File_ReadAlloc( char const* filename );

#endif // !defined( ZURAPCE_FILE_H )
