﻿/*=============================================================================
/                           Ｂａｃｋｕｐの共通設定
/
/============================================================================
/ Copyright (C) 1997-2023 Sota. All rights reserved.
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

#define USE_SAME_AS_SUCCESS 1
#define SHOW_CONSOLE 1

#ifndef ENGLISH
#include "mesg-jpn.h"
#else
#include "mesg-eng.h"
#endif

#include "helpid.h"


#define FAIL        0
#define SUCCESS     1
#define SKIP        2
#define CANCELLED   3

#define NO          0
#define YES         1
#define YES_ALL     2
#define NO_ALL      3
#define GO_ABORT    4

#define NUL         _T('\0')

#define SIZING

#define PROGRAM_VERSION         _T("1.21")      /* バージョン */
#define PROGRAM_VERSION_NUM     0x01150000      /* バージョン */

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
#define WM_MTP_TREEVIEW_DCLICK    (WM_USER+13)  /* ホストをダブルクリックで選択した */
#define WM_MAKE_PROCESSING_WINDOW   (WM_USER+14)    /* MTPオブジェクトツリー処理中ウインドウを作成 */
#define WM_DESTROY_PROCESSING_WINDOW    (WM_USER+15)    /* MTPオブジェクトツリー処理中ウインドウを消去 */

/*===== オプション =====*/

#define OPT_FORCE               0x00000001
#define OPT_RMDIR               0x00000002
#define OPT_RMFILE              0x00000004
#define OPT_NOERROR             0x00000008
#define OPT_START               0x00000010
#define OPT_CLOSE               0x00000020
#define OPT_NAME                0x00000040
#define OPT_NOTIFY_DEL          0x00000080
#define OPT_ALL                 0x00000100
#define OPT_IGNNODEL            0x00000200
#define OPT_NEWONLY             0x00000400
#define OPT_CHK_LABEL           0x00000800
#define OPT_TRASHCAN            0x00001000
#define OPT_MINIMIZED           0x00002000
#define OPT_IGN_SYSTEM          0x00004000
#define OPT_IGN_HIDDEN          0x00008000
#define OPT_INI_FILE            0x00010000
#define OPT_IGN_ATTR            0x00020000
#define OPT_PLAY_SOUND          0x00040000
#define OPT_NO_TOPDIR           0x00080000
#define OPT_IGN_TIME            0x00100000
#define OPT_NOTIFY_OVW          0x00200000
#define OPT_SHOW_COMMENT        0x00400000
#define OPT_IGN_BIG_FILE        0x00800000
#define OPT_NO_NOTIFY_DEL       0x01000000
#define OPT_SHOW_AUTHDIALOG     0x02000000
#define OPT_HIDE_AUTHDIALOG     0x04000000
#define OPT_DST_DROPBOX         0x08000000
#define OPT_MOVE_INSTEAD_DEL    0x10000000
#define OPT_ALLOW_DECRYPTED     0x20000000
#define OPT_SUPPRESS_SLEEP_AC           0x100000000LL
#define OPT_NO_SUPPRESS_SLEEP_AC        0x200000000LL
#define OPT_SUPPRESS_SLEEP_BATTERY      0x400000000LL
#define OPT_NO_SUPPRESS_SLEEP_BATTERY   0x800000000LL

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

/*===== 認証ダイアログを表示するか  =====*/

#define AUTH_DIALOG_HIDE          0       /* 認証ダイアログを表示しない */
#define AUTH_DIALOG_SHOW          1       /* 認証ダイアログを表示する   */

/*===== TreeViewのデータタイプ =====*/

#define TREE_FOLDER     0
#define TREE_FILE       1
#define TREE_ROOT       2
#define TREE_FOLDER_SEL 3
#define TREE_FILE_SEL   4
#define TREE_ERROR      -1

/*===== TreeViewのデータタイプ =====*/

#define MTP_FOLDER      0
#define MTP_FOLDER_SEL  1
#define MTP_DEVICE      2

/*===== 表示しているウインドウ =====*/

#define WIN_MAIN            0       /* メインウインドウ */
#define WIN_TRANS           1       /* 転送中ウインドウ */
#define WIN_MTP_PROCESSING  2       /* MTPファイル構造取得中ウインドウ */

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

