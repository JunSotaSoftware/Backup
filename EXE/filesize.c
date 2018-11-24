/*===========================================================================
/
/									Backup
/								ファイル容量検索
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

//#define DEBUG_LOG

#define  STRICT
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windowsx.h>
#include <commctrl.h>
#include <htmlhelp.h>

#include "common.h"
#include "resource.h"


/*===== 構造体 =====*/

typedef struct {
	LPTSTR	ScnName;
	LPTSTR	IgnFiles;
	LPTSTR	IgnDirs;
	int		IgnSys;
	int		IgnHid;
	int		BigSize;
	int		Files;
	int		Folders;
	double	Size;
} SIZE_COUNT_INFO;

/*===== プロトタイプ =====*/

static LRESULT CALLBACK SizeDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void CheckSizeGo(HWND hDlg, LPTSTR Path, SIZE_COUNT_INFO *Info);

/*==== ローカルなワーク =====*/

static int DlgSts;


/*-----------------------------------------------------------------------------
 説明	:	ファイル容量検索のダイアログを表示
 引数	:	hWnd	親ウインドウのウインドウハンドル
			Pat		バックアップパターン
 戻り値	:	なし
 備考	:	
-----------------------------------------------------------------------------*/
void FilesSizeDialog(HWND hWnd, COPYPATLIST *Pat)
{
	HWND		hDlg;
	_TCHAR		*Path;
	COPYPATLIST	*TmpPat;
	_TCHAR		Src[MY_MAX_PATH2+1];
	_TCHAR		ScnName[MY_MAX_PATH2+1];
	SIZE_COUNT_INFO	Info;

	DlgSts = 0;

	Info.Folders = 0;
	Info.Files = 0;
	Info.Size = 0;

#ifdef DEBUG_LOG
	DoPrintf(_T("\n===============\n"));
#endif

	hDlg = CreateDialog(GetBupInst(), MAKEINTRESOURCE(filesize_dlg), hWnd, SizeDlgWndProc);
	ShowWindow(hDlg, SW_SHOW);

	/* 検索するパス名を表示 */
	TmpPat = Pat;
	while(TmpPat != NULL)
	{
		Path = TmpPat->Set.Src;
		while(*Path != NUL)
		{
			SendDlgItemMessage(hDlg, FSIZE_SRCLIST, LB_ADDSTRING, 0, (LPARAM)Path);
			Path += _tcslen(Path) + 1;
		}
		TmpPat = TmpPat->Next;
	}

	SendDlgItemMessage(hDlg, FSIZE_MESSAGE, WM_SETTEXT, 0, (LPARAM)MSGJPN_107);

	/* 検索 */
	while((Pat != NULL) && (DlgSts == 0))
	{
		Path = Pat->Set.Src;
		while((*Path != NUL) && (DlgSts == 0))
		{
			_tcscpy(Src, Path);
			MakePathandFile(Src, ScnName, YES);

			Info.ScnName = ScnName;
			Info.IgnFiles = Pat->Set.IgnFile;
			Info.IgnDirs = Pat->Set.IgnDir;
			Info.IgnSys = Pat->Set.IgnSystemFile;
			Info.IgnHid = Pat->Set.IgnHiddenFile;
			Info.BigSize = -1;
			if(Pat->Set.IgnBigFile && (Pat->Set.IgnBigSize > 0))
			{
				Info.BigSize = Pat->Set.IgnBigSize;
			}

			CheckSizeGo(hDlg, Src, &Info);
			Path += _tcslen(Path) + 1;
		}
		Pat = Pat->Next;
	}

	SendDlgItemMessage(hDlg, FSIZE_MESSAGE, WM_SETTEXT, 0, (LPARAM)MSGJPN_108);

	EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
	EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
	MessageBeep((UINT)-1);

	while(DlgSts != 2)
	{
		Sleep(50);
		BackgrndMessageProc();
	}
	DestroyWindow(hDlg);
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	ファイル容量検索ダイアログのコールバック
 引数	:	hDlg	ウインドウハンドル
			message	メッセージ番号
			wParam	メッセージの WPARAM 引数
			lParam	メッセージの LPARAM 引数
 戻り値	:	メッセージに対応する戻り値
 備考	:	
-----------------------------------------------------------------------------*/
static LRESULT CALLBACK SizeDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG :
			return(TRUE);

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK :
					DlgSts = 2;
					break;

				case IDCANCEL :
					DlgSts = 1;
					break;

				case IDHELP :
					HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000007);
					break;
			}
			return(TRUE);
	}
    return(FALSE);
}


