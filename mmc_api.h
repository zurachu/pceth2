//---------------------------------------------------------------------------
// P/ECE MMC制御関連ルーチン
// 2004/05/02 by まどか
//
// H    P:http://www2.plala.or.jp/madoka/
// E-mail:madoka@olive.plala.or.jp
//
//タブは4で見てください
//---------------------------------------------------------------------------
#ifndef __MMC_CTRL_API__
#define __MMC_CTRL_API__
//---------------------------------------------------------------------------
//■■ 定数定義 ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
//#define MMC_FILESIZE_MAX	(5242880)	// = (5 * 1024 * 1024)	今回のMMCドライバで扱える最大ファイルサイズ
//#define MMC_FILESIZE_MAX	(120586240)	// = (115 * 1024 * 1024) 今回のMMCドライバで扱える最大ファイルサイズ 115MB	
#define MMC_FILESIZE_MAX	(15 * 1024 * 1024)

//---------------------------------------------------------------------------
//■■ 構造体宣言 ■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------
//CID(Card IDentification number)　カードIDレジスタ情報構造体
typedef struct tag_MMC_CID_INFO
{
	unsigned char   ucMID;		//ManufactureID(8bit)
	unsigned short  usOID;		//OEM/Application ID(16bit)
	unsigned char   ucPNM[6];	//Product name(48bit)
	unsigned char   ucPRV;		//Product revision(8bit)
	unsigned long   ulPSN;		//Product serial number(32bit)
	unsigned char   ucMDT;		//Manufacturing date(8bit)
	unsigned char   ucCRC;		//7-bit CRC checksum(7bit)
} MMC_CID_INFO;
#define MMC_CID_INFO_SIZE	(16)	//本来のCIDレジスタ情報のサイズ

//CSD(Card Specific Data)　カード特性レジスタ情報構造体
//ビットごとに色々するのが面倒なので必要最小限で
typedef struct tag_MMC_CSD_INFO
{
	unsigned short usReadBlockLength;	//読み込みブロック長(bytes)
	unsigned short usWriteBlockLength;	//書き込みブロック長(bytes)

	unsigned long ulCardSize;			//カード容量(bytes)
	unsigned long ulTatalBlocks;		//トータルブロック(セクタ)数
		
} MMC_CSD_INFO;
#define MMC_CSD_INFO_SIZE	(16)	//本来のCSDレジスタ情報のサイズ

//---------------------------------------------------------------------------
//■■ 関数のプロトタイプ宣言 ■■■■■■■■■■■■■■■■■■■■■■■■
//---------------------------------------------------------------------------

//MMCの初期化
//MMC系のAPIを使用する前に必ず1度だけ呼び出してください。
//MMC上のファイルを扱う際の、最大ファイルサイズを指定します。
//この初期化ではMMC自体の初期化とAPI内で使用するワークエリアをヒープから確保します。
//MMCの初期化に失敗した場合またはワークエリアの確保に失敗した場合は0を返します。
//正常終了した場合は1を返します。
unsigned char mmcInit(unsigned long ulMaxFileSize);

//MMC初期化時の処理結果を取得
//MMC対応カーネルVer.1.27からユーザアプリ起動前にカーネル
//側でMMCを初期化しに行くように変更しました。
//pceFile系APIもフックするようにしたので、ユーザはMMCの存在
//を気にすることは無いと思いますが、初期化に成功したかどうか
//を知りたい時の為にこの関数を用意しました。
//使用する機会が少なそうであれば、削除するかも知れません。
unsigned char mmcGetInitResult(void);

//MMCの終了
//MMC系のAPIが不要になったら必ず呼び出してください。
//ここで初期化時に確保したワークエリアを解放します。
void mmcExit(void);

//CIDレジスタの内容を取得
//取得に失敗した場合は0を、成功した場合は1を返します。
//引数にはCIDレジスタ情報の格納先アドレスを指定してください。
unsigned char mmcGetCIDInfo(MMC_CID_INFO *pCIDInfo);

//CSDレジスタの内容を取得
//取得に失敗した場合は0を、成功した場合は1を返します。
//引数にはCSIDレジスタ情報の格納先アドレスを指定してください。
unsigned char mmcGetCSDInfo(MMC_CSD_INFO *pCSDInfo);

