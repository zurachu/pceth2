/*
 *	pceth2 - �Z�[�u�E���[�h�֘A
 *
 *	(c)2005 �ĂƂ灚�ۂ���
 *
 *	2005/05/30	�J���J�n
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"

#include "common.h"
#include "pceth2_sav.h"
#include "pceth2_sys.h"
#include "pceth2_grp.h"
#include "pceth2_snd.h"
#include "pceth2_cal.h"
#include "pceth2_msg.h"
#include "pceth2_arc.h"

GLOBAL_SAVE_DATA global;
SAVE_DATA play;

static int const s_debug_mode_flag = 0x100; ///< @see common.h SAVE_DATA::gameMode

//=============================================================================
//	�O���[�o���Z�[�u�t�@�C��
//=============================================================================

#define GLOBAL_SAVE_FILE_NAME	"pceth2.sav"
/*
 *	�O���[�o���Z�[�u�̓ǂݍ���
 */
BOOL pceth2_readGlobalSaveData()
{
	memset(&global, 0, sizeof(GLOBAL_SAVE_DATA));
	global.bright = pceLCDSetBright(INVALIDVAL);
	global.masteratt = pceWaveSetMasterAtt(INVALIDVAL);

	if(File_ReadTo((unsigned char*)&global, GLOBAL_SAVE_FILE_NAME) != sizeof(GLOBAL_SAVE_DATA)
		&& pceFileCreate(GLOBAL_SAVE_FILE_NAME, sizeof(GLOBAL_SAVE_DATA)) != 0) {	// �Ȃ���΍��
		return FALSE;
	}

	if (global.bright < MIN_BRIGHT) {
		global.bright = MIN_BRIGHT;
	}
	pceLCDSetBright(global.bright);
	pceWaveSetMasterAtt(global.masteratt);
	return TRUE;
}

/*
 *	�O���[�o���Z�[�u�̏�������
 */
BOOL pceth2_writeGlobalSaveData()
{
	FILEACC pfa;

	if (pceFileOpen(&pfa, GLOBAL_SAVE_FILE_NAME, FOMD_WR) == 0) {
		pceFileWriteSct(&pfa, &global, 0, sizeof(GLOBAL_SAVE_DATA));
		pceFileClose(&pfa);
		return TRUE;
	}

	return FALSE;
}


//=============================================================================
//	�Z�[�u�t�@�C��
//=============================================================================

#define FNAMELEN_SAV	12

/*
 *	�Z�[�u�t�@�C���̓ǂݍ���
 *
 *	num	�Z�[�u�ԍ�(0-7)
 */
BOOL pceth2_readSaveData(int num)
{
	char buf[FNAMELEN_SAV + 1];

	sprintf(buf, "pceth2_%d.sav", num);
	if(File_ReadTo((unsigned char*)&play, buf) == sizeof(SAVE_DATA)) {
		debug_mode = play.gameMode & s_debug_mode_flag;
		play.gameMode &= ~s_debug_mode_flag;
		return TRUE;
	}
	return FALSE;
}


/*
 *	�Z�[�u�t�@�C���̏�������
 *
 *	num	�Z�[�u�ԍ�(0-7)
 */
BOOL pceth2_writeSaveData(int num)
{
	char buf[FNAMELEN_SAV + 1];
	FILEACC pfa;

	sprintf(buf, "pceth2_%d.sav", num);
	if (pceFileCreate(buf, sizeof(SAVE_DATA)) == 0) {
		if (pceFileOpen(&pfa, buf, FOMD_WR) == 0) {
			pceFileWriteSct(&pfa, &play, 0, sizeof(SAVE_DATA));
			pceFileClose(&pfa);
			return TRUE;
		}
	}

	return FALSE;
}

//=============================================================================
//	�^�C�g�����
//=============================================================================

#define TITLE_BG	"B001000.pgx"
#define TITLE_LOGO	"TH2_LOGO.pgx"

static void draw_object(const PIECE_BMP *pbmp, int dx, int dy)
{
	Ldirect_DrawObject(pbmp, dx, dy, 0, 0, pbmp->header.w, pbmp->header.h);
}

/*
 *	�^�C�g���摜����
 *	�ilbuff�ɕ`�悵�Ă����ɉ�����܂��j
 */
void pceth2_drawTitleGraphic()
{
	PIECE_BMP	p_title;
	BYTE		*_title;

	_title = fpk_getEntryData(TITLE_BG, NULL, NULL);	// �w�i
	PieceBmp_Construct(&p_title, _title);
	draw_object(&p_title, 0, 0);
	pceHeapFree(_title);
	_title = NULL;

	_title = fpk_getEntryData(TITLE_LOGO, NULL, NULL);	// �^�C�g��
	PieceBmp_Construct(&p_title, _title);
	draw_object(&p_title, 28, 4);
	pceHeapFree(_title);
	_title = NULL;

	Ldirect_Update();
}

