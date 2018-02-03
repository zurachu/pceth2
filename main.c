/*
 *	pceth2
 *
 *	(c)2005 �ĂƂ灚�ۂ���
 *
 *	2005/02/16	�D���D���^�}���o�����Ver.
 *	2005/03/11	�摜�͊O���t�@�C���ipar�`���j����ǂݍ��ނ悤��
 *	2005/04/09	�X�N���v�g�ǂݍ��ݒB��
 *	2005/04/20	���ꕶ���̏����ς݁A�n�[�g�ǉ�
 *	2005/04/21	�֑������i�A�B�j�ǉ��Aw3����
 *	2005/04/23	�摜�f�[�^�|�C���^���q�[�v�����Ɋm����NULL��
 *	2005/04/25	<S>�A<W>�ɑΉ��A\k��łȂ�\n�𖳎�
 *				w3���ɂ����ƑΉ����Ă��Ȃ������̂��C��
 *				���O�u����*nnk1��ǉ�
 *	2005/04/27	pceth2_grp.c�ɕ���
 *				���x���W�����v�i�����Ȃ��j��ǉ�
 *	2005/04/30	�t���O���[�h�A���x���u�����`�A�X�N���v�g�W�����v��ǉ�
 *	2005/05/01	�t���O�Z�[�u��ǉ�
 *				�֑������Ɂv�x��ǉ�
 *				pceth2_sys.c�ɕ���
 *	2005/05/07	pceth2_snd.c��ǉ�
 *	2005/05/08	pceth2_msg.c�ɕ���
 *	2005/06/11	�J�����_�[���[�h�̎���B�{�^����VBuff�������Ȃ��悤�ɏC��
 *	2005/06/12	B�{�㉺���E�ŃR���g���X�g�A���ʒ��߉\��
 *	2005/06/13	�f�o�b�O�p�r���h�ǉ�
 *	2005/06/15	��s�̕������̈Ⴂ�ɂ����s�̕␳
 *	2005/06/25	���O�u���������R���o�[�^���Ɉړ�
 *	2005/06/30	�R���g���X�g�A���ʒ��߂̑����ύX�i�E�B���h�E�����Ă��Ԃŏ㉺���E�j
 *	2005/07/19	calFlag�p�~�Apgxname[GRP_C]�Ŕ��f����悤��
 *				�X�N���v�g���̃J�����_�[���[�h�ɑΉ�
 *				BG�\�����߂ō�����t�ɍ��킹�Ȃ��P�[�X�i��z�Ȃǁj�ɑΉ�
 *				 *
 *	TODO		�X�N���v�g�̊��S���
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"
#include "usbcapt.h"
#include "muslib2.h"

#include "ufe/ufe.h" /*{{2005/06/09 Naoyuki Sawa}}*/
#include "mmc_api.h" //2005/06/11 Added by Madoka

#include "common.h"
#include "pceth2_arc.h"
#include "pceth2_sys.h"
#include "pceth2_msg.h"
#include "pceth2_sel.h"
#include "pceth2_grp.h"
#include "pceth2_snd.h"
#include "pceth2_cal.h"
#include "pceth2_sav.h"

BOOL file_load = FALSE;		// �t�@�C�����J�������ǂ���

BOOL debug_mode = FALSE;
int speed, wait, msgView;
int keyWaitX, keyWaitY;
static int bButtonMenuTime = 0;

static PrecisionTimer s_frame_timer;
static unsigned long s_frame_us, s_proc_us;

static void pceth2_initGraphicAndSound();
static int  pceth2_readScript(SCRIPT_DATA *s);
static void pceth2_waitKey();
static void pceth2_startDebugMenu();

//=============================================================================
//=============================================================================

/*
 *	������
 */
#define ARCHIVE_FILE_NAME	"pceth2.par"	// �A�[�J�C�u�t�@�C����
#define DEBUG_FILE_NAME		"999999999.scp"	// �f�o�b�O���j���[�X�N���v�g
#define PROC_PERIOD	33	// Proc/msec

