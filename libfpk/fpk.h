/////////////////////////////////////////////
// fpk.h -- extended fpk archive format
/////////////////////////////////////////////

#ifndef FPK_H
#define FPK_H

//  reference: /usr/piece/tools/filepack/デコーダsrc/filepack.c
//  original source codes are copyrighted as following:
//    Copyright:  AQUA (Leaf)  since 2001 - Programd by.  Miyakusa Masakazu

#ifndef VOID
typedef void					VOID;
#endif // !VOID

typedef char *				LPSTR;

typedef struct
{
  DWORD dwHeader;                         //  パックヘッダ
  DWORD dwFilesAmount;                    //  ファイル数
}FPKHEADER;

typedef struct
{
  CHAR  szFileName[16];                   //  ファイル名（拡張子、'\0'含みで16文字まで）
  DWORD dwOffset;                         //  オフセットアドレス
  DWORD dwSize;                           //  ファイルサイズ
}FPKENTRY;

// Yui N. による拡張
// 実際のコード中ではこの型は使われず，この型であるかのように FPKENTRY を使う。
typedef struct
{
  BYTE szFileName[15];                    // ファイル名（ヌル文字含めて 15 文字まで）
  BYTE bCompressed;                       // 拡張圧縮情報（後述）
  DWORD dwOffset;                         // オフセットアドレス（DWORD 境界）
  DWORD dwSize;                           // 圧縮後のファイルサイズ。圧縮前のファイルサイズは
                                          // データの先頭 4 bytes に埋め込まれている。
}FPKENTRY_C;

#define FPK_NO_COMPRESSION    0x00    // 0000 0000

// yui: 2005.07.05
#ifndef LIBFPK_NO_LZSS
#define FPK_LZSS_COMPRESSION  0x80    // 1000 0000
#endif // !LIBFPK_NO_LZSS

#ifndef LIBFPK_NO_ZLIB                // yui: 2005.07.03
#define FPK_ZLIB_COMPRESSION  0xc0    // 1100 0000 /*{{2005/06/23 Naoyuki Sawa}}*/
// yui: 2005.03.21 pva block header
#define PVNSPACK_HEADER 0x1c0258
#endif // !LIBFPK_NO_ZLIB

// yui: 2005.07.03 
#define FPK_LZF_COMPRESSION   0xe0    // 1110 0000


// 拡張圧縮情報について
// --------------------
// そのファイルが圧縮されているか否かを調べるには，
// bCompressed の最上位ビットを調べることで求められる。
// 現状ではそのほかのビットは使用しない。将来の拡張に備え 0 にすること。

///////////////////////////////////////////////////////////

#endif /* !FPK_H */
