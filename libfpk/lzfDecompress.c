/*
	* libfpk -- using FPK archive on P/ECE
	* 	By Yui N., 2003 - 2005.
	* http://www.autch.net/
*/

#include "libfpk_impl.h"

// yui: 2005.07.19: LZF 関連のライセンス条文はこのファイルの最後
// yui: from lzf_d.c v1.5 (LZF_VERSION == 0x0105)
DWORD lzfDecompress(HFPK hFpk, DWORD dwOffset, int dwSourceSize, BYTE* pDst, int dwDestSize)
{
  BYTE       *op = (BYTE *)pDst;
  BYTE       *const out_end = pDst + dwDestSize;

// yui: 2005.07.05
#if 1
// yui: 2005.07.03 fpkFileReadPos() の改名に伴う変更
/*---------- 単純な実装 ----------*/
#define FETCH(v) /*do*/ {				\
	unsigned char c;			\
	if(!dwSourceSize) {				\
		return;				\
	}					\
	fpkFileReadPos(hFpk, &c, dwOffset, 1);	\
	dwOffset++;					\
	dwSourceSize--;					\
	(v) = c;				\
} /*while(0)*/
#else
/*---------- 高速な実装 ----------*/
#define FETCH_SIZE 32 /*調整可*/
	unsigned char fetch_buf[FETCH_SIZE];
	int fetch_pos = FETCH_SIZE;
#define FETCH(v) {							\
	if(fetch_pos == FETCH_SIZE) {					\
		int fetch_len = FETCH_SIZE;					\
		if(fetch_len > dwSourceSize) {					\
			fetch_len = dwSourceSize;					\
			if(!fetch_len) {					\
				return;					\
			}						\
		}							\
		fetch_pos = FETCH_SIZE - fetch_len;				\
		fpkFileReadPos(hFpk, &fetch_buf[fetch_pos], dwOffset, fetch_len);	\
		dwOffset  += fetch_len;						\
		dwSourceSize -= fetch_len;						\
	}								\
	(v) = fetch_buf[fetch_pos];					\
	fetch_pos++;							\
}
#endif


  do
    {
      unsigned int ctrl;
      FETCH(ctrl);

      if (ctrl < (1 << 5)) /* literal run */
        {
          ctrl++;

          if (op + ctrl > out_end)
            {
              return 0;
            }

// yui: 2005.07.03: force using FETCH()
//#if USE_MEMCPY
//          memcpy (op, ip, ctrl);
//          op += ctrl;
//          ip += ctrl;
//#else
          do
          {
            FETCH(*op++);
          }
          while (--ctrl);
//#endif
        }
      else /* back reference */
        {
          unsigned int len = ctrl >> 5;
          unsigned int i;

          BYTE *ref = op - ((ctrl & 0x1f) << 8) - 1;

          if (len == 7)
          {
            FETCH(i);
            len += i;
          }

          FETCH(i);
          ref -= i;

          if (op + len + 2 > out_end)
            {
              return 0;
            }

          if (ref < (BYTE *)pDst)
            {
              return 0;
            }

          *op++ = *ref++;
          *op++ = *ref++;

          do
            *op++ = *ref++;
          while (--len);
        }
    }
  while (op < out_end && dwSourceSize > 0); // > 0 不要？

  return op - (BYTE *)pDst;
}

/*
 * Copyright (c) 2000-2005 Marc Alexander Lehmann <schmorp@schmorp.de>
 * 
 * Redistribution and use in source and binary forms, with or without modifica-
 * tion, are permitted provided that the following conditions are met:
 * 
 *   1.  Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 * 
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 *   3.  The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MER-
 * CHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPE-
 * CIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTH-
 * ERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the GNU General Public License version 2 (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of the above. If you wish to
 * allow the use of your version of this file only under the terms of the
 * GPL and not to allow others to use your version of this file under the
 * BSD license, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the GPL. If
 * you do not delete the provisions above, a recipient may use your version
 * of this file under either the BSD or the GPL.
 */
