/*=============================================================================
/                                   メッセージ
/
/============================================================================
/ Copyright (C) 1997-2007 Sota. All rights reserved.
/
/ Redistribution and use in source and binary forms, with or without
/ modification, are permitted provided that the following conditions
/ are met:
/
/  1. Redistributions of source code must retain the above copyright
/     notice, this list of conditions and the following disclaimer.
/  2. Redistributions in binary form must reproduce the above copyright
/     notice, this list of conditions and the following disclaimer in the
/     documentation and/or other materials provided with the distribution.
/
/ THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
/ IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
/ OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
/ IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
/ INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
/ BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
/ USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
/ ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
/ (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
/ THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/============================================================================*/

#define MSGJPN_0    _T("開始時刻：%s")
#define MSGJPN_1    _T("複写元：%s")
#define MSGJPN_2    _T("複写先：%s")
#define MSGJPN_3    _T("終了時刻：%s")
#define MSGJPN_4    _T("日")
#define MSGJPN_5    _T("月")
#define MSGJPN_6    _T("火")
#define MSGJPN_7    _T("水")
#define MSGJPN_8    _T("木")
#define MSGJPN_9    _T("金")
#define MSGJPN_10   _T("土")
#define MSGJPN_11   _T("%d年%d月%d日(%s) %d時%d分")
#define MSGJPN_12   _T("バックアップ先が指定されていません.")
#define MSGJPN_13   _T("設定をファイルから復元するためには、Backupを再起動してください。")
#define MSGJPN_14   _T("パターン名が指定されていません.")
#define MSGJPN_15   _T("オプションが間違っています. (%s)")
#define MSGJPN_16   _T("パスの指定が多すぎます. (%s)")
#define MSGJPN_17   _T("パターン名とパスは同時に指定できません. (%s)")
#define MSGJPN_18   _T("ウインドウを表示")
#define MSGJPN_19   _T("終了")
#define MSGJPN_20   _T("バックアップパターンが見つかりません. (%s)")
#define MSGJPN_21   _T("ログ")
#define MSGJPN_22   _T("タイマ")
#define MSGJPN_23   _T("その他")
#define MSGJPN_24   _T("環境設定")
#define MSGJPN_25   _T("ログファイル")
#define MSGJPN_26   _T("全てのファイル\0*\0")
#define MSGJPN_27   _T("名前")
#define MSGJPN_28   _T("バックアップ元")
#define MSGJPN_29   _T("除外１")
#define MSGJPN_30   _T("オプション")
#define MSGJPN_31   _T("パターン設定")
#define MSGJPN_32   _T("ﾌｧｲﾙ容量")
#define MSGJPN_33   _T("バックアップ先")
#define MSGJPN_34   _T("ファイルの参照")
#define MSGJPN_36   _T("バックアップ元")
#define MSGJPN_37   _T("この項目にはフォルダ名のみ指定できます。\n\n%d個のファイル名は無視しました。")
#define MSGJPN_38   _T("この項目にはファイル名のみ指定できます。\n\n%d個のフォルダ名は無視しました。")
#define MSGJPN_39   _T("Backup Ver0.22 以前の設定値を読み込みました。\n")
#define MSGJPN_40   _T("Ver0.3 以降でバックアップパターンの設定方法を変更しました。\n")
#define MSGJPN_41   _T("設定を修正してください。")
#define MSGJPN_42   _T("設定ファイルの保存")
#define MSGJPN_43   _T("Regファイル(*.reg)\0*.reg\0全てのファイル\0*\0")
#define MSGJPN_44   _T("レジストリエディタが起動できませんでした")
#define MSGJPN_45   _T("Backup-設定.ini")
#define MSGJPN_47   _T("INIファイル(*.ini)\0*.ini\0全てのファイル\0*\0")
#define MSGJPN_48   _T("設定をファイルから復元")
#define MSGJPN_49   _T("Reg/Iniファイル(*.reg;*.ini)\0*.reg;*.ini\0全てのファイル\0*\0")
#define MSGJPN_51   _T("設定ファイルは拡張子が.regか.iniでなければなりません。")
#define MSGJPN_52   "# このファイルは編集しないでください。\n"  /* ANSI */
#define MSGJPN_53   _T("INIファイルに設定が保存できません")
#define MSGJPN_54   _T("中止(&C)")
#define MSGJPN_55   _T("再バックアップ(&R)")
#define MSGJPN_56   _T("再開(&R)")
#define MSGJPN_57   _T("再バックアップ（%d分前）(&R)")
#define MSGJPN_58   _T("バックアップ中")
#define MSGJPN_60   _T("お待ちください")
#define MSGJPN_61   _T("ERROR: バックアップ先のボリュームラベルが一致しません")
#define MSGJPN_62   _T("ERROR: バックアップ先 %s が存在しません")
#define MSGJPN_63   _T("--- バックアップ元のフォルダを検索しています ---")
#define MSGJPN_64   _T("--- バックアップ先にある不要なフォルダを削除しています ---")
#define MSGJPN_65   _T("--- バックアップ先にある不要なファイルを削除しています ---")
#define MSGJPN_66   _T("--- バックアップ先にフォルダを作成しています ---")
#define MSGJPN_67   _T("--- バックアップ先にファイルを複写しています ---")
#define MSGJPN_68   _T("終了 (エラー %d, 総エラー %d) (作成 %d, 削除 %d, 複写 %d)")
#define MSGJPN_69   _T("中止 (エラー %d, 総エラー %d) (作成 %d, 削除 %d, 複写 %d)")
#define MSGJPN_70   _T("ERROR: %s が作成できません。(%s)")
#define MSGJPN_71   _T("作成 : %s")
#define MSGJPN_73   _T("  大文字小文字変更 %s\n")
#define MSGJPN_74   _T("名前 : %s")
#define MSGJPN_75   _T("  属性変更 %s\n")
#define MSGJPN_76   _T("属性 : %s (元=0x%x : 先=0x%x)")
#define MSGJPN_77   _T("更新 : %s")
#define MSGJPN_78   _T("複写 : %s")
#define MSGJPN_79   _T("ERROR: ディスクがいっぱいです。")
#define MSGJPN_80   _T("ERROR: %s がコピーできません。(%s)")
#define MSGJPN_81   _T("削除 : %s")
#define MSGJPN_82   _T("ERROR: %s が削除できません。(%s)")
#define MSGJPN_83   _T("ERROR: %s が見つかりません。")
#define MSGJPN_84   _T("バックアップ元、バックアップ先を指定してください。")
#define MSGJPN_85   _T("このパターンを削除しますか？")
#define MSGJPN_86   _T("バックアップ元を指定してください。")
#define MSGJPN_87   _T("ログファイルのビューワが実行できません。")
#define MSGJPN_88   _T("実行ファイル")
#define MSGJPN_89   _T("Exe file\0*.exe\0All file(*.*)\0*\0")
#define MSGJPN_90   _T("Windowsの設定ができません。")
#define MSGJPN_91   _T("システム")
#define MSGJPN_92   _T("%d秒前")
#define MSGJPN_93   _T("そのまま待機")
#define MSGJPN_94   _T("プログラムを閉じる")
#define MSGJPN_95   _T("Windowsをシャットダウンする")
#define MSGJPN_96   _T("Windowsをスタンバイにする")
#define MSGJPN_97   _T("Windowsを休止状態にする")
#define MSGJPN_98   _T("Windowsをシャットダウンします。")
#define MSGJPN_99   _T("Windowsをスタンバイにします。")
#define MSGJPN_100  _T("Windowsを休止状態にします。")
#define MSGJPN_101  _T("閉じる+Windowsをスタンバイにする")
#define MSGJPN_102  _T("閉じる+Windowsを休止状態にする")
#define MSGJPN_103  _T("バックアップ先")
#define MSGJPN_104  _T("サウンドファイル")
#define MSGJPN_105  _T("Waveファイル\0*.wav\0All file(*.*)\0*\0")
#define MSGJPN_106  _T("作成しようとしたフォルダと同じ名前のファイルがあります。そのファイルが削除できません。")
#define MSGJPN_107  _T("検索中です。")
#define MSGJPN_108  _T("検索が完了しました。")
#define MSGJPN_109  _T("高度な設定")
#define MSGJPN_110  _T("パス名一括置換")
#define MSGJPN_111  _T("ボリュームラベルが取得できませんでした")
#define MSGJPN_112  _T("チェックしません")
#define MSGJPN_113  _T("ソート")
#define MSGJPN_114  _T("以下のファイルを%s上書きします。よろしいですか。")
#define MSGJPN_115  _T("新しいファイルで")
#define MSGJPN_116  _T("古いファイルで")
#define MSGJPN_117  _T("サイズの違うファイルで")
#define MSGJPN_118  _T("エラーは「ログ」メニューで確認できます。")
#define MSGJPN_119  _T("表示するログがありません。")
#define MSGJPN_120  _T("Windowsをスリープにする")
#define MSGJPN_121  _T("閉じる+Windowsをスリープにする")
#define MSGJPN_122  _T("Windowsをスリープにします。")
#define MSGJPN_123  _T("中断(&P)")
#define MSGJPN_124  _T("除外２")
#define MSGJPN_125  _T("--- バックアップの準備をしています ---")

