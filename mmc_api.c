//---------------------------------------------------------------------------
// P/ECE MMC制御関連ルーチン
// 2004/05/02 by まどか 作成開始
// 2004/05/16 by まどか 読み込み時のセクタやクラスタの計算が
//                      少し間違っていたのを修正。
// 2004/08/29 by まどか mmcFileReadSctでmmcFileReadSct(&pfa, NULL, sct, 0);
//                      とした場合に、内部の代替バッファに何も読み込まれて
//                      いなかった不具合を修正。また、mmcFileReadMMCSctでも
//                      同様に修正。そして、mmcFileReadMMCSctでは代替バッファ
//                      アクセス保証サイズを512バイトに変更。
// 2004/08/30 by まどか SDカードと初期化に時間がかかるMMCに対応。
//                      情報提供してくださったnsawaさん(http://piece.no-ip.org/)
//                      ありがとうございますm(-_-)m
// 2004/11/06 by まどか スーパーフロッピー形式に対応しました。
//                      情報提供してくださったnsawaさん(http://piece.no-ip.org/)
//                      ありがとうございますm(-_-)m
// 2004/11/15 by N.SAWA MS-DOSのショートファイル名として許される文字を使っている
//                      エントリを検索結果に含める処理を追加しました
//                      ディレクトリ・エントリまたはボリュームラベルの属性を持つ
//                      エントリを検索から除外する処理を追加しました。
//                      アプリケーションに直接組み込んでコンパイルする場合に
//                      必要の無い部分を#ifdef __PCEKN__マクロにて除外しました。
// 2004/12/12 by N.SAWA mmcInit()にpceFile系APIをフックし、本体FlashROMとMMCを
//                      区別なく扱えるようにしました。
//                      また、mmcExit()時にフックを解除します。
// 2004/12/12 by N.SAWA mmcFileReadSct()にて、ファイル終端を超える読み込みバイト
//                      数が指定された場合は、読み込みバイト数を切り詰めるように
//                      変更しました。pceFileReadSct()との互換のためです。
// 2004/12/12 by N.SAWA mmcFileReadSct()にて、ファイル終端を超える読み込みバイト
//                      数が指定された場合は、読み込みバイト数を切り詰めるように
//                      変更しました。pceFileReadSct()との互換のためです。
// 2004/12/12 by N.SAWA 高速化のため、mmcByteSend()とmmcRecvData()をmmc_fram.cへ
//                      移動しました。
// 2004/12/12 by N.SAWA mmcByteRecv()は使われていないので削除しました。
//                      SEND_CLOCK_WAITも不要になったので削除しました。
// 2004/12/12 by N.SAWA 高速化のため、mmcFileReadSct()とmmcFileReadMMCSct()での
//                      接続確認は省きました。
// 2004/12/12 by N.SAWA mmcFileReadSct()は割り込み内から利用します。割り込み中
//                      はスタックをFRAM2エリアに切り替えるためスタック領域が少
//                      なく、ucaBuff[512]によってスタックが溢れるのを防ぐため、
//                      staticに変更しました。
//                      その他の関数にもやや大きめのローカル配列がありますが、
//                      割り込み内から利用しないので、そのままにしてあります。
//                      2005/03/20 追記 by まどか 
//                      上記の修正を加えようとしたのですが、カーネル内で512バイト
//                      のstatic変数を取ることは、メモリ残量の関係で見送らざるを
//                      得なくなりました。
//                      ですので、高速RAM上にスタックを切り替えて利用する場合は、
//                      ご注意ください。
// 2004/12/12 by N.SAWA 代替バッファ未割り当ての状態で、割り込み中に初回読み込み
//                      を行った場合、割り込み内でpceHeapAlloc()を使ってしまうこ
//                      とになり危険ですので、代替バッファは無条件で最初に確保し
//                      てしまうことにしました。
// 2004/12/13 by N.SAWA mmcWriteSector()を追加しました。
//                      それに伴い、mmcByteRecv()を復元しました。
// 2005/02/09 by まどか PMFプレイヤーを製作する際に、読み込み速度が間に合わないので
//                      いくつかの個所を微妙に高速化。
//                      そして、mmcSendDataCoreとmmcSendCommandAndRecvResponseを
//                      高速RAM上に移動。※カーネル側では高速RAM上に置きません。
// 2005/03/12 by まどか mmcFileReadSct内でグローバル変数をローカルに移して使用し
//                      微妙に高速化を図ってみました。
//                      また、nsawaさんの書き込み関連関数を#if 0で隠しました。
// 2005/03/12 by まどか Ver.1.27からカーネル側でMMCの初期化を試みるようにしたの
//                      で、その実行結果を取得する
//                      unsigned char mmcGetInitResult(void) を追加。
// 2005/03/20 by まどか 512MB以上1GB以内のメモリカードに対応しました。
// 2005/03/20 by まどか 高速化のため、mmcFileFindNext()での接続確認は省きました。
// 2005/04/02 by まどか システムメニューで設定するカーネル側MMC初期化有効フラグ
//                      を追加しました。
// 2005/04/05 by まどか mmcFileReadSctでptr=NULL,len=0がまた正常に動かなくなって
//                      しまっていた不具合を修正しました。
//                      この修正に伴ない、mmcFileReadMMCSctに切り詰め処理も追加。
// 2005/04/24 by まどか ファイル検索時に扱える最大サイズぴったりのファイルがはじ
//                      かれる条件式だったのを修正。(情報提供:nsawaさん)
// 2005/05/04 by まどか MMCライブラリをカーネルではなくアプリ側で利用する場合は、
//                      ワークエリアをヒープから確保しないで、グローバル配列とし
//                      て定義できるように変更。従来どおりヒープから取ることも
//                      可能です。ワークエリアをグローバル配列で取る場合は、
//                      Makefileで__MMC_WORK_GLOBAL__を定義してください。
// 2005/05/04 by まどか pceFile系APIのフックは__PCEFILE_API_HOOK__の定義をもって
//                      フックする・しないを制御することにしました。
// 2005/05/07 by まどか 扱える最大ファイルサイズがカードの容量を超える場合は、最大
//                      ファイルサイズをカードの容量に切り詰めるように修正しました。
//                      これにより、64MBのカードで50MB以上のファイルが扱えなかった
//                      不具合も修正されます。
//
//
// H    P:http://www2.plala.or.jp/madoka/
// E-mail:madoka@olive.plala.or.jp
//
//タブは4で見てください
//---------------------------------------------------------------------------
//■■ ファイルインクルード ■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
#include <s1c33cpu.h>
#include <string.h>
#include <stdio.h>
#include <piece.h>
#include "mmc_api.h"
#ifdef __PCEKN__		// 2004/11/15 Add by N.SAWA
#include "pcekn.h"
#endif //#ifdef PCEKN	// 2004/11/15 Add by N.SAWA

//---------------------------------------------------------------------------
//■■ レジスタアクセス定義 ■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
//各レジスタへのアドレス

//入出力兼用ポート関連
//CPUデータシートP.370,P.372参照
#define P0CF	(*(volatile unsigned char*)0x402D0)		//P0機能選択レジスタ
#define P0D		(*(volatile unsigned char*)0x402D1)		//P0入出力兼用ポートデータレジスタ
#define P0IOC	(*(volatile unsigned char*)0x402D2)		//P0I/O制御レジスタ
#define P1CF	(*(volatile unsigned char*)0x402D4)		//P1機能選択レジスタ
#define P1D		(*(volatile unsigned char*)0x402D5)		//P1入出力兼用ポートデータレジスタ
#define P1IOC	(*(volatile unsigned char*)0x402D6)		//P1I/O制御レジスタ
#define PCFEX	(*(volatile unsigned char*)0x402DF)		//ポート機能拡張レジスタ

//マクロ定義
#define CARD_ENABLE		{ P1D &= ~(1 << 4); }			//カードをActiveにします
#define CARD_DISABLE	{ P1D |=  (1 << 4); }			//カードをNonActiveにします	
//---------------------------------------------------------------------------
//■■ ファイル内定数定義 ■■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
//#define MMC_RECV_TIMEOUT			0xFFFF	//受信タイムアウト値
//#define MMC_POLLING_TIMEOUT		1000	//受信ポーリング時のタイムアウト回数
//											//2004/08/30 SDカードおよび、初期化に時間が
//											//かかるMMCカードに対応
//											//(情報提供:nsawaさん)
//→2004/12/13 Move to mmc_api.h by N.SAWA

#define VALIDFILEACC 0x9ce6					//"file.c"から抜粋

//#define SEND_CLOCK_WAIT 10				//速すぎるクロック用のウエイト(48MHzモード時に使用)
//											//こういう調整をしても、曲に寄ってはデコード処理に
//											//読み込みが間に合わない、または、速過ぎる通信処理に
//											//データが化ける等の原因で途中でフレームエラーになり
//											//曲が途中で終る場合もあります。
//											//今のところ、これ以上の調整は難しそうなので、いったん
//											//このくらいの調整で止めておくことにします。
//											//
//											//ちなみにこのMMC版MP3プレイヤーでは32kHz 48kbpsの
//											//ファイルしか処理が間に合いません。
//											//かといって32kbpsのファイルは音質が悪いので、オススメ
//											//できません(^^;
//→2004/12/12 Delete by N.SAWA

//2005/05/04 Added by Madoka >>>ここから

//ワークエリアがヒープから取れなくなった時に、
//グローバル変数として定義するために利用する
//定数たち
#define PIECE_SECTOR_SIZE		(4096)		//P/ECEでのセクタサイズ
#define MMC_MIN_CLUSTER_SIZE	(512)		//MMCでの最小クラスタサイズ
#define MMC_FATCHAIN_MAX		(((MMC_FILESIZE_MAX / MMC_MIN_CLUSTER_SIZE) + 1) * sizeof(unsigned short))	//最大サイズファイルのFATチェインテーブルサイズ
#define MMC_WORKAREA_MAX		(MMC_FATCHAIN_MAX + PIECE_SECTOR_SIZE)	//MMCライブラリで利用するワークエリアの最大サイズ

//2005/05/04 Added by Madoka <<<ここまで

//---------------------------------------------------------------------------
//■■ ファイル内構造体定義 ■■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
//MMCの情報構造体
typedef struct tag_MMC_INFO
{
	unsigned char bInit;		//初期化済みフラグ
	
	MMC_CID_INFO tCIDInfo;		//CID情報
	MMC_CSD_INFO tCSDInfo;		//CSD情報

} MMC_INFO;

//MBRのパーティションエントリ構造体
typedef struct tag_PARTITIONENTRY_AT
{
    unsigned char  ucStatus;				//領域状態(0x00:スリープ、0x80:アクティブ)
    unsigned char  ucStartHead;				//領域の開始ヘッド
    unsigned short usStartCylinderSecter;	//領域の開始シリンダ・セクタ
    unsigned char  ucType;					//領域の種類
    unsigned char  ucEndHead;				//領域の終了ヘッド
    unsigned short usEndCylinderSecter;		//領域の終了シリンダ・セクタ
    unsigned long  ulOffsetSectors;			//MBRの先頭から領域の先頭までのセクタ数
    unsigned long  ulSizeSector;			//領域のセクタ数

} PARTITIONENTRY_AT;

//ディレクトリエントリ構造体
typedef struct tag_DIR_ENTRY
{
	char			sFileName[8];		//ファイル名(削除ファイルは1文字目が0xE5、未使用エントリは1文字目が0x00)
    char			sExtension[3];		//拡張子(3文字に満たない場合の残りの部分は0x20、ファイル名も同様)
    unsigned char	ucAttribute;		//属性(R:0x01、H:0x02、S:0x04、A:0x20、Dir:0x10、Vol:0x08)
    unsigned char	ucaReserved[10];	//予約(VFATで利用、利用しない場合は0フィル)
    unsigned short	usTime;				//作成時間
    unsigned short	usDate;				//作成日
    unsigned short  usCluster;			//ファイルまたはディレクトリの先頭クラスタ
    unsigned long	ulFileSize;			//ファイルサイズ(Dir、Volは常に0、それ以外のファイルで0のとき、クラスタも0)

} DIR_ENTRY;

