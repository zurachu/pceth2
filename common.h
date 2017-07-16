#ifndef __PCETH2_COMMON_H_
#define __PCETH2_COMMON_H_

#define array_size(a)	(sizeof(a)/sizeof(a[0]))

#define MIN_BRIGHT		32

#define FLAG_NUM		1000
#define GLOBAL_FLAG_NUM	12
#define REG_NUM			10

#define CHOICE_NUM		8

#define GRP_NUM		4

#define EV_MORNING		0
#define EV_INTERVAL		1
#define EV_SCHOOL_HOURS	2
#define EV_LUNCH_BREAK	3
#define EV_AFTER_SCHOOL	4
#define EV_MAP_SELECT	5
#define EV_NIGHT		6
#define EV_NEXT_DAY		7

// ゲームモード
#define GM_EVSCRIPT		1
#define GM_SCRIPT		2
#define GM_KEYWAIT		3
#define GM_TIMEWAIT		4
#define GM_MAPSELECT	5
#define GM_SELECT		6
#define GM_CALENDER		7
#define GM_SLIDECHR		8
#define GM_TITLE		9
#define GM_SAVE			10

// スクリプトデータ構造体
typedef struct tagSCRIPT_DATA {
	BYTE	*data;		// スクリプトデータ
	DWORD	p;			// ポインタ
	DWORD	size;		// サイズ
	char	name[16];	// ファイル名
} SCRIPT_DATA;

// 特殊フラグのマクロ定義
#define MONTH	(play.flag[0])	// 月
#define DAY		(play.flag[1])	// 日
#define TIME	(play.flag[3])	// 時間帯
#define JUMP	(play.flag[4])	// ラベルジャンプ（0＝gosub（EVスクリプト開始時に初期化）／1＝goto）

// 選択肢構造体
typedef struct tagLANDMARK {
	int land;
	int chip;
	char scp[16];
} LANDMARK;

// グローバルセーブデータ構造体
typedef struct tagGLOBAL_SAVE_DATA {
	int	bright;		// コントラスト
	int masteratt;	// 音量
	int save_index;	// セーブメニューのインデックス
	unsigned short flag[GLOBAL_FLAG_NUM];	// グローバルフラグ
} GLOBAL_SAVE_DATA;

// セーブデータ構造体
typedef struct tagSAVE_DATA {
	int				gameMode;			// ゲーム状態（セーブ時のみ、16bit目にデバッグモードフラグを保存）
	int				msglen;				// メッセージ長さ
	char			msg[12*8*2+4];		// メッセージバッファ
	unsigned short	flag[FLAG_NUM];		// フラグ
	SCRIPT_DATA		evData;			// EVスクリプトデータ
	SCRIPT_DATA		scData;			// スクリプトデータ
	char pgxname[GRP_NUM][16];	// 画像ファイル名
	char bgopt[4];				// 背景画像ファイル名修飾子
	char pmdname[16];			// BGMファイル名
	int			selIndex;	// 選択インデックス
	int			selAmount;	// 選択肢数
	int			selReg;		// 選択結果を格納するレジスタ番号
	int			selY[CHOICE_NUM];	// 選択肢Y座標（2行にわたるものの対策）
	int			lmAmount;		// ランドマーク登録数
	LANDMARK	lm[CHOICE_NUM];	// ランドマーク
} SAVE_DATA;

extern GLOBAL_SAVE_DATA global;
extern SAVE_DATA play;
extern unsigned short reg[];
extern BOOL debug_mode;
extern int msgView, speed, wait;

void pceth2_Init();

#endif
