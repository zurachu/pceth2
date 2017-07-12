# Piece 用 makefile

# 拡張子の定義

.SUFFIXES:
.SUFFIXES:  .o .s .c

# 生成コマンド・オプションのマクロ

CC = pcc33
CFLAGS = -c -g -gp=0x0 -near -O2 -Wall -D__PCEFILE_API_HOOK__
AS = pcc33
ASFLAGS = -c -g -gp=0x0 -near
LD = pcc33
#LDFLAGS = -g -ls -lm

# 生成規則

.c.o:
	$(CC) $(CFLAGS) $<

.s.o:
	$(AS) $(ASFLAGS) $<

# 構成ファイル・生成ファイルのマクロ

PRGNAME = pceth2
FILENAME = pceth2
CAPTION = 好き好きタマお姉ちゃん
ICON = tamaki.pid
FRAM_OBJS = framtop.o fram_ld.o mmc_fram.o
OBJS = $(FRAM_OBJS) \
	ld.o pceth2_grp.o \
	mmc_api.o pceth2_arc.o pceth2_sav.o \
	pceth2_cal.o font_ex.o main.o pceth2_sys.o pceth2_msg.o pceth2_sel.o pceth2_str.o \
	pceth2_snd.o instdef2.o usbcapt.o
LIBS = libfpk\\libfpk.lib ufe\ufe.lib

LDFLAGS = -g -ls -lm \
		+codeblock FRAMC {$(FRAM_OBJS)} \
		+bssblock FRAMB {$(FRAM_OBJS)} \
		+addr 0x1000 {@FRAMC FRAMB} \
		+defsym _stacklen=0x1000

$(PRGNAME).srf : $(OBJS)
	$(LD) $(LDFLAGS) -e$(PRGNAME).srf $(OBJS) $(LIBS)

# 依存関係

# フラッシュ書き込みイメージ生成
pex : $(PRGNAME).srf
	ppack -e $(PRGNAME).srf -o$(FILENAME).pex -n$(CAPTION) -i$(ICON)

# クリーンアップ
clean:
	del $(PRGNAME).srf $(PRGNAME).pex $(PRGNAME).sym $(PRGNAME).map *.o
