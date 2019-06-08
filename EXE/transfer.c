﻿/*===========================================================================
/
/                                   Backup
/                               ファイル転送
/
/============================================================================
/ Copyright (C) 1997-2017 Sota. All rights reserved.
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


#define  STRICT
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <direct.h>
#include <windowsx.h>
#include <commctrl.h>
#include <process.h>

#include "common.h"
#include "resource.h"


//#define NO_OPERATION      /* ファイルの操作を実際にはしない（デバッグ用) */

#define READFILE_WRITEFILE      0   /* ReadFile/WriteFile関数でファイルをコピーする */
#define COPYFILEEX              1   /* CopyFileEx関数でファイルをコピーする */
#define BACKUPREAD_BACKUPWRITE  2   /* BackupRead/BackupWrite関数でファイルをコピーする */

#define FILECOPY_METHOD         COPYFILEEX

#define NORMALIZATION_TYPE_NONE     0   /* 正規化しない */
#define NORMALIZATION_TYPE_NFC      1   /* NFCに正規化 */

typedef struct dirtree {
    _TCHAR Fname[MY_MAX_PATH+1];
    struct dirtree *Next;
} DIRTREE;


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
}PROC_OPTIONS;




/*===== プロトタイプ =====*/

static void BackupThread(void *Dummy);
static void SuppressSleepThread(void *Dummy);
static int BackupProc(COPYPATLIST *Pat);
static int RemoveDisappearedDir(LPTSTR SrcPath, LPTSTR DstPath, PROC_OPTIONS *options);
static int RemoveDisappearedDirOne(LPTSTR SrcPath, LPTSTR DstPath, LPTSTR DstSub, PROC_OPTIONS *options, int *DialogResult);
static int DeleteSubDir(LPTSTR Name, int *DialogResult);
static int RemoveDisappearedFile(LPTSTR DstPath, PROC_OPTIONS *options);
static int MakeAllDirTree(LPTSTR DstPath, PROC_OPTIONS *options);
static int MakeSubDir(LPTSTR Make, LPTSTR Org, int IgnErr, int IgnAttr);
static int GoMakeDir(LPTSTR Path);
static int CopyUpdateFile(LPTSTR DstPath, UINT DrvType, PROC_OPTIONS *options);
static int GoFileCopy(LPTSTR Src, LPTSTR SrcFpos, LPTSTR Dst, LPTSTR DstFpos, UINT DrvType, PROC_OPTIONS *options);
static void CheckTimeTolerance(FILETIME *Src, FILETIME *Dst, int Tole);
static BOOL CopyFile1(LPTSTR Src, LPTSTR Dst, int Wait, UINT DrvType);
static int GoDelete1(LPTSTR Fname, int ErrRep, int *DialogResult);
static LRESULT CALLBACK DeleteNotifyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK OverWriteNotifyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void EraseSourceTree(HWND hWnd);
static int MakeSourceTree(LPTSTR SrcPath, PROC_OPTIONS *options, HWND hWnd);
static int MakeSourceTreeOne(LPTSTR SrcRoot, PROC_OPTIONS *options, HTREEITEM Parent, HWND hWnd);
static int MakeSubTree(LPTSTR SrcRoot, PROC_OPTIONS *options, HTREEITEM Parent, HWND hWnd);
static int MoveFirstItem(void);
static int MoveNextItem(void);
static int GetSrcType(void);
static int GetSrcPath(LPTSTR Src, LPTSTR ScnName);
static int GetDstPath(LPTSTR Dst, LPTSTR DstPath);
static int MakeDirTable(LPTSTR ScnPath, DIRTREE **Base, int Type);
static void ReleaseDirList(DIRTREE **Base);
static int CheckAbort(void);
static void SetFileTimeStamp(LPTSTR Src, LPTSTR Dst, UINT DrvType);
static int CheckIgnSysHid(LPTSTR Fname, int IgnSys, int IgnHid, int BigSize);
static int DoCheckIgnSysHid(WIN32_FIND_DATA *FindBuf, int IgnSys, int IgnHid, int BigSize);
static LPTSTR MakeLongPath(LPCTSTR path, int normalization);
static LPTSTR MakeLongPathNFD(LPCTSTR path);
static int CheckNormlization(LPCTSTR dest);
static int FnameCompare(LPCTSTR src, LPCTSTR dst);
static int MoveFileToDeletionFolder(LPTSTR path, LPTSTR moveTo);

/*===== ローカルなワーク ======*/

static int GoAbort = NO;
static int Pause = NO;
static HTREEITEM CurItem;

static int IgnoreErr;
static int UseTrashCan;
static int NoMakeTopDir;
static int MoveInsteadDelete;
static LPTSTR MoveToFolder;

int ErrorCount = 0;
static int TotalErrorCount = 0;
static int MkDirCount;
static int RmFileCount;
static int CopyFileCount;

static HANDLE hRunMutex;
static COPYPATLIST *CopyPatList = NULL;
static int DeleteMode = YES;
static int OverwriteMode = YES;

int TviewDispCounter=0;

int NormalizationType;

/*===== グローバルなワーク ======*/

extern int LogVerbose;
extern int SleepSuppressAC;
extern int SleepSuppressBattery;
extern int SleepSuppressBatteryPercent;



/*----- バックアップスレッドを起動する ----------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       int ステータス (=SUCCESS)
*----------------------------------------------------------------------------*/

int MakeBackupThread(void)
{
    hRunMutex = CreateMutex( NULL, TRUE, NULL );
    CopyPatList = NULL;
    _beginthread(BackupThread, 0, NULL);
    _beginthread(SuppressSleepThread, 0, NULL);

    return(SUCCESS);
}


/*----- バックアップスレッドを終了する ----------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

void CloseBackupThread(void)
{
    ReleaseMutex( hRunMutex );
    CloseHandle( hRunMutex );
    return;
}


/*----- 使用するバックアップパターンをセットする ------------------------------
*
*   Parameter
*       COPYPATLIST *Pat : パターン
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

void SetBackupPat(COPYPATLIST *Pat)
{
    CopyPatList = Pat;
    return;
}


/*----- バックアップ中止フラグをセット ----------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

void SetBackupAbort(void)
{
    GoAbort = YES;
    return;
}


/*----- ポーズフラグをセット ----------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

void SetBackupPause(void)
{
    Pause = YES;
    return;
}


/*----- ポーズフラグをリセット ----------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

void SetBackupRestart(void)
{
    Pause = NO;
    return;
}


/*----- バックアップスレッドのメインループ ------------------------------------
*
*   Parameter
*       void *Dummy : 使わない
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void BackupThread(void *Dummy)
{
    while(WaitForSingleObject(hRunMutex, 200) == WAIT_TIMEOUT)
    {
        if(CopyPatList != NULL)
        {
            GoAbort = NO;
            Pause = NO;

            if(BackupProc(CopyPatList) == SUCCESS)
                PostMessage(GetTransDlgHwnd(), WM_BACKUP_END, 0, 0);
            else
                PostMessage(GetTransDlgHwnd(), WM_BACKUP_ERROR, 0, 0);

            CopyPatList = NULL;
        }
    }
    _endthread();
}

/*----- スリープを抑止するか判断する処理  -------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       BOOL ステータス
*           TRUE/FALSE
*----------------------------------------------------------------------------*/
static BOOL CheckSuppressSleep()
{
    const BOOL DefaultSuppress = FALSE;
    SYSTEM_POWER_STATUS SystemPowerStatus;
    if (GetSystemPowerStatus(&SystemPowerStatus))
    {
        switch (SystemPowerStatus.ACLineStatus)
        {
        case 0: /* ACLineStatus: Offline (Battery) */
            if (SleepSuppressBattery && SystemPowerStatus.BatteryLifePercent >= SleepSuppressBatteryPercent)
            {
                return TRUE;
            }
            return FALSE;

        case 1: /* ACLineStatus: Online (AC) */
            if (SleepSuppressAC)
            {
                return TRUE;
            }
            return FALSE;
        }
    }
    return DefaultSuppress;
}

/*----- スリープを阻止するスレッドのメインループ ------------------------------------
*
*   Parameter
*       void *Dummy : 使わない
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void SuppressSleepThread(void *Dummy)
{
    /*
        ウェイト時間を 30 秒にしているのはスリープに入るまでの時間の最短が1分なので
        それより短い間隔で SetThreadExecutionState() を呼び出せるようにするため
    */
    while(WaitForSingleObject(hRunMutex, 30 * 1000) == WAIT_TIMEOUT)
    {
        if (CopyPatList != NULL)
        {
            BOOL IsSuppressSleep = CheckSuppressSleep();
            if (IsSuppressSleep)
            {
                SetThreadExecutionState(ES_SYSTEM_REQUIRED);
            }
        }
    }
    _endthread();
}

/*----- バックアップ処理 ------------------------------------------------------
*
*   Parameter
*       COPYPATLIST *Pat : パターン
*
*   Return Value
*       ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int BackupProc(COPYPATLIST *Pat)
{
    int Sts;
    _TCHAR Tmp[MY_MAX_PATH+1];
    UINT DestDriveType;
    PROC_OPTIONS    options;
//  _TCHAR *DestPath;

    Sts = SUCCESS;
    while((Pat != NULL) && (Sts == SUCCESS))
    {
        SelectPass(0);
        EraseSourceTree(GetDlgItem(GetTransDlgHwnd(), TRANS_DIRLIST));

        SetPatName(Pat->Set.Name);

        ErrorCount = 0;
        MkDirCount = 0;
        RmFileCount = 0;
        CopyFileCount = 0;

        IgnoreErr = Pat->Set.IgnoreErr;
        UseTrashCan = Pat->Set.UseTrashCan;
        NoMakeTopDir = Pat->Set.NoMakeTopDir;
        MoveInsteadDelete = Pat->Set.MoveInsteadDelete;
        MoveToFolder = Pat->Set.MoveToFolder;

        DeleteMode = YES;
        if(Pat->Set.NotifyDel == NO)
            DeleteMode = YES_ALL;

        OverwriteMode = YES;
        if(Pat->Set.NotifyOvw == NO)
            OverwriteMode = YES_ALL;

        if(OpenLogfile() == FAIL)
        {
            Sts = FAIL;
            break;
        }

        options.IgnoreFiles = Pat->Set.IgnFile;
        options.IgnoreDirs = Pat->Set.IgnDir;
        options.IgnSys = Pat->Set.IgnSystemFile;
        options.IgnHid = Pat->Set.IgnHiddenFile;
        options.IgnBigSize = -1;
        if(Pat->Set.IgnBigFile && (Pat->Set.IgnBigSize > 0))
        {
            options.IgnBigSize = Pat->Set.IgnBigSize;
        }
        options.IgnNoDel = Pat->Set.IgnNoDel;
        options.IgnAttr = Pat->Set.IgnAttr;
        options.NewOnly = Pat->Set.NewOnly;
        options.Tole = -1;
        if(Pat->Set.IgnTime == NO)
        {
            options.Tole = Pat->Set.Tolerance;
        }
        options.ForceCopy = Pat->Set.ForceCopy;
        options.Wait = Pat->Set.Wait;

        if(Pat->Set.NextDstNum >= StrMultiCount(Pat->Set.Dst))
        {
            Pat->Set.NextDstNum = 0;
        }
        Pat->Set.NextDst = GetSpecifiedStringFromMultiString(Pat->Set.Dst, Pat->Set.NextDstNum);

        /* バックアップ先フォルダのマクロを展開 */
