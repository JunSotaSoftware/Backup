/*===========================================================================
/
/									Backup
/								オプション設定
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
#include <htmlhelp.h>

#include "common.h"
#include "resource.h"


/*===== プロトタイプ =====*/

static LRESULT CALLBACK LogSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MiscSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*===== ローカルなワーク ======*/

static int Apply;

/*===== 外部参照 =====*/

/* 設定値 */
extern int LogSwitch;
extern int LogLimit;
extern int LogUnicode;
extern _TCHAR LogFname[MY_MAX_PATH+1];
extern _TCHAR ViewerName[MY_MAX_PATH+1];
//extern int IntervalTime;
extern int SaveWinPos;
extern int TrayIcon;
extern int RegType;
extern int ExitOnEsc;
extern int ShowComment;		/* 0=表示しない,1=ツールチップで表示、2=ウインドウで表示 */



/*----- オプション設定 --------------------------------------------------------
*
*	Parameter
*		HWND hWnd : 親ウインドウのウインドウハンドル
*
*	Return Value
*		int ステータス
*			YES/NO=取り消し
*----------------------------------------------------------------------------*/

int SetOption(HWND hWnd)
{
	PROPSHEETPAGE psp[2];
	PROPSHEETHEADER psh;

	psp[0].dwSize = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[0].hInstance = GetBupInst();
	psp[0].pszTemplate = MAKEINTRESOURCE(opt_log_dlg);
	psp[0].pszIcon = NULL;
	psp[0].pfnDlgProc = LogSettingProc;
	psp[0].pszTitle = MSGJPN_21;
	psp[0].lParam = 0;
	psp[0].pfnCallback = NULL;

	psp[1].dwSize = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags = PSP_USETITLE | PSP_HASHELP;
	psp[1].hInstance = GetBupInst();
	psp[1].pszTemplate = MAKEINTRESOURCE(opt_misc_dlg);
	psp[1].pszIcon = NULL;
	psp[1].pfnDlgProc = MiscSettingProc;
	psp[1].pszTitle = MSGJPN_23;
	psp[1].lParam = 0;
	psp[1].pfnCallback = NULL;

	psh.dwSize = sizeof(PROPSHEETHEADER);
	psh.dwFlags = PSH_HASHELP | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
	psh.hwndParent = hWnd;
	psh.hInstance = GetBupInst();
	psh.pszIcon = NULL;
	psh.pszCaption = MSGJPN_24;
	psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.nStartPage = 0;
	psh.ppsp = (LPCPROPSHEETPAGE)&psp;
	psh.pfnCallback = NULL;

	Apply = NO;
	PropertySheet(&psh);

	return(Apply);
}