/* 現在の状態 */
typedef enum {
    AUTOCLOSE_ACTION_DO_NOTHING,            /* そのまま待機 */
    AUTOCLOSE_ACTION_EXIT,                  /* プログラムを閉じる */
    AUTOCLOSE_ACTION_SHUTDOWN_WINDOWS,      /* Windowsをシャットダウンする */
    AUTOCLOSE_ACTION_STANBY,                /* Windowsをスタンバイにする */
    AUTOCLOSE_ACTION_HIBERNATE,             /* Windowsを休止状態にする */
    AUTOCLOSE_ACTION_EXIT_AND_STANBY,       /* 閉じる+Windowsをスタンバイにする */
    AUTOCLOSE_ACTION_EXIT_AND_HIBERNATE,    /* 閉じる+Windowsを休止状態にする */
#if USE_SAME_AS_SUCCESS
    AUTOCLOSE_ACTION_SAME_AS_SUCCESS = -1,  /* 成功時と同じ */
#endif /* USE_SAME_AS_SUCCESS */

    AUTOCLOSE_ACTION_DEFAULT_SUCCESS = AUTOCLOSE_ACTION_DO_NOTHING,
#if USE_SAME_AS_SUCCESS
    AUTOCLOSE_ACTION_DEFAULT_ERROR = AUTOCLOSE_ACTION_SAME_AS_SUCCESS,
#else
    AUTOCLOSE_ACTION_DEFAULT_ERROR = AUTOCLOSE_ACTION_DO_NOTHING,
#endif /* USE_SAME_AS_SUCCESS */
} AUTOCLOSE_ACTION;

/*===== バックアップ後の処理 =====*/
typedef struct {
    AUTOCLOSE_ACTION Success;
    AUTOCLOSE_ACTION Error;
} AUTOCLOSE;

/*===== バックアップパターン =====*/