/*-----------------------------------------------------------------------------
 説明	:	ファイル容量検索
 引数	:	hDlg		ダイアログボックスのウインドウハンドル
			Path		検索パス名
			Info		検索情報
 戻り値	:	なし
 備考	:	検索結果をダイアログボックスに表示する
-----------------------------------------------------------------------------*/
static void CheckSizeGo(HWND hDlg, LPTSTR Path, SIZE_COUNT_INFO *Info)
{
	HANDLE				fHnd;
	WIN32_FIND_DATA		FindBuf;
	_TCHAR				Src[MY_MAX_PATH2+1];
	_TCHAR				*Pos;
	_TCHAR				Str[40];
	unsigned __int64	tmp64;
	unsigned __int64	big;
	DWORD				Type;

#ifdef DEBUG_LOG
	DoPrintf(_T(">>'%s' : '%s' is "), Path, Info->ScnName);
#endif

	big = (unsigned __int64)Info->BigSize * 1024 * 1024;
	Type = FILE_ATTRIBUTE_DIRECTORY;
	_tcscpy(Src, Path);
	if(_tcscmp(Src+1, _T(":\\")) != 0)
	{
#ifdef DEBUG_LOG
		DoPrintf(_T(" - A"));
#endif
		/* フォルダ／ファイルがあるかチェック */
		RemoveYenTail(Src);
		if((_tcschr(Src, '*') != NULL) ||(_tcschr(Src, '?') != NULL))
		{
			Type = 0;
		}
		else
		{
			 Type = GetFileAttributes_My(Src);
		}
	}
	else
	{
		if(GetDriveType(Src) != DRIVE_NO_ROOT_DIR)
		{
#ifdef DEBUG_LOG
			DoPrintf(_T(" - B"));
#endif
			Type = GetFileAttributes_My(Src);
		}
		else
		{
#ifdef DEBUG_LOG
			DoPrintf(_T(" - C"));
#endif
		}
	}

	if((Type & FILE_ATTRIBUTE_DIRECTORY) != 0)
	{
#ifdef DEBUG_LOG
		DoPrintf(_T(" - folder\n"));
#endif
		/*===== フォルダ =====*/
		SetYenTail(Src);
		Pos = _tcschr(Src, NUL);
		_tcscpy(Pos, _T("*"));
		if((fHnd = FindFirstFile_My(Src, &FindBuf)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if(DlgSts != 0)
					break;

				BackgrndMessageProc();

				if(((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && (Info->IgnSys == YES)) ||
				   ((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && (Info->IgnHid == YES)))
				{
					/* カウントしない */
				}
				else
				{
					_tcscpy(Pos, FindBuf.cFileName);
					if(FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						if((_tcscmp(FindBuf.cFileName, _T(".")) != 0) &&
						   (_tcscmp(FindBuf.cFileName, _T("..")) != 0) &&
						   (CheckFnameWithArray(Src, Info->IgnDirs) == NO))
						{
							CheckSizeGo(hDlg, Src, Info);
							Info->Folders += 1;
						}
					}
					else
					{
						if((CheckFnameWithArray(Src, Info->IgnFiles) == NO) &&
						   ((_tcslen(Info->ScnName) == 0) || (CheckFnameWithArray(Src, Info->ScnName) == YES)))
						{
							tmp64 = (unsigned __int64)FindBuf.nFileSizeLow +
								((unsigned __int64)FindBuf.nFileSizeHigh << 32);
							if((Info->BigSize == -1) || (tmp64 < big))
							{
#ifdef DEBUG_LOG
								DoPrintf(_T("  --> File '%s'\n"), FindBuf.cFileName);
								DoPrintf(_T("       --> Count\n"));
#endif
								Info->Size += (double)tmp64;
								Info->Files += 1;
							}
						}
					}
				}
			}
			while(FindNextFile(fHnd, &FindBuf) == TRUE);
			FindClose(fHnd);
		}
	}
	else
	{
#ifdef DEBUG_LOG
		DoPrintf(_T(" - file\n"));
#endif
		/*===== ファイル =====*/
		if((fHnd = FindFirstFile_My(Src, &FindBuf)) != INVALID_HANDLE_VALUE)
		{
			do
			{
				if(DlgSts != 0)
					break;

				BackgrndMessageProc();

				if((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ||
				   ((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) && (Info->IgnSys == YES)) ||
				   ((FindBuf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) && (Info->IgnHid == YES)))
				{
					/* カウントしない */
				}
				else
				{
					tmp64 = (unsigned __int64)FindBuf.nFileSizeLow +
						((unsigned __int64)FindBuf.nFileSizeHigh << 32);
					if((Info->BigSize == -1) || (tmp64 < big))
					{
#ifdef DEBUG_LOG
						DoPrintf(_T("  --> File '%s'\n"), FindBuf.cFileName);
						DoPrintf(_T("       --> Count\n"));
#endif
						Info->Size += (double)tmp64;
						Info->Files += 1;
					}
				}
			}
			while(FindNextFile(fHnd, &FindBuf) == TRUE);
			FindClose(fHnd);
		}
	}

	_stprintf(Str, _T("%d"), Info->Folders);
	SendDlgItemMessage(hDlg, FSIZE_FOLDERS, WM_SETTEXT, 0, (LPARAM)Str);
	_stprintf(Str, _T("%d"), Info->Files);
	SendDlgItemMessage(hDlg, FSIZE_FILES, WM_SETTEXT, 0, (LPARAM)Str);
#ifdef DEBUG_LOG
	_stprintf(Str, _T("%.0f"), Info->Size);
#else
	MakeSizeString(Info->Size, Str);
#endif
	SendDlgItemMessage(hDlg, FSIZE_SIZE, WM_SETTEXT, 0, (LPARAM)Str);

	return;
}


