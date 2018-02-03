/*
 *	pceth2 - �I�����A�}�b�v�I���֘A
 *
 *	(c)2005 �ĂƂ灚�ۂ���
 *
 *	2005/05/14	�J���J�n
 *	2005/06/19	�I�����ǉ��I�����ɍs���Ȃ���s���Ȃ�
 *	2005/06/24	pceth2_map.c�Ɠ���
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"

#include "common.h"
#include "pceth2_cal.h"
#include "pceth2_sel.h"
#include "pceth2_sys.h"
#include "pceth2_msg.h"
#include "pceth2_grp.h"
#include "pceth2_snd.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"

static void pceth2_loadMapChipChara();

//=============================================================================
//	���ʕ���
//=============================================================================

/*
 *	�I���̖���`��
 */
void pceth2_drawSelArrow()
{
	Ldirect_VBuffClear(0, 0, MSG_X_MIN + FONT_W / 2 + 1, DISP_Y);
	FontFuchi_Put(MSG_X_MIN, play.selY[play.selIndex], '>');
}

/*
 *	�I���̎���
 *
 *	amount	�I������
 *
 *	return	-1=���I���^-1�ȊO=�I��l
 */
#define NO_SELECT	-1

int pceth2_SelectEx(int amount)
{
	BOOL LCDUpdate = FALSE;

	if (msgView)	// ���b�Z�[�W�\�����
	{
		if (*play.msg) { // �I����
			if (pcePadGet() & TRG_UP) {	// ��
				pceth2_playSelectSE();
				play.selIndex = (play.selIndex + amount - 1) % amount;
				LCDUpdate = TRUE;
			}
			if (pcePadGet() & TRG_DN) {	// ��
				pceth2_playSelectSE();
				play.selIndex = (play.selIndex + 1) % amount;
				LCDUpdate = TRUE;
			}
		} else { // �}�b�v�摜�I����
			if (pcePadGet() & TRG_LF) {	// ��
				pceth2_playMapSelectSE();
				play.selIndex = (play.selIndex + amount - 1) % amount;
				LCDUpdate = TRUE;
			}
			if (pcePadGet() & TRG_RI) {	// ��
				pceth2_playMapSelectSE();
				play.selIndex = (play.selIndex + 1) % amount;
				LCDUpdate = TRUE;
			}
		}

		if (LCDUpdate) {
			if (play.gameMode == GM_MAPSELECT) {	// �}�b�v�I����
				pceth2_loadMapChipChara();			// �`�b�v�L�������`���ւ�
				pceth2_drawMapSelArrow();	// ���
			} else {
				pceth2_drawSelArrow();	// ���
			}
			Ldirect_Update();
			LCDUpdate = FALSE;
		}

		if (pcePadGet() & TRG_A) {	// A
			if (*play.msg) {
				pceth2_playSaveDecideSE();
			} else {
				pceth2_playDecideSE();
			}
			pceth2_setPageTop();
			pceth2_clearMessage();
			Ldirect_Update();

			return play.selIndex;
		} else if (pcePadGet() & TRG_B) {
			pceth2_drawBButtonMenu();
		}
	}
	else			// ���b�Z�[�W��\�����
	{
		pceth2_bButtonMenu();
	}
	
	

	return NO_SELECT;
}


//=============================================================================
//	�I��
//=============================================================================

/*
 *	�I������ǉ��o�^	q[str]�i�X�y�[�X�ŏI�[�j
 */
int pceth2_addSelItem(SCRIPT_DATA *s)
{
	s->p++;

	FontFuchi_GetPos(NULL, &play.selY[play.selAmount]);	// y���W���L��

	while (*(s->data + s->p) != ' ') {	// �I������`��
		if (strncmp(s->data + s->p, "�@", 2) && strncmp(s->data + s->p, "\\n", 2)) {	// P/ECE�ł̓Y���Ă��܂��̂Ŗ���
			if (pceth2_isLineTop()) {	// �s���Ȃ�1����������
				pceth2_putKanji("�@");
			}
			pceth2_putKanji(s->data + s->p);
		}
		s->p += 2;
	}
	if (!pceth2_isLineTop()) {	// �s���łȂ���Ή��s
		pceth2_putCR();
	}
	s->p++;

	play.selAmount++;
	return 1;
}