//      MakeDestinationPath(Pat->Set.NextDst, &DestPath);

        OpenErrorLogfile();
        WriteTitleToLogfile(Pat->Set.Name, Pat->Set.Src, Pat->Set.NextDst);

        SetTaskMsg(TASKMSG_NOR, _T("=============================================="));
        SetTaskMsg(TASKMSG_NOR, MSGJPN_125);

        if (Pat->Set.Enabled == 0)
        {
            SetTaskMsg(TASKMSG_NOR, MSGJPN_138, Pat->Set.Name);
            SetTaskMsg(TASKMSG_NOR, MSGJPN_139, ErrorCount, TotalErrorCount, MkDirCount, RmFileCount, CopyFileCount);
            WriteEndTimeToLogfile();
            CloseLogfile();
            CloseErrorLogfile();
            Pat = Pat->Next;
            continue;
        }

        /* バックアップ先の正規化のタイプをチェック */
        // NormalizationType = CheckNormlization(Pat->Set.NextDst);
        NormalizationType = NORMALIZATION_TYPE_NONE;
        if (Pat->Set.DstDropbox)
        {
            NormalizationType = NORMALIZATION_TYPE_NFC;
        }

        /* バックアップ先の作成とチェック */
        if((Sts = MakeSubDir(Pat->Set.NextDst, _T(""), NO, Pat->Set.IgnAttr)) == SUCCESS)
        {
            GetCurrentDirectory(MY_MAX_PATH+1, Tmp);
            if(SetCurrentDirectory_My(Pat->Set.NextDst, YES) == TRUE)
            {
                SetCurrentDirectory_My(Tmp, NO);
                /* ボリュームラベルのチェック */
                if(Pat->Set.ChkVolLabel)
                {
                    GetVolumeLabel(Pat->Set.NextDst, Tmp, MY_MAX_PATH+1);
                    if(_tcscmp(Tmp, Pat->Set.VolLabel) != 0)
                    {
                        ErrorCount++;
                        SetTaskMsg(TASKMSG_ERR, MSGJPN_61);
                        Sts = FAIL;
                    }
                }

//              DestDriveType = GetDriveTypeFromPath(Pat->Set.NextDst);
                DestDriveType = DRIVE_UNKNOWN;
            }
            else
            {
                ErrorCount++;
                SetTaskMsg(TASKMSG_ERR, MSGJPN_62, Pat->Set.NextDst);
                Sts = FAIL;
            }
        }

        if(Sts == SUCCESS)
        {
            SelectPass(1);
            SetTaskMsg(TASKMSG_NOR, MSGJPN_63);
            Sts = MakeSourceTree(Pat->Set.Src, &options, GetDlgItem(GetTransDlgHwnd(), TRANS_DIRLIST));
        }

        if((Sts == SUCCESS) && (Pat->Set.DelDir == YES))
        {
            SelectPass(2);
            SetTaskMsg(TASKMSG_NOR, MSGJPN_64);
            Sts = RemoveDisappearedDir(Pat->Set.Src, Pat->Set.NextDst, &options);
        }

        if((Sts == SUCCESS) && (Pat->Set.DelFile == YES))
        {
            SelectPass(3);
            SetTaskMsg(TASKMSG_NOR, MSGJPN_65);
            Sts = RemoveDisappearedFile(Pat->Set.NextDst, &options);
        }

        if(Sts == SUCCESS)
        {
            SelectPass(4);
            SetTaskMsg(TASKMSG_NOR, MSGJPN_66);
            Sts = MakeAllDirTree(Pat->Set.NextDst, &options);
        }

        if(Sts == SUCCESS)
        {
            SelectPass(5);
            SetTaskMsg(TASKMSG_NOR, MSGJPN_67);
            Sts = CopyUpdateFile(Pat->Set.NextDst, DestDriveType, &options);
        }

        if(Sts == SUCCESS)
        {
            SelectPass(6);
        }

        SetFileProgress(0, 0);

        TotalErrorCount += ErrorCount;

        SetTaskMsg(TASKMSG_NOR, _T(""));

        if(Sts == SUCCESS)
            SetTaskMsg(TASKMSG_NOR, MSGJPN_68, ErrorCount, TotalErrorCount, MkDirCount, RmFileCount, CopyFileCount);
        else
            SetTaskMsg(TASKMSG_ERR, MSGJPN_69, ErrorCount, TotalErrorCount, MkDirCount, RmFileCount, CopyFileCount);

        if(ErrorCount != 0)
            SetTaskMsg(TASKMSG_NOR, MSGJPN_118);

        IncrementDstNum(Pat->Set.PatNum);
        Pat->Set.NextDstNum += 1;
        if(Pat->Set.NextDstNum >= StrMultiCount(Pat->Set.Dst))
        {
            Pat->Set.NextDstNum = 0;
        }

        WriteEndTimeToLogfile();
        CloseLogfile();
        CloseErrorLogfile();

        if(IgnoreErr == YES)
            Sts = SUCCESS;

//      free(DestPath);

        Pat = Pat->Next;
    }
    return(Sts);
}


/*----- コピー元にないサブディレクトリをコピー先から削除 ----------------------
*
*   Parameter
*       LPTSTR SrcPath : 設定上の転送元のパス名
*       LPTSTR DstPath : 設定上の転送先のパス名
        PROC_OPTIONS options : 処理オプション
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int RemoveDisappearedDir(LPTSTR SrcPath, LPTSTR DstPath, PROC_OPTIONS *options)
{
    int Sts;
    _TCHAR DirName[MY_MAX_PATH+1];
    int DialogResult;

    Sts = SUCCESS;
    DialogResult = NO;
    while(*SrcPath != NUL)
    {
        _tcscpy(DirName, SrcPath);
        MakePathandFile(DirName, NULL, NO);

        if((Sts = RemoveDisappearedDirOne(DirName, DstPath, _T(""), options, &DialogResult)) != SUCCESS)
            break;

        if(DialogResult == NO_ALL)
            break;

        SrcPath = _tcschr(SrcPath, NUL) + 1;
    }

    if(IgnoreErr == YES)
        Sts = SUCCESS;

    return(Sts);
}


/*----- コピー元にないサブディレクトリをコピー先から削除（サブ）---------------
*
*   Parameter
*       LPTSTR SrcPath : 設定上の転送元のパス名
*       LPTSTR DstPath : 設定上の転送先のパス名
*       LPTSTR DstSub : サブディレクトリ
        PROC_OPTIONS options : 処理オプション
*       int DialogResult : ダイアログで選ばれた選択肢
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int RemoveDisappearedDirOne(LPTSTR SrcPath, LPTSTR DstPath, LPTSTR DstSub, PROC_OPTIONS *options, int *DialogResult)
{
    _TCHAR Cur[MY_MAX_PATH2+1];
    _TCHAR Tmp[MY_MAX_PATH2+1];
    LPTSTR FnamePos;
    WIN32_FIND_DATA FindBuf;
    DIRTREE *DstTreeBase;
    DIRTREE *Pos;
    int Sts;
    HANDLE fHnd;
    DWORD Err;
    LPTSTR  lpBuffer;

    Sts = SUCCESS;

    _tcscpy(Cur, DstPath);      /* 転送先のディレクトリ名を作成 */
    SetYenTail(Cur);

    if(0 == NoMakeTopDir)
    {
        //ここ
        _tcscat(Cur, GetFileName(SrcPath));
        SetYenTail(Cur);
    }

    _tcscat(Cur, DstSub);
    SetYenTail(Cur);
    FnamePos = _tcschr(Cur, NUL);
    _tcscat(Cur, _T("*"));

    DstTreeBase = NULL;
    if(MakeDirTable(Cur, &DstTreeBase, 0) == SUCCESS)
    {
        Pos = DstTreeBase;
        while(Pos != NULL)
        {
            if((Sts = CheckAbort()) == FAIL)
                break;

            _tcscpy(FnamePos, Pos->Fname);

            _tcscpy(Tmp, SrcPath);
            SetYenTail(Tmp);
            _tcscat(Tmp, DstSub);
            SetYenTail(Tmp);
            _tcscat(Tmp, Pos->Fname);

            if((CheckFnameWithArray(Tmp, options->IgnoreDirs) == NO) &&
               (CheckIgnSysHid(Tmp, options->IgnSys, options->IgnHid, -1) == NO))
            {
                /* 除外するフォルダではない */
                if((fHnd = FindFirstFile_My(Tmp, &FindBuf, YES)) != INVALID_HANDLE_VALUE)
                {
                    FindClose(fHnd);
                    if((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
#ifdef NO_OPERATION
                        DoPrintf(_T("    Delete %s\n"), Cur);
                        Sts = SUCCESS;
#else
                        Sts = DeleteSubDir(Cur, DialogResult);
#endif
                    }
                    else
                    {
                        _tcscpy(Tmp, DstSub);
                        SetYenTail(Tmp);
                        _tcscat(Tmp, Pos->Fname);

                        if((Sts = RemoveDisappearedDirOne(SrcPath, DstPath, Tmp, options, DialogResult)) == FAIL)
                            break;
                    }
                }
                else if ((Err = GetLastError()) != ERROR_FILE_NOT_FOUND)
                {
                    ErrorCount++;
                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        Err,
                        LANG_USER_DEFAULT,
                        (LPTSTR )&lpBuffer,
                        0,
                        NULL );
                    RemoveReturnCode(lpBuffer);
                    SetTaskMsg(TASKMSG_ERR, MSGJPN_129, Tmp, lpBuffer);
                    LocalFree(lpBuffer);
                    Sts = FAIL;
                    break;
                }
                else
                {

#ifdef NO_OPERATION
                    DoPrintf(_T("    Delete %s\n"), Cur);
                    Sts = SUCCESS;
#else
                    Sts = DeleteSubDir(Cur, DialogResult);
#endif
                }
            }
            else if(options->IgnNoDel == NO)
            {
                /* 除外するフォルダ */
#ifdef NO_OPERATION
                DoPrintf(_T("    Delete %s\n"), Cur);
                Sts = SUCCESS;
#else
                Sts = DeleteSubDir(Cur, DialogResult);
#endif
            }

            if(*DialogResult == NO_ALL)
                break;

            Pos = Pos->Next;
        }
        ReleaseDirList(&DstTreeBase);
    }
    return(Sts);
}


/*----- サブディレクトリ以下を削除 --------------------------------------------
*
*   Parameter
*       LPTSTR Name : パス
*       int DialogResult : ダイアログで選ばれた選択肢
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int DeleteSubDir(LPTSTR Name, int *DialogResult)
{
    _TCHAR Find[MY_MAX_PATH2+1];
    LPTSTR NamePos;
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    int Sts;

    Sts = SUCCESS;

    if(UseTrashCan)
    {
        Sts = GoDelete1(Name, YES, DialogResult);
    }
    else
    {
        _tcscpy(Find, Name);
        SetYenTail(Find);
        NamePos = _tcschr(Find, NUL);

        _tcscpy(NamePos, _T("*"));
        if((fHnd = FindFirstFile_My(Find, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
        {
            do
            {
                if((Sts = CheckAbort()) == SUCCESS)
                {
                    if((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                    {
                        /* ファイル */
                        _tcscpy(NamePos, FindBuf.cFileName);
                        Sts = GoDelete1(Find, YES, DialogResult);

                        if(IgnoreErr == YES)
                            Sts = SUCCESS;

                        if(CheckAbort() == FAIL)
                            Sts = FAIL;
                    }
                    else if((_tcscmp(FindBuf.cFileName, _T(".")) != 0) &&
                            (_tcscmp(FindBuf.cFileName, _T("..")) != 0))
                    {
                        /* サブディレクトリ */
                        _tcscpy(NamePos, FindBuf.cFileName);
                        Sts = DeleteSubDir(Find, DialogResult);
                    }
                }
            }
            while((Sts == SUCCESS) && (*DialogResult != NO_ALL) && (FindNextFile(fHnd, &FindBuf) == TRUE));
            FindClose(fHnd);
        }

        if((Sts == SUCCESS) && (*DialogResult != NO_ALL))
            Sts = GoDelete1(Name, YES, DialogResult);
    }

    if(IgnoreErr == YES)
        Sts = SUCCESS;

    if(CheckAbort() == FAIL)
        Sts = FAIL;

    return(Sts);
}