void pceAppInit(void)
{	
	FramObject_Init();

	if(pcePadGetDirect() & PAD_C) {
		debug_mode = TRUE;
	}

	/*{{2005/06/09 Naoyuki Sawa*/
	if(ufe_setup() != 0)	// UFE������
	{
		//2005/06/11 Added by Madoka
		if(mmcInit(MMC_FILESIZE_MAX) != 1) {	// MMC������
			return;
		}
	}
	/*}}2005/06/09 Naoyuki Sawa*/

	usbCaptureInit();	// pceCaps������
	if(!Ldirect_Init())
	{
		return;
	}
	Ldirect_VBuffView(TRUE);
	pceLCDDispStop();

	FontProxy_Hook_Set();
	FontExtend_Hook_GetAdrs();	// ����t�H���g�ǉ�pceFontGetAdrs���t�b�N
	FontFuchi_SetType(0);
	FontFuchi_SetRange(MSG_X_MIN, MSG_Y_MIN, MSG_X_MAX, MSG_Y_MAX);
	FontFuchi_SetTxColor(0);
	FontFuchi_SetBdColor(3);
	loadInst();			// �h�������F�����L�b�g������
	InitMusic();		// ���y���C�u����������


	pceAppSetProcPeriod(PROC_PERIOD);

	if (pceth2_readGlobalSaveData()) {

		// ���s�O�̃R���g���X�g�A���ʂ�ۑ�
		Configure_Init();

		// �A�[�J�C�u�ǂݍ���
		file_load = fpk_InitHandle(ARCHIVE_FILE_NAME);
		if (file_load) {
			if(debug_mode) {
				pceth2_Init();
				pceth2_startDebugMenu();
			} else {
				pceth2_TitleInit();
			}
		}
	}

	msgView = 1;
	speed = 0;

	pceLCDDispStart();
	PrecisionTimer_Construct(&s_frame_timer);
}

/*
 *	���C��
 */
void pceAppProc(int cnt)
{
	PrecisionTimer proc_timer;
	PrecisionTimer_Construct(&proc_timer);

	/*{{2005/06/09 Naoyuki Sawa*/
//	if(!hFpk) { //���������s?
	if (!file_load) {	// 2005/07/23�ύX
		pceAppReqExit(0);
		return;
	}
	/*}}2005/06/09 Naoyuki Sawa*/

	switch (play.gameMode)
	{
        case GM_TITLE:		// �^�C�g�����
			pceth2_Title();
			break;
		case GM_SAVE:	// �Z�[�u���[�h���j���[
			pceth2_SaveMenu();
			break;
		case GM_EVSCRIPT:	// EV_�`�X�N���v�g�ǂݍ���
			while (pceth2_readScript(&play.evData));
			break;
		case GM_SCRIPT:		// �X�N���v�g�ǂݍ���
			while (pceth2_readScript(&play.scData));
			break;
		case GM_SELECT:
			pceth2_Select();
			if (pcePadGet() & PAD_C) { pceth2_SaveInit(); }
			break;
		case GM_MAPSELECT:	// �}�b�v�I��
			pceth2_MapSelect();
			if (pcePadGet() & PAD_C) { pceth2_SaveInit(); }
			break;
		case GM_CALENDER:
			pceth2_calenderDrawCircle();
			break;
		case GM_KEYWAIT:	// �L�[�҂�
			pceth2_waitKey();
			if (pcePadGet() & PAD_C && !pceth2_isCalenderMode()) { pceth2_SaveInit(); }
			break;
		case GM_TIMEWAIT:	// ���ԑ҂�
			if (wait-- <= 0 || (pcePadGet() & PAD_RI)) {
				play.gameMode = GM_SCRIPT;
			}
			break;
		case GM_MAPCLOCK:	// �}�b�v�O���v�i���ԑ҂��j
			if (wait-- <= 0 || (pcePadGet() & PAD_RI)) {
				pceth2_initMapSelect();
			}
			break;
		case GM_SLIDECHR:	// �����G�X���C�h
			pceth2_slideChara();
			break;
		case GM_OPENING:	// �I�[�v�j���O
			if (wait-- <= 0 || (pcePadGet() & TRG_A)) {
				play.gameMode = GM_SCRIPT;
			}
			break;
		case GM_TITLE_TO_INIT:
			if (wait-- <= 0) {
					pceth2_Init();
			}
			break;
	}

	if (pcePadGet() & PAD_D) {
		if(debug_mode) {
			if(!strncmp(play.scData.name, DEBUG_FILE_NAME, 6)) { // �f�o�b�O���j���[�X�N���v�g��
				pceAppReqExit(0);
			} else {
				pceth2_startDebugMenu();
			}
		} else {
			if (play.gameMode == GM_TITLE) {
				pceAppReqExit(0);
			} else {
				pceth2_TitleInit();
			}
		}
	}

	if(debug_mode) {
		pceLCDPaint(0, 0, 82, DISP_X, 6);
		pceFontSetType(2);
		pceFontSetPos(0, 82);
		pceFontSetTxColor(3);
		pceFontSetBkColor(FC_SPRITE);
		pceFontPrintf("%6lu/%6luus FREE:%8d", s_proc_us, s_frame_us, pceHeapGetMaxFreeSize());
		Ldirect_Update();
	}

	Ldirect_Trans();

	s_frame_us = PrecisionTimer_Count(&s_frame_timer);
	s_proc_us = PrecisionTimer_Count(&proc_timer);
}

