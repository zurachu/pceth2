/*
 *	pceth2map
 *	特定の16階調PGD変換用のグレースケールBMPを合成して、マップ選択用の画像を生成します。
 *
 *	(c)2017 zurachu
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

bool open_bmp(char const* filename, BITMAPFILEHEADER* out_bf
	, BITMAPINFOHEADER& out_bi, RGBQUAD* out_rgb, BYTE*& out_bmp);
BYTE* resize128(BYTE* bmp, BITMAPINFOHEADER* bi);
void combine(BYTE* dst,  BITMAPINFOHEADER const& dst_bi
	, BYTE const* src, BITMAPINFOHEADER const& src_bi);
void combine_with_mask(BYTE* dst, BITMAPINFOHEADER const& dst_bi
	, BYTE const* src, BITMAPINFOHEADER const& src_bi
	, int offset_x, int offset_y);

int main(int argc, char *argv[])
{
	FILE	*fp = NULL;
	BITMAPFILEHEADER	bf;
	BITMAPINFOHEADER	map_bi, thum_a_bi, thum_m_bi, b_bi;
	RGBQUAD	rgb[256];
	BYTE	*map_bmp = NULL, *thum_a_bmp = NULL, *thum_m_bmp = NULL, *b_bmp = NULL;
	int	i;

	if(argc != 6) {	// usage
		fprintf(stderr, "ToHeart2 マップ選択用画像合成\n");
		fprintf(stderr, "pceth2map out_filename MAPxxxx.bmp THUMxxA.bmp THUMxxM.bmp BxxxxxxT.bmp\n");
		return 1;
	}

	i = 2; if (!open_bmp(argv[i], &bf, map_bi, rgb, map_bmp)) goto ERR;
	map_bmp = resize128(map_bmp, &map_bi);
	i = 3; if (!open_bmp(argv[i], NULL, thum_a_bi, NULL, thum_a_bmp)) goto ERR;
	i = 4; if (!open_bmp(argv[i], NULL, thum_m_bi, NULL, thum_m_bmp)) goto ERR;
	i = 5; if (!open_bmp(argv[i], NULL, b_bi, NULL, b_bmp)) goto ERR;

	combine_with_mask(thum_m_bmp, thum_m_bi, b_bmp, b_bi, 6, 10);
	combine(map_bmp, map_bi, thum_a_bmp, thum_a_bi);
	combine(map_bmp, map_bi, thum_m_bmp, thum_m_bi);

	i = 1; if ((fp = fopen(argv[i], "wb")) == NULL)	goto ERR;
	fwrite(&bf, 1, sizeof(BITMAPFILEHEADER), fp);
	fwrite(&map_bi, 1, sizeof(BITMAPINFOHEADER), fp);
	fwrite(rgb, 1, sizeof(RGBQUAD) * 256, fp);
	fwrite(map_bmp, 1, map_bi.biWidth * map_bi.biHeight, fp);
	printf("%s - 成功しました。\n", argv[i]);
	goto FREE;
ERR:
		fprintf(stderr, "%s - 失敗しました。\n", argv[i]);
FREE:
	if(fp != NULL) fclose(fp);
	if(b_bmp != NULL) free(b_bmp);
	if(thum_m_bmp != NULL) free(thum_m_bmp);
	if(thum_a_bmp != NULL) free(thum_a_bmp);
	if(map_bmp != NULL) free(map_bmp);
	return 0;
}

bool open_bmp(char const* filename, BITMAPFILEHEADER* out_bf
	, BITMAPINFOHEADER& out_bi, RGBQUAD* out_rgb, BYTE*& out_bmp)
{
	FILE	*fp = NULL;
	BITMAPFILEHEADER	bf;
	BITMAPINFOHEADER	bi;
	RGBQUAD	rgb[256];
	BYTE	*bmp;

	if(out_bmp) goto ERR;
	if((fp = fopen(filename, "rb")) == NULL) goto ERR;
	// BITMAPFILEHEADER
	if(fread(&bf, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER)) goto ERR;
	if(strncmp((char*)&bf.bfType, "BM", 2)) goto ERR;
	// BITMAPINFOHEADER
	if(fread(&bi, 1, sizeof(BITMAPINFOHEADER), fp) != sizeof(BITMAPINFOHEADER)) goto ERR;
	if(bi.biSize != sizeof(BITMAPINFOHEADER)) goto ERR;
	if(bi.biBitCount != 8) goto ERR;
	fread(rgb, 1, sizeof(RGBQUAD) * 256, fp);
	// BITMAP本体（ボトムアップで読み込む）
	if((bmp = (BYTE*)malloc(bi.biWidth * abs(bi.biHeight))) == NULL) goto ERR;
	if(bi.biHeight < 0) {	// トップダウン
		bi.biHeight = -bi.biHeight;
		for(int y = bi.biHeight - 1; y >= 0; y--) {
			fread(bmp + bi.biWidth * y, 1, bi.biWidth, fp);
		}
	} else {				// ボトムアップ
		fread(bmp, 1, bi.biWidth * bi.biHeight, fp);
	}
	fclose(fp);
	if(out_bf) *out_bf = bf;
	out_bi = bi;
	if(out_rgb) memcpy(out_rgb, rgb, sizeof(RGBQUAD) * 256);
	out_bmp = bmp;
	return true;

ERR:
	if(fp != NULL) fclose(fp);
	return false;
}

BYTE* resize128(BYTE* bmp, BITMAPINFOHEADER* bi)
{
	static int const new_width = 128;
	int offset = 0, new_offset = (new_width - bi->biWidth) / 2;
	BYTE* new_bmp = (BYTE*)malloc(new_width * bi->biHeight);
	memset(new_bmp, 16, new_width * bi->biHeight);
	for (int y = 0; y < bi->biHeight; y++)
	{
		for (int x = 0; x < bi->biWidth; x++)
		{
			*(new_bmp + new_offset) = *(bmp + offset);
			new_offset++;
			offset++;
		}
		new_offset += new_width - bi->biWidth;
	}
	bi->biWidth = new_width;
	free(bmp);
	return new_bmp;
}

void combine(BYTE* dst,  BITMAPINFOHEADER const& dst_bi
	, BYTE const* src, BITMAPINFOHEADER const& src_bi)
{
	for(int y = 0; y < src_bi.biHeight; y++)
	{
		for(int x = 0; x < src_bi.biWidth; x++)
		{
			if(*src < 16)
			{
				*dst = *src;
			}
			dst++;
			src++;
		}
		dst += dst_bi.biWidth - src_bi.biWidth;
	}
}

void combine_with_mask(BYTE* dst, BITMAPINFOHEADER const& dst_bi
	, BYTE const* src, BITMAPINFOHEADER const& src_bi
	, int offset_x, int offset_y)
{
	dst += offset_y * dst_bi.biWidth + offset_x;
	for(int y = 0; y < src_bi.biHeight; y++)
	{
		for(int x = 0; x < src_bi.biWidth; x++)
		{
			if(*dst == 0 && *src < 16)
			{
				*dst = *src;
			}
			dst++;
			src++;
		}
		dst += dst_bi.biWidth - src_bi.biWidth;
	}
}