/*----- コピー元にないファイルをコピー先から削除 ------------------------------
*
*   Parameter
*       LPTSTR DstPath : 設定上の転送先のパス名
        PROC_OPTIONS options : 処理オプション
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int RemoveDisappearedFile(LPTSTR DstPath, PROC_OPTIONS *options)
{
    int Sts;
    _TCHAR Src[MY_MAX_PATH2+1];
    _TCHAR ScnName[MY_MAX_PATH2+1];
    _TCHAR Dst[MY_MAX_PATH2+1];
    LPTSTR SrcFpos;
    LPTSTR DstFpos;
    DIRTREE *DstTreeBase;
    DIRTREE *Pos;
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    int DelFlg;
    int DialogResult;
    DWORD Err;
    LPTSTR  lpBuffer;

    Sts = SUCCESS;
    DialogResult = NO;
    MoveFirstItem();
    do
    {
        if(GetSrcType() == TREE_FOLDER)
        {
            GetSrcPath(Src, ScnName);
            SetYenTail(Src);
            SrcFpos = _tcschr(Src, NUL);

            GetDstPath(Dst, DstPath);
            SetYenTail(Dst);
            DstFpos = _tcschr(Dst, NUL);
            _tcscpy(DstFpos, _T("*"));

            if(MakeDirTable(Dst, &DstTreeBase, 1) == SUCCESS)
            {
                Pos = DstTreeBase;
                while(Pos != NULL)
                {
                    if((Sts = CheckAbort()) == FAIL)
                        break;

                    _tcscpy(SrcFpos, Pos->Fname);

                    DelFlg = YES;
                    if((CheckFnameWithArray(Src, options->IgnoreFiles) == NO) &&
                       (CheckIgnSysHid(Src, options->IgnSys, options->IgnHid, options->IgnBigSize) == NO))
                    {
                        if((TCSLEN(ScnName) == 0) || (CheckFnameWithArray(Src, ScnName) == YES))
                        {
                            if((fHnd = FindFirstFile_My(Src, &FindBuf, YES)) != INVALID_HANDLE_VALUE)
                            {
                                FindClose(fHnd);
                                DelFlg = NO;
                            }
                            else if ((Err = GetLastError()) != ERROR_FILE_NOT_FOUND)
                            {
                                DelFlg = NO;
                                Sts = FAIL;
                                ErrorCount++;
                                FormatMessage(
                                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                    NULL,
                                    Err,
                                    LANG_USER_DEFAULT,
                                    (LPTSTR )&lpBuffer,
                                    0,
                                    NULL );
                                RemoveReturnCode(lpBuffer);
                                SetTaskMsg(TASKMSG_ERR, MSGJPN_129, Src, lpBuffer);
                                LocalFree(lpBuffer);
                                break;
                            }
                        }
                    }
                    else if(options->IgnNoDel == YES)
                        DelFlg = NO;

                    if(DelFlg == YES)
                    {
                        _tcscpy(DstFpos, Pos->Fname);

#ifdef NO_OPERATION
                        DoPrintf(_T("    Delete %s\n"), Dst);
                        Sts = SUCCESS;
#else
                        if((Sts = GoDelete1(Dst, YES, &DialogResult)) == FAIL)
                            break;

                        if(CheckAbort() == FAIL)
                        {
                            Sts = FAIL;
                            break;
                        }

                        if(DialogResult == NO_ALL)
                            break;
#endif
                    }
                    Pos = Pos->Next;
                }
                ReleaseDirList(&DstTreeBase);
            }
        }
    }
    while((Sts == SUCCESS) && (DialogResult != NO_ALL) && (MoveNextItem() == SUCCESS));

    if(IgnoreErr == YES)
        Sts = SUCCESS;

    if(CheckAbort() == FAIL)
        Sts = FAIL;

    return(Sts);
}


/*----- コピー先のサブディレクトリを作成 --------------------------------------
*
*   Parameter
*       LPTSTR DstPath : 設定上の転送先のパス名
        PROC_OPTIONS options : 処理オプション
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MakeAllDirTree(LPTSTR DstPath, PROC_OPTIONS *options)
{
    int Sts;
    _TCHAR Src[MY_MAX_PATH2+1];
    _TCHAR ScnName[MY_MAX_PATH2+1];
    _TCHAR Dst[MY_MAX_PATH2+1];

    Sts = SUCCESS;
    MoveFirstItem();

    if(0 != NoMakeTopDir)
    {
        Sts = MoveNextItem();
        if((Sts == SUCCESS) && (GetSrcType() == TREE_FOLDER))
        {
            Sts = MoveNextItem();
        }
    }

    if(Sts == SUCCESS)
    {
        do
        {
            if((Sts = CheckAbort()) == FAIL)
                break;

            if(GetSrcType() == TREE_FOLDER)
            {
                GetSrcPath(Src, ScnName);
                GetDstPath(Dst, DstPath);

#ifdef NO_OPERATION
                DoPrintf(_T("    Make Dir %s (%s)\n"), Dst, Src);
                Sts = SUCCESS;
#else
                if((Sts = MakeSubDir(Dst, Src, IgnoreErr, options->IgnAttr)) != SUCCESS)
                    break;
#endif
            }
        }
        while((Sts == SUCCESS) && (MoveNextItem() == SUCCESS));
    }
    if(IgnoreErr == YES)
        Sts = CheckAbort();

    return(Sts);
}


/*----- サブディレクトリを作成 ------------------------------------------------
*
*   Parameter
*       LPTSTR Make : ディレクトリ名
*       LPTSTR Org : コピー元のディレクトリ
*       int IgnErr : エラーを無視するかどうか (YES/NO)
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MakeSubDir(LPTSTR Make, LPTSTR Org, int IgnErr, int IgnAttr)
{
    int Sts;
    int GoMake;
    int GoAttr;
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    _TCHAR Tmp[MY_MAX_PATH2+1];
    DWORD Attr;
    DWORD Attr2;
    DWORD Err;
    LPTSTR  lpBuffer;

    Sts = SUCCESS;
    _tcscpy(Tmp, Make);
    RemoveYenTail(Tmp);

    if((_tcscmp(Tmp+1, _T(":")) == 0) ||
       ((_tcsncmp(Tmp, _T("\\\\"), 2) == 0) && (CountChar(Tmp, '\\') == 3)))
    {
        /* nothing */
    }
    else
    {
        GoMake = 1;
        if((fHnd = FindFirstFile_My(Tmp, &FindBuf, YES)) != INVALID_HANDLE_VALUE)
        {
            FindClose(fHnd);
            if((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                if((Sts = GoDelete1(Tmp, NO, NULL)) == FAIL)
                {
                    ErrorCount++;
                    SetTaskMsg(TASKMSG_ERR, MSGJPN_70, Tmp, MSGJPN_106);
                }
            }
            else
            {
                GoMake = 0;
//              if(_tcscmp(GetFileName(Tmp), FindBuf.cFileName) != 0)
                if(FnameCompare(GetFileName(Tmp), FindBuf.cFileName) != 0)
                {
                    GoMake = 2;     /* 大文字/小文字が違う */
                }

                GoAttr = 0;
                if(IgnAttr == 0)
                {
                    if((TCSLEN(Org) > 2) && (_tcscmp(Org+1, _T(":\\")) != 0))
                    {
                        Attr = GetFileAttributes_My(Tmp, YES);
                        Attr2 = GetFileAttributes_My(Org, NO);

                        Attr &= ~(FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE);
                        Attr2 &= ~(FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE);
                        if(Attr != Attr2)
                        {
                            GoAttr = 1;
                        }
                    }
                }
            }
        }

        if(Sts == SUCCESS)
        {
            if(GoMake == 1)
            {
                MkDirCount++;
                SetTaskMsg(TASKMSG_NOR, MSGJPN_71, Tmp);
                if(GoMakeDir(Tmp) == 0)
                {
                    if((TCSLEN(Org) > 2) && (_tcscmp(Org+1, _T(":\\")) != 0))
                    {
                        Attr = GetFileAttributes_My(Org, NO);
                        SetFileAttributes_My(Tmp, Attr, YES);
                    }
                }
                else
                {
                    ErrorCount++;
                    Err = GetLastError();
                    FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        Err,
                        LANG_USER_DEFAULT,
                        (LPTSTR )&lpBuffer,
                        0,
                        NULL );
                    RemoveReturnCode(lpBuffer);
                    SetTaskMsg(TASKMSG_ERR, MSGJPN_70, Tmp, lpBuffer);
                    LocalFree(lpBuffer);
                    Sts = FAIL;
                }
            }
            else if(GoMake == 2)
            {
                SetTaskMsg(TASKMSG_NOR, MSGJPN_74, Tmp);
                MoveFile_My(Tmp, Tmp, YES);  /* 大文字/小文字を合わせる */
            }

            if(GoAttr == 1)
            {
                SetTaskMsg(TASKMSG_NOR, MSGJPN_76, Tmp, Attr2, Attr);
                Attr = GetFileAttributes_My(Org, NO);
                SetFileAttributes_My(Tmp, Attr, YES);
            }
        }
    }

    if(IgnErr == YES)
        Sts = CheckAbort();

    return(Sts);
}


/*----- ディレクトリの作成実行 ------------------------------------------------
*
*   Parameter
*       LPTSTR Path : ディレクトリ名
*
*   Return Value
*       int ステータス
*           0=成功/else 失敗
*
*   Note
*       複数階層のディレクトリを作成する
*----------------------------------------------------------------------------*/

static int GoMakeDir(LPTSTR Path)
{
    LPTSTR Pos;
    _TCHAR Tmp[MY_MAX_PATH2+1];
    int Sts;

    Sts = 0;
    if(_tcscmp(Path+1, _T(":")) != 0)
    {
        if((Pos = _tcschr(Path, '\\')) != NULL)
        {
            while((Pos = _tcschr(Pos+1, '\\')) != NULL)
            {
                _tcsncpy(Tmp, Path, Pos - Path);
                Tmp[Pos - Path] = NUL;
                CreateDirectory_My(Tmp, NULL, YES);
            }
            Sts = !CreateDirectory_My(Path, NULL, YES);
        }
        else
        {
            SetLastError(161);  /* ERROR_BAD_PATHNAME */
            Sts = 1;
        }
    }
    return(Sts);
}


/*----- 更新されたファイルをコピー先にコピー ----------------------------------
*
*   Parameter
*       LPTSTR DstPath : 設定上の転送先のパス名
*       UINT DrvType : ドライブのタイプ
        PROC_OPTIONS options : 処理オプション
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int CopyUpdateFile(LPTSTR DstPath, UINT DrvType, PROC_OPTIONS *options)
{
    int Sts;
    _TCHAR Src[MY_MAX_PATH2+1];
    _TCHAR ScnName[MY_MAX_PATH2+1];
    _TCHAR Dst[MY_MAX_PATH2+1];
    LPTSTR SrcFpos;
    LPTSTR DstFpos;
    LPTSTR ScnPos;
    int Type;
    PROC_OPTIONS    tmp_options;

    Sts = SUCCESS;

    MoveFirstItem();
    do
    {
        if((Sts = CheckAbort()) == FAIL)
            break;

        if((Type = GetSrcType()) == TREE_FOLDER)
        {
            /*===== フォルダ単位でバックアップ =====*/

            GetDstPath(Dst, DstPath);
            SetYenTail(Dst);
            DstFpos = _tcschr(Dst, NUL);

            GetSrcPath(Src, ScnName);
            SetYenTail(Src);
            SrcFpos = _tcschr(Src, NUL);
            if(TCSLEN(ScnName) == 0)
                memcpy(ScnName, _T("*\0\0"), 3 * sizeof(_TCHAR));

            ScnPos = ScnName;
            do
            {
                _tcscpy(SrcFpos, ScnPos);
                Sts = GoFileCopy(Src, SrcFpos, Dst, DstFpos, DrvType, options);

                ScnPos = _tcschr(ScnPos, NUL) + 1;
            }
            while((Sts == SUCCESS) && (*ScnPos != NUL));

            /* フォルダのタイムスタンプをあわせる */
            *DstFpos = 0;
            *SrcFpos = 0;
            SetFileTimeStamp(Src, Dst, DrvType);
        }
        else if (Type == TREE_FILE)
        {
            /*===== ファイル単位でバックアップ =====*/

            GetSrcPath(Src, ScnName);
            SrcFpos = GetFileName(Src);

            _tcscpy(Dst, DstPath);
            SetYenTail(Dst);
            DstFpos = _tcschr(Dst, NUL);

            memcpy(&tmp_options, options, sizeof(PROC_OPTIONS));
            tmp_options.IgnoreFiles = _T("");
            tmp_options.IgnSys = NO;
            tmp_options.IgnHid = NO;

            Sts = GoFileCopy(Src, SrcFpos, Dst, DstFpos, DrvType, &tmp_options);
        }
    }
    while((Sts == SUCCESS) && (MoveNextItem() == SUCCESS));

    if(IgnoreErr == YES)
        Sts = CheckAbort();

    return(Sts);
}


