/*=============================================================================
/                           Ｂａｃｋｕｐの共通設定
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

#ifndef ENGLISH
#include "mesg-jpn.h"
#else
#include "mesg-eng.h"
#endif

#include "helpid.h"


#define FAIL        0
#define SUCCESS     1

#define NO          0
#define YES         1
#define YES_ALL     2
#define NO_ALL      3
#define GO_ABORT    4

#define NUL         _T('\0')

#define SIZING

#define PROGRAM_VERSION         _T("1.11")      /* バージョン */
#define PROGRAM_VERSION_NUM     0x010b0000      /* バージョン */

#define TIMER_INTERVAL      1
#define TIMER_ANIM          2
#define TIMER_TIPS          3
#define TIMER_COUNTDOWN     4


#define MYWEB_URL   _T("http://www2.biglobe.ne.jp/~sota/backup-qa.html")


/*===== ユーザコマンド =====*/

#define WM_BACKUP_END   (WM_USER+1)
#define WM_BACKUP_ERROR (WM_USER+2)
#define WM_BACKUP_START (WM_USER+3)
#define WM_RETURN_MAIN  (WM_USER+4)
#define WM_CLICK_ICON   (WM_USER+5)
#define WM_ADD_SRCLIST  (WM_USER+6)
#define WM_ADD_DSTLIST  (WM_USER+7)
#define WM_ADD_DIRLIST  (WM_USER+8)
#define WM_ADD_FILELIST (WM_USER+9)
#define WM_FORMAT_TEXT  (WM_USER+10)
#define WM_SIZE_CHANGE  (WM_USER+11)

/*===== オプション =====*/

#define OPT_FORCE           0x00000001
#define OPT_RMDIR           0x00000002
#define OPT_RMFILE          0x00000004
#define OPT_NOERROR         0x00000008
#define OPT_START           0x00000010
#define OPT_CLOSE           0x00000020
#define OPT_NAME            0x00000040
#define OPT_NOTIFY_DEL      0x00000080
#define OPT_ALL             0x00000100
#define OPT_IGNNODEL        0x00000200
#define OPT_NEWONLY         0x00000400
#define OPT_CHK_LABEL       0x00000800
#define OPT_TRASHCAN        0x00001000
#define OPT_MINIMIZED       0x00002000
#define OPT_IGN_SYSTEM      0x00004000
#define OPT_IGN_HIDDEN      0x00008000
#define OPT_INI_FILE        0x00010000
#define OPT_IGN_ATTR        0x00020000
#define OPT_PLAY_SOUND      0x00040000
#define OPT_NO_TOPDIR       0x00080000
#define OPT_IGN_TIME        0x00100000
#define OPT_NOTIFY_OVW      0x00200000
#define OPT_SHOW_COMMENT    0x00400000
#define OPT_IGN_BIG_FILE    0x00800000
#define OPT_NO_NOTIFY_DEL   0x01000000

/*===== トレイアイコン制御 =====*/

#define TICON_NEW       0       /* 新しく追加 */
#define TICON_CHANGE    1       /* 指定番号に変更 */
#define TICON_NEXT      2       /* 次へ */
#define TICON_DELETE    3       /* 削除 */

/*===== タスクメッセージ =====*/

#define TASKMSG_NOR     0       /* 通常のメッセージ */
#define TASKMSG_ERR     1       /* エラーメッセージ */

/*===== ログ =====*/

#define LOG_SW_OFF      0       /* ログを取らない */
#define LOG_SW_APPEND   1       /* ログを追加書き込み */
#define LOG_SW_NEW      2       /* ログを上書き */

/*===== レジストリのタイプ =====*/

#define REGTYPE_REG     0       /* レジストリ */
#define REGTYPE_INI     1       /* INIファイル */

#define REG_SECT_MAX    (16*1024)   /* レジストリの１セクションの最大データサイズ */

/*===== TreeViewのデータタイプ =====*/

#define TREE_FOLDER     0
#define TREE_FILE       1
#define TREE_ROOT       2
#define TREE_FOLDER_SEL 3
#define TREE_FILE_SEL   4
#define TREE_ERROR      -1