static int index = 0;

/*
 *	�^�C�g����ʏ�����
 */
void pceth2_TitleInit()
{
	Stop_PieceWave();
	Stop_PieceMML();
	pceth2_closeScript(&play.scData);
	pceth2_closeScript(&play.evData);

	pceth2_drawTitleGraphic();

	pceth2_writeGlobalSaveData();	// �����ŃO���[�o���Z�[�u�ۑ����������ł��邩�ɂ�H

	Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);	// �I����
	FontFuchi_SetPos(38, 56);
	FontFuchi_PutStr("�͂��߂���");
	FontFuchi_SetPos(38, 68);
	FontFuchi_PutStr("�Â�����");
	FontFuchi_Put(28, 56 + index * 12, '>');
	FontFuchi_Put(28 + 65, 56 + index * 12, '<');

	play.gameMode = GM_TITLE;
}

/*
 *	�^�C�g���ɖ߂�	Z
 *
 *	*s		�X�N���v�g�f�[�^
 *
 *	return	0�i�����߂��j
 */
int pceth2_backTitle(SCRIPT_DATA *s)
{
	s->p++;
	if(!debug_mode) {	// �f�o�b�O�ł͉������Ȃ�
		pceth2_TitleInit();
	}

	return 0;
}

void pceth2_Title()
{
	BOOL LCDUpdate = FALSE;

	if (pcePadGet() & (TRG_UP | TRG_DN)) {
		pceth2_playSelectSE();
		index ^= 1;	// 0��1�؂�ւ�
		LCDUpdate = TRUE;
	}

	if (LCDUpdate) {
		Ldirect_VBuffClear(28, 56, 8, 24);
		Ldirect_VBuffClear(92, 56, 8, 24);
		FontFuchi_Put(28, 56 + index * 12, '>');
		FontFuchi_Put(28 + 65, 56 + index * 12, '<');
		Ldirect_Update();
	}

	if (pcePadGet() & TRG_A) {	// A
		if (index == 0) {	// �͂��߂���
			pceth2_playSaveDecideSE();
			play.gameMode = GM_TITLE_TO_INIT;
			wait = 30;
			Ldirect_VBuffView(FALSE);
			Ldirect_Update();
		} else {			// �Â�����
			pceth2_playSaveDecideSE();
			pceth2_SaveInit();
		}
	}
}

//=============================================================================
//	�Z�[�u���[�h���
//=============================================================================

static void pceth2_comeBack(int musplay_flag);

#define SAVE_FILE_NUM	7
int last_gameMode;


static void pceth2_drawSaveMenu()
{
	static char * const date[] = {"��", "��", "��", "��", "��", "��", "�y"};
	char buf[FNAMELEN_SAV + 1];
	FILEACC pfa;
	int month, day, i;

	Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
	FontFuchi_SetPos(MSG_X_MIN + 5 * 5, MSG_Y_MIN);
	FontFuchi_PutStr("�Z�[�u�E���[�h\n");

	for (i = 0; i < SAVE_FILE_NUM; i++) {
		FontFuchi_Printf("%c %d.",((i == global.save_index)? '>' : ' '), i);
		sprintf(buf, "pceth2_%d.sav", i);
		if (pceFileOpen(&pfa, buf, FOMD_RD) == 0) {
			pceFileReadSct(&pfa, NULL, 0, sizeof(SAVE_DATA));
			if(((SAVE_DATA*)pfa.aptr)->gameMode & s_debug_mode_flag) {
				FontFuchi_PutStr(((SAVE_DATA*)pfa.aptr)->scData.name);
			} else {
				month = ((SAVE_DATA*)pfa.aptr)->flag[0];
				day = ((SAVE_DATA*)pfa.aptr)->flag[1];
				if (month && day) {
					FontFuchi_Printf("%2d��%2d�� %s�j��", month, day, date[pceth2_getDate(month, day)]);
				} else { // �G�s���[�O
					FontFuchi_Printf("�������� ���j��", month, day, date[pceth2_getDate(month, day)]);
				}
			}
			pceFileClose(&pfa);
		}
		FontFuchi_PutStr("\n");
	}
	Ldirect_Update();
}

void pceth2_SaveInit()
{
	last_gameMode = play.gameMode;
	play.gameMode = GM_SAVE;

	pceth2_drawSaveMenu();
}

#define LOAD	0
#define SAVE	1

