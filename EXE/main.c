/*===========================================================================
/
/									Backup
/								メインプログラム
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
#include <shlobj.h>

#include "common.h"
#include "resource.h"


/*===== プロトタイプ =====*/

static int InitApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int cmdShow);
static int CommandLineProc(LPTSTR Src, LPTSTR Dst, int Option);
static void DeleteAllObject(void);
static LRESULT CALLBACK BupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void ExitProc(HWND hWnd);
static int AnalyzeComLine(LPTSTR Str, LPTSTR Src, LPTSTR Dst, int *Opt);
static LPTSTR GetToken(LPTSTR Str, LPTSTR Buf);
static void TrayIconMenu(void);
void LoadTrayIcon(void);

/*===== ローカルなワーク ======*/

static const _TCHAR BupClassStr[] = _T("BackUp");

static HINSTANCE hInstBup;
static HWND hWndBup = NULL;

int ExecOption = 0;
int AutoClose = 0;
static int NoNotify = NO;
int Sound;
_TCHAR SoundFile[MY_MAX_PATH+1];
_TCHAR MediaPath[MY_MAX_PATH+1];
int IntervalTime = -60;

static int SaveExit = YES;

static _TCHAR HelpPath[MY_MAX_PATH+1];
static _TCHAR IniPath[MY_MAX_PATH+1];
static int TmpTrayIcon;

static const int IconData[] = {
	/* 静止時用 */
	backup, 
	/* 動画用 */
	backup_tiny_2, backup_tiny_3, backup_tiny_4, backup_tiny_5, 
	backup_tiny_6, backup_tiny_7, backup_tiny_8, backup_tiny_9, 
	backup_tiny_a, backup_tiny_b
};

static HANDLE IconHandle[11];
static DWORD dwCookie;

/*===== グローバルなワーク ======*/

/* 設定 */
int LogSwitch = LOG_SW_APPEND;
int LogLimit = 1000;					/* 最大サイズ：マイナス値なら制限なし */
int LogUnicode = YES;
_TCHAR LogFname[MY_MAX_PATH+1] = { _T("") };
_TCHAR ViewerName[MY_MAX_PATH+1] = { _T("Notepad") };
int SaveWinPos = NO;
int MainPosX = CW_USEDEFAULT;
int MainPosY = 0;
int TrayIcon = YES;
int RegType = REGTYPE_REG;
SIZE MainDlgSize = { -1, -1 };
SIZE TransDlgSize = { -1, -1 };
SIZE NotifyDlgSize = {-1, -1 };
int ExitOnEsc = 0;
int ShowComment = 1;		/* 0=表示しない,1=ツールチップで表示、2=ウインドウで表示 */



/*----- メインルーチン --------------------------------------------------------
*
*	Parameter
*		HINSTANCE hInstance : このアプリケーションのこのインスタンスのハンドル
*		HINSTANCE hPrevInstance : このアプリケーションの直前のインスタンスのハンドル
*		LPSTR lpszCmdLine : アプリケーションが起動したときのコマンドラインをさすロングポインタ
*		int cmdShow : 最初に表示するウインドウの形式。
*
*	Return Value
*		int 最後のメッセージのwParam
*----------------------------------------------------------------------------*/

int PASCAL _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
    MSG Msg;
	int Ret;
	LPTSTR	cmdLine;

	Ret = FALSE;
	hInstBup = hInstance;
	cmdLine = GetCommandLine();
	if(InitApp(hInstance, hPrevInstance, cmdLine, cmdShow) == SUCCESS)
	{
		while(GetMessage(&Msg, NULL, 0, 0))
		{
			if((IsDialogMessage(GetMainDlgHwnd(), &Msg) == FALSE) &&
			   (IsDialogMessage(GetTransDlgHwnd(), &Msg) == FALSE))
			{
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		}
		Ret = Msg.wParam;
	}
    UnregisterClass(BupClassStr, hInstBup);
	return(Ret);
}