/*----- ログ設定ウインドウのメッセージ処理 ------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		UINT message  : メッセージ番号
*		WPARAM wParam : メッセージの WPARAM 引数
*		LPARAM lParam : メッセージの LPARAM 引数
*
*	Return Value
*		メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK LogSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmhdr;
	_TCHAR Tmp[MY_MAX_PATH+1];
	_TCHAR Tmp2[MY_MAX_PATH+1];

	static const RADIOBUTTON LogButton[] = {
		{ LOG_APPEND, LOG_SW_APPEND },
		{ LOG_NEW,    LOG_SW_NEW }
	};
	#define LOGBUTTONS	(sizeof(LogButton)/sizeof(RADIOBUTTON))

	static const RADIOBUTTON LimitButton[] = {
		{ LOG_NOLIMIT, -1 },
		{ LOG_LIMIT,   1 }
	};
	#define LIMITBUTTONS	(sizeof(LimitButton)/sizeof(RADIOBUTTON))

	switch (message)
	{
		case WM_INITDIALOG :
			SendDlgItemMessage(hDlg, LOG_SWITCH, BM_SETCHECK, LogSwitch == 0 ? 0 : 1, 0);
			SetRadioButtonByValue(hDlg, LogSwitch, LogButton, LOGBUTTONS);

			SendDlgItemMessage(hDlg, LOG_UNICODE, BM_SETCHECK, LogUnicode == 0 ? 0 : 1, 0);

			SetRadioButtonByValue(hDlg, (LogLimit > 0 ? 1 : -1), LimitButton, LIMITBUTTONS);
			SendDlgItemMessage(hDlg, LOG_LIMIT_SIZE, EM_LIMITTEXT, (WPARAM)5, 0);
			_stprintf(Tmp, _T("%d"), abs(LogLimit));
			SendDlgItemMessage(hDlg, LOG_LIMIT_SIZE, WM_SETTEXT, 0, (LPARAM)Tmp);

			SendDlgItemMessage(hDlg, LOG_FNAME, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
			SendDlgItemMessage(hDlg, LOG_FNAME, WM_SETTEXT, 0, (LPARAM)LogFname);

			SendDlgItemMessage(hDlg, LOG_VIEWER, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
			SendDlgItemMessage(hDlg, LOG_VIEWER, WM_SETTEXT, 0, (LPARAM)ViewerName);

			PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(LOG_SWITCH, 0), 0);
			return(TRUE);

		case WM_NOTIFY:
			pnmhdr = (NMHDR FAR *)lParam;
			switch(pnmhdr->code)
			{
				case PSN_APPLY :
					LogSwitch = SendDlgItemMessage(hDlg, LOG_SWITCH, BM_GETCHECK, 0, 0);
					LogSwitch *= AskRadioButtonValue(hDlg, LogButton, LOGBUTTONS);
					LogUnicode = SendDlgItemMessage(hDlg, LOG_UNICODE, BM_GETCHECK, 0, 0);

					SendDlgItemMessage(hDlg, LOG_LIMIT_SIZE, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Tmp);
					LogLimit = _tstoi(Tmp);
					CheckRange2(&LogLimit, 99999, 1);
					LogLimit *= AskRadioButtonValue(hDlg, LimitButton, LIMITBUTTONS);

					SendDlgItemMessage(hDlg, LOG_FNAME, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)LogFname);
					SendDlgItemMessage(hDlg, LOG_VIEWER, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)ViewerName);
					Apply = YES;
					break;

				case PSN_RESET :
					break;

				case PSN_HELP :
					HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000027);
					break;
			}
			break;

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case LOG_SWITCH :
					if(SendDlgItemMessage(hDlg, LOG_SWITCH, BM_GETCHECK, 0, 0) == 1)
					{
						EnableWindow(GetDlgItem(hDlg, LOG_FNAME), TRUE);
						EnableWindow(GetDlgItem(hDlg, LOG_FNAME_BR), TRUE);
						EnableWindow(GetDlgItem(hDlg, LOG_APPEND), TRUE);
						EnableWindow(GetDlgItem(hDlg, LOG_NEW), TRUE);
//						EnableWindow(GetDlgItem(hDlg, LOG_UNICODE), TRUE);
						PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(LOG_APPEND, 0), 0);
					}
					else
					{
						EnableWindow(GetDlgItem(hDlg, LOG_FNAME), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_FNAME_BR), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_APPEND), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_NEW), FALSE);
//						EnableWindow(GetDlgItem(hDlg, LOG_UNICODE), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_NOLIMIT), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT_SIZE), FALSE);
					}
					break;

				case LOG_APPEND :
				case LOG_NEW :
					if(SendDlgItemMessage(hDlg, LOG_APPEND, BM_GETCHECK, 0, 0) == 1)
					{
						EnableWindow(GetDlgItem(hDlg, LOG_NOLIMIT), TRUE);
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT), TRUE);
						PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(LOG_NOLIMIT, 0), 0);
					}
					else
					{
						EnableWindow(GetDlgItem(hDlg, LOG_NOLIMIT), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT), FALSE);
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT_SIZE), FALSE);
					}
					break;

				case LOG_NOLIMIT :
				case LOG_LIMIT :
					if(SendDlgItemMessage(hDlg, LOG_NOLIMIT, BM_GETCHECK, 0, 0) == 0)
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT_SIZE), TRUE);
					else
						EnableWindow(GetDlgItem(hDlg, LOG_LIMIT_SIZE), FALSE);
					break;

				case LOG_FNAME_BR :
					_tcscpy(Tmp, _T(""));
					SendDlgItemMessage(hDlg, LOG_FNAME, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Tmp2);
					*GetFileName(Tmp2) = NUL;
					if(SelectFile(hDlg, Tmp, MSGJPN_25, MSGJPN_26, NULL, 0, 1, Tmp2) == TRUE)
						SendDlgItemMessage(hDlg, LOG_FNAME, WM_SETTEXT, 0, (LPARAM)Tmp);
					break;

                case LOG_FNAME_INIT:
                    MakeInitialLogFilename(Tmp);
					SendDlgItemMessage(hDlg, LOG_FNAME, WM_SETTEXT, 0, (LPARAM)Tmp);
                    break;

				case LOG_VIEWER_BR :
					_tcscpy(Tmp, _T(""));
					SendDlgItemMessage(hDlg, LOG_VIEWER, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Tmp2);
					*GetFileName(Tmp2) = NUL;
					if(SelectFile(hDlg, Tmp, MSGJPN_88, MSGJPN_89, NULL, 0, 1, Tmp2) == TRUE)
						SendDlgItemMessage(hDlg, LOG_VIEWER, WM_SETTEXT, 0, (LPARAM)Tmp);
					break;
			}
			return(TRUE);
	}
    return(FALSE);
}


/*----- その他設定ウインドウのメッセージ処理 ----------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		UINT message  : メッセージ番号
*		WPARAM wParam : メッセージの WPARAM 引数
*		LPARAM lParam : メッセージの LPARAM 引数
*
*	Return Value
*		メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK MiscSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	NMHDR *pnmhdr;

	static const RADIOBUTTON ShowCommentButton[] = {
		{ MISC_COMMENT_NO, 0 },
		{ MISC_COMMENT_TIP, 1 },
		{ MISC_COMMENT_WIN, 2 }
	};
	#define SHOWCOMMENTBUTTONS	(sizeof(ShowCommentButton)/sizeof(RADIOBUTTON))

	switch (message)
	{
		case WM_INITDIALOG :
			SendDlgItemMessage(hDlg, MISC_SAVEPOS, BM_SETCHECK, SaveWinPos, 0);
			SendDlgItemMessage(hDlg, MISC_TRAYICON, BM_SETCHECK, TrayIcon, 0);
			SendDlgItemMessage(hDlg, MISC_REGTYPE, BM_SETCHECK, RegType, 0);
			SendDlgItemMessage(hDlg, MISC_ESC_EXIT, BM_SETCHECK, ExitOnEsc, 0);
			SetRadioButtonByValue(hDlg, ShowComment, ShowCommentButton, SHOWCOMMENTBUTTONS);
			return(TRUE);

		case WM_NOTIFY:
			pnmhdr = (NMHDR FAR *)lParam;
			switch(pnmhdr->code)
			{
				case PSN_APPLY :
					SaveWinPos = SendDlgItemMessage(hDlg, MISC_SAVEPOS, BM_GETCHECK, 0, 0);
					TrayIcon = SendDlgItemMessage(hDlg, MISC_TRAYICON, BM_GETCHECK, 0, 0);
					RegType = SendDlgItemMessage(hDlg, MISC_REGTYPE, BM_GETCHECK, 0, 0);
					ExitOnEsc = SendDlgItemMessage(hDlg, MISC_ESC_EXIT, BM_GETCHECK, 0, 0);
					ShowComment = AskRadioButtonValue(hDlg, ShowCommentButton, SHOWCOMMENTBUTTONS);
					Apply = YES;
					break;

				case PSN_RESET :
					break;

				case PSN_HELP :
					HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000028);
					break;
			}
			break;
	}
    return(FALSE);
}