//ファイルシステム情報構造体
typedef struct tag_FILESYSTEM_INFO
{	
	unsigned char   ucSectorsParCluster;		//1クラスタ当たりのセクタ数
	unsigned char   ucSectorsParClusterShift;	//1クラスタ当たりのセクタ数を計算で使用する時のシフト数
	unsigned char   ucClustorSizeShift;			//1クラスタ当たりのバイト数を計算に使用する時のシフト数
	unsigned short  usFATOffsetSecters;			//論理セクタ0(いわゆるブートセクタ)からFAT領域までのセクタ数(実はブートレコードのサイズですが)
	unsigned short	usRootDirEntryMax;			//ルートディレクトリエントリの最大数(FAT12/16用)
	unsigned short  usFATSize;					//FATのセクタ数(FAT12/16用)
	unsigned long   ulVolumeSerialID;			//ボリュームシリアルID(フォーマット時にランダムに割り当てられる)

	unsigned long	ulFATAddress;				//FATの物理セクタ番号
	unsigned long	ulRootDirAddress;			//ルートディレクトリの物理セクタ番号
	unsigned long	ulFirstClusterAddress;		//先頭クラスタ(クラスタ番号2)の物理セクタ番号
	unsigned short	*pusaFATchain;				//FATチェインテーブル(最大5MB分まで) 注:この仕様により5MBを超えるファイルは扱えません  
	DIR_ENTRY		*ptaDirTable;				//ディレクトリエントリテーブル(1セクタ分)
	unsigned char	*pucaReadSubBuff;			//mmcFileReadSct用の代替バッファ(4096バイト分)

} FILESYSTEM_INFO;

//内部作業用ファイル検索情報構造体
typedef struct tag_MMC_FILEFIND_INFO
{
	unsigned short usDirIndex;			//検索中のディレクトリエントリインデックス
	unsigned char  ucDummy[14];			//サイズ合わせのダミー

} MMC_FILEFIND_INFO;

//---------------------------------------------------------------------------
//■■ ファイル内グローバル変数定義 ■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
static MMC_INFO			g_tMMCInfo;
static FILESYSTEM_INFO	g_tFS_Info;
static unsigned long g_ulMaxFileSize;
static unsigned char g_ucInitResult;	//2005/03/12 Add by Madoka 初期化時の戻り値保持変数

//2005/05/04 Added by Madoka
//ワークエリアをグローバル配列にとるのは、
//アプリ側でのみ許可します
#if (!defined(__PCEKN__)) && defined(__MMC_WORK_GLOBAL__)	
static unsigned char g_ucMMCWorkArea[MMC_WORKAREA_MAX];		//MMCライブラリで利用するワークエリア	
#endif //(!defined(__PCEKN__)) && defined(__MMC_WORK_GLOBAL__)	

//---------------------------------------------------------------------------
//■■ ファイル内のみの関数プロトタイプ宣言 ■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------

//ミリ秒単位の指定の時間待つ(汎用)
void mmcWait(unsigned long ulWaitTime);

//各種I/Oの初期化
void mmcInitIO(void);

//固定ワークエリアの確保
//MMCとの通信に使用する固定長のワークエリアを
//ヒープ領域から確保します。
//確保に失敗した場合0を返します。
unsigned char mmcAllocFixedWorkArea(void);

//MMC初期化のコア処理
//初期化処理に失敗した場合0を返します。
unsigned char mmcInitCore(void);

//MMCのSPIモード初期化
//初期化に失敗した場合は0を返します。正常終了は1を返します。
unsigned char mmcInitSPIMode(void);

//1バイト送信
//inline void mmcByteSend(unsigned char ucData);
//↓2004/12/12 Change by N.SAWA
void mmcByteSend(int data);

//データの送信コア
void mmcSendDataCore(unsigned char *paData,int iDataLen);

//コマンド送信＆レスポンス受信
//CMD13だけR2レスポンス(2bytes)
unsigned short mmcSendCommandAndRecvResponse(unsigned char ucCMD,
											 unsigned long ulArg);

//1バイト受信
//inline unsigned char mmcByteRecv(void);
//↓2004/12/13 Change by N.SAWA inlineはやめました。
unsigned char mmcByteRecv(void);

//データ受信
//スタートビット以降の指定のバイト数を受信します
//タイムアウトが発生した場合0を返します。正常終了は1を返します。
//unsigned char mmcRecvData(unsigned char *paRecvBuff,int iRecvLen);
//↓2004/12/13 Change by N.SAWA
int mmcRecvData(unsigned char *paRecvBuff,int iRecvLen);

//接続スチェック
//正常に接続されている場合は0を返す。
//エラーになった場合は1を返します。
unsigned char mmcCheckConnection(void);

//マスターブートレコードからブートセクタのオフセットを取得
//失敗した場合は0xFFFFFFFFを返す
unsigned long mmcGetBootSecterOffset(void);

//ブートセクタからファイルシステム情報を取得
//引数にはmmcGetBootSecterOffsetで取得したMBRからブートセクタ
//までのオフセットを入力します。
//正常に読み出せた場合は1を、失敗した場合は0を返します。
unsigned char mmcGetFileSystemInfo(unsigned long ulBootSecterOffset);

//ディレクトリエントリ検索のコア
DIR_ENTRY* mmcSearchCore(MMC_FILEFIND_INFO* pInfo);

//FATディレクトリエントリのファイル名からP/ECEのファイル名にコピー
void mmcCopyFatFileNameToPieceFileName(char *pPieceFile,char *pFatFile,char *pFatExt);

//ファイル検索
DIR_ENTRY* mmcSearchFile(const char *fname);

//FATチェインテーブルの作成
//指定のファイルの先頭クラスタとサイズを指定します。
//FATのチェインの途中で欠陥クラスタ等が見つかった場合は1を返します。
//正常終了の場合は0を返します。
int mmcCreateFATChainTable(const unsigned short usFirstCluster,
						   const unsigned long ulFileSize);

int mmcWaitStartBit(void); // 2004/12/13 Add by N.SAWA
int mmcWaitBusy(void);     // 2004/12/13 Add by N.SAWA

int mmcSendData(const unsigned char *paSendBuff,int iSendLen);