/*----- アプリケーションの初期化 ----------------------------------------------
*
*	Parameter
*		HINSTANCE hInstance : このアプリケーションのこのインスタンスのハンドル
*		HINSTANCE hPrevInstance : このアプリケーションの直前のインスタンスのハンドル
*		LPTSTR lpszCmdLine : アプリケーションが起動したときのコマンドラインをさすロングポインタ
*		int cmdShow : 最初に表示するウインドウの形式。
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int InitApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpszCmdLine, int cmdShow)
{
	int Sts;
	WNDCLASSEX wClass;
	RECT Rect;
	static _TCHAR Src[MY_MAX_PATH+1];
	_TCHAR Dst[MY_MAX_PATH+1];
	int CmdLineSts;

	Sts = FAIL;

	HtmlHelp(NULL, NULL, HH_INITIALIZE, (DWORD)&dwCookie);

	SaveUpdateBellInfo();

	GetModuleFileName(NULL, HelpPath, MY_MAX_PATH);
	_tcscpy(GetFileName(HelpPath), _T("Backup.chm"));

    MakeInitialLogFilename(LogFname);

	GetModuleFileName(NULL, IniPath, MY_MAX_PATH);
	_tcscpy(GetFileName(IniPath), _T("Backup.ini"));

	CmdLineSts = AnalyzeComLine(lpszCmdLine, Src, Dst, &ExecOption);
	if(ExecOption & OPT_INI_FILE)
		_tcscpy(IniPath, Dst);

	GetMediaPath(MediaPath, MY_MAX_PATH);

	LoadRegistory();
	if(SaveWinPos == NO)
	{
		MainPosX = CW_USEDEFAULT;
		MainPosY = 0;
	}

#if 0
	AllocConsole();
#endif

	/* メインウインドウを作成 */
	wClass.cbSize        = sizeof(WNDCLASSEX);
	wClass.style         = 0;
	wClass.lpfnWndProc   = BupWndProc;
	wClass.cbClsExtra    = 0;
	wClass.cbWndExtra    = 0;
	wClass.hInstance     = hInstance;
	wClass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(backup));
	wClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wClass.lpszMenuName  = (LPTSTR)MAKEINTRESOURCE(main_menu);
	wClass.lpszClassName = BupClassStr;
	wClass.hIconSm       = NULL;
	RegisterClassEx(&wClass);

	hWndBup = CreateWindow(BupClassStr, _T("Backup"),
				WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_THICKFRAME,
				MainPosX, MainPosY, 0, 0,
				HWND_DESKTOP, 0, hInstance, NULL);
	if(hWndBup != NULL)
	{
		if((MakeTransferDialog() == SUCCESS) && (MakeMainDialog() == SUCCESS))
		{
			ShowWindow(hWndBup, SW_HIDE);
			GetWindowRect(GetMainDlgHwnd(), &Rect);
			AdjustWindowRect(&Rect, WS_THICKFRAME | WS_CAPTION | WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, TRUE);
			SetWindowPos(hWndBup, 0, 0, 0, Rect.right - Rect.left, Rect.bottom - Rect.top, SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER | SWP_HIDEWINDOW);

			TmpTrayIcon = TrayIcon;
			LoadTrayIcon();
			SetTrayIcon(TICON_NEW, 0, NULL);

			if(cmdShow == SW_SHOWMAXIMIZED)
				cmdShow = SW_SHOWNORMAL;

			if(ExecOption & OPT_MINIMIZED)
				cmdShow = SW_SHOWMINIMIZED;

			if((TmpTrayIcon == YES) &&
			   ((cmdShow == SW_MINIMIZE) ||
			    (cmdShow == SW_SHOWMINIMIZED) ||
			    (cmdShow == SW_SHOWMINNOACTIVE)))
			{
				ShowWindow(hWndBup, SW_HIDE);
			}
			else
				ShowWindow(hWndBup, cmdShow);

			MakeBackupThread();

			if(CmdLineSts == SUCCESS)
				Sts = CommandLineProc(Src, Dst, ExecOption);
		}
	}

	if(Sts == FAIL)
		DeleteAllObject();

	return(Sts);
}