/*===== 表示しているウインドウ =====*/

#define WIN_MAIN        0       /* メインウインドウ */
#define WIN_TRANS       1       /* 転送中ウインドウ */

/*===== FSの種類 =====*/

#define FS_OTHER        0       /* 下記以外 */
#define FS_FAT          1       /* FAT */

#define MY_MAX_PATH     512
#define MY_MAX_PATH2    (MY_MAX_PATH*2)

#define PATNAME_LEN     160         /* パターン名の長さ */
#define COMMENT_LEN     640         /* コメントの長さ */
#define SRC_PATH_LEN    8192        /* バックアップ元 */
#define DST_PATH_LEN    8192        /* バックアップ先 */
#define IGN_PATH_LEN    8192        /* バックアップしないフォルダの長さ */

#define COPY_SIZE       65536       /* ファイルをコピーする時のバッファサイズ */

#define WAIT_TIMER      25

/*===== ドラッグ＆ドロップ可能な種類 ======*/

#define SEND_FOLDER     0x01
#define SEND_FILE       0x02


/*===== シャットダウンまでの時間 ======*/

#define SHUTDOWN_PERIOD 6           /* 秒 */


/*===== Windowsのバージョン =====*/

#define WINDOWS_VISTA_VERSION       6

/*===== バックアップパターン =====*/

typedef struct {
    _TCHAR Name[PATNAME_LEN+1];     /* パターン名 */
    _TCHAR Comment[COMMENT_LEN+1];  /* コメント */
    _TCHAR Src[SRC_PATH_LEN+1];     /* バックアップ元 (マルチ文字列) */
    _TCHAR Dst[DST_PATH_LEN+1];     /* バックアップ先 (マルチ文字列) */
    _TCHAR IgnDir[IGN_PATH_LEN+1];  /* バックアップしないフォルダ (マルチ文字列) */
    _TCHAR IgnFile[IGN_PATH_LEN+1]; /* バックアップしないファイル (マルチ文字列) */
    _TCHAR VolLabel[MY_MAX_PATH+1];     /* ボリュームラベル */
    _TCHAR SoundFile[MY_MAX_PATH+1];        /* サウンドファイル */
    int ForceCopy;                  /* 常にファイルをコピー */
    int DelDir;                     /* フォルダを削除する */
    int DelFile;                    /* ファイルを削除する */
    int IgnoreErr;                  /* エラーを無視する */
    int NotifyDel;                  /* 削除の確認を行なう */
    int NotifyOvw;                  /* 上書きの確認を行なう */
    int IgnNoDel;                   /* バックアップしないファイル／フォルダを削除しない */
    int NewOnly;                    /* バックアップ先が新しい時はコピーしない */
    int Wait;                       /* ファイルコピー時の待ち時間 */
    int ChkVolLabel;                /* ボリュームラベルをチェックする */
    int UseTrashCan;                /* ごみ箱を使用する */
    int Tolerance;                  /* タイムスタンプの許容誤差 */
    int AutoClose;                  /* バックアップ後の処理 (0=何もしない, 1=プログラム終了, 2=Windows終了) */
    int IgnSystemFile;              /* システムファイルは除外 */
    int IgnHiddenFile;              /* 隠しファイルは除外 */
    int IgnBigFile;                 /* 大きなファイルは除外 */
    int IgnBigSize;                 /* 大きなファイルの閾値 */
    int IgnAttr;                    /* 属性の違いは無視 */
    int Sound;                      /* バックアップ終了後にサウンドを鳴らすかどうか */
    int IntervalTime;               /* 再バックアップ待ち時間：マイナス値なら再バックアップなし */
    int NoMakeTopDir;               /* バックアップ先のフォルダを作らない */
    int IgnTime;                    /* タイムスタンプの違いは無視 */
    int ShowComment;                /* バックアップ開始時にコメントをウインドウで表示する */
    int NextDstNum;                 /* 次のバックアップ先番号 */
    /* 以下は設定値ではない。内部処理で使用する。 */
    _TCHAR *NextDst;                /* 次のバックアップ先へのポインタ */
    int PatNum;                     /* パターン番号 */
} COPYPAT;


/*===== バックアップパターンリスト =====*/