void pceth2_SaveMenu()
{
	static int phase = 0;
	static int mode = LOAD;
	BOOL LCDUpdate = FALSE;

	if (phase == 0)
	{
		if (pcePadGet() & (TRG_UP)) {
			pceth2_playSelectSE();
			global.save_index = (global.save_index - 1 + SAVE_FILE_NUM) % SAVE_FILE_NUM;
			LCDUpdate = TRUE;
		}
		if (pcePadGet() & (TRG_DN)) {
			pceth2_playSelectSE();
			global.save_index = (global.save_index + 1) % SAVE_FILE_NUM;
			LCDUpdate = TRUE;
		}
	} else {
		if (pcePadGet() & (TRG_UP | TRG_DN)) {
			pceth2_playSelectSE();
			mode ^= 1;	// 0��1�؂�ւ�
			LCDUpdate = TRUE;
		}
	}

	if (LCDUpdate) {
		Ldirect_VBuffClear(0, 0, MSG_X_MIN + FONT_W / 2 + 1, DISP_Y);
		FontFuchi_Put(MSG_X_MIN, MSG_Y_MIN + global.save_index * FONT_H + FONT_H, '>');
		if (phase == 1) {
			pceLCDPaint(3, 42, 32, 12, 24);
			pceFontSetType(0);
			pceFontSetTxColor(0);
			pceFontSetBkColor(FC_SPRITE);
			pceFontPut(44, 34 + mode * 10, '>');
		}
		Ldirect_Update();
	}

	if (pcePadGet() & TRG_A) {	// A
		if ((phase == 0 && last_gameMode == GM_TITLE) || (phase == 1 && mode == LOAD)) {
			if (pceth2_readSaveData(global.save_index)) {	// ���[�h
				pceth2_comeBack(1);
				phase = 0;
			}
		} else if (phase == 1) {	// �Z�[�u
			pceth2_playSaveDecideSE();
			play.gameMode = last_gameMode;	// �Z�[�u�p�Ɉ�u�߂�
			if(debug_mode) {
				play.gameMode |= s_debug_mode_flag;
			}
			pceth2_writeSaveData(global.save_index);
			play.gameMode = GM_SAVE;
			pceth2_drawSaveMenu();
			phase = 0;
		} else {	// ���[�h�E�Z�[�u�I����
/* �E�B���h�E������ */
			pceLCDPaint(3, 42, 32, 44, 24);
			pceFontSetType(0);
			pceFontSetTxColor(0);
			pceFontSetBkColor(FC_SPRITE);
			pceFontSetPos(54, 34);
			pceFontPutStr("���[�h");
			pceFontSetPos(54, 44);
			pceFontPutStr("�Z�[�u");
			pceFontPut(44, 34 + mode * 10, '>');
			Ldirect_Update();
			phase = 1;
		}

	} else if (pcePadGet() & TRG_B) {	// B
		if (phase == 0) {
			if (last_gameMode == GM_TITLE) {
				pceth2_TitleInit();
			} else {
			play.gameMode = last_gameMode;
			pceth2_comeBack(0);
			}
		} else {
			pceth2_drawSaveMenu();
			phase = 0;
		}
	}
}

/*
 *	�Z�[�u�f�[�^����e���ԕ��A
 *
 *	replay_flag	0�Ȃ�摜�A���y���Đ��������Ȃ��i�Z�[�u���j���[���畜�A���������̎��j
 */
static void pceth2_comeBack(int replay_flag)
{
	char	buf[16];	// �t�@�C�����ޔ�p
	SCRIPT_DATA evBackup, scBackup;	// �X�N���v�g�t�@�C�����A�|�C���^�ޔ�p
	int		i;

	// �N���A�t���O���O���[�o���Ɠ�������
	play.flag[80] = 0;
	for (i = 1; i < GLOBAL_FLAG_NUM; i++) {
		play.flag[80 + i] |= global.flag[i];
		play.flag[80] += play.flag[80 + i];
	}

	if (replay_flag) {
		for (i = 0; i < GRP_NUM; i++) {
			strcpy(buf, play.pgxname[i]);
			pceth2_clearGraphic(i);
			pceth2_loadGraphic(buf, i);
		}

		Stop_PieceWave();
		strcpy(buf, play.pmdname);
		Stop_PieceMML();
		Play_PieceMML(buf);	// BGM�Đ�

		memcpy(&evBackup, &play.evData, sizeof(SCRIPT_DATA));
		memcpy(&scBackup, &play.scData, sizeof(SCRIPT_DATA));
		pceth2_closeScript(&play.scData);
		pceth2_loadScript(&play.evData, evBackup.name);
		pceth2_loadScript(&play.scData, scBackup.name);
		play.evData.p = evBackup.p;
		play.scData.p = scBackup.p;
	}
	pceth2_DrawGraphic();	// �摜�`��

	pceth2_comeBackMessage();
}

