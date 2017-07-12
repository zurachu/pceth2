/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003.
	* http://www.autch.net/
*/

/*
	* reference: http://www.aw.wakwak.com/~hitode/piece/index.html#plz
	* author: Hitode Yamatsuki
	* for more information, see libfpk.txt
*/

#include "libfpk_impl.h"

//void hitodeLZSSDecode(BYTE *dst, BYTE *src, DWORD size)
//{
//	BYTE *base = src;
//
//	while((src - base) < size)		// yui: fpk のパディングバイトの関係でサイズチェックが必要
//	{
//		int ct = 8, flag = *src++;
//
//		do
//		{
//			if(flag & 0x80)
//			{
//				*dst++ = *src++;
//			}
//			else
//			{
//				unsigned int length, ptr = *src++;
//
//				length = (ptr >> 4) + 3;
//				if(!(ptr = ((ptr & 0xf) << 8) | (*src++)))
//					return;
//				{
//					unsigned char *rp = dst - ptr;
//					do
//					{
//						*dst++ = *rp++;
//					}while(--length);
//				}
//			}
//			flag <<= 1;
//		}while(--ct);
//	}
//}
//↓{{2005/06/24 Naoyuki Sawa}}
void hitodeLZSSDecode(HFPK hFpk, int ofs, int size, BYTE* dst)
{
#if 0
// yui: 2005.07.03 fpkFileReadPos() の改名に伴う変更
/*---------- 単純な実装 ----------*/
#define FETCH(v) do {				\
	unsigned char c;			\
	if(!size) {				\
		return;				\
	}					\
	fpkFileReadPos(hFpk, &c, ofs, 1);	\
	ofs++;					\
	size--;					\
	(v) = c;				\
} while(0)
#else
// yui: 2005.07.03 fpkFileReadPos() の改名に伴う変更
/*---------- 高速な実装 ----------*/
#define FETCH_SIZE 32 /*調整可*/
	unsigned char fetch_buf[FETCH_SIZE];
	int fetch_pos = FETCH_SIZE;
// yui: 2005.07.03 いまどきのコンパイラは複文のマクロを正しく処理できる
#define FETCH(v) /*do*/ {							\
	if(fetch_pos == FETCH_SIZE) {					\
		int len = FETCH_SIZE;					\
		if(len > size) {					\
			len = size;					\
			if(!len) {					\
				return;					\
			}						\
		}							\
		fetch_pos = FETCH_SIZE - len;				\
		fpkFileReadPos(hFpk, &fetch_buf[fetch_pos], ofs, len);	\
		ofs  += len;						\
		size -= len;						\
	}								\
	(v) = fetch_buf[fetch_pos];					\
	fetch_pos++;							\
} /*while(0)*/
#endif

	int ct;
	int flag;
	int length;
	int ptr;
	int tmp;
	unsigned char* rp;

	for(;;) {
		FETCH(flag);
		ct = 8;
		do {
			if(flag & 0x80) {
				FETCH(*dst++);
			} else {
				FETCH(ptr);
				FETCH(tmp);
				length = (ptr >> 4) + 3;
				ptr = ((ptr & 0xf) << 8) | tmp;
				if(!ptr) {
					return;
				}
				rp = dst - ptr;
				do
				{
					*dst++ = *rp++;
				} while(--length);
			}
			flag <<= 1;
		} while(--ct);
	}
}