/*----- ファイルコピーを実行 --------------------------------------------------
*
*   Parameter
*       LPTSTR Src : コピー元のパス名
*       LPTSTR SrcFpos : コピー元のファイル名のセット位置
*       LPTSTR Dst : コピー先のパス名
*       LPTSTR DstFpos : コピー先のファイル名のセット位置
*       UINT DrvType : ドライブのタイプ
        PROC_OPTIONS options : 処理オプション
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int GoFileCopy(LPTSTR Src, LPTSTR SrcFpos, LPTSTR Dst, LPTSTR DstFpos, UINT DrvType, PROC_OPTIONS *options)
{
    int Copy;
    HANDLE fHndSrc;
    WIN32_FIND_DATA SrcFinfo;
    HANDLE fHndDst;
    WIN32_FIND_DATA DstFinfo;
//  WORD SrcDate;
//  WORD SrcTime;
//  WORD DstDate;
//  WORD DstTime;
//  SYSTEMTIME SrcSysTime;
//  SYSTEMTIME DstSysTime;
    int Sts;
    DWORD Err;
    LPTSTR  lpBuffer;
    OVERWRITENOTIFYDATA overWrite;

    Sts = SUCCESS;
    if((fHndSrc = FindFirstFile_My(Src, &SrcFinfo, NO)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            SrcFinfo.dwFileAttributes &= ~(FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
            if((Sts = CheckAbort()) == FAIL)
                break;

            _tcscpy(SrcFpos, SrcFinfo.cFileName);
            if((SrcFinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
               (CheckFnameWithArray(Src, options->IgnoreFiles) == YES) ||
               (DoCheckIgnSysHid(&SrcFinfo, options->IgnSys, options->IgnHid, options->IgnBigSize) == YES))
            {
                /* コピーしない */
            }
            else
            {
                _tcscpy(DstFpos, SrcFinfo.cFileName);

                if((options->ForceCopy == NO) &&
                   ((fHndDst = FindFirstFile_My(Dst, &DstFinfo, YES)) != INVALID_HANDLE_VALUE))
                {
                    FindClose(fHndDst);

                    DstFinfo.dwFileAttributes &= ~(FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);

                    Copy = 0;
                    if(options->Tole == -1)
                    {
                        if((SrcFinfo.nFileSizeLow != DstFinfo.nFileSizeLow) ||
                           (SrcFinfo.nFileSizeHigh != DstFinfo.nFileSizeHigh))
                            Copy = 1;
                    }
                    else
                    {
                        CheckTimeTolerance(&SrcFinfo.ftLastWriteTime, &DstFinfo.ftLastWriteTime, options->Tole);
                        if(options->NewOnly == YES)
                        {
                            if(CompareFileTime(&SrcFinfo.ftLastWriteTime, &DstFinfo.ftLastWriteTime) > 0)
                                Copy = 1;
                        }
                        else
                        {
                            if((SrcFinfo.nFileSizeLow != DstFinfo.nFileSizeLow) ||
                               (SrcFinfo.nFileSizeHigh != DstFinfo.nFileSizeHigh) ||
                               (CompareFileTime(&SrcFinfo.ftLastWriteTime, &DstFinfo.ftLastWriteTime) != 0))
                            {
                                Copy = 1;
                            }
                        }
                    }

                    if(Copy != 1)
                    {
//                      if(_tcscmp(SrcFinfo.cFileName, DstFinfo.cFileName) != 0)
                        if(FnameCompare(SrcFinfo.cFileName, DstFinfo.cFileName) != 0)
                        {
#ifdef NO_OPERATION
                            DoPrintf(MSGJPN_73, Dst);
#else
                            SetTaskMsg(TASKMSG_NOR, MSGJPN_74, Dst);
                            MoveFile_My(Dst, Dst, YES);  /* 大文字/小文字を合わせる */
#endif
                        }

                        if((options->IgnAttr == 0) && (SrcFinfo.dwFileAttributes != DstFinfo.dwFileAttributes))
                        {
#ifdef NO_OPERATION
                            DoPrintf(MSGJPN_75, Dst);
#else
                            SetTaskMsg(TASKMSG_NOR, MSGJPN_76, Dst, SrcFinfo.dwFileAttributes, DstFinfo.dwFileAttributes);
                            SetFileAttributes_My(Dst, SrcFinfo.dwFileAttributes, YES);
#endif
                        }
                    }
                }
                else
                    Copy = 2;

                if(Copy == 1)
                {
                    /* 上書きの確認 */
                    if((OverwriteMode != YES_ALL) && (OverwriteMode != NO_ALL) && (OverwriteMode != GO_ABORT))
                    {
                        overWrite.Fname         = Dst;
                        overWrite.SrcTime       = SrcFinfo.ftLastWriteTime;
                        overWrite.DstTime       = DstFinfo.ftLastWriteTime;
                        overWrite.SrcSizeHigh   = SrcFinfo.nFileSizeHigh;
                        overWrite.SrcSizeLow    = SrcFinfo.nFileSizeLow;
                        overWrite.DstSizeHigh   = DstFinfo.nFileSizeHigh;
                        overWrite.DstSizeLow    = DstFinfo.nFileSizeLow;
                        OverwriteMode = DialogBoxParamI(GetBupInst(), MAKEINTRESOURCE(overwrite_notify_dlg), GetMainHwnd(), OverWriteNotifyDlgProc, (LPARAM)&overWrite);
                    }

                    if(OverwriteMode == GO_ABORT)
                    {
                        GoAbort = YES;
                        Copy = 0;
                    }
                    if((OverwriteMode == NO) || (OverwriteMode == NO_ALL))
                        Copy = 0;
                }

                if((Copy == 1) || (Copy == 2))
                {
#ifdef NO_OPERATION
                    DoPrintf(_T("  Copy %s --> %s\n"), Src, Dst);
#else
                    CopyFileCount++;
                    if(Copy == 1)
                    {
                        if(LogVerbose)
                        {
                            unsigned __int64    tmp64;
                            double              size1;
                            double              size2;
                            tmp64 = (unsigned __int64)SrcFinfo.nFileSizeLow +
                                        ((unsigned __int64)SrcFinfo.nFileSizeHigh << 32);
                            size1 = (double)tmp64;
                            tmp64 = (unsigned __int64)DstFinfo.nFileSizeLow +
                                        ((unsigned __int64)DstFinfo.nFileSizeHigh << 32);
                            size2 = (double)tmp64;
                            if(size1 != size2)
                            {
                                _TCHAR              sizestr1[80];
                                _TCHAR              sizestr2[80];
                                MakeSizeString(size1, sizestr1);
                                MakeSizeString(size2, sizestr2);
                                SetTaskMsg(TASKMSG_NOR, MSGJPN_126, Src, sizestr1, sizestr2);
                            }
                            else
                            {
                                _TCHAR              timestr1[80];
                                _TCHAR              timestr2[80];
                                FileTime2TimeString(&SrcFinfo.ftLastWriteTime, timestr1);
                                FileTime2TimeString(&DstFinfo.ftLastWriteTime, timestr2);
                                SetTaskMsg(TASKMSG_NOR, MSGJPN_127, Src, timestr1, timestr2);
                            }
                        }
                        else
                        {
                            SetTaskMsg(TASKMSG_NOR, MSGJPN_77, Src);
                        }
                    }
                    else
                    {
                        SetTaskMsg(TASKMSG_NOR, MSGJPN_78, Src);
                    }

                    GoDelete1(Dst, NO, NULL);
                    if(CopyFile1(Src, Dst, options->Wait, DrvType) != TRUE)
                    {
                        ErrorCount++;
                        if((Err = GetLastError()) == ERROR_DISK_FULL)
                            SetTaskMsg(TASKMSG_ERR, MSGJPN_79);
                        else
                        {
                            FormatMessage(
                                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                Err,
                                LANG_USER_DEFAULT,
                                (LPTSTR )&lpBuffer,
                                0,
                                NULL );
                            RemoveReturnCode(lpBuffer);
                            SetTaskMsg(TASKMSG_ERR, MSGJPN_80, Src, lpBuffer);
                            LocalFree(lpBuffer);
                        }

                        if(IgnoreErr == NO)
                        {
                            Sts = FAIL;
                            break;
                        }
                    }
#endif
                }
            }
        }
        while(FindNextFile(fHndSrc, &SrcFinfo) == TRUE);

        FindClose(fHndSrc);
    }
    return(Sts);
}




/*----- タイムスタンプの許容誤差のチェック ------------------------------------
*
*   Parameter
*       LPTSTR Src : コピー元
*       LPTSTR Dst : コピー先
*       int Tole : タイムスタンプの許容誤差
*
*   Return Value
*       BOOL ステータス
*           TRUE/FALSE
*----------------------------------------------------------------------------*/

static void CheckTimeTolerance(FILETIME *Src, FILETIME *Dst, int Tole)
{
    unsigned __int64 Tmp64_Src;
    unsigned __int64 Tmp64_Dst;
    unsigned __int64 Diff;

    Tmp64_Src = (unsigned __int64)Src->dwLowDateTime +
            ((unsigned __int64)Src->dwHighDateTime << 32);
    Tmp64_Dst = (unsigned __int64)Dst->dwLowDateTime +
            ((unsigned __int64)Dst->dwHighDateTime << 32);
    if(Tmp64_Src > Tmp64_Dst)
        Diff = Tmp64_Src - Tmp64_Dst;
    else
        Diff = Tmp64_Dst - Tmp64_Src;
    if(Diff < ((unsigned __int64)Tole * (unsigned __int64)10000000))
    {
        Src->dwLowDateTime = Dst->dwLowDateTime;
        Src->dwHighDateTime = Dst->dwHighDateTime;
    }
}




typedef struct {
    BOOL        Cancel;
    int         Wait;
} COPYCALLBACKINFO;

DWORD CALLBACK CopyProgressRoutine(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData
);


/*----- ファイルをコピーする --------------------------------------------------
*
*   Parameter
*       LPTSTR Src : コピー元
*       LPTSTR Dst : コピー先
*       int Wait : 転送時のウエイト時間
*       UINT DrvType : ドライブのタイプ
*
*   Return Value
*       BOOL ステータス
*           TRUE/FALSE
*----------------------------------------------------------------------------*/