typedef struct copypatlist {
    COPYPAT Set;                /* パターン */
    struct copypatlist *Next;
} COPYPATLIST;


/*===== ラジオボタンの設定 =====*/

typedef struct {
    int ButID;      /* ボタンのID */
    int Value;      /* 値 */
} RADIOBUTTON;


/*===== ダイアログボックス変更処理用 =====*/

typedef struct {
    int HorMoveList[20];    /* 水平に動かす部品のリスト */
    int VarMoveList[15];    /* 垂直に動かす部品のリスト */
    int ResizeList[10];     /* サイズ変更する部品のリスト */
    int ResizeHorList[10];  /* 水平方向にのみサイズ変更する部品のリスト */
    int ResizeVarList[10];  /* 垂直方向にのみサイズ変更する部品のリスト */
    SIZE MinSize;           /* 最少サイズ */
    SIZE CurSize;           /* 現在のサイズ */
} DIALOGSIZE;


/*===== 上書き確認情報 =====*/
typedef struct {
    _TCHAR          *Fname;
    FILETIME        SrcTime;
    FILETIME        DstTime;
    DWORD           SrcSizeHigh;
    DWORD           SrcSizeLow;
    DWORD           DstSizeHigh;
    DWORD           DstSizeLow;
} OVERWRITENOTIFYDATA;


/*===== プロトタイプ =====*/

/* main.c */

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow);
HWND GetMainHwnd(void);
HINSTANCE GetBupInst(void);
int AskAutoClose(void);
int AskNoNotify(void);
LPTSTR AskHelpFilePath(void);
LPTSTR AskIniFilePath(void);
char *AskIniFilePathAnsi(void);
void SetTrayIcon(int Ope, int Type, LPTSTR AddMesg);
void SetMenuHide(int Win);
void DispErrorBox(LPTSTR szFormat,...);
void DoPrintf(LPTSTR szFormat,...);
void MakeInitialLogFilename(LPTSTR buf);

/* maindlg.c */

int MakeMainDialog(void);
HWND GetMainDlgHwnd(void);
int AddPatToList(COPYPAT *Set);
int SearchPatList(LPTSTR Name);
int CopyPatFromList(int Num, COPYPAT *Set);
void CopyDefaultPat(COPYPAT *Set);
int GetPatterns(void);
int GetSelectedCount(void);
int NotifyBackup(HWND hWnd, COPYPATLIST *Pat);
void SaveMainDlgSize(void);
void AsdMainDlgMinSize(POINT *Point);
LPTSTR GetPatComment(int Num);
void IncrementDstNum(int PatNum);

/* TransDlg.c */

int MakeTransferDialog(void);
void DeleteTransferDialogResources(void);
HWND GetTransDlgHwnd(void);
int StartBackup(COPYPATLIST *Pat);
void SelectPass(int Pass);
void SetPatName(LPTSTR Name);
void SetTaskMsg(int Type, LPTSTR szFormat,...);
void SetFileProgress(LONGLONG Total, LONGLONG Done);
void SaveTransDlgSize(void);

/* Transfer.c */

int MakeBackupThread(void);
void CloseBackupThread(void);
void SetBackupPat(COPYPATLIST *Pat);
void SetBackupAbort(void);
void SetBackupPause(void);
void SetBackupRestart(void);
void MakePathandFile(LPTSTR Path, LPTSTR Fname, int Multi);

