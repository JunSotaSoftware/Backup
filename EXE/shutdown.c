﻿/*===========================================================================
/
/									Backup
/							Windowsのシャットダウン
/
/============================================================================
/ Copyright (C) 1997-2015 Sota. All rights reserved.
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
#include <windowsx.h>
#include <commctrl.h>
#include <VersionHelpers.h>

#include "common.h"
#include "resource.h"


/*===== プロトタイプ =====*/
BOOL CALLBACK CountDownDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


/*-----------------------------------------------------------------------------
 名前	:	ChangeSystemPowerMode
 説明	:	Windowsのパワーステートを変更する
 引数	:	State	ステート
 戻り値 :	BOOL	ステータス (TRUE=成功)
-----------------------------------------------------------------------------*/
BOOL ChangeSystemPowerMode(AUTOCLOSE_ACTION State)
{
	HANDLE				hToken;
	TOKEN_PRIVILEGES	TokenPri;
	BOOL				Sts;
	DWORD				Err;
	LPTSTR 				lpBuffer;

	Sts = FALSE;
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == TRUE)
	{
		if(LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &TokenPri.Privileges[0].Luid) == TRUE)
		{
			TokenPri.PrivilegeCount = 1;
			TokenPri.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			AdjustTokenPrivileges(hToken, FALSE, &TokenPri, 0, NULL, 0);
			/* AdjustTokenPrivileges は常に TRUE を返す? */
			Err = GetLastError();
			if(Err == ERROR_SUCCESS)
			{
				Sts = TRUE;
			}
		}
		else
		{
			Err = GetLastError();
		}
	}
	else
	{
		Err = GetLastError();
	}

	if(Sts == TRUE)
	{
		switch(State)
		{
			case AUTOCLOSE_ACTION_SHUTDOWN_WINDOWS: /* 2 */
				Sts = ExitWindowsEx(EWX_POWEROFF, 0);
				break;

			case AUTOCLOSE_ACTION_STANBY: /* 3 */
			case AUTOCLOSE_ACTION_EXIT_AND_STANBY: /* 5 */
				if(SetSystemPowerState(TRUE, FALSE) == 0)
				{
					Sts = FALSE;
				}
				break;

			case AUTOCLOSE_ACTION_HIBERNATE: /* 4 */
			case AUTOCLOSE_ACTION_EXIT_AND_HIBERNATE: /* 6 */
				if(SetSystemPowerState(FALSE, FALSE) == 0)
				{
					Sts = FALSE;
				}
				break;

			default :
				break;
		}
		if(Sts == FALSE)
		{
			Err = GetLastError();
		}
	}

	if(Sts == FALSE)
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
		DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(shutdown_err_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)lpBuffer);
		LocalFree(lpBuffer);
	}
	return(Sts);
}


/*-----------------------------------------------------------------------------
 名前	:	DoCountDown
 説明	:	シャットダウン前のカウントダウンウインドウ
 引数	:	State	ステート
 戻り値 :	int	ステータス (YES=シャットダウンする)
-----------------------------------------------------------------------------*/
int DoCountDown(AUTOCLOSE_ACTION State)
{
	return(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(countdown_dlg), GetDesktopWindow(), CountDownDialogProc, State));
}


/*-----------------------------------------------------------------------------
 名前	:	DoCountDown
 説明	:	カウントダウンウインドウのコールバック
 引数	:	hDlg	ウインドウハンドル
			message	メッセージ番号
			wParam	メッセージの WPARAM 引数
			lParam	メッセージの LPARAM 引数
 戻り値 :	BOOL	TRUE/FALSE
-----------------------------------------------------------------------------*/
BOOL CALLBACK CountDownDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int		Count;
	static UINT		TimerID;
	_TCHAR			Tmp[20];
    AUTOCLOSE_ACTION action;

	switch (message)
	{
		case WM_INITDIALOG :
			Count = SHUTDOWN_PERIOD;
            action = (AUTOCLOSE_ACTION)lParam;
            switch (action)
            {
            case AUTOCLOSE_ACTION_SHUTDOWN_WINDOWS:
                SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_98);
                break;
            case AUTOCLOSE_ACTION_STANBY:
            case AUTOCLOSE_ACTION_EXIT_AND_STANBY:
                if (IsWindowsVistaOrGreater())
                    SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_122);
                else
                    SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_99);
                break;
            case AUTOCLOSE_ACTION_HIBERNATE:
            case AUTOCLOSE_ACTION_EXIT_AND_HIBERNATE:
                SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_100);
                break;
            }
			_stprintf(Tmp, MSGJPN_92, Count);
			SendDlgItemMessage(hDlg, CDOWN_MSG, WM_SETTEXT, 0, (LPARAM)Tmp);
			TimerID = SetTimer(hDlg, TIMER_COUNTDOWN, 1000, NULL);
			return(TRUE);

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK :
					KillTimer(hDlg, TimerID);
					EndDialog(hDlg, NO);
					break;
			}
			return(TRUE);

		case WM_TIMER :
			Count--;
			_stprintf(Tmp, MSGJPN_92, Count);
			SendDlgItemMessage(hDlg, CDOWN_MSG, WM_SETTEXT, 0, (LPARAM)Tmp);
			if(Count == 0)
			{
				KillTimer(hDlg, TimerID);
				EndDialog(hDlg, YES);
			}
			break;
	}
    return(FALSE);
}