//---------------------------------------------------------------------------
//■■ 関数インプリメント ■■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
unsigned char mmcInit(unsigned long ulMaxFileSize)
{

	//MMCの初期化
	//MMC系のAPIを使用する前に必ず1度だけ呼び出してください。
	//API内で使用するワークエリアをヒープから確保します。
	//MMCの初期化に失敗した場合またはワークエリアの確保に失敗した場合は0を返します。
	//正常終了した場合は1を返します。

	//使用するI/Oの初期化を行います
	mmcInitIO();

	//扱うファイルの最大サイズを設定
	if(ulMaxFileSize > MMC_FILESIZE_MAX)
		g_ulMaxFileSize = MMC_FILESIZE_MAX;
	else
		g_ulMaxFileSize = ulMaxFileSize;
		
	//MMC側の初期化
	//return mmcInitCore();
	//↓2004/12/12 Change by N.SAWA
	{
		static void hook_mmc();
		
		//2005/03/12 Changed by Madoka
		//Old::
		//int retval = mmcInitCore();
		//if(retval == 1) {
		//	hook_mmc();
		//}
		//
		//return retval;
		g_ucInitResult = mmcInitCore();

//2005/05/04 Changed by Madoka
//pceFile系APIのフックは__PCEFILE_API_HOOK__が定義されている
//ときのみ実行
#ifdef __PCEFILE_API_HOOK__
		if(g_ucInitResult == 1) {
			hook_mmc();
		}
#endif //__PCEFILE_API_HOOK__
		
		return g_ucInitResult;
	}
}
//---------------------------------------------------------------------------
//↓2005/03/12 Add by Madoka
unsigned char mmcGetInitResult(void)
{
	return g_ucInitResult;
}
//---------------------------------------------------------------------------
unsigned char mmcInitCore(void)
{

	//MMC初期化のコア処理
	//初期化処理に失敗した場合0を返します

	unsigned long ulOffsetSector;	//MBRからブートセクタまでのオフセットセクタ数
	
	//初期化済みフラグクリア
	g_tMMCInfo.bInit = 0;

	//MMCをSPIモードで初期化します
	if(mmcInitSPIMode() == 0)
		return 0;	//初期化失敗

	//CID情報の取得
	if(mmcGetCIDInfo(&g_tMMCInfo.tCIDInfo) == 0)
		return 0;	//取得失敗

	//CSD情報の取得
	if(mmcGetCSDInfo(&g_tMMCInfo.tCSDInfo) == 0)
		return 0;	//取得失敗

	//MBRからブートセクタまでのオフセットを取得
	ulOffsetSector = mmcGetBootSecterOffset();

	//ちゃんと取得できたか？
	if(ulOffsetSector == 0xFFFFFFFF)
		return 0;

	//2004/11/06 Add by Madoka
	//スーパーフロッピー形式に対応(情報提供:nsawaさん(http://piece.no-ip.org/))
	//ありがとうございます。
	//
	//ブートセクタからファイルシステムの情報を取得
    if((ulOffsetSector == 0xFFFFFFFF) ||
       (mmcGetFileSystemInfo(ulOffsetSector) == 0))
    {
        //スーパーフロッピー形式と仮定してリトライ
        //(スーパーフロッピー形式にはMBRが無いため、オフセット0に)
		ulOffsetSector = 0;
        if(mmcGetFileSystemInfo(ulOffsetSector) == 0)
            return 0;
    }

	//固定長のワークエリアをヒープ領域から取得
	if(mmcAllocFixedWorkArea() == 0)
		return 0;

	//2004/12/12 Add by N.SAWA
	//代替バッファ未割り当ての状態で、割り込み中に初回読み込みを行った場合、
	//割り込み内でpceHeapAlloc()を使ってしまうことになり危険ですので、
	//代替バッファは無条件で最初に確保してしまうことにしました。
	if(g_tFS_Info.pucaReadSubBuff == NULL)	//2005/03/20 Add by Madoka 代替バッファのサイズは変わらないので二重確保防止
	{
//2005/05/04 Changed by Madoka	ワークエリアはグローバル配列で	
#if (!defined(__PCEKN__)) && defined(__MMC_WORK_GLOBAL__)	
		g_tFS_Info.pucaReadSubBuff = (unsigned char*)(g_ucMMCWorkArea + MMC_FATCHAIN_MAX);
#else
		g_tFS_Info.pucaReadSubBuff = (unsigned char*)pceHeapAlloc(PIECE_SECTOR_SIZE);
		if(g_tFS_Info.pucaReadSubBuff == NULL)
			return 0;	
#endif //(!defined(__PCEKN__)) && defined(__MMC_WORK_GLOBAL__)			
	}
	
	//初期化終了
	g_tMMCInfo.bInit = 1;	//初期化済みフラグON

	//正常終了
	return 1;

}
//---------------------------------------------------------------------------
void mmcExit(void)
{

	//MMCの終了
	//MMC系のAPIが不要になったら必ず呼び出してください。
	//ここで初期化時に確保したワークエリアを解放します。

//2005/05/04 Changed by Madoka
//pceFile系APIのフックは__PCEFILE_API_HOOK__が定義されて
//いる時のみ実行
#ifdef __PCEFILE_API_HOOK__	
	//{{2004/12/12 Add by N.SAWA
	{
		static void unhook_mmc();
		unhook_mmc();
	}
	//}}2004/12/12 Add by N.SAWA
#endif //__PCEFILE_API_HOOK__

	//確保したヒープを解放
	if(g_tFS_Info.ptaDirTable != NULL)
	{
		pceHeapFree(g_tFS_Info.ptaDirTable);
		g_tFS_Info.ptaDirTable = NULL;
		g_tFS_Info.pusaFATchain = NULL;		
	}
		
	if(g_tFS_Info.pucaReadSubBuff != NULL)
	{
		pceHeapFree(g_tFS_Info.pucaReadSubBuff);
		g_tFS_Info.pucaReadSubBuff = NULL;			
	}

}
//---------------------------------------------------------------------------
void mmcWait(unsigned long ulWaitTime)
{

	//ミリ秒単位の指定の時間待つ(汎用)

	unsigned long ulStartTime = pceTimerGetCount();
	
	//指定の時間が経つまでループ
	while((pceTimerGetCount() - ulStartTime) < ulWaitTime);

}
//---------------------------------------------------------------------------
void mmcInitIO(void)
{

	//各種I/Oの初期化

	//ポート機能拡張レジスタでP04,P05,P06,P14を
	//汎用ポートとして使用できるようにする
	//(CPUデータシートP.372 ポート機能拡張レジスタを参照)
	PCFEX &= 0x8E;

	//以下の詳細はCPUデータシートP.370を参照してください

	//CS(Chip Select)に接続されているP14ポートの設定
	P1CF  &= 0xEF;		//P14を使用可能に
	P1IOC |= (1 << 4);	//P14を出力ポートとして設定

/* 個別に設定する場合
	//SCLK(Serial CLocK)に接続されているP06ポートの設定
	P0CF &= 0xBF;		//P06を使用可能に
	P0IOC |= (1 << 6);	//P06を出力ポートとして設定
	
	//DI(Data In)に接続されているP05ポートの設定
	P0CF &= 0xDF;		//P05を使用可能に
	P0IOC |= (1 << 5);	//P05を出力ポートとして設定
	
	//DO(Data Out)に接続されているP04ポートの設定
	P0CF &= 0xEF;		//P04を使用可能に
	P0IOC &= ~(1 << 4);	//P04を入力ポートとして設定
*/

	//P04,P05,P06を一度に設定
	P0CF &= 0x8F;		//P04,P05,P06を使用可能に
	P0IOC |= 0x60;		//P05,p06は出力ポートとして設定
	P0IOC &= ~(1 << 4);	//P04は入力ポートとして設定
	
}
//---------------------------------------------------------------------------
unsigned char mmcAllocFixedWorkArea(void)
{

	//固定ワークエリアの確保
	
	//固定ワークエリアの確保
	//MMCとの通信に使用する固定長のワークエリアを
	//ヒープ領域から確保します。
	//確保に失敗した場合0を返します。
	unsigned char  ucRet = 1;
	unsigned short usClusterNum;
	unsigned long  ulHeapSize = sizeof(DIR_ENTRY) * 16;	//必要なヒープサイズ

	//MMCを制御するのに必要なワークエリアを一気に確保
	//その後、必要な部分にポインタを分ける

	//2005/05/07 Added by Madoka
	//扱う最大ファイルサイズがカードの容量を超えている場合は
	//最大ファイルサイズをカード容量のサイズに切り詰める
	if(g_ulMaxFileSize > g_tMMCInfo.tCSDInfo.ulCardSize)
		g_ulMaxFileSize = g_tMMCInfo.tCSDInfo.ulCardSize;

	//扱う最大ファイルサイズから必要なクラスタ数を算出
	//必要なテーブルのサイズを算出(クラスタ数)
	usClusterNum = g_ulMaxFileSize / (1 << g_tFS_Info.ucClustorSizeShift) + 1;
		
	//確保するヒープサイズを決定
	ulHeapSize += (sizeof(unsigned short) * usClusterNum);

	//すでに取得されていないか？
	if(g_tFS_Info.ptaDirTable != NULL)
	{
		//とりあえず解放しておく
		pceHeapFree(g_tFS_Info.ptaDirTable);
		g_tFS_Info.ptaDirTable = NULL;
		g_tFS_Info.pusaFATchain = NULL;
	}

#if (!defined(__PCEKN__)) && defined(__MMC_WORK_GLOBAL__)	
	//ワークエリアを取得
	g_tFS_Info.ptaDirTable = (DIR_ENTRY*)g_ucMMCWorkArea;
#else	
	//ヒープを確保
	g_tFS_Info.ptaDirTable = (DIR_ENTRY*)pceHeapAlloc(ulHeapSize);
#endif //(!defined(__PCEKN__)) && defined(__MMC_WORK_GLOBAL__)	

	//取得できたか？
	if(g_tFS_Info.ptaDirTable == NULL)
		ucRet = 0;

	//ポインタを分ける
	
	//FATチェインテーブルのワークエリア
	g_tFS_Info.pusaFATchain = (unsigned short*)((unsigned char*)g_tFS_Info.ptaDirTable 
												+ sizeof(DIR_ENTRY) * 16); 
	
	return ucRet;

}
//---------------------------------------------------------------------------
//void mmcByteSend(unsigned char ucData)
//↓2004/12/12 Change by N.SAWA
//void mmcByteSend(int data) →2004/12/12 Move to mmc_fram.c by N.SAWA
//---------------------------------------------------------------------------
//void mmcSendDataCore(unsigned char *paData,int iDataLen) →2005/02/09 Move to mmc_fram.c by Madoka
//---------------------------------------------------------------------------
//unsigned short mmcSendCommandAndRecvResponse(unsigned char ucCMD,unsigned long ulArg)
// →2005/02/09 Move to mmc_fram.c by Madoka
//---------------------------------------------------------------------------
//unsigned char mmcByteRecv(void) →2004/12/13 Move to mmc_fram.c by N.SAWA
//---------------------------------------------------------------------------
//unsigned char  mmcRecvData(unsigned char *paRecvBuff,int iRecvLen) →2004/12/12 Move to mmc_fram.c by N.SAWA
//---------------------------------------------------------------------------
unsigned char mmcInitSPIMode(void)
{

	//MMCのSPIモード初期化
	//初期化に失敗した場合は0を返します。正常終了は1を返します。

	unsigned char CMD0[6] = { 0x40,0x00,0x00,0x00,0x00,0x95 };	//CMD0のデータフォーマット
	unsigned char CMD1[6] = { 0x41,0x00,0x00,0x00,0x00,0xFF }; 	//CMD1のデータフォーマット
	unsigned char ucRecvBuffR1 = 0xFF;							//R1レスポンス(8bit)の受信バッファ
	int i;

	//MMCのデータシート(P.99)記載のSPIモード初期化の
	//手順にそって初期化を行います
	//MMCのデータシートは以下の場所にあります
	//http://www.renesas.com/avs/resource/japan/jpn/pdf/flashcard/j603002_mmc.pdf
	//↑の他にも以下の場所にカード自体のデータシートが公開されています。
	//参考にして下さい。
	//http://www.renesas.com/avs/resource/japan/jpn/pdf/flashcard/j203658_hb28e016mm2.pdf
	//
	//ちなみに今回採用したSanDiskのMMCデータシートはこちら(英語)
	//http://media-server.amazon.com/media/mole/MANUAL000007788.pdf
	
	//デバッグ用
	//#define INIT_MMC	"MMC_INIT"
	//SendUsbCom(INIT_MMC,strlen(INIT_MMC));
	//mmcWait(USBCOM_WAIT);	

	//(1)CS = Highとして、カードをNonActiveにします。
	//   CSにはP14ポートを接続しているので、P14をHighにします。
    CARD_DISABLE


	//(2)MMCイニシャライズ用のダミークロックを74クロック以上発行します。
	for(i = 0;i < 10;++i)
		mmcByteSend(0xFF);	//8クロック分のダミー送信


	//(3)CS = LowとしてカードをActiveにし、CMD0(GO_IDLE_STATE)を送信します。

	//CS = LowとしてカードをActiveにします。
	//CSにはP14ポートを接続しているので、P14をLowにします。
    CARD_ENABLE

	//CMD0送信
	mmcSendDataCore(CMD0,sizeof(CMD0));

	
	//(4)この時点でSPIモードに切り替わります。そしてR1レスポンス(8bit)を待ちます。
	//   ここで受信タイムアウトや0x01以外を受信した場合はエラーとします。
	if((mmcRecvData(&ucRecvBuffR1,1) == 0) ||
	   (ucRecvBuffR1 != 0x01))
	{
#if 0
		#define MMC_CMD0_ERR	"MMC_CMD0_ERR"
		SendUsbCom(MMC_CMD0_ERR,strlen(MMC_CMD0_ERR));	
#endif
		//エラーなので戻る
		CARD_DISABLE
		return 0;
	}

	//デバッグ用
	//#define MMC_CMD0_OK	"MMC_CMD0_OK"
	//SendUsbCom(MMC_CMD0_OK,strlen(MMC_CMD0_OK));	
	//mmcWait(USBCOM_WAIT);
	
	//(5)CMD1(SEND_OP_CMD)を送信し、MMCからのR1レスポンス(8bit)を待ちます。
	//   R1レスポンスが0x01の場合CMD1を再送信し、R1レスポンスが0x00になる
	//   までポーリングします。
	//   また、R1レスポンスが0x00 or 0x01以外の場合や受信タイムアウトになっ
	//   た場合はエラーとします。

	//CMD1送信
	mmcSendDataCore(CMD1,sizeof(CMD1));

	//受信ポーリング
	for(i = 0;i < MMC_POLLING_TIMEOUT;++i)
	{
	
		//受信タイムアウトになったか？
		if(mmcRecvData(&ucRecvBuffR1,1) == 0)
		{
			//抜ける
			i = MMC_POLLING_TIMEOUT;
			break;
		}

		//受信値をチェック
		if(ucRecvBuffR1 == 0x00)
		{
			//初期化完了で抜ける
			break;
		}
		else if(ucRecvBuffR1 == 0x01)
		{
			//ビジー
		
			//デバッグ用
			//#define MMC_CMD1_BUSY	"MMC_CMD1_BUSY"
			//SendUsbCom(MMC_CMD1_BUSY,strlen(MMC_CMD1_BUSY));	
			//mmcWait(USBCOM_WAIT);

			//CMD1再送
			mmcSendDataCore(CMD1,sizeof(CMD1));
		}
		else
		{
			//エラーで抜ける
			i = MMC_POLLING_TIMEOUT;
			break;
		}

	}

	//エラーか？
	if(i == MMC_POLLING_TIMEOUT)
	{
#if 0
		#define MMC_CMD1_ERR	"MMC_CMD1_ERR"
		SendUsbCom(MMC_CMD1_ERR,strlen(MMC_CMD1_ERR));	
#endif
		//エラーなので戻る
		CARD_DISABLE
		return 0;
	}

	//デバッグ用
	//#define MMC_CMD1_OK	"MMC_CMD1_OK"
	//SendUsbCom(MMC_CMD1_OK,strlen(MMC_CMD1_OK));	
	//mmcWait(USBCOM_WAIT);
	
	//用が済んだので、CS = Highとして、カードをNonActiveにします。
	//CSにはP14ポートを接続しているので、P14をHighにします。
   	CARD_DISABLE

	return 1;

}
//---------------------------------------------------------------------------
unsigned char mmcGetCIDInfo(MMC_CID_INFO *pCIDInfo)
{

	//CIDレジスタの内容を取得
	//取得に失敗した場合は0を、成功した場合は1を返します。
	//引数にはCIDレジスタ情報の格納先アドレスを指定してください。
	unsigned char ucaRecvCID[MMC_CID_INFO_SIZE];	//CID情報受信用バッファ
	unsigned char ucRet = 0;

	//カードをActiveに
	CARD_ENABLE

	//CMD10送信してR1レスポンスを受信
	if(mmcSendCommandAndRecvResponse(10,0) == 0x00)
	{
	
		//レスポンス後に送信されるCIDデータを受信
		if(mmcRecvData(ucaRecvCID,MMC_CID_INFO_SIZE) == 1)
		{
	
			//受信したデータを分解して構造体に保存

			//MID(ManufactureID)の取得
			pCIDInfo->ucMID = ucaRecvCID[0];
		
			//OID(OEM/Application ID)の取得
			pCIDInfo->usOID = (unsigned short)((ucaRecvCID[1] << 8) | ucaRecvCID[2]);

			//PNM(Product name)の取得
			pCIDInfo->ucPNM[0] = ucaRecvCID[3];
			pCIDInfo->ucPNM[1] = ucaRecvCID[4];
			pCIDInfo->ucPNM[2] = ucaRecvCID[5];
			pCIDInfo->ucPNM[3] = ucaRecvCID[6];
			pCIDInfo->ucPNM[4] = ucaRecvCID[7];
			pCIDInfo->ucPNM[5] = ucaRecvCID[8];

			//PRV(Product revision)の取得
			pCIDInfo->ucPRV = ucaRecvCID[9];

			//PSN(Product serial number)の取得
			pCIDInfo->ulPSN = (unsigned long)((ucaRecvCID[10] << 24) |
											  (ucaRecvCID[11] << 16) |
											  (ucaRecvCID[12] <<  8) |
											   ucaRecvCID[13]);
			
			//MDT(Manufacturing date)の取得
			pCIDInfo->ucMDT = ucaRecvCID[14];

			//CRC(7-bit CRC checksum)の取得
			pCIDInfo->ucCRC = ucaRecvCID[15] >> 1;	//LSB1bitは関係ないので削除
	
			//デバッグ用
			//	SendUsbCom((unsigned char*)pCIDInfo,sizeof(MMC_CID_INFO));	
			//	mmcWait(USBCOM_WAIT);
			
			//受信に成功
			ucRet = 1;

			//CIDデータの後に付いて来る
            //16bit分のCRCをダミークロックを
            //送って無視する

            //16クロック分のダミー送信
            mmcByteSend(0xFF);
            mmcByteSend(0xFF);

		}

	}

	//カードをNonActiveに
	CARD_DISABLE

	return ucRet;

}
//---------------------------------------------------------------------------
unsigned char mmcGetCSDInfo(MMC_CSD_INFO *pCSDInfo)
{

	//CSDレジスタの内容を取得
	//取得に失敗した場合は0を、成功した場合は1を返します。
	//引数にはCSDレジスタ情報の格納先アドレスを指定してください。
	unsigned char ucaRecvCSD[MMC_CSD_INFO_SIZE];	//CSD情報受信用バッファ
	unsigned char ucRet = 0;

	//カードをActiveに
	CARD_ENABLE

	//CMD9送信してR1レスポンスを受信
	if(mmcSendCommandAndRecvResponse(9,0) == 0x00)
	{

		//レスポンス後に送信されるCSDデータを受信
		if(mmcRecvData(ucaRecvCSD,MMC_CSD_INFO_SIZE) == 1)
		{
	
			//受信したデータを分解して構造体に保存
			//
			//本当は16バイト(128bit)分全部構造体で保存しておくのが
			//良いのだろうと思うけど、このCSDに関しては各種データが
			//ビット単位で区切られてて、構造体にビットフィールドとか
			//使って保存しようとしても、その内容を確認するためにUSB
			//でVCのアプリと通信した時にP/ECEのGCCとVCでビットフィー
			//ルドの仕様が違うのか、上手く値が渡せなかったりして大変
			//だったので、今回は必要最小限の内容のみを保存することに
			//します。

			unsigned short C_SIZE;
			unsigned char C_SIZE_MULT;
			unsigned long MULT;
			unsigned long BLOCKNR;
            
			//読み込みブロック長を取得
			//SanDiskの64MBでは512でした
			pCSDInfo->usReadBlockLength = (unsigned short)(1 << (ucaRecvCSD[5] & 0x0F));

			//書き込みブロック長を取得
			//SanDiskの64MBでは512でした
			pCSDInfo->usWriteBlockLength = (unsigned short)(1 << (((ucaRecvCSD[12] & 0x03) << 2) |
																  ((ucaRecvCSD[13] & 0xC0) >> 6)));
		
			//注 MMCは読み書きに対するブロック長がデフォ
			//   ルトで512バイトというのが標準のようなの
			//   で、今回製作したデータの読み書きAPIでは
			//   ブロック長512バイトとして処理を行ってい
			//   ます。
			//   もし、お使いのMMCのブロック長が512バイ
			//   ト以外の場合は、CMD16でブロック長を512
			//   バイトに設定するか、APIの中身を適切な
			//   ものに書き換えてください。


			//カード容量を計算
			//
			//カード容量はC_SIZE,C_SIZE_MULTおよびREAD_BLK_LEN
			//から以下の様に計算されます。
			//
			//memory_capacity = BLOCKNR * BLOCK_LEN
			//
			//BLOCKNR   = (C_SIZE+1) * MULT
			//MULT      = 2^(C_SIZE_MULT+2)
			//BLOCK_LEN = 2^READ_BLK_LEN
			//
			//(このうちBLOCK_LENはすでにusReadBlockLengthとして
			// 計算済みなので、その値を利用します)
			//
			
			//CSIZEを取得
			//SanDiskの64MBでは3919でした
			C_SIZE = (unsigned short)(((ucaRecvCSD[6] & 0x03) << 10) |
									   (ucaRecvCSD[7] << 2) | 
									  ((ucaRecvCSD[8] & 0xC0) >> 6));
	
			//C_SIZE_MULTを取得
			//SanDiskの64MBでは3でした
			C_SIZE_MULT = (unsigned char)(((ucaRecvCSD[ 9] & 0x03) << 1) | 
										  ((ucaRecvCSD[10] & 0x80) >> 7));

			//MULTを計算
			MULT = 1 << (C_SIZE_MULT+2);

			//BLOCKNRの計算と保存
			//SanDiskの64MBでは125440になりました
			BLOCKNR = (C_SIZE+1) * MULT;
			pCSDInfo->ulTatalBlocks = BLOCKNR;	

			//カード容量を計算
			//SanDiskの64MBでは64225280になりました
			pCSDInfo->ulCardSize = (unsigned long)(BLOCKNR * pCSDInfo->usReadBlockLength);

			//デバッグ用
			//	SendUsbCom((unsigned char*)pCSDInfo,sizeof(MMC_CSD_INFO));	
			//	mmcWait(USBCOM_WAIT);
				
			//受信に成功
			ucRet = 1;

			//CSDデータの後に付いて来る
            //16bit分のCRCをダミークロックを
            //送って無視する

            //16クロック分のダミー送信
            mmcByteSend(0xFF);
            mmcByteSend(0xFF);
			
		}

	}

	//カードをNonActiveに
	CARD_DISABLE

	return ucRet;

}
//---------------------------------------------------------------------------
unsigned char mmcCheckConnection(void)
{

	//接続チェック
	//正常に接続されている場合は0を返す。
	//エラーになった場合は1を返します。
	
	unsigned char ucRet = 0;
	unsigned short usRes;

	//ステータス問い合わせ

	//カードをActiveに
	CARD_ENABLE

	//CMD13送信してR2レスポンスを受信
	usRes = mmcSendCommandAndRecvResponse(13,0);

	//カードをNonActiveに
	CARD_DISABLE

	//レスポンスチェック
	if(usRes != 0)
	{
		//初期化処理を試してみる
		if(mmcInitSPIMode() == 0)
			ucRet = 1;	//エラー
	}
	
	return ucRet;

}
//---------------------------------------------------------------------------
unsigned char mmcReadSector(unsigned long ulSector,unsigned char *pucBuff)
{

	//セクタリード
	//MMCから1セクタ分(512バイトを想定)データを読み出します
	//
	//unsigned long ulSector	読み出し対象セクタ(物理セクタ番号)
	//unsigned char *pBuff		バッファ(必ず512バイト以上あること)
	//
	//正常に読み出せた場合は1を、エラーが発生した場合は0を返します
	unsigned long ulAddress = ulSector * g_tMMCInfo.tCSDInfo.usReadBlockLength;	//アドレス
	unsigned char ucRet = 0;

	//カードをActiveに
	CARD_ENABLE

	//CMD17送信してR1レスポンスを受信
	if(mmcSendCommandAndRecvResponse(17,ulAddress) == 0x00)
	{

		//レスポンス後に送信されるセクタデータを受信
		if(mmcRecvData(pucBuff,g_tMMCInfo.tCSDInfo.usReadBlockLength) == 1)
		{
	
			//セクタデータの最後に付いて来る
			//16bit分のCRCをダミークロックを
			//送って無視する
			
			//16クロック分のダミー送信
			mmcByteSend(0xFF);
			mmcByteSend(0xFF);

			//デバッグ用
			//	SendUsbCom((unsigned char*)pBuff,g_tMMCInfo.tCSDInfo.usReadBlockLength);	
			//	mmcWait(USBCOM_WAIT);

			//正常終了
			ucRet = 1;

		}
		
	}
	
	//カードをNonActiveに
	CARD_DISABLE

	return ucRet;

}
//---------------------------------------------------------------------------
unsigned long mmcGetBootSecterOffset(void)
{

	//マスターブートレコードからブートセクタのオフセットを取得

	unsigned char ucaMBR[512];					//マスターブートレコードのデータ
	PARTITIONENTRY_AT tPartition[4];			//パーティションエントリテーブル
	unsigned long ulOffsetSector = 0xFFFFFFFF;	//MBRからブートセクタまでのオフセットセクタ数
	int i;

	//以下の操作は
	//http://hp.vector.co.jp/authors/VA013937/editdisk/tech.html
	//の情報を参考にしています。

	//MBR(Master Boot Record)の読み込み
	if(mmcReadSector(0,ucaMBR) == 1)
	{
	
		//FAT署名(0xAA55)をチェック
		if((ucaMBR[510] == 0x55) && (ucaMBR[511] == 0xAA))
		{

			//パーティションテーブルの読み込み
			memcpy(tPartition,&ucaMBR[446],sizeof(PARTITIONENTRY_AT) << 2);

			//アクティブなパーティションを探す
			for(i = 0;i < 4;++i)
			{
				//状態をチェック
				if(tPartition[i].ucStatus == 0x80)
					break;
			}

			//見つかったか？
			if(i < 4)
			{
				
				//MBRからブートセクタまでのオフセットセクタ数を取得
				ulOffsetSector = tPartition[i].ulOffsetSectors;
	
				//デバッグ用
				//SendUsbCom((unsigned char*)&ulOffsetSector,4);	
				//mmcWait(USBCOM_WAIT);
				
			}	
			else
			{
				//見つからなかった場合は一番最初のパーティションを
				//アクティブと見なします。
				//※SDカード用の対処
				//(情報提供:nsawaさん)
				//
				//もし、SDカード関連デバイスの開発でライセンスがどう
				//こうとか言われたら、この部分をコメント化すればSD
				//カードでは動かないと思います。
				//

				//MBRからブートセクタまでのオフセットセクタ数を取得
				ulOffsetSector = tPartition[0].ulOffsetSectors;
			}

		}
        	
	}

	return ulOffsetSector;

}
//---------------------------------------------------------------------------
unsigned char mmcGetFileSystemInfo(unsigned long ulBootSecterOffset)
{

	//ブートセクタからファイルシステム情報を取得
	//引数にはmmcGetBootSecterOffsetで取得したMBRからブートセクタ
	//までのオフセットを入力します。
	//正常に読み出せた場合は1を、失敗した場合は0を返します。
	
	unsigned char ucaBoot[512];
	unsigned long ulClustorSize;
	unsigned char ucRet = 0;
    	
	//以下の操作は
	//http://triaez.kaisei.org/~s-zouda/pc/fat32.html
	//を参考にしています。

	//ブートセクタの読み込み
	if(mmcReadSector(ulBootSecterOffset,ucaBoot) == 1)
	{
	
		//FAT署名(0xAA55)をチェック
		if((ucaBoot[510] == 0x55) && (ucaBoot[511] == 0xAA))
		{

			//1クラスタあたりのセクタ数を取得
			g_tFS_Info.ucSectorsParCluster = ucaBoot[0x0D];

			//1クラスタあたりのセクタ数から、計算で使用する際
			//のシフト数を算出して保存
			for(g_tFS_Info.ucSectorsParClusterShift = 0;
				g_tFS_Info.ucSectorsParClusterShift < 16;
				++g_tFS_Info.ucSectorsParClusterShift)
			{
                if((1 << g_tFS_Info.ucSectorsParClusterShift)
				   == g_tFS_Info.ucSectorsParCluster) 
					break;
			}			
			
			//1クラスタあたりのセクタ数から、1クラスタあたりのバイト数を
			//計算で使用する際のシフト数を算出して保存
			ulClustorSize = (g_tMMCInfo.tCSDInfo.usReadBlockLength *
							 g_tFS_Info.ucSectorsParCluster);
			for(g_tFS_Info.ucClustorSizeShift = 0;
				g_tFS_Info.ucClustorSizeShift < 16;
				++g_tFS_Info.ucClustorSizeShift)
			{
                if((1 << g_tFS_Info.ucClustorSizeShift) == ulClustorSize) 
					break;
			}
			
			//論理セクタ0(いわゆるブートセクタ)からFAT領域までのセクタ数を取得
			memcpy(&g_tFS_Info.usFATOffsetSecters,&ucaBoot[0x0E],2);

			//ルートディレクトリエントリの最大数を取得
			memcpy(&g_tFS_Info.usRootDirEntryMax,&ucaBoot[0x11],2);

			//FATのセクタ数を取得
			memcpy(&g_tFS_Info.usFATSize,&ucaBoot[0x16],2);

			//ボリュームラベルシリアルIDを取得
			memcpy(&g_tFS_Info.ulVolumeSerialID,&ucaBoot[0x27],4);

			//FATの位置を算出
			g_tFS_Info.ulFATAddress
		     = ulBootSecterOffset + g_tFS_Info.usFATOffsetSecters;

			//ルートディレクトリの位置を算出
			g_tFS_Info.ulRootDirAddress
			 = g_tFS_Info.ulFATAddress + 2 * g_tFS_Info.usFATSize;

			//先頭クラスタ(クラスタ番号2)の位置を算出
			g_tFS_Info.ulFirstClusterAddress 
			 = g_tFS_Info.ulRootDirAddress 
			   + (g_tFS_Info.usRootDirEntryMax >> 4);	// = g_tFS_Info.usRootDirEntryMax * 32 / 512;
														// = (g_tFS_Info.usRootDirEntryMax << 5) >> 9 	
		
			//読み込み終了
			ucRet = 1;

		}
        	
	}	

	return ucRet;

}
//---------------------------------------------------------------------------
static int mmcCheck_fnameForDirEntry( const char *p )
{

	//ディレクトリエントリ用ファイル名チェック関数
	//P/ECEカーネルソース"file.c"のcheck_fname
	//を参考にしました。
	//
	//P/ECEは英小文字と数字しか扱えませんが、
	//FAT16のディレクトリエントリには大文字で
	//登録されるので、ここでは大文字OKとして
	//チェックします。
	//
	//また8文字に満たないファイル名では、空白
	//で埋められるので、ディレクトリエントリ
	//でのファイル名でのみ空白を許可します。
	//
	//ディレクトリエントリのファイル名は8文字
	//固定なので、8文字以上はチェックしません。
    //
	//また、1文字でも小文字の混ざったファイル名は
	//ロングファイル名として扱われるので、ここでは
	//検索できません。
	//ここでチェックできるのは全部大文字か全部小文字
	//のファイル名で8.3形式のものだけです。注意。
	//
	int i;
	char a;

	for(i = 0;i < 8;++i)
	{
		//1文字取得
		a = *p++;
		
		//P/ECEで許される文字か？
		if(!(// ↓ファイル名に許される文字セット+α [0-9A-Z_ ]
			 ( a >= '0' && a <= '9' ) ||
			 ( a >= 'A' && a <= 'Z' ) ||
			 ( a == '_' ) ||
			 ( a == ' ' ) 
			 //{{2004/11/15 Add by N.SAWA
			 //  MS-DOSのショートファイル名として許される文字を追加しました。(カナ、漢字を除く)
			 //  MS-DOSのショートファイル名として許される文字は、次のとおりです。
			 //    A～Z 0～9 $ & # { } ~ % ' ( ) - @ ^ ` ! _ カナ 漢字
			 //  【参考】アスキー出版局『標準MS-DOSハンドブック』第1版 p.25 Table2.1 ファイル名
			              ||
			 ( a == '$' ) ||
			 ( a == '&' ) ||
			 ( a == '#' ) ||
			 ( a == '{' ) || ( a == '}' ) ||
			 ( a == '~' ) ||
			 ( a == '%' ) ||
			 ( a == '\'' ) ||
			 ( a == '(' ) || ( a == ')' ) ||
			 ( a == '-' ) ||
			 ( a == '@' ) ||
			 ( a == '^' ) ||
			 ( a == '`' ) ||
			 ( a == '!' )
			 //}}2004/11/15 Add by N.SAWA
		    )
		   ) break;	//許可されていない文字だと抜ける
	}

	//8文字以内に許可されない文字があったか？
	if(i < 8) return 2;

	//それ以外は正常なファイル名とする
	return 0;

}
//---------------------------------------------------------------------------
DIR_ENTRY* mmcSearchCore(MMC_FILEFIND_INFO* pInfo)
{

	//ディレクトリエントリ検索のコア
	DIR_ENTRY *pDir = NULL;
	unsigned char ucFirst;

	//ディレクトリエントリを取得するワークエリアは確保されているか？
	if(g_tFS_Info.ptaDirTable == NULL)
		return NULL;	//失敗
	
	//有効なディレクトリエントリが見つかるまでループ
	while(1)
	{
		//最後まで検索したか？
		if(pInfo->usDirIndex >= g_tFS_Info.usRootDirEntryMax)
			break;

		//デバッグ用
		//SendUsbCom("ディレクトリエントリチェック  開始",34);	
		//mmcWait(USBCOM_WAIT);

		//新たにディレクトリエントリを読み込む必要があるか？
		if((pInfo->usDirIndex % 16) == 0)
		{

			//デバッグ用
			//SendUsbCom("ディレクトリエントリ読み込み",28);	
			//mmcWait(USBCOM_WAIT);

			//次のセクタを読み込む
			if(mmcReadSector(g_tFS_Info.ulRootDirAddress + (pInfo->usDirIndex >> 4),	// >> 4 = / 16
							 (unsigned char*)g_tFS_Info.ptaDirTable) == 0)
				break;		//セクタの読み込みに失敗			
		}
	
		//ディレクトリエントリのポインタを取得
		pDir = &g_tFS_Info.ptaDirTable[pInfo->usDirIndex % 16];

		//デバッグ用
		//SendUsbCom((unsigned char *)pDir,sizeof(DIR_ENTRY));	
		//mmcWait(USBCOM_WAIT);
		
		//ファイル名の1文字目を取得
		ucFirst = (unsigned char)pDir->sFileName[0];

		//次のディレクトリエントリへ進めておく
		pInfo->usDirIndex++;

		//有効なディレクトリエントリ(ファイルを表す)か？
		if((ucFirst != 0x00) && (ucFirst != 0xE5) && 
		   (ucFirst != 0x2E) && (ucFirst != 0x05))
		{

			//ファイルサイズが扱える最大サイズを超えていないか？
			//またはファイル名がP/ECEで利用できるものか？
            if((pDir->ulFileSize > g_ulMaxFileSize) ||		//2004/04/24 Changed by Madoka >= → >
			   //{{2004/11/15 Add by N.SAWA
			   //  ディレクトリ・エントリまたはボリュームラベルの属性を持っていないか？
			   //  VFATのロングファイル名スロットは、読出し専用ファイル、不可視属性、システムファイル、ボリュームラベルの
			   //  属性を持っているので、ボリュームラベルの属性を持つエントリを除外することによってスキップできます。
			   //  【参考】『JF: Linux Kernel 2.4 Documentation: vfat.txt』
			   //          http://www.linux.or.jp/JF/JFdocs/kernel-docs-2.4/filesystems/vfat.txt.html
			   (pDir->ucAttribute & (0x10 | 0x08)) ||
			   //}}2004/11/15 Add by N.SAWA
			   (mmcCheck_fnameForDirEntry(pDir->sFileName)))
			{
			
#if 0	//今は使わないので無視させる

				//デバッグ用
				//SendUsbCom("ファイルエラー",14);	
				//mmcWait(USBCOM_WAIT);

				if(pDir->ulFileSize >= g_ulMaxFileSize)
				{
					//デバッグ用
					//SendUsbCom("ファイルサイズが大きい",22);	
					//mmcWait(USBCOM_WAIT);

					//SendUsbCom((unsigned char *)&pDir->ulFileSize,4);	
					//mmcWait(USBCOM_WAIT);
				}

				if(mmcCheck_fnameForDirEntry(pDir->sFileName))
				{
					//デバッグ用
					//SendUsbCom("ファイル名が変",14);	
					//mmcWait(USBCOM_WAIT);

                    //{
					//	char sss;
					//	sss = '0' + (unsigned char)mmcCheck_fnameForDirEntry(pDir->sFileName);
					//	SendUsbCom(&sss,1);	
					//	mmcWait(USBCOM_WAIT);
					//}

					//SendUsbCom((unsigned char *)pDir->sFileName,sizeof(pDir->sFileName));	
					//mmcWait(USBCOM_WAIT);				
				}

#endif	//今は使わないので無視させる

				//ディレクトリエントリのポインタを無効化
				pDir = NULL;

				//次に進める
				continue;

			}
			
			//見つかったのでループを抜ける
			break;
 
		}
		
		//ディレクトリエントリのポインタを初期化
		pDir = NULL;

		//デバッグ用
		//SendUsbCom("だめでした",10);	
		//mmcWait(USBCOM_WAIT);

	}
	
	return pDir;

}
//---------------------------------------------------------------------------
void mmcCopyFatFileNameToPieceFileName(char *pPieceFile,char *pFatFile,char *pFatExt)
{

	//FATディレクトリエントリのファイル名からP/ECEのファイル名にコピー

	int i;
	
	//まずはファイル名をコピー
	//P/ECEはファイル名に空白が使えないので、
	//空白が出てきた時点で終わりにする
	for(i = 0;i < 8;++i)
	{
		//空白か？
		if(*pFatFile == ' ')
			break;
		
		//1文字コピー
		*pPieceFile = *pFatFile;

		//小文字変換
		if((*pPieceFile >= 'A') && (*pPieceFile <= 'Z'))
			*pPieceFile += 0x20;

		//次の文字へ
		pPieceFile++;
		pFatFile++;
	}
	
	//'.'を追加
	*pPieceFile = '.';
	pPieceFile++;
		
	//拡張子を追加
	for(i = 0;i < 3;++i)
	{
		//空白か？
		if(*pFatExt == ' ')
			break;
		
		//1文字コピー
		*pPieceFile = *pFatExt;

		//小文字変換
		if((*pPieceFile >= 'A') && (*pPieceFile <= 'Z'))
			*pPieceFile += 0x20;

		//次の文字へ
		pPieceFile++;
		pFatExt++;
	}
	
	//最後には終端文字をセット
	*pPieceFile = '\0';

}
//---------------------------------------------------------------------------
int mmcFileFindOpen(FILEINFO *pfi)
{

	//MMC版pceFileFindOpen
	//正常に終了した場合は0を返します。
	//MMCが接続されていない等のエラーが発生した場合は1を返します。

	MMC_FILEFIND_INFO *pWork;
    
	//MMCの接続確認
	if(mmcCheckConnection() == 1)
		return 1;	//MMC接続エラー
 	
	//ファイル検索の準備をします

	//検索先ディレクトリエントリ情報を初期化
	pWork = (MMC_FILEFIND_INFO*)pfi->works;

	pWork->usDirIndex = 0;
	
	return 0;

}
//---------------------------------------------------------------------------
int mmcFileFindNext(FILEINFO *pfi)
{

	//MMC版pceFileFindNext
	//戻り値	0:ファイルが見つからない(pfiの内容は無効)
	//		1:ファイルが見つかった(pfiの内容は有効)
	
	DIR_ENTRY *pDir;

	//MMCの接続確認
	//if(mmcCheckConnection() == 1)
	//	return 0;	//MMC接続エラー
 	//→2005/03/20 Delete by Madoka 高速化のため接続確認は省きました。
 	
	//ディレクトリエントリの検索
	pDir = mmcSearchCore((MMC_FILEFIND_INFO*)pfi->works);

	//有効なディレクトリエントリが見つかったか？
	if(pDir == NULL) return 0;	//見つかりませんでした
	
	//見つかったディレクトリエントリから必要な情報を取得
	mmcCopyFatFileNameToPieceFileName(pfi->filename,
								      pDir->sFileName,pDir->sExtension);
	pfi->length = pDir->ulFileSize;
	//pfi->attr = pDir->ucAttribute;	//今は予約されているので
	pfi->attr   = 0;					//0を入れておきます
	pfi->adrs   = 0;
		
	return 1;

}
//---------------------------------------------------------------------------
int mmcFileFindClose(FILEINFO *pfi)
{

	//MMC版pceFileFindClose
	//戻り値は常に0です(正常終了)

	//P/ECE版でもまだ何もしていないようなので、
	//MMC版も何もせずに返します
	
	return 0;

}
//---------------------------------------------------------------------------
DIR_ENTRY* mmcSearchFile(const char *fname)
{

	//ファイル検索
	DIR_ENTRY *pDir = NULL;
	FILEINFO tFileInfo;
	MMC_FILEFIND_INFO *pWork = (MMC_FILEFIND_INFO*)tFileInfo.works;
	
	//ディレクトリエントリの最初から検索
	pWork->usDirIndex = 0;

	while(1)
	{

		//有効なディレクトリエントリを取得
		pDir = mmcSearchCore(pWork);
		
		//見つからなかったのでおわり
		if(pDir == NULL) break;

		//ファイル名をP/ECE用に変換
		mmcCopyFatFileNameToPieceFileName(tFileInfo.filename,
										  pDir->sFileName,pDir->sExtension);
	
		//ファイル名比較
		if(strcmp(tFileInfo.filename,fname) == 0) break;
	
	}

	return pDir;
}
//---------------------------------------------------------------------------
int mmcFileOpen(FILEACC *pfa,const char *fname,int mode)
{

	//MMC版pceFileOpen
	//正常終了の場合0を返します。
	//ファイルが見つからない場合は1を返します。
	//またファイルのサイズが大きすぎる場合は2を返します。
	//ファイルのデータがあるクラスタに欠陥等が見つかった場合は3を返します。
	//FATチェインテーブルを作成するのに必要なヒープ領域が足りない場合は4を
	//返します。
	//その他のエラーは5を返します。

	DIR_ENTRY *pDir = NULL;

	//MMCの接続確認
	if(mmcCheckConnection() == 1)
		return 5;	//MMC接続エラー
 	
	//ファイル名で検索し、ディレクトリエントリ情報を取得
	pDir = mmcSearchFile(fname);
	
	//一応チェック
	if(pfa == NULL) return 5;

	//初期化
	pfa->valid = 0;

	if(pDir == NULL) return 1;	// no file

	//ファイルサイズは大きすぎないか？
	if(pDir->ulFileSize > g_ulMaxFileSize) return 2;	//サイズオーバー

	//FILEACC構造体に値をとりあえずセット
	pfa->fsize = pDir->ulFileSize;		//ファイルのサイズ
	pfa->chain = 0;						//MMC版では使わないので0
	pfa->valid = VALIDFILEACC;			//とりあえずこうしておく

	
	//FATを参照してFATチェインテーブルを作成します。
	//ここで1つしかないFATチェインテーブルに値をセットするので、
	//mmcFileOpenは複数ファイルを同時に開けません。注意。
	switch(mmcCreateFATChainTable(pDir->usCluster,pDir->ulFileSize))
	{
		case 0:	//正常終了
			break;

		case 1:	//FATチェインテーブルの作成に失敗(ヒープ領域が足りない)
			return 4;

		case 2:	//クラスタに欠陥等が見つかった
			return 3;	
	}

	//正常終了
	return 0;

}
//---------------------------------------------------------------------------
int mmcCreateFATChainTable(const unsigned short usFirstCluster,
						   const unsigned long ulFileSize)
{

	//FATチェインテーブルの作成
	//指定のファイルの先頭クラスタを指定します。
	//必要なサイズのFATチェインテーブルが作成できなかった場合は1を返します。
	//FATのチェインの途中で欠陥クラスタ等が見つかった場合は2を返します。
	//正常終了の場合は0を返します。

	unsigned short usNextCluster = usFirstCluster;	
	unsigned char  ucFATBuff[512];
	unsigned long  ulFATSecter;
	unsigned long  ulLastFATSecter = 0;
	int iFATOffset;
	int iChainIndex = 0;
	int iRet = 0;

	//最初のクラスタ番号を保存
	g_tFS_Info.pusaFATchain[iChainIndex] = usNextCluster;
	iChainIndex++;

	//クラスタチェインが終るまでループ
	while(usNextCluster != 0xFFFF)
	{
		
		//該当クラスタに対応するFAT情報がある物理セクタ番号を算出
		//MMCで使われるFATはFAT16で、FATにはファイルのデータが
		//入っている先頭クラスタに続くクラスタ番号が各クラスタに
		//対応するFATに書かれている。1FAT当たりの情報量は2バイト。
		//
		//よって、欲しい物理セクタ番号は
		// FAT先頭の物理セクタ番号 +
		// (usNextCluster * 2 / 512) 
		// 
		ulFATSecter = g_tFS_Info.ulFATAddress 
					   + (usNextCluster >> 8);	// (a * 2 / 512) = ((a << 1) >> 9)
												// ((a << 1) >> 9) = a >> 8 	
														
		//前回読み込んだ範囲と違うところにあるか？
		if(ulLastFATSecter != ulFATSecter)
		{
			//該当クラスタ付近のFAT情報を1セクタ分読み込む
			mmcReadSector(ulFATSecter,ucFATBuff);

			//今回読み込んだ物理セクタ番号を保存
			ulLastFATSecter = ulFATSecter;
		}

		//取得したFAT情報は512bytes / 2bytes = 256クラスタ単位なので、
		//該当クラスタに対応するFATへのオフセットを算出
		iFATOffset = (int)(usNextCluster % 256);

		//該当クラスタに対応するFAT情報を取得
		usNextCluster = *((unsigned short*)ucFATBuff + iFATOffset);

		//次のクラスタが正常範囲内か？
		if(((usNextCluster < 0x0002) || (usNextCluster > 0xFFF6)) &&
		   (usNextCluster != 0xFFFF))	
		{
			//欠陥クラスタ等が見つかりました
			iRet = 1;

			//チェック終了
			break;
		}

		//クラスタチェインを保存
		g_tFS_Info.pusaFATchain[iChainIndex++] = usNextCluster;

	}

	return iRet;

}
//---------------------------------------------------------------------------
int mmcFileReadSct(FILEACC *pfa,void *ptr,int sct,int len)
{

	//MMC版pceFileReadSct
	//MMCからP/ECEでの1セクタ(4096bytes)分を読み込みます。
	//sctにはP/ECEでのセクタ番号を指定して下さい。
	//戻り値は読み込んだバイト数です。
	//セクタ番号が適切ではない場合や、pfarがNULLの場合、またMMCが接続
	//されていない場合は失敗として0を返します。
	//
	//P/ECEでの拡張機能であるptr=NULLでの呼び出しは一応サポート
	//していますが、結局代替バッファに読み込んでいるので、効率は悪いです。
	//また代替負バッファへ4096バイトを超えるアクセスは保証されません。
	
	int iReadBytes = 0;
	unsigned char *pReadBuff;				//書き込み先バッファへのポインタ
#ifndef __PCEKN__		// 2005/04/02 Add by Madoka
	//↓2004/12/12 Naoyuki Sawa スタック溢れ抑制のため、STATICに変更
	static unsigned char ucaBuff[512];		//セクタ読み込み用ワーク	
#else
	unsigned char ucaBuff[512];			//セクタ読み込み用ワーク
	//↓2004/12/12 Naoyuki Sawa スタック溢れ抑制のため、STATICに変更
	//static unsigned char ucaBuff[512];		//セクタ読み込み用ワーク
	//2005/03/20 追記 by まどか
	//512バイトのstatic変数は現在のカーネル領域には入らないため、
	//static化を見送りました。ソース先頭のコメント通り、高速RAM内
	//にスタックを切り替えて利用する場合は、mmcFileReadSct呼び出し
	//位置に注意してください。
#endif //__PCEKN__
	unsigned long ulReadSector;				//読み込み物理セクタ番号
	//↓4つ グローバル変数の値をローカルに移す為に定義 2005/03/12 Add by Madoka
	unsigned long ulFirstClusterAddress;	//クラスタ2の物理セクタ番号
	unsigned char ucSectorsParCluster;		//1クラスタ当たりのセクタ数
	unsigned char ucSectorsParClusterShift;	//1クラスタ当たりのセクタ数になる左シフト回数
	unsigned short *pusaFATchain;			//FATチェインテーブルのポインタ
	int iFATChainIndex;
	int iReadedSector = 0;					//512MB以上のカード用のダミー読み込み済みセクタ数 2005/03/16 Add by Madoka
	int i;

	////MMCの接続確認
	//if(mmcCheckConnection() == 1)
	//	return 0;	//MMC接続エラー
 	//→2004/12/12 Delete by N.SAWA 高速化のため、読み込み時の接続確認は省きました。
 	
	//ファイル情報の有無をチェック
	if(pfa == NULL) return 0;	//エラー

	////セクタ番号の有効性をチェック
	////指定セクタ番号の位置がファイル内にあるかどうかを調べる
	//if(sct > (((pfa->fsize + 4095) >> 12) - 1)) return 0;	//セクタ番号エラー ( >> 12 = / 4096)
	//→2004/12/12 Delete by N.SAWA 以下の切り詰め処理で判定できるので、もう不要です。

	//{{2004/12/12 Add by N.SAWA
	//読み込むバイト数がファイル終端を超えていたら、ファイル終端までに切り詰めます。
	//pceFileReadSct()互換の振る舞いとするためには、この処理が必須です。
	//if(len > pfa->fsize - sct * 4096)
	if(len > pfa->fsize - (sct << 12))	//2005/02/08 Change by Madoka	少しでも高速化させるためにシフトに変更
	{
		//len = pfa->fsize - sct * 4096;
		len = pfa->fsize - (sct << 12);	//2005/02/08 Change by Madoka	少しでも高速化させるためにシフトに変更
	}
	//↓2005/04/05 Changed by Madoka 
	//ptr=NULL,len=0の場合を許可するように修正
	if((len <= 0) && (ptr != NULL)) return 0; //0バイト読み込み要求、またはセクタ番号が終端を超えている
	//}}2004/12/12 Add by N.SAWA
	
	//書き込み先バッファのポインタを設定

	//保存用バッファのポインタがNULLじゃないか？
	if(ptr != NULL)
	{
		//そのまま使う
		pReadBuff = ptr;
	}
	else
	{
		//拡張機能を一応サポート

		//代替バッファを確認
		//if(g_tFS_Info.pucaReadSubBuff == NULL)
		//{
		//	//確保されていないので確保を試みる
		//	g_tFS_Info.pucaReadSubBuff = (unsigned char*)pceHeapAlloc(4096);
		//	
		//	//確保できたか？
		//	if(g_tFS_Info.pucaReadSubBuff == NULL)
		//	{
		//		//このAPIは失敗しました。
		//		pfa->aptr = NULL;
		//		return 0;
		//	}
		//}
		//→2004/12/12 Delete by N.SAWA 代替バッファは初期化時に確保済みです

		//代替バッファのポインタをセット
		pReadBuff = g_tFS_Info.pucaReadSubBuff;
		pfa->aptr = pReadBuff;

		//2004/08/29 Add by Madoka
		if(len == 0) len = PIECE_SECTOR_SIZE;	//サイズ指定が無い場合は最大サイズにする
	}
	
	//該当する場所のFATチェインテーブルインデックスを算出
	//
	//欲しい物理セクタ番号は、FATチェインテーブルのクラスタ番号から
	//算出できます。
	//
	//まず、次に読み込むP/ECEでのセクタ番号から読み込み済みのバイト
	//数を算出し、それを1クラスタ当たりのバイト数で割ると、FATチェイン
	//テーブルのインデックスがわかるので、そのインデックスの位置に
	//書かれたクラスタ番号から物理セクタ番号を算出します。
	//
	//
	//↓ << 12 = * 4096
	//↓ 左シフト値からg_tFS_Info.ucClustorSizeShiftを
	//↓ 引いているのはg_tFS_Info.ucClustorSizeShift分
	//↓ 右にシフトする必要があるからです。
	//↓ ちなみに >> g_tFS_Info.ucClustorSizeShift = / sizeof(Clustor) 
	//↓ (ただしこの方法は1クラスタのサイズが4096bytes以下の場合に限ります。注意。)
	//Old::
	//iFATChainIndex = sct << (12 - g_tFS_Info.ucClustorSizeShift);
    //2005/03/20 Changed by Madoka 512MB以上1GB以内のメモリカードに対応
	if(g_tFS_Info.ucClustorSizeShift > 12)
	{
		int iShift = (g_tFS_Info.ucClustorSizeShift - 12);

		iFATChainIndex = sct >> iShift;
		iReadedSector  = (sct & ((1 << iShift) - 1)) << 3;

		//↑の計算は、1クラスタが4096バイト以上のカードの場合に、
		//クラスタ内でのオフセットセクタ数を出すためのもので、
		//sctが右シフトされて無くなる部分の値に4096 / 512 = 8
		//をかけて、512バイトのセクタ何個分がオフセットとなるか
		//を算出しています。
	}
	else
		iFATChainIndex = sct << (12 - g_tFS_Info.ucClustorSizeShift);
    		
	//読み込みループ
	ulFirstClusterAddress    = g_tFS_Info.ulFirstClusterAddress;
	ucSectorsParCluster      = g_tFS_Info.ucSectorsParCluster;
	ucSectorsParClusterShift = g_tFS_Info.ucSectorsParClusterShift;
	pusaFATchain             = &g_tFS_Info.pusaFATchain[iFATChainIndex];

	//(指定バイト読み込むまでループ)
	while(iReadBytes < len)
	{
		
		//読み込み物理セクタ番号を算出
		ulReadSector = ulFirstClusterAddress			//クラスタ2の物理セクタ番号
						+ ((*pusaFATchain - 2)			//オフセットセクタ数
						   << ucSectorsParClusterShift);					
		ulReadSector += iReadedSector;					//ダミー読み込み済みセクタ数を加算(512MB以上のカード用)
														//↑2005/03/20 Add by Madoka

		//1クラスタ分読み込む
		for(i = iReadedSector;i < ucSectorsParCluster;++i)
		{
            		
			//1セクタ分読み込み
			mmcReadSector(ulReadSector,ucaBuff);

			//未読み込みデータは1セクタ以上あるか？
			if((len - iReadBytes) > 512)
			{
				//全部コピー
				memcpy(pReadBuff,(void*)ucaBuff,512);
				
				//1セクタ分ポインタを進める
				pReadBuff  += 512;
				iReadBytes += 512;

				//次のセクタへ
				ulReadSector++;
			}
			else
			{
				//残り分だけコピー
				memcpy(pReadBuff,(void*)ucaBuff,len - iReadBytes);
				
				//全部コピーしました
				iReadBytes = len;
				break;
			}
			
		}

		//次のチェインへ
		pusaFATchain++;

	}

	return iReadBytes;

}
//---------------------------------------------------------------------------
int mmcFileReadMMCSct(FILEACC *pfa,void *ptr,int sct,int len)
{

	//MMC版pceFileReadSct縮小版
	//MMCからMMCでの1セクタ(512bytes)分を読み込みます。
	//sctにはファイルの先頭からのMMCでのセクタ数を指定して下さい。
	//戻り値は読み込んだバイト数です。
	//クラスタ番号が適切ではない場合や、pfarがNULLの場合、またMMCが接続
	//されていない場合は失敗として0を返します。
	//
	//P/ECEでの拡張機能であるptr=NULLでの呼び出しは一応サポート
	//していますが、結局代替バッファに読み込んでいるので、効率は悪いです。
	//また代替負バッファへ512バイトを超えるアクセスは保証されません。

	unsigned char *pReadBuff;		//書き込み先バッファへのポインタ
	unsigned long ulReadSector;		//読み込み物理セクタ番号
	int iFATChainIndex;

	////MMCの接続確認
	//if(mmcCheckConnection() == 1)
	//	return 0;	//MMC接続エラー
 	//→2004/12/12 Delete by N.SAWA 高速化のため、読み込み時の接続確認は省きました。
 	
	//ファイル情報の有無をチェック
	if(pfa == NULL) return 0;	//エラー

	//セクタ番号の有効性をチェック
	//指定セクタ番号の位置がファイル内にあるかどうかを調べる
	//if(sct > (((pfa->fsize + 511) >> 9) - 1)) return 0;	//セクタ番号エラー ( >> 9 = / 512)
	//→2005/04/05 Deleted by Madoka 以下の切り詰め処理で判定できるので、もう不要です。

	//{{2005/04/05 Add by Madoka
	//読み込むバイト数がファイル終端を超えていたら、ファイル終端までに切り詰めます。
	//pceFileReadSct()互換の振る舞いとするためには、この処理が必須です。
	if(len > pfa->fsize - (sct << 9))
	{
		len = pfa->fsize - (sct << 9);
	}
	if((len <= 0) && (ptr != NULL)) return 0; //0バイト読み込み要求、またはセクタ番号が終端を超えている
	//}}2005/04/05 Add by Madoka



	//書き込み先バッファのポインタを設定

	//保存用バッファのポインタがNULLじゃないか？
	if(ptr != NULL)
	{
		//そのまま使う
		pReadBuff = ptr;
	}
	else
	{
		//拡張機能を一応サポート
		
		////代替バッファを確認
		//if(g_tFS_Info.pucaReadSubBuff == NULL)
		//{
		//	//確保されていないので確保を試みる
		//	g_tFS_Info.pucaReadSubBuff = (unsigned char*)pceHeapAlloc(512);
		//
		//	//確保できたか？
		//	if(g_tFS_Info.pucaReadSubBuff == NULL)
		//	{
		//		//このAPIは失敗しました。
		//		pfa->aptr = NULL;
		//		return 0;
		//	}
		//}
		//→2004/12/12 Delete by N.SAWA 代替バッファは初期化時に確保済みです

		//代替バッファのポインタをセット
		pReadBuff = g_tFS_Info.pucaReadSubBuff;
		pfa->aptr = pReadBuff;

		//2004/08/29 Add by Madoka
		if(len == 0) len = 512;	//サイズ指定が無い場合は最大サイズにする
	}

	//該当する場所のFATチェインテーブルインデックスを算出
	//
	//欲しい物理セクタ番号は、FATチェインテーブルのクラスタ番号から
	//算出できます。
	//
	//まず、次に読み込むP/ECEでのセクタ番号から読み込み済みのバイト
	//数を算出し、それを1クラスタ当たりのバイト数で割ると、FATチェイン
	//テーブルのインデックスがわかるので、そのインデックスの位置に
	//書かれたクラスタ番号から物理セクタ番号を算出します。
	//
	//
	//↓ << 9 = * 512
	//↓ 本来左に9シフトしてから右にg_tFS_Info.ucClustorSizeShift分
	//↓ シフトする必要があるのですが、右にシフトする回数の方が、512
	//↓ 分左に9シフトする回数より多くなるので(1クラスタは512bytes以上)
	//↓ (g_tFS_Info.ucClustorSizeShift - 9)回分右にシフトして計算
    //↓ しています。
	//↓
	//↓ ちなみに、g_tFS_Info.ucClustorSizeShiftは64MBのMMCだと
	//↓ 10 = 1クラスタ = 1024bytes で128MBだと
	//↓ 11 = 1クラスタ = 2048bytes となります。
	iFATChainIndex = sct >> (g_tFS_Info.ucClustorSizeShift - 9);
    
	//読み込み物理セクタ番号を算出
	ulReadSector = g_tFS_Info.ulFirstClusterAddress						//クラスタ2の物理セクタ番号
					+ ((g_tFS_Info.pusaFATchain[iFATChainIndex] - 2)	//オフセットセクタ数
					   << g_tFS_Info.ucSectorsParClusterShift);	

	//クラスタ内の読み込むセクタの位置を調整
	ulReadSector += (sct % g_tFS_Info.ucSectorsParCluster);
	
	//1セクタ分読み込み
	mmcReadSector(ulReadSector,pReadBuff);

	return len;

}
//---------------------------------------------------------------------------
int mmcFileClose(FILEACC *pfa)
{

	//MMC版pceFileClose
	//正常終了の場合0を返します。

	//一応チェック
	if(pfa == NULL) return 1;
	if(pfa->valid != VALIDFILEACC) return 1;

	//ファイル操作終わり
	pfa->valid = 0;

	return 0;

}
//---------------------------------------------------------------------------
#ifdef __PCEKN__		// 2004/11/15 Add by N.SAWA
//2005/04/02 Add by Madoka
unsigned char g_bMMCInitEnable;		//カーネル側MMC初期化有効フラグ