//MMC版pceFileFindOpen
//正常に終了した場合は0を返します。
//MMCが接続されていない等のエラーが発生した場合は1を返します。
int mmcFileFindOpen(FILEINFO *pfi);

//MMC版pceFileFindNext
//戻り値	0:ファイルが見つからない(pfiの内容は無効)
//		1:ファイルが見つかった(pfiの内容は有効)
int mmcFileFindNext(FILEINFO *pfi);

//MMC版pceFileFindClose
//戻り値は常に0です(正常終了)
int mmcFileFindClose(FILEINFO *pfi);

//MMC版pceFileOpen
//正常終了の場合0を返します。
//ファイルが見つからない場合は1を返します。
//またファイルのサイズが大きすぎる場合は2を返します。
//ファイルのデータがあるクラスタに欠陥等が見つかった場合は3を返します。
//FATチェインテーブルを作成するのに必要なヒープ領域が足りない場合は4を
//返します。
//その他のエラーは5を返します。
int mmcFileOpen(FILEACC *pfa,const char *fname,int mode);

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
int mmcFileReadSct(FILEACC *pfa,void *ptr,int sct,int len);

//MMC版pceFileReadSct縮小版
//MMCからMMCでの1セクタ(512bytes)分を読み込みます。
//sctにはファイルの先頭からのMMCでのセクタ数を指定して下さい。
//戻り値は読み込んだバイト数です。
//クラスタ番号が適切ではない場合や、pfarがNULLの場合、またMMCが接続
//されていない場合は失敗として0を返します。
//
//P/ECEでの拡張機能であるptr=NULLでの呼び出しは一応サポート
//していますが、結局代替バッファに読み込んでいるので、効率は悪いです。
//また代替負バッファへ4096バイトを超えるアクセスは保証されません。
int mmcFileReadMMCSct(FILEACC *pfa,void *ptr,int sct,int len);

//MMC版pceFileClose
//正常終了の場合0を返します。
int mmcFileClose(FILEACC *pfa);

//MMC上のpexファイルを実行
//戻り値は常に0です
int mmcAppExecFile(const char *fname,	//ファイル名
				   int resv);			//予約(必ず0にして下さい)

//2004/11/02 nsawaさんからの御要望により外部に公開
//
//セクタリード(低レベルMMC制御関数)
//MMCから1セクタ分(512バイトを想定)データを読み出します
//
//unsigned long ulSector	読み出し対象セクタ(物理セクタ番号)
//unsigned char *pBuff		バッファ(必ず512バイト以上あること)
//
//正常に読み出せた場合は1を、エラーが発生した場合は0を返します
unsigned char mmcReadSector(unsigned long ulSector,unsigned char *pucBuff);

#if 0  //2005/03/12 Vanished by Madoka
//{{2004/12/13 Add by Naoyuki Sawa
extern int mbr_protect;
unsigned char mmcWriteSector(unsigned long ulSector,const unsigned char *pucBuff);
//}}2004/12/13 Add by Naoyuki Sawa
#endif //2005/03/12 Vanished by Madoka

//#define MMC_RECV_TIMEOUT		0xFFFF		//受信タイムアウト値
//↓2004/12/13 Change by N.SAWA 高速化した分、ちょっと長めにしました。
#define MMC_RECV_TIMEOUT		0xFFFFF		//受信タイムアウト値
//#define MMC_POLLING_TIMEOUT		1000		//受信ポーリング時のタイムアウト回数
//↓2004/12/13 Change by N.SAWA 高速化した分、ちょっと長めにしました。
#define MMC_POLLING_TIMEOUT		10000		//受信ポーリング時のタイムアウト回数

//2005/03/12 Add by Madoka
#define APPNF_INITMMC 7     //pceAppNotifyで通知されるカーネル側MMC初期化問い合わせ
                            //の通知タイプ
                            //
                            //pceAppNotifyの返答での処置
                            //APPNR_ACCEPT  or APPNR_IGNORE … MMCを初期化します
                            //APPNR_SUSPEND or APPNR_REJECT … MMCを初期化しません
                            //
//---------------------------------------------------------------------------
#endif	//#ifndef __MMC_CTRL_API__
//---------------------------------------------------------------------------
