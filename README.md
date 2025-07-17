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


RTLシミュレーション

Arty A7向けのPLLモジュールをバイパスすればどのシミュレータでも動作する。