/*----- コマンドラインの指定に従って処理 --------------------------------------
*
*	Parameter
*		Src		バックアップ元
*		Dst		バックアップ先
*		Option	オプション
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int CommandLineProc(LPTSTR Src, LPTSTR Dst, int Option)
{
	static COPYPATLIST cInfo;
	int Sts;

	Sts = FAIL;

	if(Option & OPT_START)
		NoNotify = YES;

	Sts = SUCCESS;
	if(Option & OPT_ALL)
	{
		PostMessage(GetMainDlgHwnd(), WM_COMMAND, MAKEWPARAM(MAIN_ALLSTART, 0), 0);
	}
	else if(_tcslen(Src) != 0)
	{
		if(Option & OPT_NAME)
		{
			PostMessage(GetMainDlgHwnd(), WM_COMMAND, MAKEWPARAM(MENU_START_NAME, 0), (LPARAM)Src);
		}
		else
		{
			if(_tcslen(Dst) != 0)
			{
				CopyDefaultPat(&cInfo.Set);
                memset(cInfo.Set.Src, 0, SRC_PATH_LEN+1);
				_tcscpy(cInfo.Set.Src, Src);
                memset(cInfo.Set.Dst, 0, DST_PATH_LEN+1);
				_tcscpy(cInfo.Set.Dst, Dst);
				cInfo.Set.ForceCopy = (Option & OPT_FORCE) ? 1 : 0;
				cInfo.Set.DelDir = (Option & OPT_RMDIR) ? 1 : 0;
				cInfo.Set.DelFile = (Option & OPT_RMFILE) ? 1 : 0;
				cInfo.Set.IgnoreErr = (Option & OPT_NOERROR) ? 1 : 0;
                cInfo.Set.NotifyDel = (Option & OPT_NO_NOTIFY_DEL) ? 0 : 1;
                if(Option & OPT_CLOSE)
					cInfo.Set.AutoClose = 1;
				cInfo.Next = NULL;
				if(NotifyBackup(GetMainDlgHwnd(), &cInfo) == YES)
				{
					StartBackup(&cInfo);
				}
				else
					Sts = FAIL;
			}
			else
			{
				DispErrorBox(MSGJPN_12);
				Sts = FAIL;
			}
		}
	}
	return(Sts);
}


/*----- メインウインドウのウインドウハンドルを返す ----------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		HWND ウインドウハンドル
*----------------------------------------------------------------------------*/

HWND GetMainHwnd(void)
{
	return(hWndBup);
}


/*----- プログラムのインスタンスを返す ----------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		HINSTANCE インスタンス
*----------------------------------------------------------------------------*/

HINSTANCE GetBupInst(void)
{
	return(hInstBup);
}


/*----- オートクローズかどうかを返す ------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		int オートクローズかどうか (YES/NO
*----------------------------------------------------------------------------*/

int AskAutoClose(void)
{
	return(AutoClose);
}


/*----- 確認を行なわないかどうかを返す ----------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		int 確認を行なわないかどうか (YES/NO)
*----------------------------------------------------------------------------*/

int AskNoNotify(void)
{
	return(NoNotify);
}


/*----- ヘルプファイルのパス名を返す ------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		LPTSTR パス名
*----------------------------------------------------------------------------*/

LPTSTR AskHelpFilePath(void)
{
	return(HelpPath);
}


/*----- INIファイルのパス名を返す ---------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		LPTSTR パス名
*----------------------------------------------------------------------------*/

LPTSTR AskIniFilePath(void)
{
	return(IniPath);
}


/*----- INIファイルのパス名を返す(ANSI) ----------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		char パス名 (freeすること)
*----------------------------------------------------------------------------*/

char *AskIniFilePathAnsi(void)
{
	char	*oemStr;	/* not _TCHAR */
	int		length;

	/* convert filename to OEM character set */
	length = WideCharToMultiByte(CP_OEMCP, 0, IniPath, -1, NULL, 0, NULL, NULL);
	oemStr = (char*)malloc(length+1);
	memset(oemStr, NUL, length+1);
	WideCharToMultiByte(CP_OEMCP, 0, IniPath, -1, oemStr, length, NULL, NULL);
	return(oemStr);
}