void InitMMC_API(void)
{

	//MMC用APIの初期化

	//APIを登録
	pceVectorSetKs(KSNO_MMC_AppExecFile,mmcAppExecFile);
	pceVectorSetKs(KSNO_MMC_Exit,mmcExit);
	pceVectorSetKs(KSNO_MMC_FileClose,mmcFileClose);
	pceVectorSetKs(KSNO_MMC_FileFindClose,mmcFileFindClose);
	pceVectorSetKs(KSNO_MMC_FileFindNext,mmcFileFindNext);
	pceVectorSetKs(KSNO_MMC_FileFindOpen,mmcFileFindOpen);
	pceVectorSetKs(KSNO_MMC_FileOpen,mmcFileOpen);
	pceVectorSetKs(KSNO_MMC_FileReadMMCSct,mmcFileReadMMCSct);
	pceVectorSetKs(KSNO_MMC_FileReadSct,mmcFileReadSct);
	pceVectorSetKs(KSNO_MMC_GetCIDInfo,mmcGetCIDInfo);
	pceVectorSetKs(KSNO_MMC_GetCSDInfo,mmcGetCSDInfo);
	pceVectorSetKs(KSNO_MMC_Init,mmcInit);
	pceVectorSetKs(KSNO_MMC_ReadSector,mmcReadSector);			//2004/11/02 Add by Madoka
	pceVectorSetKs(KSNO_MMC_GetInitResult,mmcGetInitResult);	//2005/03/12 Add by Madoka

	//扱える最大ファイルサイズを初期化
	g_ulMaxFileSize = 0;

	//グローバルワークエリアへのポインタを初期化
	g_tFS_Info.ptaDirTable = NULL;
	g_tFS_Info.pusaFATchain = NULL;
	g_tFS_Info.pucaReadSubBuff = NULL;

	//2005/03/12 Add by Madoka
	//初期化時の戻り値保持変数を初期化
	g_ucInitResult = 0;

	//2005/04/02 Add by Madoka
	g_bMMCInitEnable = 0;		//デフォルトはMMC初期化しない
}
#endif //#ifdef PCEKN	// 2004/11/15 Add by N.SAWA
//---------------------------------------------------------------------------
#ifdef __PCEKN__		// 2004/11/15 Add by N.SAWA
int mmcAppExecFile(const char *fname,int resv)
{

	//MMC上のpexファイルを実行
	
	//実行状態を更新
	appstat = 5;	//アプリを終了し、MMC上のファイルを読み込む
		
	//読み込むファイル名を保存
	strncpy(exec_fname, fname, sizeof(exec_fname));

	return 0;

}
#endif //#ifdef PCEKN	// 2004/11/15 Add by N.SAWA
//---------------------------------------------------------------------------