/*
 *	�I��
 */
void pceAppExit(void)
{
	StopMusic();
	pceWaveStop(0);

	/*** �ǂݍ��񂾃t�@�C���𖾎��I�ɉ�����ĂȂ������Ȃ��H ***/

	// �O���[�o���f�[�^�i�t���O�A�R���g���X�g�A���ʁj��ۑ�
	pceth2_writeGlobalSaveData();

	// ���s�O�̃R���g���X�g�A���ʂɖ߂�
	Configure_Exit();

	fpk_ReleaseHandle();
	FontExtend_Unhook_GetAdrs();	// pceFontGetAdrs�����ɖ߂�
	FontProxy_Unhook_Set();
	Ldirect_Exit();
	usbCaptureRelease();	// pceCaps���

	//2005/06/11 Added by Madoka
	mmcExit();

	/*{{2005/06/09 Naoyuki Sawa*/
	ufe_stop();
	/*}}2005/06/09 Naoyuki Sawa*/
}

//2005/06/11 Added by Madoka
/*
 *	�V�X�e���ʒm
 */
int pceAppNotify(int type, int param)
{	
	
	//MMC�Ή��J�[�l��Ver.1.27�ȍ~�ł̏���
	//�J�[�l�����ł�MMC�������𖳌��ɂ���
	//�������Ȃ��ƁA�傫���t�@�C���������Ȃ�����
	if(type == APPNF_INITMMC)
	{
		return APPNR_REJECT;
	}

	return APPNR_IGNORE;	//�f�t�H���g�̏���
}

//=============================================================================
//	
//=============================================================================

/*
 *	�͂��߂���
 */
void pceth2_Init()
{
	memset(play, 0, sizeof(SAVE_DATA));

	MONTH	= START_MONTH;	// ��
	DAY		= START_DAY;	// ��
	TIME	= EV_MORNING;	// ����
	// �N���A�t���O���O���[�o���Ɠ�������
	memcpy(&play.flag[80], &global.flag, GLOBAL_FLAG_NUM * sizeof(unsigned short));

	memset(reg, 0, REG_NUM);	// ���W�X�^

	pceth2_initGraphicAndSound();

	pceth2_loadEVScript(&play.evData);

//	play.gameMode = GM_EVSCRIPT;
}

void pceth2_initGraphicAndSound()
{
	int i;

	pceth2_setPageTop();
	pceth2_clearMessage();

	msgView = 1;
	speed = 0;

	for (i = 0; i <= GRP_NUM; i++) {
		pceth2_clearGraphic(i);
	}
	pceth2_DrawGraphic();
	BG_TIME = BG_WEATHER = '0';	// �w�i�摜�t�@�C�����C���q

	Stop_PieceWave();
	Stop_PieceMML();

	Ldirect_Update();
}

/*
 *	�L�[�҂�
 */
