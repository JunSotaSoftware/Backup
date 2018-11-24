/*===========================================================================
/
/									Backup
/								ログファイル
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

#define  STRICT
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>
#include <windowsx.h>
#include <commctrl.h>

#include "common.h"
#include "resource.h"


/*===== プロトタイプ =====*/

static void MakeLogDir(LPTSTR Path);
static void CheckLogFileSize(void);
static LPTSTR GetTimeString(void);
static void MakeLogFileName(LPTSTR Fname, LPTSTR Buf);
static LPTSTR GetTimeStringForFname(void);

/*===== ローカルなワーク ======*/

static LPTSTR	RealLogFname = NULL;
static FILE		*LogStrm = NULL;
static int		DeleteLog = NO;
static LPTSTR	ErrorLogFname = NULL;
static FILE		*ErrorStrm = NULL;

/*===== 外部参照 =====*/

/* 設定値 */
extern int LogSwitch;
extern int LogLimit;
extern int LogUnicode;
extern _TCHAR LogFname[MY_MAX_PATH+1];
extern _TCHAR ViewerName[MY_MAX_PATH+1];



/*-----------------------------------------------------------------------------
 説明	:	ログファイルをオープンする
 引数	:	なし
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int OpenLogfile(void)
{
	char	*oemStr;	/* not _TCHAR */
	int		length;
	int		result;

	result = SUCCESS;
	if(RealLogFname == NULL)
		RealLogFname = malloc((MY_MAX_PATH+10+1) * sizeof(_TCHAR));
	MakeLogFileName(LogFname, RealLogFname);
    MakeLogDir(RealLogFname);

	if((LogSwitch == LOG_SW_NEW) && (DeleteLog == NO))
		DeleteFile_My(RealLogFname);
	DeleteLog = YES;

	LogStrm = NULL;
	if(LogSwitch != LOG_SW_OFF)
	{
		if(LogUnicode)
		{
			LogStrm = _tfopen(RealLogFname, _T("a+t, ccs=UTF-8"));		/* unicode stream */
		}
		else
		{
			/* convert filename to OEM character set and open ANSI stream */
			length = WideCharToMultiByte(CP_OEMCP, 0, RealLogFname, -1, NULL, 0, NULL, NULL);
			oemStr = (char*)malloc(length+1);
			memset(oemStr, NUL, length+1);
			WideCharToMultiByte(CP_OEMCP, 0, RealLogFname, -1, oemStr, length, NULL, NULL);
			LogStrm = fopen(oemStr, "a+t");	/* ANSI stream */
			free(oemStr);
		}

		if(LogStrm == NULL)
		{
			free(RealLogFname);
			RealLogFname = NULL;
			if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(log_err_notify_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)_T("")) == NO)
			{
				result = FAIL;
			}
		}
	}
	return(result);
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイルを作成するフォルダを作る
 引数	:	Path : ログファイル名
 戻り値 :	なし
 備考	:	ここではエラーチェックしない。失敗してもfopenでエラーとなる。