static BOOL CopyFile1(LPTSTR Src, LPTSTR Dst, int Wait, UINT DrvType)
{
#if FILECOPY_METHOD==READFILE_WRITEFILE
    HANDLE hRead;
    HANDLE hWrite;
    SECURITY_ATTRIBUTES SecRead;
    SECURITY_ATTRIBUTES SecWrite;
    static _TCHAR Buf[COPY_SIZE];
    DWORD nRead;
    DWORD nWrite;
    DWORD Attr;
    BOOL Sts;
    FILETIME CreTime;
    FILETIME AccTime;
    FILETIME ModTime;
//  FILETIME CreTimeUTC;
//  FILETIME AccTimeUTC;
//  FILETIME ModTimeUTC;
    DWORD SizeLow;
    LONGLONG Size;
    LONGLONG Copied;
//  SYSTEMTIME sTime;

    Sts = FALSE;

    SetFileProgress(0, 0);

    SecRead.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecRead.lpSecurityDescriptor = NULL;
    SecRead.bInheritHandle = FALSE;
    if((hRead = CreateFile_My(Src, GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE, &SecRead, OPEN_EXISTING, 0, NULL, NO)) != INVALID_HANDLE_VALUE)
    {
        SecWrite.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecWrite.lpSecurityDescriptor = NULL;
        SecWrite.bInheritHandle = FALSE;
        if((hWrite = CreateFile_My(Dst, GENERIC_READ|GENERIC_WRITE,
            FILE_SHARE_READ|FILE_SHARE_WRITE, &SecWrite, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, YES)) != INVALID_HANDLE_VALUE)
        {
            SizeLow = GetFileSize(hRead, &(DWORD)Size);
            Size <<= 32;
            Size |= SizeLow;
            Copied = 0;
            SetFileProgress(Size, Copied);

            Sts = TRUE;
            while(ReadFile(hRead, Buf, COPY_SIZE, &nRead, NULL) == TRUE)
            {
                if(nRead == 0)
                    break;
                if(CheckAbort() == FAIL)
                    break;

                if(WriteFile(hWrite, Buf, nRead, &nWrite, NULL) == FALSE)
                {
                    Sts = FALSE;
                    break;
                }
                Copied += nRead;
                SetFileProgress(Size, Copied);

                if(Wait != 0)
                    Sleep(Wait * WAIT_TIMER);
            }

            if(Wait != 0)
                Sleep(Wait * WAIT_TIMER);

            /* ファイルのタイムスタンプを合わせる */
            if(Sts == TRUE)
            {
                if(GetFileTime(hRead, &CreTime, &AccTime, &ModTime) != 0)
                {
//                  if(DrvType == DRIVE_CDROM)
//                  {
//                      LocalFileTimeToFileTime(&CreTime, &CreTimeUTC);
//                      LocalFileTimeToFileTime(&AccTime, &AccTimeUTC);
//                      LocalFileTimeToFileTime(&ModTime, &ModTimeUTC);
//                      SetFileTime(hWrite, &CreTimeUTC, &AccTimeUTC, &ModTimeUTC);
//                  }
//                  else
                        SetFileTime(hWrite, &CreTime, &AccTime, &ModTime);
                }
            }

            CloseHandle(hWrite);
        }
        CloseHandle(hRead);
    }

    /* ファイルの属性を合わせる */
    if(Sts == TRUE)
    {
        if((Attr = GetFileAttributes_My(Src, NO)) != 0xFFFFFFFF)
            SetFileAttributes_My(Dst, Attr, YES);
    }
//  SetFileProgress(0, 0);

    return(Sts);
#endif
#if FILECOPY_METHOD==BACKUPREAD_BACKUPWRITE
    HANDLE hRead;
    HANDLE hWrite;
    SECURITY_ATTRIBUTES SecRead;
    SECURITY_ATTRIBUTES SecWrite;
    static BYTE Buf[COPY_SIZE];
    DWORD nRead;
    DWORD nWrite;
    DWORD Attr;
    BOOL Sts;
    FILETIME CreTime;
    FILETIME AccTime;
    FILETIME ModTime;
//  FILETIME CreTimeUTC;
//  FILETIME AccTimeUTC;
//  FILETIME ModTimeUTC;
    DWORD SizeLow;
    LONGLONG Size;
    LONGLONG Copied;
//  SYSTEMTIME sTime;
    LPVOID readContext;
    LPVOID writeContext;

    Sts = FALSE;

    SetFileProgress(0, 0);

    SecRead.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecRead.lpSecurityDescriptor = NULL;
    SecRead.bInheritHandle = FALSE;
    if((hRead = CreateFile_My(Src, GENERIC_READ,
        FILE_SHARE_READ|FILE_SHARE_WRITE, &SecRead, OPEN_EXISTING, 0, NULL, NO)) != INVALID_HANDLE_VALUE)
    {
        SecWrite.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecWrite.lpSecurityDescriptor = NULL;
        SecWrite.bInheritHandle = FALSE;
        if((hWrite = CreateFile_My(Dst, GENERIC_READ|GENERIC_WRITE,
            FILE_SHARE_READ|FILE_SHARE_WRITE, &SecWrite, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL, YES)) != INVALID_HANDLE_VALUE)
        {
            SizeLow = GetFileSize(hRead, &(DWORD)Size);
            Size <<= 32;
            Size |= SizeLow;
            Copied = 0;
            SetFileProgress(Size, Copied);

            Sts = TRUE;
            readContext = NULL;
            writeContext = NULL;
            while(Sts)
            {
                if(!BackupRead(hRead, Buf, COPY_SIZE, &nRead, FALSE, FALSE, &readContext))
                {
                    Sts = FALSE;
                    break;
                }
                if(nRead == 0)
                    break;
                if(CheckAbort() == FAIL)
                    break;

                if(!BackupWrite(hWrite, Buf, nRead, &nWrite, FALSE, FALSE, &writeContext))
                {
                    Sts = FALSE;
                    break;
                }
                Copied += nRead;
                SetFileProgress(Size, Copied);

                if(Wait != 0)
                    Sleep(Wait * WAIT_TIMER);
            }

            /* free context memory */
            BackupRead(hRead, Buf, COPY_SIZE, &nRead, TRUE, FALSE, &readContext);
            BackupWrite(hWrite, Buf, nRead, &nWrite, TRUE, FALSE, &writeContext);

            if(Wait != 0)
                Sleep(Wait * WAIT_TIMER);

            /* ファイルのタイムスタンプを合わせる */
            if(Sts == TRUE)
            {
                if(GetFileTime(hRead, &CreTime, &AccTime, &ModTime) != 0)
                {
//                  if(DrvType == DRIVE_CDROM)
//                  {
//                      LocalFileTimeToFileTime(&CreTime, &CreTimeUTC);
//                      LocalFileTimeToFileTime(&AccTime, &AccTimeUTC);
//                      LocalFileTimeToFileTime(&ModTime, &ModTimeUTC);
//                      SetFileTime(hWrite, &CreTimeUTC, &AccTimeUTC, &ModTimeUTC);
//                  }
//                  else
                        SetFileTime(hWrite, &CreTime, &AccTime, &ModTime);
                }
            }

            CloseHandle(hWrite);
        }
        CloseHandle(hRead);
    }

    /* ファイルの属性を合わせる */
    if(Sts == TRUE)
    {
        if((Attr = GetFileAttributes_My(Src, NO)) != 0xFFFFFFFF)
            SetFileAttributes_My(Dst, Attr, YES);
    }
//  SetFileProgress(0, 0);

    return(Sts);
#endif
#if FILECOPY_METHOD==COPYFILEEX
    HANDLE hRead;
    HANDLE hWrite;
    SECURITY_ATTRIBUTES SecRead;
    SECURITY_ATTRIBUTES SecWrite;
    FILETIME CreTime;
    FILETIME AccTime;
    FILETIME ModTime;

    BOOL sts = TRUE;
    LPTSTR lSrc = MakeLongPath(Src, NO);
    LPTSTR lDst = MakeLongPath(Dst, YES);
    COPYCALLBACKINFO *info = malloc(sizeof(COPYCALLBACKINFO));
    info->Cancel = FALSE;
    info->Wait = Wait;

    if(CopyFileEx(lSrc, lDst, CopyProgressRoutine, info, &info->Cancel, 0) == 0)
    {
        if(info->Cancel == FALSE)
        {
            sts = FALSE;
        }
    }

//  SetFileProgress(0, 0);

    if (sts == TRUE)
    {
        SecRead.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecRead.lpSecurityDescriptor = NULL;
        SecRead.bInheritHandle = FALSE;
        if((hRead = CreateFile_My(Src, GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE, &SecRead, OPEN_EXISTING, 0, NULL, NO)) != INVALID_HANDLE_VALUE)
        {
            SecWrite.nLength = sizeof(SECURITY_ATTRIBUTES);
            SecWrite.lpSecurityDescriptor = NULL;
            SecWrite.bInheritHandle = FALSE;
            if((hWrite = CreateFile_My(Dst, GENERIC_READ|GENERIC_WRITE,
                FILE_SHARE_READ|FILE_SHARE_WRITE, &SecWrite, OPEN_EXISTING, 0, NULL, YES)) != INVALID_HANDLE_VALUE)
            {
                if(GetFileTime(hRead, &CreTime, &AccTime, &ModTime) != 0)
                {
//                  if(DrvType == DRIVE_CDROM)
//                  {
//                      LocalFileTimeToFileTime(&CreTime, &CreTimeUTC);
//                      LocalFileTimeToFileTime(&AccTime, &AccTimeUTC);
//                      LocalFileTimeToFileTime(&ModTime, &ModTimeUTC);
//                      SetFileTime(hWrite, &CreTimeUTC, &AccTimeUTC, &ModTimeUTC);
//                  }
//                  else
                        SetFileTime(hWrite, &CreTime, &AccTime, &ModTime);
                }
            }
            CloseHandle(hWrite);
        }
        CloseHandle(hRead);
    }

    free(info);
    free(lDst);
    free(lSrc);

    return sts;
#endif
}


#if FILECOPY_METHOD==COPYFILEEX
/*----- CopyFileEx関数のコールバック ------------------------------------------
*
*   Parameter
*       CopyFileEx関数を参照
*
*   Return Value
*       CopyFileEx関数を参照
*----------------------------------------------------------------------------*/
DWORD CALLBACK CopyProgressRoutine(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData
)
{
    DWORD ret = PROGRESS_CONTINUE;
    COPYCALLBACKINFO *info = (COPYCALLBACKINFO*)lpData;

    SetFileProgress(TotalFileSize.QuadPart, TotalBytesTransferred.QuadPart);

    if(CheckAbort() == FAIL)
    {
        info->Cancel = TRUE;
        ret = PROGRESS_CANCEL;
    }
    else
    {
        if(info->Wait != 0)
        {
            Sleep(info->Wait * WAIT_TIMER);
        }
    }
    return ret;
}
#endif


