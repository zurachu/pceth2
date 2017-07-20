# pgd16cnv
[zurapce library](https://github.com/zurachu/zurapce)用16階調拡張P/ECE BMPコンバータ

8bit or 4bitのWindows BMPを、
[zurapce library](https://github.com/zurachu/zurapce)のLDirect_DrawObject()で描画できる
16階調拡張P/ECE BMPに変換します。

## 8bit or 4bit Windoes BMP の作成

グレースケールで、パレットの0番～15番を白～黒にして作成して下さい。
8bit の場合は、それ以外の色は透過色として扱われます。
幅は、8の倍数にしてください（通常のP/ECE BMPと同様の制約）

## コンバータ実行方法

    pgd16cnv [option] bmpfile ...
    option -t テキスト出力 (.c) デフォルト
           -b バイナリ出力 (.pgx)
    bmpfile Windows BMP ファイル名 複数指定可能、ワイルドカードも使えます
