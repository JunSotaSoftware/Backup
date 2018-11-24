/*===========================================================================
/
/									Backup
/								転送ダイアログ
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
#include <stdarg.h>
#include <windowsx.h>
#include <commctrl.h>

#include "common.h"
#include "resource.h"


#define TASK_BUFSIZE	(16*1024)


/*===== プロトタイプ =====*/

static LRESULT CALLBACK TransferDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*===== ローカルなワーク ======*/

static HWND hWndTransDlg = NULL;
static RECT RectPar;
static COPYPATLIST *CopyPatList;
static WNDPROC LogProcPtr;
static HBITMAP ProcPicture[3] = { NULL, NULL, NULL };
static HIMAGELIST hImage = NULL;

static DIALOGSIZE DlgSize = {
	{ TRANS_TITLE2, TRANS_BOX1, TRANS_PREPARE, TRANS_SCAN, TRANS_RMDIR, TRANS_RMFILE, TRANS_MKDIR, TRANS_COPY, TRANS_PIC_PREPARE, TRANS_PIC_SCAN, TRANS_PIC_RMDIR, TRANS_PIC_RMFILE, TRANS_PIC_MKDIR, TRANS_PIC_COPY, TRANS_GRIP, -1 },
	{ TRANS_EXEC, TRANS_PROGRESS, TRANS_STOP, TRANS_RETURN, TRANS_QUIT, TRANS_GRIP, -1 },
	{ TRANS_DIRLIST, -1 },
	{ TRANS_EXEC, TRANS_PROGRESS, TRANS_LINE1, -1 },
	{ -1 },
	{ 0, 0 },
	{ 0, 0 }
};

/*===== グローバルなワーク ======*/

extern int Sound;
extern _TCHAR SoundFile[MY_MAX_PATH+1];

/* 設定 */
extern int IntervalTime;
extern SIZE TransDlgSize;
extern int ExitOnEsc;



/*----- 転送中ダイアログを作成する --------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

int MakeTransferDialog(void)
{
	int Sts;

	Sts = FAIL;
	if((hWndTransDlg = CreateDialog(GetBupInst(), MAKEINTRESOURCE(transfer_dlg), GetMainHwnd(), TransferDlgWndProc)) != NULL)
	{
//		ShowWindow(hWndMainDlg, SW_HIDE);
//		SetMenuHide(WIN_MAIN);

		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- 転送中ダイアログのリソースを破棄する ----------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void DeleteTransferDialogResources(void)
{
	if(hImage != NULL)
		ImageList_Destroy(hImage);
	if(ProcPicture[0] != NULL)
		DeleteObject(ProcPicture[0]);
	if(ProcPicture[1] != NULL)
		DeleteObject(ProcPicture[1]);
	if(ProcPicture[2] != NULL)
		DeleteObject(ProcPicture[2]);
}


/*----- 転送中ウインドウのウインドウハンドルを返す ----------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		HWND ウインドウハンドル
*----------------------------------------------------------------------------*/

HWND GetTransDlgHwnd(void)
{
	return(hWndTransDlg);
}


/*----- バックアップを開始する ------------------------------------------------
*
*	Parameter
*		COPYPATLIST *Pat : バックアップパターン
*
*	Return Value
*		int ステータス (=SUCCESS)
*----------------------------------------------------------------------------*/

int StartBackup(COPYPATLIST *Pat)
{
	CopyPatList = Pat;

	ShowWindow(GetMainDlgHwnd(), SW_HIDE);
	ShowWindow(hWndTransDlg, SW_SHOW);
//	SetFocus(GetDlgItem(hWndTransDlg, TRANS_ABORT));
	SetFocus(GetDlgItem(hWndTransDlg, TRANS_EXEC));
	SetMenuHide(WIN_TRANS);
	PostMessage(hWndTransDlg, WM_BACKUP_START, 0, 0);

	return(SUCCESS);
}


typedef enum {
	PROCESSING_STOP,
	PROCESSING_RUN,
	PROCESSING_PAUSE
} PROCESSING_STATUS;


typedef enum {
	NEXT_TO_STAY,
	NEXT_TO_MAIN,
	NEXT_TO_QUIT
} NEXT_TO_STATUS;