/*----- １つのファイル／ディレクトリの削除 ------------------------------------
*
*   Parameter
*       LPTSTR Fname : ファイル名
*       int ErrRep : エラー報告するかどうか (YES/NO)
*       int DialogResult : ダイアログで選ばれた選択肢
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int GoDelete1(LPTSTR Fname, int ErrRep, int *DialogResult)
{
    int Sts;
    DWORD Attr;
    DWORD Err;
    LPTSTR  lpBuffer;

    Sts = SUCCESS;

    if(ErrRep == YES)
    {
        if((DeleteMode != YES_ALL) && (DeleteMode != NO_ALL) && (DeleteMode != GO_ABORT))
            DeleteMode = DialogBoxParamI(GetBupInst(), MAKEINTRESOURCE(del_notify_dlg), GetMainHwnd(), DeleteNotifyDlgProc, (LPARAM)Fname);

        if(DialogResult != NULL)
            *DialogResult = DeleteMode;

        if(DeleteMode == GO_ABORT)
            GoAbort = YES;
    }

    if((ErrRep == NO) || (DeleteMode == YES) || (DeleteMode == YES_ALL))
    {
        if(ErrRep == YES)
        {
            RmFileCount++;
            SetTaskMsg(TASKMSG_NOR, MSGJPN_81, Fname);
        }

        if(UseTrashCan)
        {
            if(MoveFileToTrashCan(Fname) != 0)
                Sts = FAIL;
        }
        else
        {
            Attr = GetFileAttributes_My(Fname, YES);
            if(Attr & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM))
                SetFileAttributes_My(Fname, FILE_ATTRIBUTE_NORMAL, YES);

            if(Attr & FILE_ATTRIBUTE_DIRECTORY)
            {
                if(RemoveDirectory_My(Fname, YES) == FALSE)
                    Sts = FAIL;
            }
            else
            {
                if(MoveInsteadDelete)
                {
                    if(MoveFileToDeletionFolder(Fname, MoveToFolder) != 0)
                        Sts = FAIL;
                }
                else
                {
                    if(DeleteFile_My(Fname, YES) == FALSE)
                        Sts = FAIL;
                }
            }
        }

        if(ErrRep == YES)
        {
            if(Sts != SUCCESS)
            {
                ErrorCount++;
                Err = GetLastError();
                FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    Err,
                    LANG_USER_DEFAULT,
                    (LPTSTR )&lpBuffer,
                    0,
                    NULL );
                RemoveReturnCode(lpBuffer);
                SetTaskMsg(TASKMSG_ERR, MSGJPN_82, Fname, lpBuffer);
                LocalFree(lpBuffer);
            }
        }
    }
    return(Sts);
}


/*----- ファイル削除確認のコールバック関数 ------------------------------------
*
*   Parameter
*       HWND hDlg : ウインドウハンドル
*       UINT message : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       LRESULT TRUE/FALSE
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK DeleteNotifyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG :
            PlaySound(_T("SystemQuestion"), NULL, SND_ALIAS|SND_ASYNC);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, EM_LIMITTEXT, 1024, 0);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, WM_SETTEXT, 0, lParam);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, EM_SETSEL, 0, -1);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, EM_SETSEL, (WPARAM)-1, -1);
            return(TRUE);

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK :
                    EndDialog(hDlg, YES);
                    break;

                case IDCANCEL :
                    EndDialog(hDlg, NO);
                    break;

                case DELNOT_YESALL :
                    EndDialog(hDlg, YES_ALL);
                    break;

                case DELNOT_NOALL :
                    EndDialog(hDlg, NO_ALL);
                    break;

                case DELNOT_ABORT :
                    EndDialog(hDlg, GO_ABORT);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- ファイル上書き確認のコールバック関数 -----------------------------------
*
*   Parameter
*       HWND hDlg : ウインドウハンドル
*       UINT message : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       LRESULT TRUE/FALSE
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK OverWriteNotifyDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    OVERWRITENOTIFYDATA *overWrite;
    unsigned __int64    tmp64;
    double              size;
    _TCHAR              str[80];
    _TCHAR              *newOld;
    long                sts;

    switch (message)
    {
        case WM_INITDIALOG :
            PlaySound(_T("SystemQuestion"), NULL, SND_ALIAS|SND_ASYNC);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, EM_LIMITTEXT, 1024, 0);

            overWrite = (OVERWRITENOTIFYDATA*)lParam;

            tmp64 = (unsigned __int64)overWrite->SrcSizeLow +
                        ((unsigned __int64)overWrite->SrcSizeHigh << 32);
            size = (double)tmp64;
            MakeSizeString(size, str);
            SendDlgItemMessageI(hDlg, DELNOT_SRC_SIZE, WM_SETTEXT, 0, (LPARAM)str);

            tmp64 = (unsigned __int64)overWrite->DstSizeLow +
                        ((unsigned __int64)overWrite->DstSizeHigh << 32);
            size = (double)tmp64;
            MakeSizeString(size, str);
            SendDlgItemMessageI(hDlg, DELNOT_DST_SIZE, WM_SETTEXT, 0, (LPARAM)str);


            FileTime2TimeString(&overWrite->SrcTime, str);
            SendDlgItemMessageI(hDlg, DELNOT_SRC_TIME, WM_SETTEXT, 0, (LPARAM)str);

            FileTime2TimeString(&overWrite->DstTime, str);
            SendDlgItemMessageI(hDlg, DELNOT_DST_TIME, WM_SETTEXT, 0, (LPARAM)str);

            newOld = _T("");
            sts = CompareFileTime(&overWrite->SrcTime, &overWrite->DstTime);
            if(sts > 0)
                newOld = MSGJPN_115;
            else if(sts < 0)
                newOld = MSGJPN_116;
            else
            {
                if((overWrite->SrcSizeLow != overWrite->DstSizeLow) ||
                   (overWrite->SrcSizeHigh != overWrite->DstSizeHigh))
                {
                    newOld = MSGJPN_117;
                }
            }
            _stprintf(str, MSGJPN_114, newOld);
            SendDlgItemMessageI(hDlg, DELNOT_MESSAGE, WM_SETTEXT, 0, (LPARAM)str);

            SendDlgItemMessageI(hDlg, DELNOT_FNAME, WM_SETTEXT, 0, (LPARAM)overWrite->Fname);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, EM_SETSEL, 0, -1);
            SendDlgItemMessageI(hDlg, DELNOT_FNAME, EM_SETSEL, (WPARAM)-1, -1);
            return(TRUE);

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK :
                    EndDialog(hDlg, YES);
                    break;

                case IDCANCEL :
                    EndDialog(hDlg, NO);
                    break;

                case DELNOT_YESALL :
                    EndDialog(hDlg, YES_ALL);
                    break;

                case DELNOT_NOALL :
                    EndDialog(hDlg, NO_ALL);
                    break;

                case DELNOT_ABORT :
                    EndDialog(hDlg, GO_ABORT);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- TreeViewを消去----------------- --------------------------------------
*
*   Parameter
*       HWND hWnd : TreeViewコントロールのハンドル
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void EraseSourceTree(HWND hWnd)
{
    SendMessageI(hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);
}

/*----- ディレクトリ構造をTreeViewに作成 --------------------------------------
*
*   Parameter
*       LPTSTR SrcRoot : 設定上の転送元のパス名
*       LPTSTR IgnoreDirs : バックアップしないフォルダ
        PROC_OPTIONS options : 処理オプション
*       HWND hWnd : TreeViewコントロールのハンドル
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MakeSourceTree(LPTSTR SrcPath, PROC_OPTIONS *options, HWND hWnd)
{
    TV_INSERTSTRUCT TvIns;
    HTREEITEM hItem;
    int Sts;

    Sts = FAIL;
    SendMessageI(hWnd, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);

    TvIns.hParent = TVI_ROOT;
    TvIns.hInsertAfter = TVI_LAST;
    TvIns.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    TvIns.item.cChildren = 1;
    TvIns.item.pszText = _T("");
    TvIns.item.iImage = TREE_ROOT;
    TvIns.item.iSelectedImage = TREE_ROOT;
    if((hItem = (HTREEITEM)SendMessage(hWnd, TVM_INSERTITEM, 0, (LPARAM)&TvIns)) != NULL)
    {
        SendMessageI(hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);

        Sts = SUCCESS;
        while(*SrcPath != NUL)
        {
            if(MakeSourceTreeOne(SrcPath, options, hItem, hWnd) == FAIL)
            {
//              if(IgnoreErr == NO)
//              {
                    Sts = FAIL;
                    break;
//              }
            }
            SrcPath += TCSLEN(SrcPath) + 1;
        }

        SendMessageI(hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);
        SendMessageI(hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
        CurItem = hItem;
        MoveFirstItem();
    }

    if(Sts == SUCCESS)
        Sts = CheckAbort();

    return(Sts);
}


/*----- ディレクトリ構造をTreeViewに作成（サブ）-------------------------------
*
*   Parameter
*       LPTSTR SrcRoot : 設定上の転送元のパス名
        PROC_OPTIONS options : 処理オプション
*       HTREEITEM Parent : 親ノードのハンドル
*       HWND hWnd : TreeViewコントロールのハンドル
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MakeSourceTreeOne(LPTSTR SrcRoot, PROC_OPTIONS *options, HTREEITEM Parent, HWND hWnd)
{
    TV_INSERTSTRUCT TvIns;
    int Sts;
    _TCHAR Dname[MY_MAX_PATH2+1];
    _TCHAR Fname[MY_MAX_PATH2+1];
    _TCHAR Tmp[MY_MAX_PATH2+1];
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    DWORD Type;
    LPTSTR Pos;
    _TCHAR Dname2[MY_MAX_PATH2 + 1];

    Sts = SUCCESS;

    _tcscpy(Dname, SrcRoot);
    MakePathandFile(Dname, Fname, NO);

    Type = FILE_ATTRIBUTE_DIRECTORY;
//    if(_tcscmp(SrcRoot+1, _T(":\\")) != 0)        //20150317 バックアップ元に D:\;*.mp3 などとした時の動作がおかしい
    if(_tcscmp(Dname+1, _T(":\\")) != 0)
    {
        /* フォルダ／ファイルがあるかチェック */
        RemoveYenTail(Dname);
        if((_tcschr(Dname, '*') != NULL) || (_tcschr(Dname, '?') != NULL))
        {
            // ワイルドカード使用のファイル単位でのバックアップ
            // 20150626 バックアップ元のフォルダ（ドライブ）が存在するかチェック
            _tcscpy(Dname2, Dname);
            Pos = _tcsrchr(Dname2, '\\');
            *(Pos + 1) = 0;
            if (_tcscmp(Dname2 + 1, _T(":\\")) != 0)    // ドライブの指定？ (D:\*.txt のような場合）
            {
                // フォルダがあるかチェック (D:\src\*.txt のような場合に D:\src があるか）
                Type = 0;
                RemoveYenTail(Dname2);
                if (GetFileAttributes_My(Dname2, NO) == 0xFFFFFFFF)
                {
                    Type = 0xFFFFFFFF;
                    ErrorCount++;
                    SetTaskMsg(TASKMSG_ERR, MSGJPN_83, Dname2);
                    Sts = FAIL;
                }
            }
            else
            {
                // ドライブがあるかチェック (D:\*.txt のような場合に D:\ があるか）
                Type = 0;
                if (GetDriveType(Dname2) == DRIVE_NO_ROOT_DIR)
                {
                    Type = 0xFFFFFFFF;
                    ErrorCount++;
                    SetTaskMsg(TASKMSG_ERR, MSGJPN_83, Dname2);
                    Sts = FAIL;
                }
            }
        }
        else if((Type = GetFileAttributes_My(Dname, NO)) != 0xFFFFFFFF)
        {
            /* 大文字／小文字を合わせるための処理 */
            if((fHnd = FindFirstFile_My(Dname, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
            {
                FindClose(fHnd);
                _tcscpy(GetFileName(Dname), FindBuf.cFileName);
            }
        }
        else
        {
            ErrorCount++;
            SetTaskMsg(TASKMSG_ERR, MSGJPN_83, Dname);
            Sts = FAIL;
        }
    }
    else
    {
//        if((GetDriveType(SrcRoot) == DRIVE_NO_ROOT_DIR) ||        //20150317 バックアップ元に D:\;*.mp3 などとした時の動作がおかしい
        if((GetDriveType(Dname) == DRIVE_NO_ROOT_DIR) ||
           ((Type = GetFileAttributes_My(Dname, NO)) == 0xFFFFFFFF))
        {
            ErrorCount++;
            SetTaskMsg(TASKMSG_ERR, MSGJPN_83, Dname);
            Sts = FAIL;
        }
    }

    if((Sts == SUCCESS) && (Type != 0xFFFFFFFF))
    {
        _tcscpy(Tmp, Dname);
        if(TCSLEN(Fname) > 0)
        {
            _tcscat(Tmp, _T(";"));
            _tcscat(Tmp, Fname);
        }

        TvIns.hParent = Parent;
        TvIns.hInsertAfter = TVI_LAST;
        TvIns.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        TvIns.item.pszText = Tmp;
        if((Type & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            TvIns.item.cChildren = 1;
            TvIns.item.iImage = TREE_FOLDER;
            TvIns.item.iSelectedImage = TREE_FOLDER_SEL;
        }
        else
        {
            TvIns.item.cChildren = 0;
            TvIns.item.iImage = TREE_FILE;
            TvIns.item.iSelectedImage = TREE_FILE_SEL;
        }

        if((Parent = (HTREEITEM)SendMessage(hWnd, TVM_INSERTITEM, 0, (LPARAM)&TvIns)) != NULL)
        {
            if((Type & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                Sts = MakeSubTree(Dname, options, Parent, hWnd);
                SendMessageI(hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)Parent);
            }
        }
    }
    return(Sts);
}


/*----- サブディレクトリ構造をTreeViewに作成（サブ２）-------------------------
*
*   Parameter
*       LPTSTR SrcRoot : パス名
        PROC_OPTIONS options : 処理オプション
*       HTREEITEM Parent : 親ノードのハンドル
*       HWND hWnd : TreeViewコントロールのハンドル
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MakeSubTree(LPTSTR SrcRoot, PROC_OPTIONS *options, HTREEITEM Parent, HWND hWnd)
{
    _TCHAR Src[MY_MAX_PATH2+1];
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    TV_INSERTSTRUCT TvIns;
    HTREEITEM hItem;
    int Sts;
    DWORD Err = 0;
    BOOL Next;
    LPTSTR  lpBuffer;

    Sts = SUCCESS;
    _tcscpy(Src, SrcRoot);
    SetYenTail(Src);
    _tcscat(Src, _T("*"));
    if((fHnd = FindFirstFile_My(Src, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if((Sts = CheckAbort()) == FAIL)
                break;

            _tcscpy(Src, SrcRoot);
            SetYenTail(Src);
            _tcscat(Src, FindBuf.cFileName);

            if(FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if((_tcscmp(FindBuf.cFileName, _T(".")) == 0) ||
                   (_tcscmp(FindBuf.cFileName, _T("..")) == 0) ||
                   (CheckFnameWithArray(Src, options->IgnoreDirs) == YES) ||
                   ((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && (options->IgnSys == YES)) ||
                   ((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && (options->IgnHid == YES)))
                {
                    /* 除外 */
                }
                else
                {
                    TvIns.hParent = Parent;
                    TvIns.hInsertAfter = TVI_LAST;
                    TvIns.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
                    TvIns.item.cChildren = 1;
                    TvIns.item.pszText = FindBuf.cFileName;
                    TvIns.item.iImage = TREE_FOLDER;
                    TvIns.item.iSelectedImage = TREE_FOLDER_SEL;

                    if((hItem = (HTREEITEM)SendMessage(hWnd, TVM_INSERTITEM, 0, (LPARAM)&TvIns)) == NULL)
                    {
                        Sts = FAIL;
                        break;
                    }
                    if((Sts = MakeSubTree(Src, options, hItem, hWnd)) != SUCCESS)
                        break;

                    SendMessageI(hWnd, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);
                    /* 適当なタイミングでTreeViewを再表示する */
                    if(++TviewDispCounter == 200)
                    {
                        SendMessageI(hWnd, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
                        TviewDispCounter=0;
                    }
                }
            }

            Next = FindNextFile(fHnd, &FindBuf);
            if (Next != TRUE)
            {
                Err = GetLastError();
                if (Err != ERROR_NO_MORE_FILES)
                {
                    Sts = FAIL;
                    break;
                }
            }
        }
        while(Next == TRUE);

        FindClose(fHnd);
    }
    else
    {
        Err = GetLastError();
        if ((Err != ERROR_FILE_NOT_FOUND) && (Err != ERROR_ACCESS_DENIED))
        {
            Sts = FAIL;
        }
    }

    if((Sts == FAIL) && (Err != 0))
    {
        ErrorCount++;
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            Err,
            LANG_USER_DEFAULT,
            (LPTSTR )&lpBuffer,
            0,
            NULL );
        RemoveReturnCode(lpBuffer);
        SetTaskMsg(TASKMSG_ERR, MSGJPN_129, Src, lpBuffer);
        LocalFree(lpBuffer);
    }

    return(Sts);
}