-----------------------------------------------------------------------------*/
static void MakeLogDir(LPTSTR Path)
{
    LPTSTR Pos;
    _TCHAR Tmp[MY_MAX_PATH+1];
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;

    if((_tcslen(Path) > 3) &&
            ((_tcsncmp(Path+1, _T(":\\"), 2) == 0) || (_tcsncmp(Path, _T("\\\\"), 2) == 0)))
    {
        Pos = Path + 2;
        if(_tcsncmp(Path, _T("\\\\"), 2) == 0)
        {
            Pos = _tcschr(Path+2, _T('\\'));
            if(Pos != NULL)
            {
                Pos = _tcschr(Pos+1, _T('\\'));
            }
        }
        while((Pos != NULL) && ((Pos = _tcschr(Pos+1, _T('\\'))) != NULL))
        {
            _tcsncpy(Tmp, Path, Pos - Path);
            Tmp[Pos - Path] = NUL;
            if((fHnd = FindFirstFile(Tmp, &FindBuf)) == INVALID_HANDLE_VALUE)
            {
                CreateDirectory(Tmp, NULL);
            }
            else
            {
                FindClose(fHnd);
            }
        }
    }
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイルをクローズする
 引数	:	なし
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int CloseLogfile(void)
{
	if(LogStrm != NULL)
		fclose(LogStrm);
	LogStrm = NULL;

	return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイル名情報を削除する
 引数	:	なし
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int DeleteLogFilename(void)
{
	if(RealLogFname != NULL)
	{
		free(RealLogFname);
		RealLogFname = NULL;
	}
	return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイルにメッセージを記録
 引数	:	Msg		[in] メッセージ
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int WriteMsgToLogfile(LPTSTR Msg)
{
	char	*oemStr;	/* not _TCHAR */
	int		length;

	if(LogStrm != NULL)
	{
		CheckLogFileSize();

		if(LogUnicode)
		{
			_ftprintf(LogStrm, _T("%s\n"), Msg);
		}
		else
		{
			/* convert to OEM character set and write it */
			length = WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, NULL, 0, NULL, NULL);
			oemStr = (char*)malloc(length+1);
			memset(oemStr, NUL, length+1);
			WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, oemStr, length, NULL, NULL);
			fprintf(LogStrm, "%s\n", oemStr);	/* ANSI */
			free(oemStr);
		}
	}
	return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイルのサイズをチェック（必要なら新しいのを作成）
 引数	:	なし
 戻り値 :	なし
 備考	:	
-----------------------------------------------------------------------------*/
static void CheckLogFileSize(void)
{
	_TCHAR Tmp[MY_MAX_PATH+1+15];
	WIN32_FIND_DATA FindBuf;
	HANDLE fHnd;
	int Max;

	if((LogSwitch == LOG_SW_APPEND) && (LogLimit > 0))
	{
		if((ftell(LogStrm) / 1024) >= LogLimit)
		{
			CloseLogfile();

			Max = 0;
			_stprintf(Tmp, _T("%s.*"), RealLogFname);
			if((fHnd = FindFirstFile_My(Tmp, &FindBuf)) != INVALID_HANDLE_VALUE)
			{
				do
				{
					Max = max(Max, _tstoi(GetFileExt(FindBuf.cFileName)));
				}
				while(FindNextFile(fHnd, &FindBuf) == TRUE);
				FindClose(fHnd);
			}

			_stprintf(Tmp, _T("%s.%d"), RealLogFname, Max+1);
			MoveFile_My(RealLogFname, Tmp);

			OpenLogfile();
		}
	}
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイルにタイトル等を書き込む
 引数	:	SrcPath		[in] バックアップ元
			DstPath		[in] バックアップ先
 戻り値 :	なし
 備考	:	
-----------------------------------------------------------------------------*/
void WriteTitleToLogfile(LPTSTR SrcPath, LPTSTR DstPath)
{
	_TCHAR Tmp[256];

	WriteMsgToLogfile(_T(""));
	_stprintf(Tmp, _T("#### Backup Ver %s ####"), PROGRAM_VERSION);
	WriteMsgToLogfile(Tmp);

	_stprintf(Tmp, MSGJPN_0, GetTimeString());
	WriteMsgToLogfile(Tmp);

	while(*SrcPath != NUL)
	{
		_stprintf(Tmp, MSGJPN_1, SrcPath);
		WriteMsgToLogfile(Tmp);
		SrcPath = _tcschr(SrcPath, NUL) + 1;
	}

	_stprintf(Tmp, MSGJPN_2, DstPath);
	WriteMsgToLogfile(Tmp);
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイルに終了時刻を書き込む
 引数	:	なし
 戻り値 :	なし
 備考	:	
-----------------------------------------------------------------------------*/
void WriteEndTimeToLogfile(void)
{
	_TCHAR Tmp[256];

	_stprintf(Tmp, MSGJPN_3, GetTimeString());
	WriteMsgToLogfile(Tmp);
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	現在の時刻文字列を返す
 引数	:	なし
 戻り値 :	時刻文字列
 備考	:	
-----------------------------------------------------------------------------*/
static LPTSTR GetTimeString(void)
{
	static _TCHAR Buf[80];
	static const LPTSTR WeekStr[] = { MSGJPN_4, MSGJPN_5, MSGJPN_6, MSGJPN_7, MSGJPN_8, MSGJPN_9, MSGJPN_10 };
	SYSTEMTIME lTime;

	GetLocalTime(&lTime);
	_stprintf(Buf, MSGJPN_11, lTime.wYear, lTime.wMonth, lTime.wDay, WeekStr[lTime.wDayOfWeek], lTime.wHour, lTime.wMinute);

	return(Buf);
}


/*-----------------------------------------------------------------------------
 説明	:	ログファイル名を作成する
 引数	:	Fname	[in] ファイル名
			Buf		[out] バッファ
 戻り値 :	なし
 備考	:	
-----------------------------------------------------------------------------*/
static void MakeLogFileName(LPTSTR Fname, LPTSTR Buf)
{
	int Flag;

	Flag = 1;
	while(*Fname != NUL)
	{
		if(*Fname == _T('$'))
		{
			Fname++;
			if(*Fname == NUL)
				break;
			if(*Fname == _T('$'))
				*Buf++ = _T('$');
			else if((toupper(*Fname) == _T('D')) && (Flag))
			{
				Flag = 0;
				_tcscpy(Buf, GetTimeStringForFname());
				Buf = _tcschr(Buf, NUL);
			}
			Fname++;
		}
		else
			*Buf++ = *Fname++;
	}
	*Buf = NUL;
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	ファイル名用の日付文字列を返す
 引数	:	なし
 戻り値 :	日付文字列
 備考	:	
-----------------------------------------------------------------------------*/
static LPTSTR GetTimeStringForFname(void)
{
	static _TCHAR Buf[10];
	SYSTEMTIME lTime;

	GetLocalTime(&lTime);
	_stprintf(Buf, _T("%04d%02d%02d"), lTime.wYear, lTime.wMonth, lTime.wDay);

	return(Buf);
}


/*-----------------------------------------------------------------------------
 説明	:	ログをビューワで表示
 引数	:	なし
 戻り値 :	なし
 備考	:	
-----------------------------------------------------------------------------*/
void DispLogWithViewer(void)
{
	if(RealLogFname != NULL)
		ExecViewer(RealLogFname, ViewerName);
	else
		DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_119);
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	エラー記録専用ログファイルをオープンする
 引数	:	なし
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int OpenErrorLogfile(void)
{
	int		length;
	int		result;
	char	*oemStr;	/* not _TCHAR */

	result = FAIL;
	if(ErrorLogFname == NULL)
	{
		ErrorLogFname = malloc((MY_MAX_PATH+1) * sizeof(_TCHAR));
		GetTempPath(MY_MAX_PATH, ErrorLogFname);
		if(GetTempFileName(ErrorLogFname, _T("$$$"), 0, ErrorLogFname) == 0)
		{
			free(ErrorLogFname);
			ErrorLogFname = NULL;
		}
	}

	if(ErrorLogFname != NULL)
	{
		if(LogUnicode)
		{
			ErrorStrm = _tfopen(ErrorLogFname, _T("wt, ccs=UTF-8"));		/* unicode stream */
		}
		else
		{
			/* convert filename to OEM character set and open ANSI stream */
			length = WideCharToMultiByte(CP_OEMCP, 0, ErrorLogFname, -1, NULL, 0, NULL, NULL);
			oemStr = (char*)malloc(length+1);
			memset(oemStr, NUL, length+1);
			WideCharToMultiByte(CP_OEMCP, 0, ErrorLogFname, -1, oemStr, length, NULL, NULL);
			ErrorStrm = fopen(oemStr, "wt");	/* ANSI stream */
			free(oemStr);
		}

		if(ErrorStrm != NULL)
		{
			result = SUCCESS;
		}
		else
		{
			free(ErrorLogFname);
			ErrorLogFname = NULL;
		}
	}
	return(result);
}


/*-----------------------------------------------------------------------------
 説明	:	エラー記録専用ログファイルをクローズする
 引数	:	なし
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int CloseErrorLogfile(void)
{
	if(ErrorStrm != NULL)
	{
		fclose(ErrorStrm);
	}
	ErrorStrm = NULL;

	return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 説明	:	エラー記録専用ログファイルを削除する
 引数	:	なし
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int DeleteErrorLogfile(void)
{
	if(ErrorLogFname != NULL)
	{
		DeleteFile_My(ErrorLogFname);
		free(ErrorLogFname);
		ErrorLogFname = NULL;
	}
	return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 説明	:	エラー専用ログファイルにメッセージを記録
 引数	:	Msg		[in] メッセージ
 戻り値 :	ステータス
 備考	:	
-----------------------------------------------------------------------------*/
int WriteMsgToErrorLogfile(LPTSTR Msg)
{
	int		length;
	char	*oemStr;	/* not _TCHAR */

	if(ErrorStrm != NULL)
	{
		if(LogUnicode)
		{
			_ftprintf(ErrorStrm, _T("%s\n"), Msg);
		}
		else
		{
			/* convert to OEM character set and write it */
			length = WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, NULL, 0, NULL, NULL);
			oemStr = (char*)malloc(length+1);
			memset(oemStr, NUL, length+1);
			WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, oemStr, length, NULL, NULL);
			fprintf(ErrorStrm, "%s\n", oemStr);	/* ANSI */
			free(oemStr);
		}
	}
	return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 説明	:	エラー専用ログをビューワで表示
 引数	:	なし
 戻り値 :	なし
 備考	:	
-----------------------------------------------------------------------------*/
void DispErrorLogWithViewer(void)
{
	if(ErrorLogFname != NULL)
		ExecViewer(ErrorLogFname, ViewerName);
	else
		DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_119);
	return;
}