/*----- バックアップ中ウインドウのメッセージ処理 ------------------------------
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

static LRESULT CALLBACK TransferDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROCESSING_STATUS Processing;
	static NEXT_TO_STATUS NextTo;
	static int TickCount;
	_TCHAR Tmp[40];
	HWND hWndChild;
	RECT Rect;
	POINT Point;
	RECT *pRect;
	int sts;

	switch (message)
	{
		case WM_INITDIALOG :
			GetClientRect(GetMainHwnd(), &Rect);
			Point.x = Rect.left;
			Point.y = Rect.top;
			ClientToScreen(GetMainHwnd(), &Point);
			SetWindowPos(hDlg, 0, Point.x, Point.y, 0, 0, SWP_NOSIZE | /*SWP_NOREDRAW | */SWP_NOZORDER);

			hImage = ImageList_LoadBitmap(GetBupInst(), MAKEINTRESOURCE(list_bmp), 16, 8, RGB(255,0,0));

			ProcPicture[0] = LoadImage(GetBupInst(), MAKEINTRESOURCE(box_bmp), IMAGE_BITMAP, 14, 10, LR_LOADMAP3DCOLORS);
			ProcPicture[1] = LoadImage(GetBupInst(), MAKEINTRESOURCE(current_bmp), IMAGE_BITMAP, 14, 10, LR_LOADMAP3DCOLORS);
			ProcPicture[2] = LoadImage(GetBupInst(), MAKEINTRESOURCE(done_bmp), IMAGE_BITMAP, 14, 10, LR_LOADMAP3DCOLORS);

			SendDlgItemMessage(hDlg, TRANS_DIRLIST, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImage);
