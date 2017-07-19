# pceth2
『好き好きタマお姉ちゃん』

## 概要
[PS2版『ToHeart2』](http://aquaplus.jp/th2/)（アクアプラス）を
同じアクアプラスの携帯端末である[P/ECE](http://aquaplus.jp/piece/)で動かせるようにコンバートして、実行するソフトです。
ゲームデータを入れるには、P/ECEのフラッシュメモリ容量が圧倒的に足りないので、USB接続したPCからデータを読み込みます。
[MMC/SDカードを読み込めるよう改造した](http://www2.plala.or.jp/madoka/Piece_ele/mmc/mmc.htm)P/ECEなら、
データをまるごと入れて持ち歩けます。

コミックマーケット68で頒布した同人誌『Re[2]: P/ECE』に収録されていました。

## 遊び方

### ゲームデータ変換
```
mk.bat
```
をダブルクリックするなどして実行してください。
PS2版『ToHeart2』のDVD内データが入っているフォルダを聞かれるので、
入力するとゲームデータ変換が始まります。かなり時間がかかります。
最終的に「pceth2.par」というファイルが生成されます。
これがP/ECE用の『ToHeart2』ゲームデータファイルです。

### P/ECEにデータ転送
「pceth2.pex」と「drum.arc」を「P/ECEコミュニケータ」等でP/ECEに転送して下さい。
また、ゲームデータをMMC／SDカードから読み込んで実行する場合は、
あらかじめ「pceth2.par」をカードにコピーしておいて下さい。

### 実行
P/ECEのランチャから「pceth2.pex」を実行して下さい。
以下のような画面になります。
```
+----------------------+
| ファイルサーバからの |
| 接続を待っています… |
| (SELECTでキャンセル) |
+----------------------+
```
1. MMC／SDカードからゲームデータを読み込む場合
SELECTボタンを押して下さい。
MMC／SDカード上の「pceth2.par」を読みに行きます。
2. USB接続したPCから読み込む場合
「pceth2.par」が生成されたフォルダにある「ufesvrw.exe」を実行して下さい。
USB経由で、同じフォルダにある「pceth2.par」を読み込みに行きます。
PC側とP/ECE側、どちらを先に起動しても問題ありません。

#### デバッグモード
P/ECEのランチャから「pceth2.pex」を実行する際にSTARTボタンを押しながら実行
（大体のランチャはAボタンで実行なので問題なくできると思います）すると、
デバッグモードに入ります（ゲームデータに入っている、個別スクリプト再生メニューのスクリプトを実行）。

### 操作方法

|ボタン|内容|
|---|---|
|↑・↓|選択肢、セーブデータの選択|
|Ａ|メッセージ送り・決定|
|→|高速メッセージ送り|
|Ｂ|（メッセージ送り待ち時、選択肢時のみ）メッセージ消去|
|START|セーブメニューを開く|
|SELECT|タイトル画面に戻る／（タイトル画面時）終了|
|SELECT＋START長押し|終了|

#### メッセージ消去時

|ボタン|内容|
|---|---|
|←・→|コントラスト調節|
|↑・↓|音量調節|
|Ａ・Ｂ|メッセージ復帰|

### セーブデータについて
pceth2_0.sav～pceth2_6.savの7つ、シナリオ進行をセーブできます。
「P/ECEコミュニケータ」等でPCに抜き出したり、リネームすることで、他のセーブデータと入れ替え可能です。
シナリオクリアフラグと、前回実行時の前回のコントラスト、音量は、グローバルセーブデータ「pceth2.sav」に保存されます。