//↓2005/05/04 Added by Madoka
//pceFile系APIのフックは__PCEFILE_API_HOOK__が定義されて
//いる時のみ実行
#ifdef __PCEFILE_API_HOOK__
//{{2004/12/12 Add by N.SAWA

/****************************************************************************
 *	hook_mmc / unhook_mmc
 ****************************************************************************/

//2005/0611 Changed by Madoka
//#include <vector.h>
#include "ufe/vector.h"

#define KSNO_MMC_FileOpen	230
#define KSNO_MMC_FileClose	226
#define KSNO_MMC_FileReadSct	232
#define KSNO_MMC_FileFindOpen	229
#define KSNO_MMC_FileFindClose	227
#define KSNO_MMC_FileFindNext	228

static int hooked; /* 0 = フックしていない, 1 = フックしている */
static int (*old_pceFileOpen)(FILEACC* pfa, const char* fname, int mode);
static int (*old_pceFileClose)(FILEACC* pfa);
static int (*old_pceFileReadSct)(FILEACC* pfa, void* ptr, int sct, int len);
static int (*old_pceFileFindOpen)(FILEINFO* pfi);
static int (*old_pceFileFindClose)(FILEINFO* pfi);
static int (*old_pceFileFindNext)(FILEINFO* pfi);

static int
new_pceFileOpen(FILEACC* pfa, const char* fname, int mode)
{
	int retval;

	/* 本体フラッシュメモリ、MMCの順にトライ */
	retval = old_pceFileOpen(pfa, fname, mode);
	if(retval != 0) {
		retval = mmcFileOpen(pfa, fname, mode);
	}

	return retval;
}

