/*
 *	pceth2 - SE�ABGM�֘A
 *
 *	(c)2005 �ĂƂ灚�ۂ���
 *
 *	2005/05/07	�J���J�n
 *	2005/05/27	Win��pceth2bin2�ɑΉ��i�X�N���v�g�`���𑽏��ύX
 */

#include <string.h>
#include <piece.h>
#include "zurapce/zurapce.h"
#include "muslib2.h"

#include "common.h"
#include "pceth2_snd.h"
#include "pceth2_arc.h"
#include "pceth2_str.h"

extern SAVE_DATA play;

//=============================================================================
//	PCEWAVEINFO���쐬�A�Đ��A���s�[�g�Đ��A���
//=============================================================================
/*
typedef struct tagPCEWAVEINFO {
	volatile unsigned char stat;					// 0  �X�e�[�^�X
	unsigned char type;								// 1  �f�[�^�`��
	unsigned short resv;							// 2  �\��
	const void *pData;								// 4  �f�[�^�ւ̃|�C���^
	unsigned long len;								// 8  �f�[�^�̒���(�T���v����)
	struct tagPCEWAVEINFO *next;					// 12 ���ւ̃|�C���^
	void (*pfEndProc)( struct tagPCEWAVEINFO *);	// 16 �I�����R�[���o�b�N��������g���ă��s�[�g�Đ��A������s���܂��B
} PCEWAVEINFO;
*/

static PCEWAVEINFO	pwav;
static BYTE			*ppd;

/*
 *	���s�[�g�Đ��p�R�[���o�b�N
 *
 *	*pWav	�R�[���o�b�N�Ăяo����PCEWACEINFO�̃|�C���^
 */
static void Play_PieceWaveEx(PCEWAVEINFO *pWav)
{
	pceWaveDataOut(SND_CH, pWav);
}

/*
 *	����p�R�[���o�b�N
 *
 *	*pWav	�R�[���o�b�N�Ăяo����PCEWACEINFO�̃|�C���^
 */
static void Stop_PieceWaveEx(PCEWAVEINFO *pWav)
{
	Stop_PieceWave();
}

/*
 *	PCEWAVEINFO�쐬�g���ipfEndProc�Ń��s�[�g�Đ��j
 *
 *	*pWav	PCEWAVEDATA�\���̂̃|�C���^
 *	*data	���������ppd�t�@�C���f�[�^
 *	rep		���s�[�g�t���O�i0=�Ȃ��^0�ȊO�����s�[�g�j
 */
static void Get_PieceWaveEx(PCEWAVEINFO *pWav, BYTE *data, const int rep)
{
	PceWaveInfo_Construct(pWav, data);
	if (rep) {	// ���s�[�g
		pWav->pfEndProc = Play_PieceWaveEx;
	} else {	// �Đ��I��
		pWav->pfEndProc = Stop_PieceWaveEx;
	}
}

//=============================================================================
//	SE�Đ��A��~
//=============================================================================

void Play_PieceWave(const char *fName, int rep)
{
	Stop_PieceWave(&pwav);	// �O�̍Đ����L�閳����킸�~�߂�

	ppd = fpk_getEntryData((char*)fName, NULL, NULL);
	if (ppd != NULL) {
		Get_PieceWaveEx(&pwav, ppd, rep);
		Play_PieceWaveEx(&pwav);
	}
}

void Stop_PieceWave()
{
	pceWaveAbort(SND_CH);
	pceHeapFree(ppd);
	ppd = NULL;
}

//=============================================================================
//	SE�Đ��i�t�@�C�����Ȃ���Β�~�j
//=============================================================================

/*
 *	SE��ǂݍ���	SE_0000.ppd,0
 *
 *	*s		�X�N���v�g�f�[�^
 *
 *	return	1�i�����������s�j
 */
#define FNAMELEN_SE		11

int pceth2_loadSE(SCRIPT_DATA *s)
{
	char buf[FNAMELEN_SE + 1];

	// �t�@�C�������o�b�t�@�ɃR�s�[
	pceth2_strcpy(buf, s, FNAMELEN_SE);
	s->p++;	// ,
	Play_PieceWave(buf, (int)(*(s->data + s->p++) - '0'));

	return 1;
}

//=============================================================================
//	����SE�Đ�
//=============================================================================
void pceth2_playSelectSE()
{
	Play_PieceWave("SE_NONAME14.pp", 0);
}

void pceth2_playMapSelectSE()
{
	Play_PieceWave("SE_NONAME12.pp", 0);
}

void pceth2_playDecideSE()
{
	Play_PieceWave("SE_NONAME20.pp", 0);
}

void pceth2_playSaveDecideSE()
{
	Play_PieceWave("SE_NONAME06.pp", 0);
}

//=============================================================================
//	pmd�Đ��A��~
//=============================================================================

// �q�[�v�̃t���O�����e�[�V������������邽�߁A�Œ�Ŋm��
// ���̃T�C�Y�𒴂��邱�Ƃ͖����Ǝv�����A�������瑝�₷����
// �iv1.04 ���_�A�ő�� M35.pmd �� 3,998�j
static BYTE	pmd[4096];

/*
 *	BGM�Đ�
 *
 *	*fName	pmd�t�@�C����
 */
void Play_PieceMML(const char *fName)
{
	if (strcmp(play.pmdname, fName) != 0) {
		Stop_PieceMML();
		strcpy(play.pmdname, fName);
		if (fpk_getEntryData(play.pmdname, NULL, pmd)) {
			PlayMusic(pmd);
		} else {
			*play.pmdname = '\0';
		}
	}
}

/*
 *	BGM��~
 */
void Stop_PieceMML()
{
	*play.pmdname = '\0';
	StopMusic();
	pceWaveAbort(BGM_CH);
}

//=============================================================================
//	BGM�Đ��i�t�@�C�����Ȃ���Β�~�j
//=============================================================================

/*
 *	BGM���Đ�	M00.pmd
 *
 *	*s		�X�N���v�g�f�[�^
 *
 *	return	1�i�����������s�j
 */
#define FNAMELEN_BGM	7

int pceth2_loadBGM(SCRIPT_DATA *s)
{
	char buf[FNAMELEN_BGM + 1];
	pceth2_strcpy(buf, s, FNAMELEN_BGM);
    Play_PieceMML(buf);

	return 1;
}