typedef struct {
    int Enabled;                    /* 有効 */
    _TCHAR Name[PATNAME_LEN+1];     /* パターン名 */
    _TCHAR Comment[COMMENT_LEN+1];  /* コメント */
    _TCHAR Src[SRC_PATH_LEN+1];     /* バックアップ元 (マルチ文字列) */
    _TCHAR Dst[DST_PATH_LEN+1];     /* バックアップ先 (マルチ文字列) */
    _TCHAR IgnDir[IGN_PATH_LEN+1];  /* バックアップしないフォルダ (マルチ文字列) */
    _TCHAR IgnFile[IGN_PATH_LEN+1]; /* バックアップしないファイル (マルチ文字列) */
    _TCHAR VolLabel[MY_MAX_PATH+1]; /* ボリュームラベル */
    _TCHAR SoundFile[MY_MAX_PATH+1];/* サウンドファイル */
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
    AUTOCLOSE AutoClose;            /* バックアップ後の処理 */
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
    int DstDropbox;                 /* バックアップ先はDropbox */
    int MoveInsteadDelete;          /* 削除の代わりにファイルを移動する */
    int AllowDecrypted;             /* EFSによる暗号化不可でも成功させる */
    _TCHAR MoveToFolder[MY_MAX_PATH+1]; /* ファイル移動先のフォルダー */
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
    int VarMoveList[20];    /* 垂直に動かす部品のリスト */
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


/*===== MTPオブジェクト情報 =====*/
typedef enum {
    ObjectTypeFolder,
    ObjectTypeFile,
    ObjectTypeBoth,
    ObjectTypeDevice,
    ObjectTypeNone,
} MTP_OBJECT_TYPE;

typedef struct {
    PWSTR ObjectID;
    PWSTR ObjectName;
    MTP_OBJECT_TYPE ObjectType;
    FILETIME ObjectModifiedTime;
    ULONGLONG ObjectSize;
} MTP_OBJECT_INFO;

/*===== MTPデバイスのフォルダツリー =====*/
typedef enum {
    ErrorInvalidUrl,
    ErrorDeviceNotFound,
    ErrorFolderNotFound,
} MTP_MAKE_OBJECT_TREE_ERROR;

typedef struct {
    MTP_MAKE_OBJECT_TREE_ERROR ErrorId;
    PWSTR ObjectName;
} MTP_MAKE_OBJECT_TREE_ERROR_INFO;

typedef struct _mtpfoldertree {
    MTP_OBJECT_INFO Info;
    BOOL Deleted;
    struct _mtpfoldertree* Child;
    struct _mtpfoldertree* Sibling;
} MTP_OBJECT_TREE;


typedef struct {
    LPTSTR  IgnoreFiles;
    LPTSTR  IgnoreDirs;
    int     IgnSys;
    int     IgnHid;
    int     IgnBigSize;
    int     IgnNoDel;
    int     IgnAttr;
    int     NewOnly;
    int     Tole;
    int     ForceCopy;
    int     Wait;
    int     AllowDecrypted;
    MTP_OBJECT_TREE* MtpObjectTreeTop;
} PROC_OPTIONS;

typedef struct {
    int IsMtp;
    union {
        HANDLE hFindFirstFile;
        MTP_OBJECT_TREE* tree;
    };
} FIND_FILE_HANDLE;

/* transfer.c : DWORD CALLBACK CopyProgressRoutine() をコールバックするためのtypedef */
typedef DWORD(CALLBACK* COPY_PROGRESS_ROUTINE)(LARGE_INTEGER, LARGE_INTEGER, LARGE_INTEGER, LARGE_INTEGER, DWORD, DWORD, HANDLE, HANDLE, LPVOID);

/* trandlg.c : BOOL MtpTreeProcessingRoutine() をコールバックするためのtypedef */
typedef BOOL(CALLBACK* MTP_TREE_PROCESSING_ROUTINE)(PWSTR);


/*===== プロトタイプ =====*/

/* main.c */

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow);
HWND GetMainHwnd(void);
HINSTANCE GetBupInst(void);
AUTOCLOSE_ACTION AskAutoClose(void);
int AskNoNotify(void);
LPTSTR AskHelpFilePath(void);
LPTSTR AskIniFilePath(void);
char *AskIniFilePathAnsi(void);
void SetTrayIcon(int Ope, int Type, LPTSTR AddMesg);
void SetMenuHide(int Win);
void DispErrorBox(LPTSTR szFormat,...);
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
DWORD ShowWNetUseConnection(HWND hWnd, LPTSTR lpRemoteName);
int ShowAuthDialogForUNCPaths(HWND hWnd, COPYPATLIST *Pat);
int MyPathIsUNCServerShare(_TCHAR *str);
void SelectPass(int Pass);
void SetPatName(LPTSTR Name);
void SetTaskMsg(int Type, LPTSTR szFormat,...);
void SetFileProgress(LONGLONG Total, LONGLONG Done);
void SaveTransDlgSize(void);
void MakeMtpProcessingWindow(void);
void DestroyMtpProcessingWindow(void);
HWND GetProcessingDlgHwnd(void);

/* Transfer.c */

int MakeBackupThread(void);
void CloseBackupThread(void);
void SetBackupPat(COPYPATLIST *Pat);
void SetBackupAbort(void);
void SetBackupPause(void);
void SetBackupRestart(void);
void MakePathandFile(LPTSTR Path, LPTSTR Fname, int Multi);