/*----- TreeViewの最初の項目に移動 --------------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MoveFirstItem(void)
{
    HTREEITEM hItem;
    int Sts;

    Sts = FAIL;
    if((hItem = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_ROOT, 0)) != NULL)
    {
        CurItem = hItem;
        SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
        Sts = SUCCESS;
    }
    return(Sts);
}


/*----- TreeViewの次の項目に移動 ----------------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MoveNextItem(void)
{
    HTREEITEM hItem;
    HTREEITEM hNow;
    int Sts;

    Sts = SUCCESS;
    SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)CurItem);
    hNow = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_CARET, 0);
    if((hItem = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hNow)) == NULL)
    {
        while((hItem = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hNow)) == NULL)
        {
            if((hNow = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hNow)) == NULL)
            {
                Sts = FAIL;
                break;
            }
        }
    }

    if(Sts == SUCCESS)
    {
        CurItem = hItem;
        SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
    }
    return(Sts);
}


/*----- TreeViewの現在の項目の種類を返す --------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       int タイプ (TREE_xxx)
*----------------------------------------------------------------------------*/

static int GetSrcType(void)
{
    HTREEITEM hItem;
    TV_ITEM TvItem;

    TvItem.iImage = TREE_ERROR;
    SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)CurItem);
    if((hItem = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_CARET, 0)) != NULL)
    {
        TvItem.mask = TVIF_IMAGE;
        TvItem.hItem = hItem;
        SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETITEM, 0, (LPARAM)&TvItem);
    }
    return(TvItem.iImage);
}


/*----- TreeViewの現在の項目のパスを返す --------------------------------------
*
*   Parameter
*       LPTSTR Src : パス名のコピー先
*       LPTSTR ScnName : 対象ファイル名のリスト（マルチ文字列）
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*
*   Note
*       対象ファイル名のリストは、バックアップ元をフォルダ名＋ファイル名で指定
*       した場合のファイル名部分
*           D:\backup;*.txt;*.log
*               --> パス名：D:\backup、  対象ファイル名：*.txt *.log
*----------------------------------------------------------------------------*/

static int GetSrcPath(LPTSTR Src, LPTSTR ScnName)
{
    HTREEITEM hItem[128];
    TV_ITEM TvItem;
    _TCHAR Tmp[MY_MAX_PATH2+1];
    int i;
    int Sts;
    int First;

    Sts = FAIL;
    i = 0;
    SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)CurItem);
    if((hItem[i] = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_CARET, 0)) != NULL)
    {
        while(hItem[i] != NULL)
        {
            i++;
            hItem[i] = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem[i-1]);
        }

        *Src = NUL;
        *ScnName = NUL;
        if(i > 1)
        {
            First = YES;
            for(i -= 2; i >= 0; i--)
            {
                SetYenTail(Src);
                TvItem.mask = TVIF_TEXT;
                TvItem.hItem = hItem[i];
                TvItem.pszText = Tmp;
                TvItem.cchTextMax = MY_MAX_PATH;
                SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETITEM, 0, (LPARAM)&TvItem);
                _tcscat(Src, Tmp);

                if(First == YES)
                {
                    MakePathandFile(Src, ScnName, YES);
                    First = NO;
                }
            }
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- TreeViewの現在の項目をから転送先のパスを作成して返す ------------------
*
*   Parameter
*       LPTSTR Dst : パス名のコピー先
*       LPTSTR DstPath : 設定上の転送先のパス名
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int GetDstPath(LPTSTR Dst, LPTSTR DstPath)
{
    HTREEITEM hItem[128];
    TV_ITEM TvItem;
    _TCHAR Tmp[MY_MAX_PATH2+1];
    int i;
    int Sts;
    int First;

    Sts = FAIL;
    i = 0;
    SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_SELECTITEM, TVGN_CARET, (LPARAM)CurItem);
    if((hItem[i] = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_CARET, 0)) != NULL)
    {
        while(hItem[i] != NULL)
        {
            i++;
            hItem[i] = (HTREEITEM)SendDlgItemMessage(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem[i-1]);
        }

        _tcscpy(Dst, DstPath);
        if(i > 1)
        {
            First = YES;
            for(i -= 2; i >= 0; i--)
            {
                TvItem.mask = TVIF_TEXT;
                TvItem.hItem = hItem[i];
                TvItem.pszText = Tmp;
                TvItem.cchTextMax = MY_MAX_PATH;
                SendDlgItemMessageI(GetTransDlgHwnd(), TRANS_DIRLIST, TVM_GETITEM, 0, (LPARAM)&TvItem);
                if(First == YES)
                {
                    if(0 == NoMakeTopDir)
                    {
                        MakePathandFile(Tmp, NULL, NO);

                        //ここ

                        if(TCSLEN(GetFileName(Tmp)) > 0)
                        {
                            SetYenTail(Dst);
                            _tcscat(Dst, GetFileName(Tmp));
                        }
                    }
                    First = NO;
                }
                else
                {
                    SetYenTail(Dst);
                    _tcscat(Dst, Tmp);
                }
            }
            Sts = SUCCESS;
        }
    }

//SetTaskMsg(TASKMSG_NOR, _T("RET=%d, >%s<, >%s<"), Sts, Dst, DstPath);


    return(Sts);
}


/*----- カレントディレクトリのディレクトリリストを作成する --------------------
*
*   Parameter
*       LPTSTR ScnPath : 検索パス名
*       DIRTREE **Base : ディレクトリリストのベースポインタ
*       int Type : リストのタイプ (0=フォルダ, 1=ファイル)
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int MakeDirTable(LPTSTR ScnPath, DIRTREE **Base, int Type)
{
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    DIRTREE *Pos;
    DIRTREE *Prev;
    int Sts;

    Sts = SUCCESS;
    *Base = NULL;
    if((fHnd = FindFirstFile_My(ScnPath, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(((Type == 0) &&
                (FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                (_tcscmp(FindBuf.cFileName, _T(".")) != 0) &&
                (_tcscmp(FindBuf.cFileName, _T("..")) != 0)) ||
               ((Type == 1) &&
                ((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)))
            {
                if((Pos = malloc(sizeof(DIRTREE))) != NULL)
                {
                    if(*Base == NULL)
                        *Base = Pos;
                    else
                        Prev->Next = Pos;
                    _tcscpy(Pos->Fname, FindBuf.cFileName);
                    Pos->Next = NULL;
                    Prev = Pos;
                }
                else
                {
                    Sts = FAIL;
                    break;
                }
            }
        }
        while(FindNextFile(fHnd, &FindBuf) == TRUE);

        FindClose(fHnd);
    }

    if(Sts == FAIL)
        ReleaseDirList(Base);

    return(Sts);
}


/*----- ディレクトリリストを削除する ------------------------------------------
*
*   Parameter
*       DIRTREE **Base : ディレクトリリストのベースポインタ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void ReleaseDirList(DIRTREE **Base)
{
    DIRTREE *Pos;
    DIRTREE *Next;

    Pos = *Base;
    while(Pos != NULL)
    {
        Next = Pos->Next;
        free(Pos);
        Pos = Next;
    }
    *Base = NULL;
    return;
}


/*----- 処理の中止が行われたかどうかをチェック --------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL=中止された
*----------------------------------------------------------------------------*/

static int CheckAbort(void)
{
    int Sts;

    BackgrndMessageProc();

    while((GoAbort != YES) && (Pause == YES))
    {
        BackgrndMessageProc();
        Sleep(50);
    }

    Sts = SUCCESS;
    if(GoAbort == YES)
        Sts = FAIL;

    return(Sts);
}


/*----- バックアップ元をパス名とファイル名に分ける ----------------------------
*
*   Parameter
*       LPTSTR Path : バックアップ元／パス名を返すワーク
*       LPTSTR Fname : ファイル名を返すワーク (NULL=返さない)
*       int Multi : ファイル名をマルチ文字列にするかどうか (YES/NO)
*
*   Return Value
*       なし
*
*   Note
*       Path = _T("C:\Home;*.log;*.txt") , Multi = NO の場合の戻り値
*           Path = _T("C:\Home") , Fname = _T("*.log;*.txt")
*
*       Path = _T("C:\Home;*.log;*.txt") , Multi = YES の場合の戻り値
*           Path = _T("C:\Home") , Fname = _T("*.log\0*.txt\0")
*----------------------------------------------------------------------------*/

void MakePathandFile(LPTSTR Path, LPTSTR Fname, int Multi)
{
    LPTSTR Pos;

    if(Fname != NULL)
        _tcscpy(Fname, _T(""));

    if((Pos = _tcschr(Path, ';')) != NULL)
    {
        *Pos = NUL;

        if(Fname != NULL)
        {
            _tcscpy(Fname, Pos+1);
            if(Multi == YES)
            {
                *(_tcschr(Fname, NUL)+1) = NUL;     /* 末尾のNULは２つ */
                Pos = Fname;
                while((Pos = _tcschr(Pos, ';')) != NULL)
                    *Pos++ = NUL;
            }
        }
    }
    return;
}


/*----- タイムスタンプをあわせる ----------------------------------------------
*
*   Parameter
*       LPTSTR Src : バックアップ元
*       LPTSTR Dst : バックアップ先
*       UINT DrvType : ドライブのタイプ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void SetFileTimeStamp(LPTSTR Src, LPTSTR Dst, UINT DrvType)
{
    HANDLE hFile;
    SECURITY_ATTRIBUTES Sec;
    BOOL Sts;
    FILETIME CreTime;
    FILETIME AccTime;
    FILETIME ModTime;
//  FILETIME CreTimeUTC;
//  FILETIME AccTimeUTC;
//  FILETIME ModTimeUTC;
    DWORD Attr;

    if((TCSLEN(Src) > 2) && (_tcscmp(Src+1, _T(":\\")) != 0))
    {
        Sec.nLength = sizeof(SECURITY_ATTRIBUTES);
        Sec.lpSecurityDescriptor = NULL;
        Sec.bInheritHandle = FALSE;
        if((hFile = CreateFile_My(Src, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, &Sec, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL, NO)) != INVALID_HANDLE_VALUE)
        {
            Sts = GetFileTime(hFile, &CreTime, &AccTime, &ModTime);
            CloseHandle(hFile);
            if(Sts != 0)
            {
                // GENERIC_WRITEを指定するためにReadOnlyを解除
                if((Attr = GetFileAttributes_My(Dst, YES)) != 0xFFFFFFFF)
                    SetFileAttributes_My(Dst, Attr & ~FILE_ATTRIBUTE_READONLY, YES);

                if((hFile = CreateFile_My(Dst, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, &Sec, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL, YES)) != INVALID_HANDLE_VALUE)
                {
//                  if(DrvType == DRIVE_CDROM)
//                  {
//                      LocalFileTimeToFileTime(&CreTime, &CreTimeUTC);
//                      LocalFileTimeToFileTime(&AccTime, &AccTimeUTC);
//                      LocalFileTimeToFileTime(&ModTime, &ModTimeUTC);
//                      SetFileTime(hFile, &CreTimeUTC, &AccTimeUTC, &ModTimeUTC);
//                  }
//                  else
                        SetFileTime(hFile, &CreTime, &AccTime, &ModTime);
                    CloseHandle(hFile);
                }

                // アトリビュートを元に戻しておく
                if(Attr != 0xFFFFFFFF)
                    SetFileAttributes_My(Dst, Attr, YES);
            }
        }
    }

    return;
}


/*----- 除外すべきファイルかチェック ------------------------------------------
*
*   Parameter
*       LPTSTR Fname : ファイル名
*       int IgnSys : システムファイルを除外フラグ
*       int IgnHid : 隠しファイルを除外フラグ
*       int BigSize : このサイズ(MB)以上のファイルを除外(-1=除外しない)
*
*   Return Value
*       ステータス YES=除外する
*----------------------------------------------------------------------------*/

