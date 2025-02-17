Rogue: Exploring the Dungeons of Doom

	Copyright (C) 1980-1983, 1985, 1999 Michael Toy, Ken Arnold and 
	Glenn Wichman All rights reserved.  

	Revision by Y.Oz (Y.Oz Vox)  [yozvox at yahoo dot co dot jp]



I revised it about the following items.  

1. I ported this software to Windows console (Cygwin).  Of course you 
  can run this software on Linux.  

2px. You can use the "color" option in environment variable 
  "ROGUEOPTS".  This adds a color to a map of the dungeon.  I took a 
  color of IBM-PC version Rogue into account.  You should use the 
  "bright" option if you expect the color of NEC PC-9801/8801 version.  

3. I changed a Rogue program to be able to set a user name with a 
  command-line option.  From a command-line, please input a user name 
  after "-n" option (or, "-u") as follows, and run.  You can use "?" 
  for name character string to input a name after starting software.  

	rogue -n username
	rogue -n ?

4. You can use the "name=?" option in environment variable 
  "ROGUEOPTS".  When you begin a game, Rogue will hear your name.  

5x. You can use the "rank" option to use the rank name in an 
  environment variable "ROGUEOPTS".  

6x. You can use the "slime" option to let "Slime" and "Ur-vile" appear 
  instead of "Snake" and "black Unicorn" in an environment variable 
  "ROGUEOPTS".  When you fight with "Slime", "Slime" sometimes 
  multiplies.  Therefore you must pay attention for slime.  

7px. You can use the "idscrl" option to change identify scroll to only 
  one kind in an environment variable "ROGUEOPTS".  You cannot change 
  it during a game.  

8x. You can use the "pcmode" option to change kinds of scrolls, 
  potions, e.t.c. to like sets of IBM-PC or NEC PC-x801 in an 
  environment variable "ROGUEOPTS".  You cannot change it during a 
  game.  

9px. You can use "-10", "-14" or "-16" option with a command-line 
  option.  It is the same as 
  "ROGUEOPTS=passgo,color,bright,rank,slime,idscrl,pcmode,name=?" 
  ("slime" is the case of "-14" and "-16", "bright" is the case of 
  "-16").  You can use "-10 username" or "-16 username", too.  
  In case of "5.4p", "ROGUEOPTS=passgo,color,bright,idscrl,name=?".  
  For details, see the end of this chapter.  

10px. When you make a "lan.cfg" file of contents with "/home/username/" 
  or "C:\", Rogue comes to make a score file there.  The path name must 
  be over in "/" or "\".  When you set shared folders such as the 
  office LAN in a "lan.cfg" file, you can compete with an in-house 
  member for score of Rogue.  

11px. When you press the "-" key (toggle), keys for movement 
  ("hjklyubn") will be treated with "[Ctrl]+" those keys.  

12. For a person except the English zone, I bundled batch file to run a 
  command-line with English mode.  Please double-click "cmd_us.bat".  

note) "p" are for "5.4p" version.  "x" are for "5.4x" version.  

---*---*---*---

以下の点について修正を行いました。

1. これはRogue 5.4を、Windowsのコンソールプログラム（Cygwin）として移植
  したものです。もちろん、Linux上で動作させることも可能です。

2px. 環境変数「ROGUEOPTS」に「color」オプションが使えます。これはマップ
  をカラー表示にするためのものです。配色は、IBM-PC版Rogueを参考にしてい
  ます。「bright」にすると、NEC PC-9801/8801版の配色を参考にしたものにな
  ります。

3. ユーザの名前をコマンドラインから指定できるようにプログラムを変更して
  あります。コマンドラインから、「-n」（もしくは「-u」）に続けて名前を指
  定して実行してください。ゲーム開始後に名前の文字列を入力するようにする
  には、名前に「?」を指定してください。

	rogue -n username
	rogue -n ?

4. 環境変数「ROGUEOPTS」に「name=?」を用いることもできます。ゲームを始め
  ると、Rogueは名前を聞いてくるでしょう。

5x. 「ROGUEOPTS」環境変数で、階級名オプション「rank」が使えます。

6x. 「ROGUEOPTS」環境変数で、Slimeオプション「slime」が使えます。
  「Snake」と「black Unicorn」の代わりに、「Slime」と「Ur-vile」が登場し
  ます。「Slime」は戦闘中分裂するので、注意が必要です。

7px. 「ROGUEOPTS」環境変数で、Identify Scrollを1種類だけにするオプション
  「idscrl」が使えます。ゲーム中に切り替える事はできません。

8x. 「ROGUEOPTS」環境変数で、巻物や水薬などをIBM-PC/PC-x801相当にするオ
  プション「pcmode」が使えます。ゲーム中に切り替える事はできません。

9px. コマンドラインから、「-10」、「-14」、「-16」オプションが使えます。
  "ROGUEOPTS=passgo,color,bright,rank,slime,idscrl,pcmode,name=?"
  （"slime"は「-14」と「-16」、"bright"は「-16」の場合のみ）で起動したの
  と同じ状態になります。「-10 username」や「-16 username」も使えます。
  「5.4p」の場合は"ROGUEOPTS=passgo,color,bright,idscrl,name=?"となりま
  す。詳しくは、この章末を参照してください。

10px. あなたが、「/home/username/」とか「C:\」とかの内容の「lan.cfg」
  ファイルを作成しておくと、Rogueはそこにスコアファイルを作成するように
  なります。パス名は、「/」か「\」で終わっていなければなりません。あなた
  が、社内LAN等の共有フォルダを、「lan.cfg」ファイルに設定しておくと、あ
  なたは社内のメンバーとRogueのスコアを競うことができます。

11px. 「-」（トグル）を押すと、移動キー（「hjklyubn」）が「[Ctrl]+」で扱
  われます。

12. 英語圏以外の人のために、英語モードのコマンドラインを立ち上げるバッチ
  ファイルを添付しました。「cmd_us.bat」をダブルクリックしてください。

注）「p」は「5.4p」バージョン用です。「x」は「5.4x」バージョン用です。

---*---*---*---

    option:  color/nocolor, "Use color mode / カラーモードを使う"
    option:  bright/nobright, "Change bright color mode / 明るいカラー
             モードにする"
    option:  rank/norank, "Rank name / 階級名"
    option:  slime/noslime, "Slime & Ur-vile / スライムとアーバイル"
    option:  idscrl/noidscrl, "1 kind of identify scroll / identify 
             scrollを1種類にする"
    option:  pcmode/nopcmode, "Pc mode / Pcモードにする"
    option:  name, "Name / 名前"

    default: color, idscrl, pcmode, name=?

                _ noidscrl, nopcmode*
               / _ nopcmode*
              / / _ passgo
             / / /  _ slime*, rank*
            / / /  / _ bright
           / / /  / / _ nocolor
          / / /  / / /
    -10  - - x  - - - : IBM-PC v1.0(v1.1)
    -11  - - x  - - x :  ↓ Monochrome
    -12  - - x  - x - :  ↓ NEC PC color
    -14  - - x  x - - : IBM-PC v1.4
    -15  - - x  x - x :  ↓ Monochrome
    -16  - - x  x x - : NEC PC x801 v1.6
    -53  x - x  -(x)x : UNIX v5.3
    -50  x - x  - - - :  ↓ IBM-PC color
    -52  x - x  - x - :  ↓ NEC PC color
    -33  - x x  -(x)x : UNIX v5.3 w/1 kind of identify scroll
    -30  - x x  - - - :  ↓ IBM-PC color
    -32  - x x  - x - :  ↓ NEC PC color
         4 2 1  4 2 1
    (*: x only)