static int
new_pceFileReadSct(FILEACC* pfa, void* ptr, int sct, int len)
{
	int retval;

	if(pfa->chain) { /* 本体フラッシュメモリ */
		retval = old_pceFileReadSct(pfa, ptr, sct, len);
	} else { /* MMC */
		retval = mmcFileReadSct(pfa, ptr, sct, len);
	}

	return retval;
}

static int
new_pceFileClose(FILEACC* pfa)
{
	int retval;

	if(pfa->chain) { /* 本体フラッシュメモリ */
		retval = old_pceFileClose(pfa);
	} else { /* MMC */
		retval = mmcFileClose(pfa);
	}

	return retval;
}

/* FILEINFO.works[15] */
#define FF_PCE	0xAB	/* 本体フラッシュメモリ操作中 */
#define FF_MMC	0xCD	/* MMC操作中 */

static int
new_pceFileFindOpen(FILEINFO* pfi)
{
	int retval;

	retval = old_pceFileFindOpen(pfi);
	if(retval == 0) {
		pfi->works[15] = FF_PCE; /* 本体フラッシュメモリ走査開始 */
	} else { /* ありえないが... */
		retval = mmcFileFindOpen(pfi);
		if(retval == 0) {
			pfi->works[15] = FF_MMC; /* MMC走査開始 */
		}
	}

	return retval;
}

