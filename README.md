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




