#===============================================================================
# project makefile
#===============================================================================

.PHONY: all lib pex clean

#=======================================
# プロジェクト設定
#=======================================
include project.mak

#=======================================
# サフィックスルールを無視
#=======================================
.SUFFIXES:

#=======================================
# 生成コマンド・オプション
#=======================================
CC := pcc33
CFLAGS := -c -g -gp=0x0 -near -O2 -Wall $(addprefix -I,$(CINCS)) $(addprefix -D,$(CDEFS))
AS := pcc33
ASFLAGS := -c -g -gp=0x0 -near
LD := pcc33
LDFLAGS := -g -ls -lm +defsym _stacklen=0x1000

LIB := lib33
CPP := cpp

#=======================================
# 生成ファイル
#=======================================
TARGET := $(PROJECT_NAME).srf
IMAGE := $(PROJECT_NAME).pex
SYMBOL := $(PROJECT_NAME).sym
MEMMAP := $(PROJECT_NAME).map

all : $(TARGET)

#=======================================
# 構成ファイル
#=======================================
# zurapce library
ZURAPCE_LIBRARY_DIR := zurapce
ZURAPCE_LIBRARY := $(ZURAPCE_LIBRARY_DIR)/zurapce.lib

SOURCES := $(wildcard *.c)
DEPENDS := $(patsubst %.c,%.depend,$(SOURCES))
OBJECTS := $(patsubst %.c,%.o,$(SOURCES))
LIBRARIES := $(LIBRARIES)
LIBRARIES += $(ZURAPCE_LIBRARY)

#=======================================
# 16階調描画関連コードを高速 RAM に配置する場合
#=======================================
ifeq ($(strip $(USE_LDIRECT_ON_FRAM)),true)

FRAM_LDIRECT := Ldirect_Fram.o
FRAM_OBJECTS += $(FRAM_LDIRECT)

$(FRAM_LDIRECT) : $(ZURAPCE_LIBRARY)
	$(LIB) -x $< $@

endif

#=======================================
# 縁取りフォント関連コードを高速 RAM に配置する場合
#=======================================
ifeq ($(strip $(USE_FONT_FUCHI_ON_FRAM)),true)

FRAM_FONT_FUCHI := FontFuchi.o
FRAM_OBJECTS += $(FRAM_FONT_FUCHI)

$(FRAM_FONT_FUCHI) : $(ZURAPCE_LIBRARY)
	$(LIB) -x $< $@

endif

#=======================================
# 高速 RAM に配置する場合
#=======================================
ifneq ($(strip $(FRAM_OBJECTS)),)

FRAM_OBJECT_TOP := FramObject_Top.o
LDFLAGS +=	+codeblock FRAMC {$(FRAM_OBJECT_TOP) $(FRAM_OBJECTS)} \
			+bssblock FRAMB {$(FRAM_OBJECT_TOP) $(FRAM_OBJECTS)} \
			+addr 0x1000 {@FRAMC FRAMB} 
OBJECTS := $(FRAM_OBJECT_TOP) $(FRAM_OBJECTS) $(filter-out $(FRAM_OBJECTS),$(OBJECTS))

$(FRAM_OBJECT_TOP) : $(ZURAPCE_LIBRARY)
	$(LIB) -x $< $@

endif

#=======================================
# 依存関係
#=======================================

$(TARGET) : $(OBJECTS) $(LIBRARIES)
	$(LD) $(LDFLAGS) -e$@ $(subst /,\,$^)

%.o : %.c
	$(CC) $(CFLAGS) $<

#=======================================
# 自動依存関係生成
#=======================================
%.depend : %.c
	$(CPP) -MM -I$(PIECE_INC_DIR) $< > $@

-include $(DEPENDS)

#=======================================
# zurapce library
#=======================================

lib : $(ZURAPCE_LIBRARY)

$(ZURAPCE_LIBRARY) :
	make -C $(ZURAPCE_LIBRARY_DIR)

#=======================================
# フラッシュ書き込みイメージ生成
#=======================================

pex : $(IMAGE)

$(IMAGE) : $(TARGET) $(ICON)
	ppack -e $< -o$@ -n$(CAPTION) -i$(ICON)

#=======================================
# クリーンアップ
#=======================================
clean :
	del $(IMAGE)
	del $(TARGET)
	del $(SYMBOL)
	del $(MEMMAP)
	del *.o
	del *.depend