BOOL SetCurrentDirectory_My(LPCTSTR lpPathName);
HANDLE FindFirstFile_My(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
DWORD GetFileAttributes_My(LPCTSTR lpFileName);
BOOL SetFileAttributes_My(LPCTSTR lpFileName, DWORD dwFileAttributes);
BOOL MoveFile_My(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName);
HANDLE CreateFile_My(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
BOOL RemoveDirectory_My(LPCTSTR lpPathName);
BOOL CreateDirectory_My(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes);
BOOL DeleteFile_My(LPCTSTR lpFileName);

/* Registory.c */

int SaveRegistory(void);
int LoadRegistory(void);
void ClearRegistory(void);
void SaveSettingsToFile(void);
int LoadSettingsFromFile(void);
int GetMediaPath(LPTSTR MediaPath, int Max);

/* Logfile.c */

int OpenLogfile(void);
int CloseLogfile(void);
int DeleteLogFilename(void);
int WriteMsgToLogfile(LPTSTR Msg);
void WriteTitleToLogfile(LPTSTR SrcPath, LPTSTR DstPath);
void WriteEndTimeToLogfile(void);
void DispLogWithViewer(void);
int OpenErrorLogfile(void);
int CloseErrorLogfile(void);
int DeleteErrorLogfile(void);
int WriteMsgToErrorLogfile(LPTSTR Msg);
void DispErrorLogWithViewer(void);

/* option.c */

int SetOption(HWND hWnd);

/* patman.c */

int DispHostSetDlg(HWND hWnd, COPYPAT *Pat);

/* misc.c */

void SetYenTail(LPTSTR Str);
void SetCharTail(LPTSTR Str, LPTSTR Ch);
void RemoveYenTail(LPTSTR Str);
void RemoveReturnCode(LPTSTR Str);
int CountChar(LPTSTR Str, _TCHAR Ch);
void ReplaceAll(LPTSTR Str, int Len, _TCHAR Src, _TCHAR Dst);
int ReplaceAllStr(LPTSTR Str, LPTSTR Find, LPTSTR Repl, int Scan);
void GetSamePartOfString(LPTSTR Str1, LPTSTR Str2);
LPTSTR GetFileName(LPTSTR Path);
LPTSTR GetFileExt(LPTSTR Path);
int StrMultiLen(LPTSTR Str);
int StrMultiCount(LPTSTR Str);
LPTSTR GetSpecifiedStringFromMultiString(LPTSTR Str, int Num);
BOOL CALLBACK ExeEscDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int SelectDir(HWND hWnd, LPTSTR Buf, int MaxLen, LPTSTR Title);
int SelectFile(HWND hWnd, LPTSTR Fname, LPTSTR Title, LPTSTR Filters, LPTSTR Ext, int Flags, int Save, LPTSTR Dir);
void SetRadioButtonByValue(HWND hDlg, int Value, const RADIOBUTTON *Buttons, int Num);
int AskRadioButtonValue(HWND hDlg, const RADIOBUTTON *Buttons, int Num);
void MakeSizeString(double Size, LPTSTR Buf);
void BackgrndMessageProc(void);
void SortListBoxItem(HWND hWnd);
void GetRootPath(LPTSTR Path, LPTSTR Buf);
int GetDriveFormat(LPTSTR Path);
BOOL GetVolumeLabel(LPTSTR Path, LPTSTR Buf, int Size);
UINT GetDriveTypeFromPath(LPTSTR Path);
int MoveFileToTrashCan(LPTSTR Path);
void SendDropFilesToControl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int Type);
void ExecViewer(LPTSTR Fname, LPTSTR App);
void CheckRange2(int *Cur, int Max, int Min);
void FileTime2TimeString(FILETIME *Time, LPTSTR Buf);

/* wildcard.c */

int CheckFnameWithArray(LPTSTR Fname, LPTSTR Array);
int CheckFname(LPTSTR str, LPTSTR regexp);

/* filesize.c */

void FilesSizeDialog(HWND hWnd, COPYPATLIST *Pat);

/* dlgsize.c */

void DlgSizeInit(HWND hDlg, DIALOGSIZE *Dt, SIZE *Size, BOOL Move);
void AskDlgSize(DIALOGSIZE *Dt, SIZE *Size);
void AskDlgMinSize(DIALOGSIZE *Dt, POINT *Point);
void DlgSizeChange(HWND hDlg, DIALOGSIZE *Dt, RECT *New, int Flg);

/* quickdlg.c */

int GetQuickBackupParam(COPYPATLIST **Top, HWND hWnd);

/* lbtips.c */

int InitListBoxTips(HWND hWnd, HINSTANCE hInst);
void CheckTipsDisplay(LPARAM lParam);

/* updatebell.c */

void SaveUpdateBellInfo(void);


/* shutdown.c */
BOOL ChangeSystemPowerMode(int State);
int DoCountDown(int State);