void pceth2_waitKey()
{
	if (msgView)	// ���b�Z�[�W�\�����
	{
		if (!pceth2_isCalenderMode()) {
			wait++;	// �L�[�҂��L���\��
			if (wait == 15) {
				pceLCDPaint(3, keyWaitX, keyWaitY, 3, 5);
				pceLCDPaint(0, keyWaitX + 1, keyWaitY + 1, 1, 3);
				Ldirect_Update();
			} else if (wait >= 30) {
				Ldirect_VBuffClear(keyWaitX, keyWaitY, 3, 5);
				Ldirect_Update();
				wait = 0;
			}
		}
		if (pcePadGet() & (TRG_A | PAD_RI)) {	// �X�N���v�g��i�߂�
			Ldirect_VBuffClear(keyWaitX, keyWaitY, 3, 5);
			Ldirect_Update();
			if (pceth2_isPageTop()) {
				pceth2_clearMessage();
			}
			if (pceth2_isCalenderMode()) {	// �J�����_�[���[�h��
				pceth2_clearGraphic(GRP_C);	// �J�����_�[�摜����
			}
			play.gameMode = GM_SCRIPT;
		} else if (pcePadGet() & TRG_B) {
			if (!pceth2_isCalenderMode()) {	// �J�����_�[�̎��͏����Ȃ�
				pceth2_drawBButtonMenu();
			}
		}
	}
	else			// ���b�Z�[�W��\�����
	{
		pceth2_bButtonMenu();
	}
}

void pceth2_bButtonMenu()
{
	if(bButtonMenuTime > 0) {
		if(--bButtonMenuTime == 0) {
			Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
		}
	}
	if (pcePadGet() & (TRG_A | TRG_B)) {
		pceth2_comeBackMessage();
	}
	// �{�㉺���E�ŃR���g���X�g�A���ʂ̒���
	if (pcePadGet() & TRG_LF) {
		if(global.bright > 0) {
			pceLCDSetBright(--global.bright);
		}
		pceth2_drawBButtonMenu();
	}
	if (pcePadGet() & TRG_RI) {
		if(global.bright < 63) {
			pceLCDSetBright(++global.bright);
		}
		pceth2_drawBButtonMenu();
	}
	if (pcePadGet() & TRG_DN) {
		if(global.masteratt < 127) {
			pceWaveSetMasterAtt(++global.masteratt);
		}
		pceth2_drawBButtonMenu();
	}
	if (pcePadGet() & TRG_UP) {
		if(global.masteratt > 0) {
			pceWaveSetMasterAtt(--global.masteratt);
		}
		pceth2_drawBButtonMenu();
	}
}

void pceth2_drawBButtonMenu()
{
	msgView = 0;
	Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
	pceLCDPaint(0, 0, 0, DISP_X, 6);
	pceFontSetType(2);
	pceFontSetPos(0, 0);
	pceFontSetTxColor(3);
	pceFontSetBkColor(FC_SPRITE);
	pceFontPrintf("<>BRIGHT:%2d ^VOL:%3d", global.bright, 127 - global.masteratt);
	pceFontPut(48, 1, 'v');
	Ldirect_Update();
	bButtonMenuTime = 3 * 1000 / PROC_PERIOD;
}

/*
 *	�X�N���v�g��ǂ�
 *	return	1�̊�pceAppProc()����J��Ԃ��ČĂяo�����
 */