/*
 *	�I����������	Q0
 */
int pceth2_initSelect(SCRIPT_DATA *s)
{
	s->p++;
	play.selReg = (int)(*(s->data + s->p++) - '0');	// ���ʊi�[���W�X�^�ԍ����擾
	play.gameMode = GM_SELECT;

	pceth2_drawSelArrow();	//	���

	return 0;
}

/*
 *	�I��
 */
void pceth2_Select()
{
	if (pceth2_SelectEx(play.selAmount) != NO_SELECT) {
		reg[play.selReg] = play.selIndex;
		play.selIndex = play.selAmount = 0;
		play.gameMode = GM_SCRIPT;
	}
}

//=============================================================================
//	�}�b�v�I��
//=============================================================================

BOOL pceth2_dayHasMapSelect()
{
	// �����͈ȉ����
	// https://github.com/autch/aquaplus_gpl/blob/master/ToHeart2/ScriptEngine/src/GM_Avg.cpp#L4459-L4471
	if (MONTH == 3 && DAY == 20) return FALSE;
	if (MONTH == 4 && DAY == 29) return FALSE;
	if (MONTH == 5 && DAY == 3) return FALSE;
	if (MONTH == 5 && DAY == 4) return FALSE;
	if (MONTH == 5 && DAY == 5) return FALSE;
	if (MONTH == 3 && DAY >= 25) return FALSE;
	if (MONTH == 4 && DAY <= 7) return FALSE;
	if (pceth2_getDate(MONTH, DAY) == 0) return FALSE;

	return TRUE;
}

#define CH_NOTHING	0
#define LM_MYHOME	0

/*
 *	�}�b�v�I���̖���`��
 */
void pceth2_drawMapSelArrow()
{
	if (*play.msg) { // ������I����
		pceth2_drawSelArrow();
	} else {
		Ldirect_VBuffClear(0, 0, DISP_X, DISP_Y);
		FontFuchi_SetPos(MSG_X_MIN, MSG_Y_MIN);
		FontFuchi_Printf("<< %d / %d >>", play.selIndex + 1, play.lmAmount);
	}
}

/*
 *	�`�b�v�L������ǂݍ���ŉ�ʂ��ĕ`��
 */
#define FNAMELEN_CHIP	13

static void pceth2_loadMapChipChara()
{
	char buf[FNAMELEN_CHIP + 1];

	pcesprintf(buf, "MAP%02d.pgx", play.lm[play.selIndex].land + 1);
	pceth2_loadGraphic(buf, GRP_C);
	if (play.lm[play.selIndex].chip == CH_NOTHING) {	// �`�b�v�L�����Ȃ�
		pceth2_clearGraphic(GRP_R);
	} else {
		pcesprintf(buf, "CHIP%03d01.pgx", play.lm[play.selIndex].chip);
		pceth2_loadGraphic(buf, GRP_R);
	}

	pceth2_DrawGraphic();	// �`������
}

/*
 *	�I������`��
 */
static void pceth2_putMapItem()
{
	static const char * const landName[] = {
		"����",		"���X�X",	"�Q�[���Z���^�[",	"����",		"���w�Z",	"�⓹",		"�Z��O",
		"�Z��",		"���֏�",	"����",				"���ʔ�",	"�̈��",	"����",		"�}����",
		"�����o��",	"�P�K�L��",	"�Q�K�L��",			"�P�K����",	"�Q�K����",	"�R�K�L��",	"�R�K����",
	};
	int i;

	pceth2_setPageTop();
	pceth2_clearMessage();
	if (!*play.pgxname[GRP_C]) { // �}�b�v�摜�������ꍇ�͕�����I����
		for (i = 0; i < play.lmAmount; i++) {
			pceth2_putKanji("�@");
			FontFuchi_GetPos(NULL, &play.selY[i]);	// y���W���L��
			FontFuchi_Printf("%s\n", landName[play.lm[i].land]);
			play.msglen += pcesprintf(play.msg + play.msglen, "%s\n", landName[play.lm[i].land]);
		}
		Ldirect_Update();
	}
}