//			SendDlgItemMessage(hDlg, TRANS_DIRLIST, TVM_SETBKCOLOR, 0, (LPARAM)RGB(255,255,255));
//			SendDlgItemMessage(hDlg, TRANS_DIRLIST, TVM_SETTEXTCOLOR, 0, (LPARAM)RGB(0,0,0));

			SendDlgItemMessage(hDlg, TRANS_EXEC, EM_LIMITTEXT, TASK_BUFSIZE, 0);
			SendDlgItemMessage(hDlg, TRANS_PROGRESS, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
			SendDlgItemMessage(hDlg, TRANS_PROGRESS, PBM_SETSTEP, 1, 0);
			SendDlgItemMessage(hDlg, TRANS_PROGRESS, PBM_SETPOS, 0, 0);

			/* ログ表示ウインドウをサブクラス化 */
			hWndChild = GetDlgItem(hDlg, TRANS_EXEC);
			LogProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)LogWndProc);
			/* ダイアログサイズの初期化 */
			DlgSizeInit(hDlg, &DlgSize, &TransDlgSize, FALSE);
			Processing = PROCESSING_STOP;
			NextTo = NEXT_TO_STAY;
			return(TRUE);

		case WM_BACKUP_START :
			SendDlgItemMessage(hDlg, TRANS_STOP, WM_SETTEXT, 0, (LPARAM)MSGJPN_123);
			EnableWindow(GetDlgItem(hDlg, TRANS_STOP), TRUE);
			EnableWindow(GetDlgItem(hDlg, TRANS_RETURN), TRUE);
			EnableWindow(GetDlgItem(hDlg, TRANS_QUIT), TRUE);
			SetTimer(hDlg, TIMER_ANIM, 600, NULL);	/* 0.6秒おき */
			Processing = PROCESSING_RUN;
			NextTo = NEXT_TO_STAY;
			SetBackupPat(CopyPatList);
			break;

		case WM_BACKUP_END :
		case WM_BACKUP_ERROR :
			Processing = PROCESSING_STOP;
			KillTimer(hDlg, TIMER_ANIM);
			SendMessage(GetMainHwnd(), WM_SETTEXT, 0, (LPARAM)_T("Backup"));
			SendDlgItemMessage(hDlg, TRANS_STOP, WM_SETTEXT, 0, (LPARAM)MSGJPN_55);
			EnableWindow(GetDlgItem(hDlg, TRANS_STOP), TRUE);
			EnableWindow(GetDlgItem(hDlg, TRANS_RETURN), TRUE);
			EnableWindow(GetDlgItem(hDlg, TRANS_QUIT), TRUE);

			if(NextTo == NEXT_TO_MAIN)
			{
				KillTimer(hDlg, TIMER_INTERVAL);
				ShowWindow(hWndTransDlg, SW_HIDE);
				ShowWindow(GetMainDlgHwnd(), SW_SHOW);
				SetFocus(GetDlgItem(GetMainDlgHwnd(), MAIN_LIST));
				SetMenuHide(WIN_MAIN);
				SetTrayIcon(TICON_CHANGE, 0, NULL);
				PostMessage(GetMainDlgHwnd(), WM_RETURN_MAIN, 0, 0);
			}
			else if(NextTo == NEXT_TO_QUIT)
			{
				PostMessage(GetMainHwnd(), WM_CLOSE, wParam, lParam);
			}
			else
			{
				if(Sound == YES)
					PlaySound(SoundFile, NULL, SND_SYNC);

				if((IntervalTime <= 0) || (AskAutoClose() != 0))
				{
					SetTrayIcon(TICON_CHANGE, 0, NULL);
					PostMessage(GetMainHwnd(), message, 0, 0);
				}
				else
				{
					TickCount = IntervalTime;
					SetTimer(hDlg, TIMER_INTERVAL, 60*1000, NULL);	/* 1分おき */
					_stprintf(Tmp, MSGJPN_57, TickCount);
					SendDlgItemMessage(hDlg, TRANS_STOP, WM_SETTEXT, 0, (LPARAM)Tmp);
					SetTrayIcon(TICON_CHANGE, 0, Tmp);
				}
			}
			break;

		case WM_TIMER :
			if(wParam == TIMER_ANIM)
			{
				SetTrayIcon(TICON_NEXT, 0, MSGJPN_58);
			}
			else
			{
				if(--TickCount == 0)
				{
					KillTimer(hDlg, TIMER_INTERVAL);
					PostMessage(hDlg, WM_BACKUP_START, 0, 0);
				}
				else
				{
					_stprintf(Tmp, MSGJPN_57, TickCount);
					SendDlgItemMessage(hDlg, TRANS_STOP, WM_SETTEXT, 0, (LPARAM)Tmp);
					SetTrayIcon(TICON_CHANGE, 0, Tmp);
				}
			}
			break;

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case TRANS_STOP :
					if(Processing == PROCESSING_STOP)
					{
						/* 再バックアップ開始 */
						KillTimer(hDlg, TIMER_INTERVAL);
						PostMessage(hDlg, WM_BACKUP_START, 0, 0);
					}
					else if(Processing == PROCESSING_PAUSE)
					{
						/* 一時停止からの復帰 */
						SetBackupRestart();
						Processing = PROCESSING_RUN;
						SetTimer(hDlg, TIMER_ANIM, 600, NULL);	/* 0.6秒おき */
						SendDlgItemMessage(hDlg, TRANS_STOP, WM_SETTEXT, 0, (LPARAM)MSGJPN_123);
					}
					else
					{
						/* 一時停止への移行 */
						SetBackupPause();
						Processing = PROCESSING_PAUSE;
						KillTimer(hDlg, TIMER_ANIM);
						SendDlgItemMessage(hDlg, TRANS_STOP, WM_SETTEXT, 0, (LPARAM)MSGJPN_56);
					}
					break;

				case TRANS_RETURN :
					if(Processing != PROCESSING_STOP)
					{
						sts = YES;
						if(Processing == PROCESSING_RUN)
						{
							SetBackupPause();
							sts = DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(quit_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)_T(""));
							if(sts == NO)
							{
								SetBackupRestart();
							}
						}
						if(sts == YES)
						{
							EnableWindow(GetDlgItem(hDlg, TRANS_STOP), FALSE);
							EnableWindow(GetDlgItem(hDlg, TRANS_RETURN), FALSE);
							EnableWindow(GetDlgItem(hDlg, TRANS_QUIT), FALSE);
							NextTo = NEXT_TO_MAIN;
							SetBackupAbort();
						}
					}
					else
					{
						KillTimer(hDlg, TIMER_INTERVAL);
						ShowWindow(hWndTransDlg, SW_HIDE);
						ShowWindow(GetMainDlgHwnd(), SW_SHOW);
						SetFocus(GetDlgItem(GetMainDlgHwnd(), MAIN_LIST));
						SetMenuHide(WIN_MAIN);
						SetTrayIcon(TICON_CHANGE, 0, NULL);
						PostMessage(GetMainDlgHwnd(), WM_RETURN_MAIN, 0, 0);
					}
					break;

				case TRANS_QUIT :
					if(Processing != PROCESSING_STOP)
					{
						sts = YES;
						if(Processing == PROCESSING_RUN)
						{
							SetBackupPause();
							sts = DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(quit_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)_T(""));
							if(sts == NO)
							{
								SetBackupRestart();
							}
						}
						if(sts == YES)
						{
							EnableWindow(GetDlgItem(hDlg, TRANS_STOP), FALSE);
							EnableWindow(GetDlgItem(hDlg, TRANS_RETURN), FALSE);
							EnableWindow(GetDlgItem(hDlg, TRANS_QUIT), FALSE);
							NextTo = NEXT_TO_QUIT;
							SetBackupAbort();
						}
					}
					else
					{
						KillTimer(hDlg, TIMER_INTERVAL);
						PostMessage(GetMainHwnd(), WM_CLOSE, wParam, lParam);
					}
					break;

				case IDCANCEL :
					if(ExitOnEsc == 1)
					{
						if(Processing != PROCESSING_STOP)
						{
							sts = YES;
							if(Processing == PROCESSING_RUN)
							{
								SetBackupPause();
								sts = DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(quit_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)_T(""));
								if(sts == NO)
								{
									SetBackupRestart();
								}
							}
							if(sts == YES)
							{
								EnableWindow(GetDlgItem(hDlg, TRANS_STOP), FALSE);
								EnableWindow(GetDlgItem(hDlg, TRANS_RETURN), FALSE);
								EnableWindow(GetDlgItem(hDlg, TRANS_QUIT), FALSE);
								NextTo = NEXT_TO_QUIT;
								SetBackupAbort();
							}
						}
						else
						{
							KillTimer(hDlg, TIMER_INTERVAL);
							PostMessage(GetMainHwnd(), WM_CLOSE, wParam, lParam);

						}
					}
					break;
			}
			return(TRUE);

		case WM_SIZE_CHANGE :
			pRect = (RECT *)lParam;
			SetWindowPos(hDlg, NULL, 0, 0, pRect->right - pRect->left, pRect->bottom - pRect->top, SWP_NOMOVE|SWP_NOZORDER);
			DlgSizeChange(hDlg, &DlgSize, pRect, (int)wParam);
			free((void*)lParam);
		    break;
	}
    return(FALSE);
}