static int CheckIgnSysHid(LPTSTR Fname, int IgnSys, int IgnHid, int BigSize)
{
    HANDLE              fHnd;
    WIN32_FIND_DATA     FindBuf;
    int                 Sts;

    Sts = NO;
    if((fHnd = FindFirstFile_My(Fname, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
    {
        FindClose(fHnd);
        Sts = DoCheckIgnSysHid(&FindBuf, IgnSys, IgnHid, BigSize);
    }
    return(Sts);
}


/*----- 除外すべきファイルかチェック ------------------------------------------
*
*   Parameter
*       WIN32_FIND_DATA *FindBuf : 検索したファイル情報
*       int IgnSys : システムファイルを除外フラグ
*       int IgnHid : 隠しファイルを除外フラグ
*       int BigSize : このサイズ(MB)以上のファイルを除外(-1=除外しない)
*
*   Return Value
*       ステータス YES=除外する
*----------------------------------------------------------------------------*/

static int DoCheckIgnSysHid(WIN32_FIND_DATA *FindBuf, int IgnSys, int IgnHid, int BigSize)
{
    int                 Sts;
    unsigned __int64    big;
    unsigned __int64    size;

    Sts = NO;
    if(((FindBuf->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && (IgnSys == YES)) ||
       ((FindBuf->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && (IgnHid == YES)))
    {
        Sts = YES;
    }
    else if(BigSize != -1)
    {
        big = (unsigned __int64)BigSize * 1024 * 1024;
        size = (unsigned __int64)FindBuf->nFileSizeLow +
               ((unsigned __int64)FindBuf->nFileSizeHigh << 32);
        if(size >= big)
        {
            Sts = YES;
        }
    }
    return(Sts);
}


/*----- MAX_PATH以上のパス名に対応させる --------------------------------------
*
*   Parameter
*       path : パス名
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       パス名 (使用後はfreeすること）
*----------------------------------------------------------------------------*/

static LPTSTR MakeLongPath(LPCTSTR path, int normalization)
{
    LPTSTR newPath;

    BOOL toNFC = FALSE;
    int length = TCSLEN(path) + 1;
    if (normalization == YES)
    {
        if (NormalizationType == NORMALIZATION_TYPE_NFC)
        {
            length = NormalizeString(NormalizationC, path, -1, NULL, 0);
            if (length > 0)
            {
                toNFC = TRUE;
            }
            else
            {
                length = TCSLEN(path) + 1;
            }
        }
    }

    if(_tcsncmp(path, _T("\\\\"), 2) != 0)
    {
        newPath = malloc(sizeof(_TCHAR) * (length + 4));
        _tcscpy(newPath, _T("\\\\?\\"));
        if (toNFC)
        {
            NormalizeString(NormalizationC, path, -1, newPath+4, length);
        }
        else
        {
            _tcscpy(newPath+4, path);
        }
    }
    else
    {
        newPath = malloc(sizeof(_TCHAR) * (length + 8));
        _tcscpy(newPath, _T("\\\\?\\UNC\\"));
        if (toNFC)
        {
            NormalizeString(NormalizationC, path+2, -1, newPath+8, length);     /* skip // */
        }
        else
        {
            _tcscpy(newPath+8, path+2);     /* skip // */
        }
    }
    return newPath;
}


/*----- MAX_PATH以上のパス名に対応させる（常にNFDに変換） --------------------------
*
*   Parameter
*       path : パス名
*
*   Return Value
*       パス名 (使用後はfreeすること）
*----------------------------------------------------------------------------*/

static LPTSTR MakeLongPathNFD(LPCTSTR path)
{
    LPTSTR newPath;

    int length = NormalizeString(NormalizationD, path, -1, NULL, 0);
    if (length > 0)
    {
        if(_tcsncmp(path, _T("\\\\"), 2) != 0)
        {
            newPath = malloc(sizeof(_TCHAR) * (length + 4));
            _tcscpy(newPath, _T("\\\\?\\"));
            NormalizeString(NormalizationD, path, -1, newPath+4, length);
        }
        else
        {
            newPath = malloc(sizeof(_TCHAR) * (length + 8));
            _tcscpy(newPath, _T("\\\\?\\UNC\\"));
            NormalizeString(NormalizationD, path+2, -1, newPath+8, length);     /* skip // */
        }
    }
    else
    {
        /* ここには来ないはず */
        newPath = malloc(sizeof(_TCHAR) * (TCSLEN(path) + 1));
        _tcscpy(newPath, path);
    }
    return newPath;
}


/*----- SetCurrentDirectoryのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       SetCurrentDirectory関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       SetCurrentDirectory関数と同じ
*----------------------------------------------------------------------------*/
BOOL SetCurrentDirectory_My(LPCTSTR lpPathName, int normalization)
{
    BOOL ret;
    LPTSTR path;

    path = MakeLongPath(lpPathName, normalization);
    ret = SetCurrentDirectory(path);
    free(path);

    return ret;
}

/*----- FindFirstFileのMAX_PATH以上への拡張 -----------------------------------
*
*   Parameter
*       FindFirstFile関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       FindFirstFile関数と同じ
*----------------------------------------------------------------------------*/
HANDLE FindFirstFile_My(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData, int normalization)
{
    HANDLE ret;
    LPTSTR path;

    path = MakeLongPath(lpFileName, normalization);
    ret = FindFirstFile(path, lpFindFileData);
    free(path);
    if ((ret == INVALID_HANDLE_VALUE) && (normalization == YES) && (NormalizationType == NORMALIZATION_TYPE_NFC))
    {
        path = MakeLongPathNFD(lpFileName);
        ret = FindFirstFile(path, lpFindFileData);
        free(path);
    }
    return ret;
}

/*----- GetFileAttributesのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       GetFileAttributes関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       GetFileAttributes関数と同じ
*----------------------------------------------------------------------------*/
DWORD GetFileAttributes_My(LPCTSTR lpFileName, int normalization)
{
    DWORD ret;
    LPTSTR path;

    path = MakeLongPath(lpFileName, normalization);
    ret = GetFileAttributes(path);
    free(path);

    return ret;
}

/*----- GetFileAttributesのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       GetFileAttributes
*       GetLastError() の戻り値を返す
*
*   Return Value
*       GetFileAttributes関数と同じ
*----------------------------------------------------------------------------*/
DWORD GetFileAttributes_My2(LPCTSTR lpFileName, DWORD * pLastError)
{
    DWORD ret;
    LPTSTR path;

    path = MakeLongPath(lpFileName, NO);
    ret = GetFileAttributes(path);
    if( pLastError )
    {
        *pLastError = GetLastError();
    }
    free(path);
    return ret;
}

/*----- SetFileAttributesのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       SetFileAttributes関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       SetFileAttributes関数と同じ
*----------------------------------------------------------------------------*/
BOOL SetFileAttributes_My(LPCTSTR lpFileName, DWORD dwFileAttributes, int normalization)
{
    BOOL ret;
    LPTSTR path;

    path = MakeLongPath(lpFileName, normalization);
    ret = SetFileAttributes(path, dwFileAttributes);
    free(path);

    return ret;
}

/*----- MoveFileのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       MoveFile関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       MoveFile関数と同じ
*----------------------------------------------------------------------------*/
BOOL MoveFile_My(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, int normalization)
{
    BOOL ret;
    LPTSTR path1;
    LPTSTR path2;

    path1 = MakeLongPath(lpExistingFileName, normalization);
    path2 = MakeLongPath(lpNewFileName, normalization);
    ret = MoveFile(path1, path2);
    free(path1);
    free(path2);

    return ret;
}

/*----- CreateFileのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       CreateFile関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       CreateFile関数と同じ
*----------------------------------------------------------------------------*/
HANDLE CreateFile_My(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, int normalization)
{
    HANDLE ret;
    LPTSTR path;

    path = MakeLongPath(lpFileName, normalization);
    ret = CreateFile(path, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    free(path);

    return ret;
}

/*----- RemoveDirectoryのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       RemoveDirectory関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       RemoveDirectory関数と同じ
*----------------------------------------------------------------------------*/
BOOL RemoveDirectory_My(LPCTSTR lpPathName, int normalization)
{
    BOOL ret;
    LPTSTR path;

    path = MakeLongPath(lpPathName, normalization);
    ret = RemoveDirectory(path);
    free(path);

    return ret;
}

/*----- CreateDirectoryのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       CreateDirectory関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       CreateDirectory関数と同じ
*----------------------------------------------------------------------------*/
BOOL CreateDirectory_My(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, int normalization)
{
    BOOL ret;
    LPTSTR path;

    path = MakeLongPath(lpPathName, normalization);
    ret = CreateDirectory(path, lpSecurityAttributes);
    free(path);

    return ret;
}

/*----- DeleteFileのMAX_PATH以上への拡張 -----------------------------
*
*   Parameter
*       DeleteFile関数と同じ
*       normalization : 正規化フラグ (YES/NO)
*
*   Return Value
*       DeleteFile関数と同じ
*----------------------------------------------------------------------------*/
BOOL DeleteFile_My(LPCTSTR lpFileName, int normalization)
{
    BOOL ret;
    LPTSTR path;

    path = MakeLongPath(lpFileName, normalization);
    ret = DeleteFile(path);
    free(path);

    return ret;
}

/*
* CheckNormlization関数は、バックアップ先がDropboxかどうかを判断することを目的に
* 作成したもの。
* バックアップ先がDropboxのフォルダの場合、UnicodeのNFD（合成文字を使う）のファイル名で
* 書き込んでも、Dropboxの機能によりNFCに正規化される。それを検出しようと目論んだ。
* だが、以下の理由により、CheckNormlization関数はうまく動作しない。
* NFDの名前のファイルを書き込んでから、それをDropboxがNFCに変換するまで、若干の時間がかかる。
* 変換が完了するまではNFDのファイルが見つかってしまい、さらに、どれくらい待てばNFCに変換されるかが
* 不明であるため、NFCに変換される＝Dropboxかどうかの確実な判断ができない。
*/
#if 0
/*----- バックアップ先の正規化のタイプを取得 ------------------------------------
*
*   Parameter
*       LPCTSTR dest : バックアップ先のパス
*
*   Return Value
*       int 正規化のタイプ (NORMALIZATION_TYPE_xxx)
*----------------------------------------------------------------------------*/
static int CheckNormlization(LPCTSTR dest)
{
    int type = NORMALIZATION_TYPE_NONE;
    LPCTSTR nfdPath = _T(".e\x0301");
    TCHAR path[MAX_PATH];
    int error = GetTempFileName(dest, nfdPath, 0, path);

    Sleep(1000);    //NFCへの変換待ち

    if (error != 0)
    {
        WIN32_FIND_DATA buf;
        HANDLE hFind = FindFirstFile(path, &buf);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            FindClose(hFind);
            DeleteFile(path);
        }
        else
        {
            TCHAR nfcPath[MAX_PATH];
            error = NormalizeString(NormalizationC, path, -1, nfcPath, MAX_PATH);
            if (error > 0)
            {
                hFind = FindFirstFile(nfcPath, &buf);
                if (hFind != INVALID_HANDLE_VALUE)
                {
                    FindClose(hFind);
                    DeleteFile(nfcPath);
                    type = NORMALIZATION_TYPE_NFC;
                }
                else
                {
                    DeleteFile(path);
                }
            }
            else
            {
                DeleteFile(path);
            }
        }
    }
    return type;
}
#endif

/*----- NFCに正規化してファイル名を比較 ----------------------------------------
*
*   Parameter
*       LPCTSTR src : ファイル名1
*       LPCTSTR dst : ファイル名2
*
*   Return Value
*       int 比較結果 (_tcscmpの値)
*----------------------------------------------------------------------------*/
static int FnameCompare(LPCTSTR src, LPCTSTR dst)
{
    int ret;
    if (NormalizationType == NORMALIZATION_TYPE_NFC)
    {
        _TCHAR nfcSrc[MAX_PATH];
        _TCHAR nfcDst[MAX_PATH];
        NormalizeString(NormalizationC, src, -1, nfcSrc, MAX_PATH);
        NormalizeString(NormalizationC, dst, -1, nfcDst, MAX_PATH);
        ret = _tcscmp(nfcSrc, nfcDst);
    }
    else
    {
        ret = _tcscmp(src, dst);
    }
    return ret;
}

/*----- ファイルを削除先フォルダへ移動 -------------------------------------------
*
*   Parameter
*       LPCTSTR path : 削除するファイル
*       LPCTSTR moveTo : 移動先
*
*   Return Value
*       int ステータス (0=正常終了)
*----------------------------------------------------------------------------*/
static int MoveFileToDeletionFolder(LPTSTR path, LPTSTR moveTo)
{
    int sts = 0;
    _TCHAR destFolder[MY_MAX_PATH+1];
    _TCHAR destFname[MY_MAX_PATH+1];
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;
    LPCTSTR fname;
    int num;

    fname = GetFileName(path);
    _tcscpy(destFolder, moveTo);
    SetYenTail(destFolder);
    _stprintf(destFname, _T("%s%s"), destFolder, fname);
    num = 1;
    while((fHnd = FindFirstFile_My(destFname, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
    {
        FindClose(fHnd);
        _stprintf(destFname, _T("%s%s(%d)"), destFolder, fname, num);
        num++;
    }
    SetTaskMsg(TASKMSG_ERR, MSGJPN_131, destFname);
    if(MoveFile_My(path, destFname, NO) == 0)
    {
        SetTaskMsg(TASKMSG_ERR, MSGJPN_132, path, destFname);
        sts = 1;
    }
    return sts;
}