/*
 *	�����h�}�[�N�ǉ��o�^�̎���
 *
 *	land	�����h�}�[�N�ԍ�
 *	chip	�`�b�v�L�����ԍ�
 *	*fName	�W�����v��X�N���v�g�t�@�C����
 */
static void pceth2_addMapItemEx(int land, int chip, const char *fName)
{
	play.lm[play.lmAmount].land = land;
	play.lm[play.lmAmount].chip = chip;
	strcpy(play.lm[play.lmAmount].scp, fName);
	play.lmAmount++;
}

/*
 *	�����h�}�[�N��ǉ��o�^	m?,?,000000000.scp
 */
int pceth2_addMapItem(SCRIPT_DATA *s)
{
	int l, c;
	char buf[FNAMELEN_SCP + 1];

	s->p++;	// m
	l = pceth2_getNum(s);	// �����h�}�[�N�ԍ�
	s->p++;	// ,
	c = pceth2_getNum(s);	// �`�b�v�L�����ԍ�
	s->p++;	// ,
	pceth2_strcpy(buf, s, FNAMELEN_SCP);	// �W�����v��

	pceth2_addMapItemEx(l, c, buf);
	return 1;
}

#define BG_MAPCLOCK	"B009001.pgx"
#define FNAMELEN_CLOCK 11

/*
 *	�}�b�v�I��O���v��������
 *	EV_????AFTER_SCHOOL��ǂݏI��������ɌĂяo����܂�
 *	return �����h�}�[�N�o�^����0���\�����Ȃ��ꍇ FALSE
 */
void pceth2_initMapClock()
{
	char buf[FNAMELEN_CLOCK + 1];
	int const time = (pceth2_getDate(MONTH, DAY) == 6) ? 11 : 19; // �y�j�� 12:10, ������ 14:50

	pceth2_loadGraphic(BG_MAPCLOCK, GRP_BG);
	pcesprintf(buf, "CLOCK%02d.pgx", time);
	pceth2_loadGraphic(buf, GRP_C);
	pceth2_clearGraphic(GRP_L);
	pceth2_clearGraphic(GRP_R);
	pceth2_DrawGraphic();
	pceth2_clearMessage();
	wait = 30;
	play.gameMode = GM_MAPCLOCK;
}

#define BG_MAPSELECT	"MAP_BG.pgx"
#define BGM_MAPSELECT	"M10.pmd"

/*
 *	�}�b�v�I����������
 */
void pceth2_initMapSelect()
{
	// �����ǉ�
	pceth2_addMapItemEx(LM_MYHOME, CH_NOTHING, "");

	pceth2_loadGraphic(BG_MAPSELECT, GRP_BG);	// �w�i
	pceth2_clearGraphic(GRP_L);
	pceth2_clearGraphic(GRP_C);
	play.gameMode = GM_MAPSELECT;	// pceth2_loadMapChipChara() ����ɕς��ĕ`�揇�𒲐�
	pceth2_loadMapChipChara();					// �`�b�v�L����
	pceth2_putMapItem();						// �I����
	pceth2_drawMapSelArrow();					// ���
	Play_PieceMML(BGM_MAPSELECT);				// BGM
}




/*
 *	�}�b�v�I��
 */
void pceth2_MapSelect()
{
	if (pceth2_SelectEx(play.lmAmount) != NO_SELECT) {
		if (play.lm[play.selIndex].land == LM_MYHOME) {	// ����
			pceth2_loadEVScript();
		} else {
			pceth2_loadScript(&play.scData, play.lm[play.selIndex].scp);
			play.gameMode = GM_SCRIPT;
		}
		pceth2_clearGraphic(GRP_R);
		Stop_PieceMML();
		play.selIndex = play.lmAmount = 0;
	}
}