static int
new_pceFileFindNext(FILEINFO* pfi)
{
	int retval = 0;

	if(pfi->works[15] == FF_PCE) { /* 本体フラッシュメモリ走査中 */
		retval = old_pceFileFindNext(pfi);
		if(!retval) { /* 本体フラッシュメモリ走査終了 */
			old_pceFileFindClose(pfi);
			if(mmcFileFindOpen(pfi) == 0) {
				pfi->works[15] = FF_MMC; /* MMC走査開始 */
				/* FALLTHRU */
			}
		}
	}
	if(pfi->works[15] == FF_MMC) { /* MMC走査中 */
		retval = mmcFileFindNext(pfi);
	}

	return retval;
}

static int
new_pceFileFindClose(FILEINFO* pfi)
{
	int retval = -1;

	switch(pfi->works[15]) {
	case FF_PCE: /* 本体フラッシュメモリ走査中 */
		retval = old_pceFileFindClose(pfi);
		break;
	case FF_MMC: /* MMC走査中 */
		retval = mmcFileFindClose(pfi);
		break;
	}

	return retval;
}

static void
hook_mmc()
{
	if(!hooked) {
		old_pceFileOpen      = pceVectorSetKs(KSNO_FileOpen     , new_pceFileOpen     );
		old_pceFileReadSct   = pceVectorSetKs(KSNO_FileReadSct  , new_pceFileReadSct  );
		old_pceFileClose     = pceVectorSetKs(KSNO_FileClose    , new_pceFileClose    );
		old_pceFileFindOpen  = pceVectorSetKs(KSNO_FileFindOpen , new_pceFileFindOpen );
		old_pceFileFindNext  = pceVectorSetKs(KSNO_FileFindNext , new_pceFileFindNext );
		old_pceFileFindClose = pceVectorSetKs(KSNO_FileFindClose, new_pceFileFindClose);
		hooked = 1;
	}
}