/*----- 全てのオブジェクトを削除 ----------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void DeleteAllObject(void)
{
	CloseBackupThread();
	SetTrayIcon(TICON_DELETE, 0, NULL);

	DeleteTransferDialogResources();

	if(hWndBup != NULL)
		DestroyWindow(hWndBup);
	return;
}


/*----- メインウインドウのメッセージ処理 --------------------------------------
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

static LRESULT CALLBACK BupWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	RECT *Rect;
	LPMINMAXINFO lpmmi;
	POINT Point;

	switch (message)
	{
		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case MENU_BACKUP :
				case MENU_QUICK_BACKUP :
					PostMessage(GetMainDlgHwnd(), WM_COMMAND, wParam, lParam);
					break;

				case MENU_FILE_EXIT :
					PostMessage(hWnd, WM_CLOSE, 0, 0L);
					break;

				case MENU_SETENV :
					SetOption(hWnd);
					break;

				case MENU_HELP_CONTENT :
					HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000002);
					break;

				case MENU_HELP_ABOUT :
					DialogBoxParam(hInstBup, MAKEINTRESOURCE(about_dlg), hWndBup, ExeEscDialogProc, (LPARAM)_T(""));
					break;

				case MENU_HELP_QA :
					ShellExecute(NULL, _T("open"), MYWEB_URL, NULL, _T("."), SW_SHOW);
					break;

				case MENU_OPEN :
					ShowWindow(hWndBup, SW_RESTORE);
					SetWindowPos(hWndBup, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
					SetWindowPos(hWndBup, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
					break;

				case MENU_REGINIT :
					if(DialogBoxParam(hInstBup, MAKEINTRESOURCE(reginit_dlg), hWnd, ExeEscDialogProc, (LPARAM)_T("")) == YES)
					{
						ClearRegistory();
						SaveExit = NO;
						PostMessage(hWnd, WM_CLOSE, 0, 0L);
					}
					break;

				case MENU_REGSAVE :
					SaveMainDlgSize();
					SaveTransDlgSize();
					SaveRegistory();
					SaveSettingsToFile();
					break;

				case MENU_REGLOAD :
					if(LoadSettingsFromFile() == YES)
					{
						DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), hWnd, ExeEscDialogProc, (LPARAM)MSGJPN_13);
						SaveExit = NO;
						PostMessage(hWnd, WM_CLOSE, 0, 0L);
					}
					break;

				case MENU_DISPLOG :
					DispLogWithViewer();
					break;

				case MENU_ERROR_LOG :
					DispErrorLogWithViewer();
					break;
			}
			break;

		case WM_BACKUP_END :
		case WM_BACKUP_ERROR :
			if(AutoClose == 1)
				PostMessage(hWnd, WM_CLOSE, 0, 0L);
			else if(AutoClose >= 2)
			{
				if(DoCountDown(AutoClose) == YES)
				{
					ChangeSystemPowerMode(AutoClose);
					if((AutoClose == 2) || (AutoClose == 5) || (AutoClose == 6))
						PostMessage(hWnd, WM_CLOSE, 0, 0L);
				}
			}
			break;

		case WM_CLICK_ICON :
			/* トレイアイコンがクリックされた */
			if((lParam & 0x00000007L) == 2)
				SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(MENU_OPEN, 0), 0);
			else if((lParam & 0x00000007L) == 5)
				TrayIconMenu();
			break;

		case WM_SIZE :
			if(wParam == SIZE_RESTORED)
			{
				Rect = malloc(sizeof(RECT));
				if(Rect != NULL)
				{
					GetClientRect(hWnd, Rect);
					SendMessage(GetMainDlgHwnd(), WM_SIZE_CHANGE, WMSZ_RIGHT|WMSZ_BOTTOM, (LPARAM)Rect);
				}
				Rect = malloc(sizeof(RECT));
				if(Rect != NULL)
				{
					GetClientRect(hWnd, Rect);
					SendMessage(GetTransDlgHwnd(), WM_SIZE_CHANGE, WMSZ_RIGHT|WMSZ_BOTTOM, (LPARAM)Rect);
				}
			}
			if(TmpTrayIcon == YES)
			{
				/* ウインドウのアイコン化、または元に戻したときの処理 */
				if(wParam == SIZE_MINIMIZED)
					ShowWindow(hWndBup, SW_HIDE);
				else if(wParam == SIZE_RESTORED)
				{
					ShowWindow(hWndBup, SW_SHOW);
					SetWindowPos(hWndBup, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
				}
			}
			else
				return(DefWindowProc(hWnd, message, wParam, lParam));
			break;

		case WM_GETMINMAXINFO :
			AsdMainDlgMinSize(&Point);
			Point.x += GetSystemMetrics(SM_CYSIZEFRAME) * 2;
			Point.y += GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYSIZEFRAME) * 2 +  + GetSystemMetrics(SM_CYCAPTION);
			lpmmi = (LPMINMAXINFO)lParam;
			lpmmi->ptMinTrackSize.x = Point.x;
			lpmmi->ptMinTrackSize.y = Point.y;
			break;

		case WM_MOVING :
			MainPosX = ((RECT *)lParam)->left;
			MainPosY = ((RECT *)lParam)->top;
			return(DefWindowProc(hWnd, message, wParam, lParam));

		case WM_PAINT :
		    BeginPaint(hWnd, (LPPAINTSTRUCT) &ps);
		    EndPaint(hWnd, (LPPAINTSTRUCT) &ps);
			break;

		case WM_DESTROY :
			PostQuitMessage(0);
			break;

		case WM_QUERYENDSESSION :
			ExitProc(hWnd);
			return(TRUE);

		case WM_CLOSE :
			ExitProc(hWnd);
			break;

		default :
			return(DefWindowProc(hWnd, message, wParam, lParam));
	}
    return(0L);
}


