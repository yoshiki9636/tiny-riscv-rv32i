# tiny-riscv-rv32i

tiny riscv core which is not pipelined CPU

Japanese only

省スペース向け riscv rv32i コア

スペック
- RSICV命令セットを使用しているが、ステートマシンタイプの最小構成
- 命令セット　RISCV 32i 32bit最小命令セット
- Mモードのみ対応(S,Uモード非対応）
- fence等特殊命令一部非サポート
- 割込み信号1本のみサポート
- ecallサポート
- exception illegal opsのみサポート
- メモリとしてQSPIをサポート　(リードディレイ8サイクル固定⇒v0.1現在）
- QSPI分周比1:6(v0.1現在)
- uartはrx側モニター機能あり　ただし、普通のUARTとして使えない( v0.1現在 )
  ⇒　runコマンド実行時にrxとして使えないか検討
- 40bit Free Run Counterを装備、タイマー割込みサポート(未検証)
- GPIO out 4bitのみ。 in 4bit inout 4bit追加予定。

FPGAでの使い方

1. FPGA内蔵メモリでの動作

(1) 本レポジトリをcloneして以下のディレクトリを単一ディレクトリ(以下fsimとする)にコピーする。
     ./bus/
     ./cpu/
	 ./io/
	 ./mon/
	 ./qspi_innermem/

（2）　Vivadoでfsim内部の.vファイルを取り込む。タイミングファイルは./syn/tiny_inner_io_pins.xdcを指定する。

(3) PLLが必要なので、Vivadoで指定する。100MHzではタイミングエラーは出るがマルチサイクルパスらしく動作する。
　　　　このとき、uart_if.v内の定数を周波数に応じて変更しておく。

(4)　コンパイルしてArty A7に書き込み、teratermなどで通信速度を指定の速度にして受信側の改行コードをautoに設定して通信する。
　　　　コマンドは以下の通り。
command format
g : goto PC address ( run program until quit ) : format: g <start addess>
Ctrl-c : quit from any command : format: Ctrl-c
w : write date to memory : format: w <start adderss> <data> ....<data> Ctrl-c
r : read data from memory : format: r <start address> <end adderss>
t : trashed memory data and 0 clear : format: t　(未検証)
p : read I/O memory : format: p <start address> <end adderss>
i : write I/O memory : format: i <start adderss> <data> ....<data> Ctrl-c

2. FPGAのQSPIメモリでの動作 (未検証、まだ動かない）

(1) 本レポジトリをcloneして以下のディレクトリを単一ディレクトリ(以下fsimとする)にコピーする。
     ./bus/
     ./cpu/
	 ./fpga/
	 ./io/
	 ./mon/
	 ./qspi/ (simtop.vは不要)

（2）　Vivadoでfsim内部の.vファイルを取り込む。タイミングファイルは./syn/tiny_qpsi_io_pins.xdcを指定する。

(3) PLLが必要なので、Vivadoで指定する。100MHzではタイミングエラーは出るがマルチサイクルパスらしく動作する。
　　　　このとき、uart_if.v内の定数を周波数に応じて変更しておく。

(4)　コンパイルしてArty A7に書き込み、teratermなどで通信速度を指定の速度にして受信側の改行コードをautoに設定して通信する。
　　　　コマンドは以下の通り。
command format
g : goto PC address ( run program until quit ) : format: g <start addess>
Ctrl-c : quit from any command : format: Ctrl-c
w : write date to memory : format: w <start adderss> <data> ....<data> Ctrl-c
r : read data from memory : format: r <start address> <end adderss>
t : trashed memory data and 0 clear : format: t　(未検証)

テスト向けアセンブラ使用方法

・実行バイナリテキスト作成
　./riscv-asm1.pl <finename>.asm > <filename>.txt

・アドレスオフセット付き実行バイナリテキスト作成 (アドレスオフセット 0x100の場合　0x100くらいが限界)
　./riscv-asm1.pl -p 0x100 <finename>.asm > <filename>.txt

・ビット配列、アドレス等デバッグ情報ダンプ
./riscv-asm1.pl -v <finename>.asm >


テスト向けCプログラムコンパイル方法

・事前準備　：　riscv-gnu-toolchainのインストール （linux環境)
 (1) riscv-gnu-toolchainをgitのレポジトリからclone
    https://github.com/riscv-collab/riscv-gnu-toolchain
	他いくつかある模様
 (2) README.mdに従いconfigure 
     この時、①/opt/riscvあたりにprefixを置いて、②rv32iをターゲットにして、③newlibを作成するようにする
 (3) makeとmake install
 (4) tiny-riscv-rv32i/ctestの中の cmpl.shとcmph2.shのライブラリのパスを上記のインストールした場所に書き換える

・コンパイル
 ・*_test.c　のファイル（printfで浮動小数点を扱わないもの）
 　./cmpl.sh <filename>.c
 ・*_test2.c *.test3.c　のファイル(printfで浮動小数点を扱うもの)
  ./cmpl2.sh <filename>.c
  
・関連ファイル　適度に値を調整してください
 start.s  : スタート番地とスタックポインタの設定をしている
　link.ld  : リンク情報　各種メモリの位置を決めている

RTLシミュレーション

Arty A7向けのPLLモジュールをバイパスすればどのシミュレータでも動作する。

仕様：　簡易版

1.メモリマップ

- 0x0000_0000 – 0x00FF_FFFF : メモリ0領域　ce_n[0]が出力される

- 0x0100_0000 – 0x01FF_FFFF : メモリ1領域　ce_n[1]が出力される

- 0x0200_0000 – 0x02FF_FFFF : メモリ3領域　ce_n[2]が出力される

- 0x0300_0000 – 0xBFFF_FFFF : 不使用領域

- 0xC000_0000 – 0xC000_FFFF :  I/Oバス領域　内部I/Oレジスタ向けI/Oバスに接続

- 0xC001_0000 – 0xFFFF_FFFF : 不使用領域

2.I/Oレジスタマップ

0xC000_XXXX下位16ビットの表記


- 0xF400  [3:0] RW  QSPI read latency 0  メモリ領域0向けレイテンシ値
- 0xF404  [3:0] RW QSPI read latency 1  メモリ領域1向けレイテンシ値
- 0xF408  [3:0] RW QSPI read latency 2  メモリ領域2向けレイテンシ値

- 0xF800 [31:0] 　RW  Free Run Counter Lower 32bit  read:現在値　write:即時反映
- 0xF804 [7:0]   RW  Free Run Counter Upper 32bit  read:現在値　write:即時反映
- 0xF808 [31:0] 　RW  Free Run Counter 比較値 Lower 32bit  read:現在値　write:即時反映
- 0xF80C [7:0]   RW  Free Run Counter 比較値 Upper 32bit  read:現在値　write:即時反映
                   Free Run Counter 比較値はFree Run Counterと比較され、
                   現在値が比較値以上となった時点でタイマー割込み信号が発信される
- 0xF810 [2:0]  RW   Free Run Counterコントロールレジスタ
                     [0] : enable bit　1:enable 0:disable
         　           [1] : WO 1書き込みでカウンタリセット
                     [2] : 割込み状態ビット read: 1:タイマー割込み発生 0:タイマー割込みなし
                                                                    write : 0:割込み状態クリア
- 0xFA00  [1:0] RW External Interrupt Enable   1:enable 0:disable
                   [0] : RX受付インタラプト
                   [1] : Interrupt_0 ピンインタラプト
- 0xFA04  [1:0] RW External Interrupt Status/Clear   ビット配置はenableと同一
                  read : 1:インタラプト受信　0:インタラプト未受信
                  write : 1:クリアマスク 0: クリア　例：双方のビットをクリアしたい場合は2’b00を書き込む

- 0xFC00 [7:0]  RW uart tx 出力キャラクタ　　write : txに即時出力　read : 最後に書いた値が読める
- 0xFC04 [0]     RO  uart tx FIFO full  tx fifoの状態を確認できる 1:full
- 0xFC08 [15:0] RW uart 通信周波数比　write: 周波数比の変更　read: 現在の値が読める
　　　　　　　　　通信周波数比計算式　？？
- 0xFC0C [9:0]  RO uart rx 入力キャラクタ　[7:0]最後に入力されたキャラクタ
                [8]　以前に読まれていない場合1
				[9] 読まれる前に上書きされたとき1

- 0xFE00 [2:0] RW  GPOut  wirte: out value read: current out value
- 0xFE04 [5:0] RO  GPIn read: currnt in value ただし [5:4] init_uart, [3] init_cpu_start, [2:1] init_latency, [0] gpi
- 0xFE10 [3:0] RW GPIO output value     write: out value  read: current out value
- 0xFE14 [3:0] RO  GPIO input value  current pin value
- 0xFE18 [3:0] RW  GPIO enable value    write: out enable each pin  read: current out enable status

3. FPGAピン割り当て仕様
　- GPO : 3色LED 一番右 LD0
 - GPIO[2:0] : 3色LED 右から２番目 LD1
 - GPIO[3]   : 3色LED 右から3番目 LD2の青
 - Interrupt0 : 一番右のボタン BTN0
 - init_latency : 真ん中2つのボタン BTN2 BTN1
 - init_cpu_start : 一番左のボタン BTN3
 - init_uart : 右2つのスイッチ　SW1,SW0
 - GPI[0] : 右3番目のスイッチ SW2


