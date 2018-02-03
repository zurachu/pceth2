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

// �Q�[�����[�h
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
#define GM_MAPCLOCK		11
#define GM_OPENING		12
#define GM_TITLE_TO_INIT	13

// �X�N���v�g�f�[�^�\����
typedef struct tagSCRIPT_DATA {
	BYTE	*data;		// �X�N���v�g�f�[�^
	DWORD	p;			// �|�C���^
	DWORD	size;		// �T�C�Y
	char	name[16];	// �t�@�C����
} SCRIPT_DATA;

// ����t���O�̃}�N����`
#define MONTH	(play.flag[0])	// ��
#define DAY		(play.flag[1])	// ��
#define TIME	(play.flag[3])	// ���ԑ�
#define JUMP	(play.flag[4])	// ���x���W�����v�i0��gosub�iEV�X�N���v�g�J�n���ɏ������j�^1��goto�j

// �I�����\����
typedef struct tagLANDMARK {
	int land;
	int chip;
	char scp[16];
} LANDMARK;

// �O���[�o���Z�[�u�f�[�^�\����
typedef struct tagGLOBAL_SAVE_DATA {
	int	bright;		// �R���g���X�g
	int masteratt;	// ����
	int save_index;	// �Z�[�u���j���[�̃C���f�b�N�X
	unsigned short flag[GLOBAL_FLAG_NUM];	// �O���[�o���t���O
} GLOBAL_SAVE_DATA;

// �Z�[�u�f�[�^�\����
typedef struct tagSAVE_DATA {
	int				gameMode;			// �Q�[����ԁi�Z�[�u���̂݁A16bit�ڂɃf�o�b�O���[�h�t���O��ۑ��j
	int				msglen;				// ���b�Z�[�W����
	char			msg[12*8*2+4];		// ���b�Z�[�W�o�b�t�@
	unsigned short	flag[FLAG_NUM];		// �t���O
	SCRIPT_DATA		evData;			// EV�X�N���v�g�f�[�^
	SCRIPT_DATA		scData;			// �X�N���v�g�f�[�^
	char pgxname[GRP_NUM][16];	// �摜�t�@�C����
	char bgopt[4];				// �w�i�摜�t�@�C�����C���q
	char pmdname[16];			// BGM�t�@�C����
	int			selIndex;	// �I���C���f�b�N�X
	int			selAmount;	// �I������
	int			selReg;		// �I�����ʂ��i�[���郌�W�X�^�ԍ�
	int			selY[CHOICE_NUM];	// �I����Y���W�i2�s�ɂ킽����̂̑΍�j
	int			lmAmount;		// �����h�}�[�N�o�^��
	LANDMARK	lm[CHOICE_NUM];	// �����h�}�[�N
} SAVE_DATA;

extern GLOBAL_SAVE_DATA global;
extern SAVE_DATA play;
extern unsigned short reg[];
extern BOOL debug_mode;
extern int msgView, speed, wait;
extern int keyWaitX, keyWaitY;

void pceth2_Init();
void pceth2_bButtonMenu();
void pceth2_drawBButtonMenu();

#endif