/*----- プログラム終了時の処理 ------------------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void ExitProc(HWND hWnd)
{
	SaveMainDlgSize();
	SaveTransDlgSize();

	if(SaveExit == YES)
	{
		SaveRegistory();
	}
	DeleteAllObject();
	DeleteLogFilename();
	DeleteErrorLogfile();
	HtmlHelp(NULL, NULL, HH_UNINITIALIZE, dwCookie); 
	return;
}


/*----- アイコントレイ用のアイコンをロード ------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
void LoadTrayIcon(void)
{
	int i;

	if(TmpTrayIcon == YES)
	{
		for(i = 0; i < (sizeof(IconData)/sizeof(int)); i++)
		{
			IconHandle[i] = LoadImage(hInstBup, MAKEINTRESOURCE(IconData[i]), IMAGE_ICON, 16, 16, LR_DEFAULTSIZE);
		}
	}
	return;
}


/*----- アイコントレイにアイコンをセット --------------------------------------
*
*	Parameter
*		int Ope : 操作方法 (TICON_xxx)
*		int Type : アイコン番号 (TICON_CHANGEの時のみ)
*		LPTSTR AddMesg : 追加メッセージ (Backup - ????)
*			NULL=なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SetTrayIcon(int Ope, int Type, LPTSTR AddMesg)
{
	NOTIFYICONDATA ntData;
	static int CurIcon;

	if(TmpTrayIcon == YES)
	{
		if(Ope != TICON_DELETE)
		{
			if(Ope == TICON_NEXT)
			{
				if(++CurIcon >= (sizeof(IconData)/sizeof(int)))
					CurIcon = 1;	/* 動画用の最初へ */
				Type = CurIcon;
			}
			else
				CurIcon = Type;

			/* アイコントレイにセット */
			ntData.cbSize = sizeof(NOTIFYICONDATA);
			ntData.hWnd = hWndBup;
			ntData.uID = 1;
			ntData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
			ntData.uCallbackMessage = WM_CLICK_ICON;
			ntData.hIcon = IconHandle[Type];
			_tcscpy(ntData.szTip, _T("Backup"));
			if(AddMesg != NULL)
			{
				_tcscat(ntData.szTip, _T(" - "));
				_tcscat(ntData.szTip, AddMesg);
			}
			if(Ope == TICON_NEW)
				Shell_NotifyIcon(NIM_ADD, &ntData);
			else
				Shell_NotifyIcon(NIM_MODIFY, &ntData);
		}
		else
		{
			/* アイコントレイから消去 */
			ntData.cbSize = sizeof(NOTIFYICONDATA);
			ntData.hWnd = hWndBup;
			ntData.uID = 1;
			ntData.uFlags = 0;
			Shell_NotifyIcon(NIM_DELETE, &ntData);
		}
	}
	return;
}


/*----- コマンドラインを解析 --------------------------------------------------
*
*	Parameter
*		LPTSTR Str : コマンドライン文字列
*		LPTSTR Src : バックアップ元フォルダ (NULL=Src,Dstを取得しない)
*		LPTSTR Dst : バックアップ先フォルダ
*		int *Opt : オプション
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*
*	Note
*		source destination --force --rmdir --rmfile --noerror --start --close --name --all
*		source destination -f -d -r -e -s -c -n -a
*		-h --help -? はヘルプを表示
*----------------------------------------------------------------------------*/