/*----- 現在の処理段階を表示する ----------------------------------------------
*
*	Parameter
*		int Pass : 処理段階
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SelectPass(int Pass)
{
	static const int pictureTable[7][6] = {
		{ 1, 0, 0, 0, 0, 0 },
		{ 2, 1, 0, 0, 0, 0 },
		{ 2, 2, 1, 0, 0, 0 },
		{ 2, 2, 2, 1, 0, 0 },
		{ 2, 2, 2, 2, 1, 0 },
		{ 2, 2, 2, 2, 2, 1 },
		{ 2, 2, 2, 2, 2, 2 }
	};

	SendMessage(GetDlgItem(hWndTransDlg, TRANS_PIC_PREPARE), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ProcPicture[pictureTable[Pass][0]]);
	SendMessage(GetDlgItem(hWndTransDlg, TRANS_PIC_SCAN), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ProcPicture[pictureTable[Pass][1]]);
	SendMessage(GetDlgItem(hWndTransDlg, TRANS_PIC_RMDIR), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ProcPicture[pictureTable[Pass][2]]);
	SendMessage(GetDlgItem(hWndTransDlg, TRANS_PIC_RMFILE), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ProcPicture[pictureTable[Pass][3]]);
	SendMessage(GetDlgItem(hWndTransDlg, TRANS_PIC_MKDIR), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ProcPicture[pictureTable[Pass][4]]);
	SendMessage(GetDlgItem(hWndTransDlg, TRANS_PIC_COPY), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)ProcPicture[pictureTable[Pass][5]]);
	return;
}


/*----- パターン名を表示する --------------------------------------------------
*
*	Parameter
*		LPTSTR Name : パターン名
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SetPatName(LPTSTR Name)
{
	_TCHAR Tmp[PATNAME_LEN+20];

	_stprintf(Tmp, _T("%s - Backup"), Name);
	SendMessage(GetMainHwnd(), WM_SETTEXT, 0, (LPARAM)Tmp);
	return;
}


/*----- ログメッセージを表示する ----------------------------------------------
*
*	Parameter
*		int Type : タイプ (TASKMSG_xxx)
*		LPTSTR szFormat ... : フォーマット文字列
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SetTaskMsg(int Type, LPTSTR szFormat,...)
{
	int Pos;
	va_list vaArgs;
	_TCHAR szBuf[MY_MAX_PATH2 + 100];

	va_start(vaArgs,szFormat);
	if(_vstprintf(szBuf, szFormat, vaArgs)!=EOF)
	{
		/* debug window */
		DoPrintf(_T("%s\n"), szBuf);

		/* log file */
		WriteMsgToLogfile(szBuf);

		if(Type == TASKMSG_ERR)
		{
			WriteMsgToErrorLogfile(szBuf);
		}

		/* ウインドウの横幅に合わせて整形 */
		SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, WM_FORMAT_TEXT, 0, (LPARAM)szBuf);
		_tcscat(szBuf, _T("\r\n"));

		Pos = SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_GETLINECOUNT, 0, 0);
		Pos = SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_LINEINDEX, Pos-1, 0);

		/* テキストサイズのリミット値をチェック */
		if((Pos + _tcslen(szBuf)) >= TASK_BUFSIZE)
		{
			Pos = SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_LINEFROMCHAR, TASK_BUFSIZE/10, 0) + 1;
			Pos = SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_LINEINDEX, Pos, 0);
			SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_SETSEL, 0, Pos);
			SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_REPLACESEL, FALSE, (LPARAM)_T(""));

			Pos = SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_GETLINECOUNT, 0, 0);
			Pos = SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_LINEINDEX, Pos-1, 0);
		}
		SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_SETSEL, Pos, Pos);
		SendDlgItemMessage(hWndTransDlg, TRANS_EXEC, EM_REPLACESEL, FALSE, (LPARAM)szBuf);
	}
	va_end(vaArgs);
	return;
}