static void
unhook_mmc()
{
	if(hooked) {
		pceVectorSetKs(KSNO_FileOpen     , old_pceFileOpen     );
		pceVectorSetKs(KSNO_FileReadSct  , old_pceFileReadSct  );
		pceVectorSetKs(KSNO_FileClose    , old_pceFileClose    );
		pceVectorSetKs(KSNO_FileFindOpen , old_pceFileFindOpen );
		pceVectorSetKs(KSNO_FileFindNext , old_pceFileFindNext );
		pceVectorSetKs(KSNO_FileFindClose, old_pceFileFindClose);
		hooked = 0;
	}
}

//}}2004/12/12 Add by N.SAWA
#endif //__PCEFILE_API_HOOK__
//↑2005/05/04 Added by Madoka

#if 0  //2005/03/12 Vanished by Madoka
//{{2004/12/13 Add by N.SAWA

int mbr_protect = 1;	/* セクタ#0プロテクト 0:書き込み許可/1:書き込み禁止 */

//セクタライト(低レベルMMC制御関数)
//MMCへ1セクタ分(512バイトを想定)データを書き込みます。
//
//unsigned long ulSector	読み出し対象セクタ(物理セクタ番号)
//const unsigned char *pBuff	バッファ(必ず512バイト以上あること)
//
//正常に読み出せた場合は1を、エラーが発生した場合は0を返します
unsigned char mmcWriteSector(unsigned long ulSector,const unsigned char *pucBuff)
{

	//セクタライト
	//MMCへ1セクタ分(512バイトを想定)データを書き込みます
	//
	//      unsigned long ulSector	読み出し対象セクタ(物理セクタ番号)
	//const unsigned char *pBuff	バッファ(必ず512バイト以上あること)
	//
	//正常に読み出せた場合は1を、エラーが発生した場合は0を返します
	unsigned long ulAddress = ulSector * g_tMMCInfo.tCSDInfo.usReadBlockLength;	//アドレス
	unsigned char ucRet = 0;		//戻り値をここに設定する
	unsigned char ucWork1;			//データレスポンストークンを受け取る
	unsigned char ucWork2;			//ビジー完了待ちの結果を受け取る
	//int iIndex;						//送信バッファのインデックス

////////////////////////////////////////////////////////////////////////////////
//{{セクタ#0プロテクションが必要ない場合は、この範囲のコードを削除してください。
//////
	// * 万一、mmcInit()を呼ばずにmmcWriteSector()が呼ばれると、
	//   g_tMMCInfo.tCSDInfo.usReadBlockLengthが0のままなので、ulAddressの計算結果は常に0となり、
	//   すなわちMBRの書き換えになってしまいます。
	//   非常に危険ですので、セクタ#0の書き換えだけはできないようにしました。
	// * 明示的にulSectorに0を指定してmmcWriteSector()を呼びだした場合も、書き換え不可とします。
	//   たとえば、フォーマット時にulSector=0の書き換えが発生し、これを失敗扱いとするため、
	//   本プログラムではMMC/SDカードをフォーマットできません。
	//   しかしまあ常時使用するものでもないので、フォーマットできないことよりも安全を採りました。
	// ↓2004/11/05追記
	// * mbr_protectグローバル変数によって書き込み許可できるようにしました。
	if(ulAddress == 0 && mbr_protect) {
		return 0; /* 失敗 */
	}
//////
//}}セクタ#0プロテクションが必要ない場合は、この範囲のコードを削除してください。
////////////////////////////////////////////////////////////////////////////////

	//カードをActiveに
	CARD_ENABLE

	//CMD24送信してR1レスポンスを受信
	if(mmcSendCommandAndRecvResponse(24,ulAddress) == 0x00)
	{

		// [参考]
		// http://www.renesas.com/avs/resource/japan/jpn/pdf/flashcard/j203658_hb28e016mm2.pdf
		// p.64 データレスポンス、データトークン
		// p.70 SPIバスタイミング・シングルブロックライト

		//8クロック分のダミー送信(Nwr)
		mmcByteSend(0xFF);

		//{{データブロック送信開始

		//スタートデータブロックトークン送信
		mmcByteSend(0xFE);

		//ユーザデータ送信
		//for(iIndex = 0;iIndex < g_tMMCInfo.tCSDInfo.usReadBlockLength;++iIndex)
		//{
		//	mmcByteSend(pucBuff[iIndex]);
		//}
		mmcSendData(pucBuff, g_tMMCInfo.tCSDInfo.usReadBlockLength);

		//16ビットCRC送信 (SPIモードは初期状態でCRC Offなのでダミーで構いません)
		mmcByteSend(0xFF);
		mmcByteSend(0xFF);

		//}}データブロック送信完了

		//データブロック送信完了直後に、データレスポンストークンが返送されます。
		//データブロック送信の後に、ダミーの送信やクロックを入れてはいけません。
		//データレスポンストークンには、スタートビットはありません。
		//
		//★余談ですけれど、ビジー完了まで書き込みは終わってないはずなのに、
		//  なぜそれに先立って"110"ライトエラーが検出できるのか不思議です...
		
		//データレスポンストークン受信
		// xxx0sss1
		// |||||||+- 1固定
		// ||||+++-- status "010"-データが受け入れられた。
		// ||||             "101"-CRCエラーによりデータは拒否された。
		// ||||             "110"-ライトエラーによりデータは拒否された。
		// |||+----- 0固定
		// +++------ 不定(BUFFALO RSDC-32Mでは111となるようです)
		ucWork1 = mmcByteRecv();

		//ビジー完了待ち
		ucWork2 = mmcWaitBusy();

		//データレスポンストークンが“データが受け入れられた”で、
		//ビジー完了待ちがタイムアウトでなければ、
		//書き込み成功、1を返す
		if(((ucWork1 & 0x1F) == 0x05) && (ucWork2 == 1))
		{
			ucRet = 1;
		}

	}
	
	//カードをNonActiveに
	CARD_DISABLE

	return ucRet;

}

//}}2004/12/13 Add by N.SAWA
#endif //2005/03/12 Vanished by Madoka