static int AnalyzeComLine(LPTSTR Str, LPTSTR Src, LPTSTR Dst, int *Opt)
{
	int Sts;
	_TCHAR Tmp[MY_MAX_PATH+1];

	if(Src != NULL)
	{
		_tcscpy(Src, _T(""));
		_tcscpy(Dst, _T(""));
	}
	*Opt = 0;

	Sts = SUCCESS;
	Str = GetToken(Str, Tmp);
	while((Str = GetToken(Str, Tmp)) != NULL)
	{
		if((Tmp[0] == _T('/')) || (Tmp[0] == _T('-')))
		{
			_tcslwr(Tmp);
			if((_tcscmp(&Tmp[1], _T("f")) == 0) || (_tcscmp(&Tmp[1], _T("-force")) == 0))
				*Opt |= OPT_FORCE;
			else if((_tcscmp(&Tmp[1], _T("d")) == 0) || (_tcscmp(&Tmp[1], _T("-rmdir")) == 0))
				*Opt |= OPT_RMDIR;
			else if((_tcscmp(&Tmp[1], _T("r")) == 0) || (_tcscmp(&Tmp[1], _T("-rmfile")) == 0))
				*Opt |= OPT_RMFILE;
			else if((_tcscmp(&Tmp[1], _T("e")) == 0) || (_tcscmp(&Tmp[1], _T("-noerror")) == 0))
				*Opt |= OPT_NOERROR;
			else if((_tcscmp(&Tmp[1], _T("s")) == 0) || (_tcscmp(&Tmp[1], _T("-start")) == 0))
				*Opt |= OPT_START;
			else if((_tcscmp(&Tmp[1], _T("c")) == 0) || (_tcscmp(&Tmp[1], _T("-close")) == 0))
				*Opt |= OPT_CLOSE;
			else if((_tcscmp(&Tmp[1], _T("a")) == 0) || (_tcscmp(&Tmp[1], _T("-all")) == 0))
				*Opt |= OPT_ALL;
			else if((_tcscmp(&Tmp[1], _T("i")) == 0) || (_tcscmp(&Tmp[1], _T("-minimized")) == 0))
				*Opt |= OPT_MINIMIZED;
			else if(_tcscmp(&Tmp[1], _T("-nonotify")) == 0)
				*Opt |= OPT_NO_NOTIFY_DEL;
			else if((_tcscmp(&Tmp[1], _T("n")) == 0) || (_tcscmp(&Tmp[1], _T("-name")) == 0))
			{
				*Opt |= OPT_NAME;
				if(Src != NULL)
				{
					if((Str = GetToken(Str, Tmp)) != NULL)
						_tcscpy(Src, Tmp);
					else
					{
						DispErrorBox(MSGJPN_14);
						Sts = FAIL;
						break;
					}
				}
			}
			else if((_tcscmp(&Tmp[1], _T("h")) == 0) || (_tcscmp(&Tmp[1], _T("?")) == 0) || (_tcscmp(&Tmp[1], _T("-help")) == 0))
				HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000008);
			else if((_tcscmp(&Tmp[1], _T("p")) == 0) || (_tcscmp(&Tmp[1], _T("-spec")) == 0))
			{
				*Opt |= OPT_INI_FILE;
				if(Dst != NULL)
				{
					if((Str = GetToken(Str, Tmp)) != NULL)
						_tcscpy(Dst, Tmp);
					else
					{
						DispErrorBox(MSGJPN_14);
						Sts = FAIL;
						break;
					}
				}
			}
			else
			{
				DispErrorBox(MSGJPN_15, Tmp);
				Sts = FAIL;
				break;
			}
		}
		else if((*Opt & OPT_NAME) == 0 && (*Opt & OPT_INI_FILE) == 0)
		{
			if(_tcslen(Src) == 0)
				_tcscpy(Src, Tmp);
			else if(_tcslen(Dst) == 0)
				_tcscpy(Dst, Tmp);
			else
			{
				DispErrorBox(MSGJPN_16, Tmp);
				Sts = FAIL;
				break;
			}
		}
		else
		{
			DispErrorBox(MSGJPN_17, Tmp);
			Sts = FAIL;
		}
	}
	return(Sts);
}


/*----- トークンを返す --------------------------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*		LPTSTR Buf : 文字列を返すバッファ
*
*	Return Value
*		LPTSTR 返したトークンの末尾
*			NULL=終わり
*----------------------------------------------------------------------------*/