/*----- 転送中のプログレスバーを表示する --------------------------------------
*
*	Parameter
*		LONGLONG Total : トータルのサイズ
*		LONGLONG Done : 転送したサイズ
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SetFileProgress(LONGLONG Total, LONGLONG Done)
{
	int Per;

	if(Total <= 0)
		Per = 0;
	else if(Total < 1024*1024)
		Per = (int)(Done * 100 / Total);
	else
		Per = (int)((Done / 1024) * 100 / (Total / 1024));

	SendDlgItemMessage(hWndTransDlg, TRANS_PROGRESS, PBM_SETPOS, Per, 0);
	return;
}


/*----- ログ表示ウインドウのプロシージャ --------------------------------------
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

static LRESULT CALLBACK LogWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT  rectWnd;
	DWORD dwFormat;
	HWND hWndParent;
	HFONT hFontOld;
	HDC hDC;
	HDC hDC2;

	switch(message)
	{
		// テキスト整形
		case WM_FORMAT_TEXT :
			hDC2 = GetDC(hWnd);
			hDC = CreateCompatibleDC(hDC2);
			hWndParent = GetParent(hWnd);
			if(hWndParent)
				hFontOld = SelectObject(hDC, (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0));
			GetClientRect(hWnd, &rectWnd);
			rectWnd.right -= GetSystemMetrics(SM_CXVSCROLL);
			dwFormat = DT_LEFT | DT_NOPREFIX | DT_VCENTER | DT_PATH_ELLIPSIS | DT_MODIFYSTRING;
			DrawTextEx(hDC, (_TCHAR*)lParam, _tcslen((_TCHAR*)lParam), &rectWnd, dwFormat, NULL);
			if(hWndParent)
				SelectObject(hDC, hFontOld);
			DeleteDC(hDC);
			ReleaseDC(hWnd, hDC2);
		    return(0);

		default :
			return(CallWindowProc(LogProcPtr, hWnd, message, wParam, lParam));
	}
}


/*----- 転送ダイアログのサイズを保存 ------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
void SaveTransDlgSize(void)
{
	AskDlgSize(&DlgSize, &TransDlgSize);
}