int pceth2_readScript(SCRIPT_DATA *s)
{
	/* ���߉�̓e�[�u�� */
	static const struct {
		char com;
		int  (*func)(SCRIPT_DATA *);
	} com_table[] =	   {{'B',  pceth2_loadBG},		// �w�i�摜�iB6.pgx��11�����{�����G�����t���O1�����j
						{'G',  pceth2_setBGOption},	// �w�i�摜�̃I�v�V�����w��
						{'V',  pceth2_loadBG},		// �C�x���g�摜�iV6.pgx��11�����{�����G�����t���O1�����j
						{'C',  pceth2_loadChara},	// �����G�摜�iC8.pgx��13�����{�ʒu1�����{�X�V�����t���O1�����j
						{'c',  pceth2_clearChara},	// �����G�����ic3�����{�X�V�����t���O1�����j
						{'S',  pceth2_loadSE},		// SE�Đ��iSE_4.ppd��11�����{���s�[�g�t���O1�����j
						{'M',  pceth2_loadBGM},		// BGM���t�iM2.pmd��7�����j�ywin���������z
						{'w',  pceth2_wait},		// ���ԃE�F�C�g�iw3�����j
						{'m',  pceth2_addMapItem},	// �}�b�v�I������ǉ��i9.scp��13�����{�ꏊ2�����{�`�b�v�L����2�����j
						{'J',  pceth2_jumpScript},	// �X�N���v�g�W�����v�i9.scp��13�����j
						{'j',  pceth2_jumpLabel},	// ���x���W�����v
						{'b',  pceth2_branchLabel},	// �����t�����x���W�����v
						{'l',  pceth2_loadFlag},	// �t���O�����W�X�^�Ƀ��[�h
						{'s',  pceth2_saveFlag},	// �t���O����������
						{'=', pceth2_setReg},		// ���W�X�^�ɒl���Z�b�g
						{'+', pceth2_incReg},		// ���W�X�^���C���N�������g
						{'-', pceth2_decReg},		// ���W�X�^���f�N�������g
						{'q',  pceth2_addSelItem},	// �I������ǉ�
						{'Q',  pceth2_initSelect},	// �I��
						{'@',  pceth2_memoryLabel},	// ���x���i���߂̂��̂��L���j
						{'<',  pceth2_procControl},	// ���b�Z�[�W����
						{'\\', pceth2_procEscape},	// �G�X�P�[�v�V�[�P���X����
						{'D', pceth2_calenderInitEx},	// �X�N���v�g����J�����_�[���[�h�Ɉڍs
						{'o', pceth2_startOpening},	// �I�[�v�j���O��
						{'z', pceth2_goEpilogue},	// �G�s���[�O��
						{'Z', pceth2_backTitle},	// �^�C�g���ɖ߂�
	};
	int i;

	// �Ō�܂œǂ񂾂�I��
	if (s->p >= s->size) {
		if(debug_mode) {	// �f�o�b�O���[�h�̏ꍇ�f�o�b�O���j���[�ɖ߂�
			pceth2_startDebugMenu();
		} else {
			switch(play.gameMode)
			{
				case GM_EVSCRIPT:
					JUMP = 0;	// 2005/06/20 �܂邵������āF1e, 4, 1��������goto�̏�����
					if (TIME == EV_MAP_SELECT) {
						if (pceth2_dayHasMapSelect()) {
							pceth2_initMapClock();
						}
						TIME++;
					} else if (TIME > EV_NIGHT) {	// ����I��
						play.lmAmount = 0;	// �}�b�v�I���������邪�}�b�v�I���ɍs���Ȃ������ꍇ�A�����ŏ�����
						TIME = EV_MORNING;
						DAY++;
						pceth2_calenderInit();	// �J�����_�[
					} else {
						pceth2_loadEVScript();	// ����EV�X�N���v�g��ǂ�
					}
	//				if (play.evData.size == 0) {	// �ǂ߂Ȃ�������I��
	//					pceAppReqExit(0);
	//				}
					break;
				case GM_SCRIPT:
					Stop_PieceWave();
					pceth2_closeScript(&play.scData);
					play.gameMode = GM_EVSCRIPT;
					break;
			}
		}
		return 0;
	}


	// ���߉��
	for (i = 0; i < array_size(com_table); i++) {
		if (*(s->data + s->p) == com_table[i].com) {
			return com_table[i].func(s);
		}
	}

	// �c��͉�ʕ\�����������̂͂��ł���
	if (pceth2_jpnHyphenation(s->data + s->p + 2) || pceth2_lineFeed(s->data + s->p)) {
		pceth2_putCR();
		if (pceth2_isPageTop()) {
			play.gameMode = GM_KEYWAIT;
			goto UPDATE;
		}
	}
	// �A���󔒂͈�����\�����Ȃ��i����Ŏ蓮�Z���^�����O������ł���H�j
	if (strncmp(play.msg + play.msglen - 2, "�@", 2) || strncmp(s->data + s->p, "�@", 2)) {
		pceth2_putKanji(s->data + s->p);
	}
	s->p += 2;
	if (pceth2_isPageTop()) {
		play.gameMode = GM_KEYWAIT;
		goto UPDATE;
	}

	if (pcePadGet() & PAD_RI) {	// ���������Ă����readScript���ČĂяo���i�X�L�b�v�\���j
		return 1;
	}

UPDATE:
	Ldirect_Update();
	return 0;
}

void pceth2_startDebugMenu()
{
	pceth2_initGraphicAndSound();
	pceth2_loadScript(&play.scData, DEBUG_FILE_NAME);
	play.gameMode = GM_SCRIPT;
}