static LPTSTR GetToken(LPTSTR Str, LPTSTR Buf)
{
	int InQuote;

	while(*Str != NUL)
	{
		if((*Str != _T(' ')) && (*Str != _T('\t')))
			break;
		Str++;
	}

	if(*Str != NUL)
	{
		InQuote = 0;
		while(*Str != NUL)
		{
			if(*Str == 0x22)
				InQuote = !InQuote;
			else
			{
				if(((*Str == _T(' ')) || (*Str == _T('\t'))) &&
				   (InQuote == 0))
				{
					break;
				}
				*Buf++ = *Str;
			}
			Str++;
		}
	}
	else
		Str = NULL;

	*Buf = NUL;

	return(Str);
}


/*----- メニューのハイド処理を行う --------------------------------------------
*
*	Parameter
*		int Win : ウインドウ番号 (WIN_xxx)
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SetMenuHide(int Win)
{
	int Flg;
	int Flg2;
	HMENU hMenu;

	Flg = MF_ENABLED;
	Flg2 = MF_ENABLED;
	if(Win == WIN_TRANS)
	{
		Flg = MF_GRAYED;
	}
	if(LogSwitch == LOG_SW_OFF)
	{
		Flg2 = MF_GRAYED;
	}

	hMenu = GetMenu(hWndBup);
	EnableMenuItem(hMenu, MENU_BACKUP, Flg);
	EnableMenuItem(hMenu, MENU_QUICK_BACKUP, Flg);
	EnableMenuItem(hMenu, MENU_SETENV, Flg);

	EnableMenuItem(hMenu, MENU_DISPLOG, Flg2);
	return;
}


/*----- トレイアイコンのメニューを表示 ----------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void TrayIconMenu(void)
{
	HMENU hMenu;
	POINT point;

	hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, MENU_OPEN, MSGJPN_18);
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenu, MF_STRING, MENU_FILE_EXIT, MSGJPN_19);

	SetForegroundWindow(GetMainHwnd());		/* ウィンドウをフォアグラウンドに移動 */
	SetFocus(GetMainHwnd());				/* これをしないとメニューが消えない */

	GetCursorPos(&point);
	TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, 0, GetMainHwnd(), NULL);
	DestroyMenu(hMenu);
	return;
}


/*----- エラーメッセージボックスを表示する ------------------------------------
*
*	Parameter
*		LPTSTR szFormat ... : フォーマット文字列
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void DispErrorBox(LPTSTR szFormat,...)
{
	va_list vaArgs;
	static _TCHAR szBuf[MY_MAX_PATH + 256];

	va_start(vaArgs,szFormat);
	if(_vstprintf(szBuf,szFormat,vaArgs)!=EOF)
		MessageBox(NULL, szBuf, _T("Backup"), MB_OK);
	va_end(vaArgs);
}


/*----- デバッグコンソールにメッセージを表示する ------------------------------
*
*	Parameter
*		LPTSTR szFormat ... : フォーマット文字列
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void DoPrintf(LPTSTR szFormat,...)
{
#if 0
	va_list vaArgs;
	static _TCHAR szBuf[256];
	DWORD Tmp;

	va_start(vaArgs,szFormat);
	if(_vstprintf(szBuf,szFormat,vaArgs)!=EOF)
		WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), szBuf, _tcslen(szBuf), &Tmp, NULL);
	va_end(vaArgs);
#endif
}


/*----- デフォルトのログファイル名を返す---------------------------------------
*
*	Parameter
*		LPTSTR buf : ログファイル名を返すバッファ
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
void MakeInitialLogFilename(LPTSTR buf)
{
    GetModuleFileName(NULL, buf, MY_MAX_PATH);
	_tcscpy(GetFileName(buf), _T("Backup.log"));
    {
        LPITEMIDLIST pidl;
        LPMALLOC pMalloc;
        SHGetMalloc(&pMalloc);
        if(SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &pidl)))
        {
            SHGetPathFromIDList(pidl, buf);
            _tcscat(buf, _T("\\sota\\Backup\\Backup.log"));
            pMalloc->lpVtbl->Free(pMalloc, pidl);
        }
        pMalloc->lpVtbl->Release(pMalloc);
    }
}