BOOL SetCurrentDirectory_My(LPCTSTR lpPathName, int normalization);
FIND_FILE_HANDLE* FindFirstFile_My(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData, int normalization, PROC_OPTIONS* options);
BOOL FindNextFile_My(FIND_FILE_HANDLE* hFindFile, LPWIN32_FIND_DATA lpFindFileData);
BOOL FindClose_My(FIND_FILE_HANDLE* hFindFile);
DWORD GetFileAttributes_My(LPCTSTR lpFileName, int normalization, PROC_OPTIONS* options);
DWORD GetFileAttributes_My2(LPCTSTR lpFileName, DWORD * pLastError);
BOOL SetFileAttributes_My(LPCTSTR lpFileName, DWORD dwFileAttributes, int normalization, PROC_OPTIONS* options);
BOOL MoveFile_My(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, int normalization, PROC_OPTIONS* options);
HANDLE CreateFile_My(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, int normalization);
BOOL RemoveDirectory_My(LPCTSTR lpPathName, int normalization, PROC_OPTIONS* options);
BOOL CreateDirectory_My(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, int normalization, PROC_OPTIONS* options);
BOOL DeleteFile_My(LPCTSTR lpFileName, int normalization, PROC_OPTIONS* options);


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
void WriteTitleToLogfile(LPTSTR Name, LPTSTR SrcPath, LPTSTR DstPath);
void WriteEndTimeToLogfile(void);
void DispLogWithViewer(void);
int OpenErrorLogfile(void);
int CloseErrorLogfile(void);
int DeleteErrorLogFilename(void);
int WriteMsgToErrorLogfile(LPTSTR Msg);
void DispErrorLogWithViewer(void);
void OpenLogDir(void);

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
void DuplicateComboBox(HWND hDlg, int idCopyFrom, int idCopyTo);
LPTSTR InsertNumberBeforeExtension(LPTSTR path, int number);

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
BOOL ChangeSystemPowerMode(AUTOCLOSE_ACTION State);
int DoCountDown(int State);

/* .c と .cpp のあいだで相互コールする関数のプロトタイプ　*/
#ifdef __cplusplus
extern "C" {
#endif

/* mtpsupport.cpp */
int IsMtpDevice(PCWSTR url);

/* mtpobjecttree.cpp */
int MakeMtpObjectTree(PWSTR url, MTP_OBJECT_TYPE objectType, MTP_OBJECT_TREE** top, MTP_MAKE_OBJECT_TREE_ERROR_INFO* ErrorInfo, MTP_TREE_PROCESSING_ROUTINE processingCallback);
void DispMtpObjectTree(MTP_OBJECT_TREE* top, int level);
void ReleaseMtpObjectTree(MTP_OBJECT_TREE* top);
MTP_OBJECT_TREE* FindObjectFromTree(PCWSTR url, MTP_OBJECT_TREE* treeTop, MTP_OBJECT_TREE** parent);
MTP_OBJECT_TREE* FindNextSiblingObjectFromTree(MTP_OBJECT_TREE* object);
MTP_OBJECT_TREE* FindSpecifiedChildObjectFromTree(PCWSTR name, MTP_OBJECT_TREE* parent);
int DeleteObjectFromTree(MTP_OBJECT_TREE* object, MTP_OBJECT_TREE* parent);
int AddObjectToTree(MTP_OBJECT_INFO* object, MTP_OBJECT_TREE* parent);

/* mtpfileoperation.cpp */
int DeleteObjectFromMtpDevice(PWSTR deviceId, PWSTR objectId);
int CreateFolderOnMtpDevice(PWSTR deviceId, PWSTR parentObjectId, PWSTR folderName, PWSTR* objectId);
int ChangeObjectTimeStampOnMtpDevice(PWSTR deviceId, PWSTR objectId, FILETIME* time);
int TransferFileToMtpDevice(PWSTR deviceId, PWSTR parentObjectId, PWSTR destinationFileName, PWSTR sourcePathName, PWSTR* objectId, ULONGLONG* fileSize, FILETIME* modifiedTime, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data);
int ChangeObjectNameOnMtpDevice(PWSTR deviceId, PWSTR objectId, PWSTR name);
int TransferFileFromMtpDevice(PWSTR deviceId, PWSTR objectId, PCWSTR destinationPathName, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data);

/* mtpdirsel.cpp */
int ShowMtpFolderSelectDialog(HINSTANCE hInst, HWND hWndParent, LPTSTR* url);

/* main.c */
void DoPrintf(LPTSTR szFormat, ...);

/* misc.c */
BOOL CALLBACK ExeEscDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/* transfer.c */
DWORD CALLBACK CopyProgressRoutine(LARGE_INTEGER TotalFileSize, LARGE_INTEGER TotalBytesTransferred, LARGE_INTEGER StreamSize, LARGE_INTEGER StreamBytesTransferred, DWORD dwStreamNumber, DWORD dwCallbackReason, HANDLE hSourceFile, HANDLE hDestinationFile, LPVOID lpData);

/* transdlg.c */
BOOL CALLBACK MtpTreeProcessingRoutine(LPTSTR filename);

#ifdef __cplusplus
}
#endif

