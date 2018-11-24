/*===========================================================================
/
/									Backup
/								メインダイアログ
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

static LRESULT CALLBACK PatListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK MainDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void ResetAllSel(HWND hDlg, int Ctrl);
static void SetMainDlgButtonHide(HWND hDlg);
static int UpdatePatToList(int Num, COPYPAT *Set);
static int DelPatFromList(int Num);
static int ExchangeListItem(int Num1, int Num2);
static void SendAllPatNames(HWND hWnd, int Cmd);
static void SwapInt(int *Num1, int *Num2);
static int GetSelectedPat(COPYPATLIST **Top, int All);
static int GetNamedPat(COPYPATLIST **Top, LPTSTR Name);
static void DeletePatList(COPYPATLIST **Top);
static LRESULT CALLBACK NotifyDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void DispSrcAndDest(HWND hDlg, COPYPATLIST *Pat, int Num);
static BOOL CheckNullPat(int Num);
static void DispCommentToWin(HWND hDlg);
static BOOL CALLBACK ShowCommentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*===== グローバルなワーク =====*/

extern SIZE MainDlgSize;
extern SIZE NotifyDlgSize;
extern int ExitOnEsc;
extern int ShowComment;		/* 0=表示しない,1=ツールチップで表示、2=ウインドウで表示 */
extern int AutoClose;
extern int ExecOption;
extern int Sound;
extern _TCHAR SoundFile[MY_MAX_PATH+1];
extern int IntervalTime;

/*===== ローカルなワーク ======*/

static HWND hWndMainDlg = NULL;

static HWND hWndPatList;
static WNDPROC PatListProcPtr;


static COPYPATLIST *PatListTop = NULL;
static int Patterns = 0;

static COPYPATLIST *TmpPatList;

static DIALOGSIZE MainDlgSizeInfo = {
	{ MAIN_NEW, MAIN_SET, MAIN_COPY, MAIN_DEL, MAIN_UP, MAIN_DOWN, MAIN_SIZE, MAIN_GRIP, -1 },
	{ MAIN_START, MAIN_ALLSTART, MAIN_QUIT, MAIN_LINE_2, MAIN_GRIP, MAIN_COMMENT, -1 },
	{ MAIN_LIST, -1 },
	{ MAIN_LINE_1, MAIN_LINE_2, MAIN_COMMENT, -1 },
	{ -1 },
	{ 0, 0 },
	{ 0, 0 }
};

static DIALOGSIZE NotifyDlgSizeInfo = {
	{ TRNOT_NEXT, TRNOT_GRIP, -1 },
	{ IDOK, IDCANCEL, TRNOT_QUIT, TRNOT_SHUTDOWN, TRNOT_DST, TRNOT_VOL, TRNOT_PREV, TRNOT_NEXT, TRNOT_GRIP, TRNOT_MSG1, TRNOT_MSG2, TRNOT_MSG3, TRNOT_MSG4, TRNOT_SYSTEM, -1 },
	{ TRNOT_SRCLIST, -1 },
	{ TRNOT_DST, TRNOT_VOL, -1 },
	{ -1 },
	{ 0, 0 },
	{ 0, 0 }
};


/*----- メインダイアログを作成する --------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

int MakeMainDialog(void)
{
	int Sts;

	Sts = FAIL;
	if(ShowComment == 2)
		hWndMainDlg = CreateDialog(GetBupInst(), MAKEINTRESOURCE(main_comment_dlg), GetMainHwnd(), MainDlgWndProc);
	else
		hWndMainDlg = CreateDialog(GetBupInst(), MAKEINTRESOURCE(main_dlg), GetMainHwnd(), MainDlgWndProc);

	if(hWndMainDlg != NULL)
	{
		hWndPatList = GetDlgItem(hWndMainDlg, MAIN_LIST);

		if(ShowComment == 1)
			InitListBoxTips(hWndPatList, GetBupInst());

		PatListProcPtr = (WNDPROC)SetWindowLong(hWndPatList, GWL_WNDPROC, (LONG)PatListWndProc);

		ShowWindow(hWndMainDlg, SW_SHOW);

		SetMenuHide(WIN_MAIN);
		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- リストウインドウのメッセージ処理 ---------------------------------------
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

static LRESULT CALLBACK PatListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_MOUSEMOVE :
			if(ShowComment == 1)
				CheckTipsDisplay(lParam);
			/* ここにbreakはない */

		default :
			return(CallWindowProc(PatListProcPtr, hWnd, message, wParam, lParam));
	}
}


/*----- メインダイアログのウインドウハンドルを返す ----------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		HWND ウインドウハンドル
*----------------------------------------------------------------------------*/

HWND GetMainDlgHwnd(void)
{
	return(hWndMainDlg);
}


/*----- メインダイアログのメッセージ処理 --------------------------------------
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

static LRESULT CALLBACK MainDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int Cur;
	COPYPAT TmpPat;
	static COPYPATLIST *CopyPatList;
	RECT Rect;
	POINT Point;
	RECT *pRect;

	switch (message)
	{
		case WM_INITDIALOG :
			GetClientRect(GetMainHwnd(), &Rect);
			Point.x = Rect.left;
			Point.y = Rect.top;
			ClientToScreen(GetMainHwnd(), &Point);
			SetWindowPos(hDlg, 0, Point.x, Point.y, 0, 0, SWP_NOSIZE | SWP_NOREDRAW | SWP_NOZORDER);

			SendAllPatNames(GetDlgItem(hDlg, MAIN_LIST), LB_ADDSTRING);
			SetMainDlgButtonHide(hDlg);
			SendDlgItemMessage(hDlg, MAIN_LIST, LB_SETSEL, FALSE, 0);
			DispCommentToWin(hDlg);
			CopyPatList = NULL;
			/* ダイアログサイズの初期化 */
			DlgSizeInit(hDlg, &MainDlgSizeInfo, &MainDlgSize, FALSE);
			return(TRUE);

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case MAIN_ALLSTART :
				case MAIN_START :
				case MENU_BACKUP :
					Cur = NO;
					if(GET_WM_COMMAND_ID(wParam, lParam) == MAIN_ALLSTART)
						Cur = YES;
					if(GetSelectedPat(&CopyPatList, Cur) > 0)
					{
						if(((CopyPatList->Set.ShowComment == NO) || 
							(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(show_comment_dlg), hDlg, ShowCommentDlgProc, (LPARAM)CopyPatList->Set.Comment) == YES)) &&
						   (NotifyBackup(hDlg, CopyPatList) == YES))
						{
							StartBackup(CopyPatList);
						}
						else
						{
							DeletePatList(&CopyPatList);
							SetFocus(GetDlgItem(GetMainDlgHwnd(), MAIN_LIST));
						}
					}
					break;

				case MENU_START_NAME :
					if(GetNamedPat(&CopyPatList, (LPTSTR )lParam) > 0)
					{
						if(NotifyBackup(hDlg, CopyPatList) == YES)
							StartBackup(CopyPatList);
						else
						{
							DeletePatList(&CopyPatList);
							SetFocus(GetDlgItem(GetMainDlgHwnd(), MAIN_LIST));
						}
					}
					break;

				case MENU_QUICK_BACKUP :
					if(GetQuickBackupParam(&CopyPatList, hDlg) > 0)
					{
						if(NotifyBackup(hDlg, CopyPatList) == YES)
							StartBackup(CopyPatList);
						else
						{
							DeletePatList(&CopyPatList);
							SetFocus(GetDlgItem(GetMainDlgHwnd(), MAIN_LIST));
						}
					}
					break;

				case MAIN_SIZE :
					if(GetSelectedPat(&CopyPatList, NO) > 0)
					{
						FilesSizeDialog(hDlg, CopyPatList);
						DeletePatList(&CopyPatList);
						SetFocus(GetDlgItem(GetMainDlgHwnd(), MAIN_LIST));
					}
					break;

				case MAIN_END :
					PostMessage(GetMainHwnd(), WM_CLOSE, 0, 0L);
					break;

				case MAIN_NEW :
					CopyDefaultPat(&TmpPat);
					if(DispHostSetDlg(GetMainHwnd(), &TmpPat) == YES)
					{
						AddPatToList(&TmpPat);
						SendDlgItemMessage(hDlg, MAIN_LIST, LB_ADDSTRING, 0, (LPARAM)TmpPat.Name);
						Cur = Patterns - 1;
						ResetAllSel(hDlg, MAIN_LIST);
						SendDlgItemMessage(hDlg, MAIN_LIST, LB_SETSEL, TRUE, Cur);
					}
					DispCommentToWin(hDlg);
					SetMainDlgButtonHide(hDlg);
					break;

				case MAIN_SET :
					if((Cur = SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
					{
						CopyPatFromList(Cur, &TmpPat);
						if(DispHostSetDlg(GetMainHwnd(), &TmpPat) == YES)
						{
							UpdatePatToList(Cur, &TmpPat);
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_DELETESTRING, Cur, 0);
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_INSERTSTRING, Cur, (LPARAM)TmpPat.Name);
							ResetAllSel(hDlg, MAIN_LIST);
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_SETSEL, TRUE, Cur);
						}
					}
					DispCommentToWin(hDlg);
					SetMainDlgButtonHide(hDlg);
					break;

				case MAIN_COPY :
					if((Cur = SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
					{
						CopyPatFromList(Cur, &TmpPat);
						AddPatToList(&TmpPat);
						SendDlgItemMessage(hDlg, MAIN_LIST, LB_ADDSTRING, 0, (LPARAM)TmpPat.Name);
						Cur = Patterns - 1;
						ResetAllSel(hDlg, MAIN_LIST);
						SendDlgItemMessage(hDlg, MAIN_LIST, LB_SETSEL, TRUE, Cur);
					}
					DispCommentToWin(hDlg);
					SetMainDlgButtonHide(hDlg);
					break;

				case MAIN_DEL :
					if(SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETSELCOUNT, 0,  0) > 0)
					{
						if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(del_pat_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_85) == YES)
						{
							Cur = SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETCOUNT, 0,  0) - 1;
							for(; Cur >= 0; Cur--)
							{
								if(SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETSEL, Cur,  0) != 0)
								{
									DelPatFromList(Cur);
									SendDlgItemMessage(hDlg, MAIN_LIST, LB_DELETESTRING, Cur, 0);
								}
							}
							DispCommentToWin(hDlg);
							SetMainDlgButtonHide(hDlg);
						}
					}
					break;

				case MAIN_UP :
					if((Cur = SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
					{
						if(ExchangeListItem(Cur, Cur-1) == SUCCESS)
						{
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_RESETCONTENT, 0, 0);
							SendAllPatNames(GetDlgItem(hDlg, MAIN_LIST), LB_ADDSTRING);
							Cur--;
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_SETSEL, TRUE, Cur);
						}
					}
					DispCommentToWin(hDlg);
					SetMainDlgButtonHide(hDlg);
					break;

				case MAIN_DOWN :
					if((Cur = SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
					{
						if(ExchangeListItem(Cur, Cur+1) == SUCCESS)
						{
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_RESETCONTENT, 0, 0);
							SendAllPatNames(GetDlgItem(hDlg, MAIN_LIST), LB_ADDSTRING);
							Cur++;
							SendDlgItemMessage(hDlg, MAIN_LIST, LB_SETSEL, TRUE, Cur);
						}
					}
					DispCommentToWin(hDlg);
					SetMainDlgButtonHide(hDlg);
					break;

				case MAIN_LIST :
					switch(GET_WM_COMMAND_CMD(wParam, lParam))
					{
						case LBN_DBLCLK :
							PostMessage(hDlg, WM_COMMAND, MAKEWORD(MAIN_START, 0), 0);
							break;

						case LBN_SELCHANGE :
						case LBN_SELCANCEL :
							DispCommentToWin(hDlg);
							SetMainDlgButtonHide(hDlg);
							break;
					}
					break;

				case MAIN_QUIT :
					PostMessage(GetMainHwnd(), WM_CLOSE, wParam, lParam);
					break;

				case IDCANCEL :
					if(ExitOnEsc == 1)
						PostMessage(GetMainHwnd(), WM_CLOSE, wParam, lParam);
					break;

			}
			return(TRUE);

		case WM_RETURN_MAIN :
			DeletePatList(&CopyPatList);
			break;

//		case WM_CLOSE :
//			SendMessage(GetMainHwnd(), message, wParam, lParam);
//			break;

		case WM_SIZE :
			SendMessage(GetMainHwnd(), message, wParam, lParam);
			break;

//		case WM_MOUSEMOVE :
//			{
//				Point.x = LOWORD(lParam);
//				Point.y = HIWORD(lParam);
//				GetWindowRect(GetDlgItem(hDlg, MAIN_SEPA), &Rect);
//				if(PtInRect(&Rect, Point))
//				{
//
//				Point.x = 0;
//
//
//				}
//			}
//			break;

		case WM_SIZE_CHANGE :
			pRect = (RECT *)lParam;
			SetWindowPos(hDlg, NULL, 0, 0, pRect->right - pRect->left, pRect->bottom - pRect->top, SWP_NOMOVE|SWP_NOZORDER);
			DlgSizeChange(hDlg, &MainDlgSizeInfo, pRect, (int)wParam);
			free((void*)lParam);
		    break;
	}
    return(FALSE);
}


/*----- メインダイアログのボタンのハイド処理 ----------------------------------
*
*	Parameter
*		HWND hDlg : メインダイアログのウインドウハンドル
*		int Ctrl : リストボックスのコントロール
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void ResetAllSel(HWND hDlg, int Ctrl)
{
	int i;

	i = SendDlgItemMessage(hDlg, Ctrl, LB_GETCOUNT, 0, 0) - 1;
	for(; i >= 0; i--)
		SendDlgItemMessage(hDlg, Ctrl, LB_SETSEL, FALSE, i);
	return;
}


/*----- メインダイアログのボタンのハイド処理 ----------------------------------
*
*	Parameter
*		HWND hDlg : メインダイアログのウインドウハンドル
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void SetMainDlgButtonHide(HWND hDlg)
{
	if(GetSelectedCount() > 0)
	{
		EnableWindow(GetDlgItem(hDlg, MAIN_START), TRUE);
		EnableWindow(GetDlgItem(hDlg, MAIN_SIZE), TRUE);
		EnableWindow(GetDlgItem(hDlg, MAIN_DEL), TRUE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, MAIN_START), FALSE);
		EnableWindow(GetDlgItem(hDlg, MAIN_SIZE), FALSE);
		EnableWindow(GetDlgItem(hDlg, MAIN_DEL), FALSE);
	}

	if(GetSelectedCount() == 1)
	{
		EnableWindow(GetDlgItem(hDlg, MAIN_SET), TRUE);
		EnableWindow(GetDlgItem(hDlg, MAIN_COPY), TRUE);
//		EnableWindow(GetDlgItem(hDlg, MAIN_DEL), TRUE);
		EnableWindow(GetDlgItem(hDlg, MAIN_UP), TRUE);
		EnableWindow(GetDlgItem(hDlg, MAIN_DOWN), TRUE);
	}
	else
	{
		EnableWindow(GetDlgItem(hDlg, MAIN_SET), FALSE);
		EnableWindow(GetDlgItem(hDlg, MAIN_COPY), FALSE);
//		EnableWindow(GetDlgItem(hDlg, MAIN_DEL), FALSE);
		EnableWindow(GetDlgItem(hDlg, MAIN_UP), FALSE);
		EnableWindow(GetDlgItem(hDlg, MAIN_DOWN), FALSE);
	}
	return;
}


/*----- 設定値リストに追加 ----------------------------------------------------
*
*	Parameter
*		COPYPAT *Set : 追加する設定値
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

int AddPatToList(COPYPAT *Set)
{
	int Sts;
	COPYPATLIST *New;
	COPYPATLIST *Last;

	Sts = FAIL;
	if((New = malloc(sizeof(COPYPATLIST))) != NULL)
	{
		memcpy(&New->Set, Set, sizeof(COPYPAT));
		New->Next = NULL;

		if(PatListTop == NULL)
			PatListTop = New;
		else
		{
			Last = PatListTop;
			while(Last->Next != NULL)
				Last = Last->Next;
			Last->Next = New;
		}
		Patterns++;
		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- 設定値リストを更新する ------------------------------------------------
*
*	Parameter
*		int Num : 設定値号番号
*		COPYPAT *Set : 設定値
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int UpdatePatToList(int Num, COPYPAT *Set)
{
	int Sts;
	COPYPATLIST *Pos;

	Sts = FAIL;
	if(Num < Patterns)
	{
		Pos = PatListTop;
		for(; Num > 0; Num--)
			Pos = Pos->Next;

		memcpy(&Pos->Set, Set, sizeof(COPYPAT));
		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- 設定値リストから削除 --------------------------------------------------
*
*	Parameter
*		int Num : 削除する番号
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int DelPatFromList(int Num)
{
	int Sts;
	COPYPATLIST *Pos;
	COPYPATLIST *Prev;

	Sts = FAIL;
	if(Num < Patterns)
	{
		Pos = PatListTop;
		if(Num == 0)
			PatListTop = Pos->Next;
		else
		{
			for(; Num > 0; Num--)
			{
				Prev = Pos;
				Pos = Pos->Next;
			}
			Prev->Next = Pos->Next;
		}
		free(Pos);
		Patterns--;
		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- 設定値リストの２つの項目を入れ換える ----------------------------------
*
*	Parameter
*		int Num1 : 項目１
*		int Num2 : 項目２
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int ExchangeListItem(int Num1, int Num2)
{
	int Sts;
	COPYPATLIST *Pos1;
	COPYPATLIST *Prev1;
	COPYPATLIST *Pos2;
	COPYPATLIST *Prev2;
	COPYPATLIST *Tmp;

	Sts = FAIL;

	if(Num2 < Num1)
		SwapInt(&Num1, &Num2);

	if((Num1 >= 0) && (Num1 < Patterns) && (Num2 < Patterns))
	{
		if(Num1 != Num2)
		{
			Pos2 = PatListTop;
			for(; Num2 > 0; Num2--)
			{
				Prev2 = Pos2;
				Pos2 = Pos2->Next;
			}

			if(Num1 == 0)
			{
				Prev2->Next = Pos2->Next;
				Pos2->Next = PatListTop;
				PatListTop = Pos2;
			}
			else
			{
				Pos1 = PatListTop;
				for(; Num1 > 0; Num1--)
				{
					Prev1 = Pos1;
					Pos1 = Pos1->Next;
				}

				if(Prev2 != Pos1)
				{
					Tmp = Pos1->Next;
					Pos1->Next = Pos2->Next;
					Pos2->Next = Tmp;
					Prev1->Next = Pos2;
					Prev2->Next = Pos1;
				}
				else
				{
					Pos1->Next = Pos2->Next;
					Pos2->Next = Pos1;
					Prev1->Next = Pos2;
				}
			}
		}
		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- 設定を検索する --------------------------------------------------------
*
*	Parameter
*		LPTSTR Name : 設定名
*
*	Return Value
*		int 設定番号 (0～ : -1=見つからない)
*----------------------------------------------------------------------------*/

int SearchPatList(LPTSTR Name)
{
	int Ret;
	int i;
	COPYPATLIST *Pos;

	Ret = -1;
	Pos = PatListTop;
	for(i = 0; i < Patterns; i++)
	{
		if(_tcsicmp(Name, Pos->Set.Name) == 0)
		{
			Ret = i;
			break;
		}
		Pos = Pos->Next;
	}
	return(Ret);
}


/*----- 設定値リストから設定値を取り出す --------------------------------------
*
*	Parameter
*		int Num : 設定値号番号
*		COPYPAT *Set : 設定値をコピーするワーク
*
*	Return Value
*		int ステータス
*			SUCCESS/FAIL
*----------------------------------------------------------------------------*/

int CopyPatFromList(int Num, COPYPAT *Set)
{
	int Sts;
	COPYPATLIST *Pos;

	Sts = FAIL;
	if((Num >= 0) && (Num < Patterns))
	{
		Pos = PatListTop;
		for(; Num > 0; Num--)
			Pos = Pos->Next;

		memcpy(Set, &Pos->Set, sizeof(COPYPAT));
		Sts = SUCCESS;
	}
	return(Sts);
}


/*----- バックアップ元、先が空白のパターンかチェック --------------------------
*
*	Parameter
*		int Num : 設定値号番号
*
*	Return Value
*		BOOL ステータス
*			TRUE/FALSE
*----------------------------------------------------------------------------*/
static BOOL CheckNullPat(int Num)
{
	BOOL Sts;
	COPYPATLIST *Pos;

	Sts = FALSE;
	if((Num >= 0) && (Num < Patterns))
	{
		Pos = PatListTop;
		for(; Num > 0; Num--)
			Pos = Pos->Next;

		if((_tcslen(Pos->Set.Src) == 0) || (_tcslen(Pos->Set.Dst) == 0))
			Sts = TRUE;
	}
	return(Sts);
}



/*----- 設定値リストからコメントを取り出す ------------------------------------
*
*	Parameter
*		int Num : 設定値号番号
*
*	Return Value
*		LPTSTR コメント
*			NULL/""=なし
*----------------------------------------------------------------------------*/

LPTSTR GetPatComment(int Num)
{
	LPTSTR Ret;
	LPTSTR Get;
	LPTSTR Put;
	int i;
	COPYPATLIST *Pos;
	unsigned int Max;
	unsigned int Len;

	Ret = NULL;
	if(Num < Patterns)
	{
		Pos = PatListTop;
		for(i = 0; i < Num; i++)
			Pos = Pos->Next;

		Get = Pos->Set.Comment;
		if(Get != NULL)
		{
			Max = MY_MAX_PATH;
			Len = 0;
			Ret = malloc(Max * sizeof(_TCHAR));
			Put = Ret;
			while(*Get != NUL)
			{
				if((Len + 3) >= Max)
				{
					Max += MY_MAX_PATH;
					Ret = realloc(Ret, Max * sizeof(_TCHAR));
					Put = Ret + Len;
				}

				if(*Get == _T('$'))
				{
					Get++;
					if(*Get == NUL)
						break;
					else if(*Get == _T('$'))
					{
						*Put++ = _T('$');
						Len++;
					}
					else if(toupper(*Get) == _T('L'))
					{
						if(Pos->Set.ChkVolLabel == 0)
						{
							*Put++ = _T('-');
							*Put++ = _T('-');
							Len += 2;
						}
						else
						{
							if((Len + _tcslen(Pos->Set.VolLabel) + 1) >= Max)
							{
								Max += _tcslen(Pos->Set.VolLabel);
								Ret = realloc(Ret, Max * sizeof(_TCHAR));
								Put = Ret + Len;
							}
							_tcscpy(Put, Pos->Set.VolLabel);
							Put += _tcslen(Pos->Set.VolLabel);
							Len += _tcslen(Pos->Set.VolLabel);
						}
					}
					else
					{
						*Put++ = _T('$');
						*Put++ = *Get;
						Len += 2;
					}
					Get++;
				}
				else
				{
					*Put++ = *Get++;
					Len++;
				}
			}
			*Put = NUL;
		}
	}
	return(Ret);
}


/*----- デフォルト設定値を取り出す --------------------------------------------
*
*	Parameter
*		COPYPAT *Set : 設定値をコピーするワーク
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void CopyDefaultPat(COPYPAT *Set)
{
	_tcscpy(Set->Name, _T(""));
	_tcscpy(Set->Comment, _T(""));
	memcpy(Set->Src, _T("\0\0"), 2 * sizeof(_TCHAR));
	_tcscpy(Set->Dst, _T(""));
	memcpy(Set->IgnDir, _T("\0\0"), 2 * sizeof(_TCHAR));
	memcpy(Set->IgnFile, _T("\0\0"), 2 * sizeof(_TCHAR));
	_tcscpy(Set->VolLabel, _T(""));
	_tcscpy(Set->SoundFile, _T(""));
	Set->ForceCopy = NO;
	Set->DelDir = YES;
	Set->DelFile = YES;
	Set->IgnoreErr = YES;
	Set->NotifyDel = YES;
	Set->NotifyOvw = NO;
	Set->IgnNoDel = NO;
	Set->NewOnly = NO;
	Set->Wait = 0;
	Set->ChkVolLabel = NO;
	Set->UseTrashCan = NO;
	Set->Tolerance = 2;
	Set->AutoClose = 0;
	Set->IgnSystemFile = NO;
	Set->IgnHiddenFile = NO;
	Set->IgnBigFile = NO;
	Set->IgnBigSize = 100;
	Set->IgnAttr = NO;
	Set->Sound = NO;
	Set->IntervalTime = -60;
	Set->NoMakeTopDir = NO;
	Set->IgnTime = NO;
	Set->ShowComment = NO;
    Set->NextDstNum = 0;
    Set->NextDst = NULL;
    Set->PatNum = 0;
	return;
}


/*----- 設定名一覧をウィンドウに送る ------------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		int Cmd : コマンド
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void SendAllPatNames(HWND hWnd, int Cmd)
{
	int i;
	COPYPATLIST *Pos;

	Pos = PatListTop;
	for(i = 0; i < Patterns; i++)
	{
		SendMessage(hWnd, Cmd, 0, (LPARAM)Pos->Set.Name);
		Pos = Pos->Next;
	}
	return;
}


/*----- int値の入れ替え -------------------------------------------------------
*
*	Parameter
*		int *Num1 : 数値１
*		int *Num2 : 数値２
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void SwapInt(int *Num1, int *Num2)
{
	int Tmp;

	Tmp = *Num1;
	*Num1 = *Num2;
	*Num2 = Tmp;
	return;
}


/*----- 設定の数を返す --------------------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		int 設定の数
*----------------------------------------------------------------------------*/

int GetPatterns(void)
{
	return(Patterns);
}


/*----- 選択されている設定の数を返す ------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		int 設定の数
*----------------------------------------------------------------------------*/

int GetSelectedCount(void)
{
	return(SendDlgItemMessage(hWndMainDlg, MAIN_LIST, LB_GETSELCOUNT, 0, 0));
}


/*----- 選択されている設定のリストを作成する ----------------------------------
*
*	Parameter
*		COPYPATLIST **Top : リストの先頭
*		int All : 全項目を選ぶかどうか (YES/NO)
*
*	Return Value
*		int 設定の数
*----------------------------------------------------------------------------*/

static int GetSelectedPat(COPYPATLIST **Top, int All)
{
	COPYPATLIST *Pos;
	COPYPATLIST *Prev;
	int Cur;
	int Num;

	Num = 0;
	for(Cur = 0; Cur < Patterns; Cur++)
	{
		if((All == YES) || 
		   (SendDlgItemMessage(hWndMainDlg, MAIN_LIST, LB_GETSEL, Cur, 0) > 0))
		{
			if(CheckNullPat(Cur) == FALSE)
			{
				if((Pos = malloc(sizeof(COPYPATLIST))) != NULL)
				{
					Num++;
					CopyPatFromList(Cur, &Pos->Set);
					Pos->Next = NULL;
					if(*Top == NULL)
						*Top = Pos;
					else
						Prev->Next = Pos;
					Prev = Pos;
				}
			}
		}
	}
	return(Num);
}


/*----- 指定された名前のリストを作成する --------------------------------------
*
*	Parameter
*		COPYPATLIST **Top : リストの先頭
*		LPTSTR Name : 名前
*
*	Return Value
*		int 設定の数
*----------------------------------------------------------------------------*/

static int GetNamedPat(COPYPATLIST **Top, LPTSTR Name)
{
	COPYPATLIST *pNew;
	COPYPATLIST *pPrv;
	COPYPATLIST *Pos = PatListTop;
	int Num = 0;
	int Cur = 0;

	for (Cur = 0; Cur < Patterns; Cur++)
	{
		if(CheckFname(Pos->Set.Name, Name) == YES)
		{
			if ((pNew = malloc(sizeof(COPYPATLIST))) != NULL)
			{
				Num++;
				CopyPatFromList(Cur, &pNew->Set);
				pNew->Next = NULL;
				if (*Top == NULL)
					*Top = pNew;
				else
					pPrv->Next = pNew;
				pPrv = pNew;
			}
		}
		Pos = Pos->Next;
	}
	if (Num == 0)
		DispErrorBox(MSGJPN_20, Name);

	return(Num);
}


/*----- 選択されている設定のリストを削除する ----------------------------------
*
*	Parameter
*		COPYPATLIST **Top : リストの先頭
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void DeletePatList(COPYPATLIST **Top)
{
	COPYPATLIST *Pos;
	COPYPATLIST *Prev;

	Pos = *Top;
	while(Pos != NULL)
	{
		Prev = Pos->Next;
		free(Pos);
		Pos = Prev;
	}
	*Top = NULL;

	return;
}


/*----- バックアップ開始確認ダイアログ ----------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		COPYPATLIST *Pat : リストの先頭
*
*	Return Value
*		int ステータス
*			YES/NO=中止
*
*	Note
*		確認を行なわない指定がされている時は常にYES
*----------------------------------------------------------------------------*/

int NotifyBackup(HWND hWnd, COPYPATLIST *Pat)
{
	int Sts;

	Sts = YES;

	if(ExecOption & OPT_CLOSE)
		AutoClose = 1;
	else
		AutoClose = Pat->Set.AutoClose;

	Sound = Pat->Set.Sound;
	_tcscpy(SoundFile, Pat->Set.SoundFile);
	IntervalTime = Pat->Set.IntervalTime;

	if(AskNoNotify() == NO)
	{
		TmpPatList = Pat;
		Sts = DialogBox(GetBupInst(), MAKEINTRESOURCE(tr_notify_dlg), hWnd, NotifyDlgWndProc);
	}
	return(Sts);
}


/*----- バックアップ開始確認ダイアログのメッセージ処理 ------------------------
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

static LRESULT CALLBACK NotifyDlgWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int CurNum;

	switch (message)
	{
		case WM_INITDIALOG :
			CurNum = 0;
			SendDlgItemMessage(hDlg, TRNOT_SRCLIST, LB_SETHORIZONTALEXTENT, 2048, 0);
			DispSrcAndDest(hDlg, TmpPatList, CurNum);

			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_93);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_94);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_95);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_96);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_97);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_101);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_102);
			SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_SETCURSEL, AutoClose, 0);

			/* ダイアログサイズの初期化 */
			DlgSizeInit(hDlg, &NotifyDlgSizeInfo, &NotifyDlgSize, TRUE);
			return(TRUE);

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK :
					AutoClose = SendDlgItemMessage(hDlg, TRNOT_SYSTEM, CB_GETCURSEL, 0, 0);
					AskDlgSize(&NotifyDlgSizeInfo, &NotifyDlgSize);
					EndDialog(hDlg, YES);
					break;

				case IDCANCEL :
					AskDlgSize(&NotifyDlgSizeInfo, &NotifyDlgSize);
					EndDialog(hDlg, NO);
					break;

				case TRNOT_PREV :
					CurNum--;
					DispSrcAndDest(hDlg, TmpPatList, CurNum);
					break;

				case TRNOT_NEXT :
					CurNum++;
					DispSrcAndDest(hDlg, TmpPatList, CurNum);
					break;
			}
			return(TRUE);

		case WM_SIZING :
			DlgSizeChange(hDlg, &NotifyDlgSizeInfo, (RECT *)lParam, (int)wParam);
			break;
	}
    return(FALSE);
}


/*----- バックアップ元とバックアップ先を表示する ------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		COPYPATLIST *Pat : リストの先頭
*		int Num : 設定の数
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

static void DispSrcAndDest(HWND hDlg, COPYPATLIST *Pat, int Num)
{
	LPTSTR Pos;

	if(Num == 0)
		EnableWindow(GetDlgItem(hDlg, TRNOT_PREV), FALSE);
	else
		EnableWindow(GetDlgItem(hDlg, TRNOT_PREV), TRUE);

	for(; Num > 0; Num--)
		Pat = Pat->Next;

	if(Pat->Next == NULL)
		EnableWindow(GetDlgItem(hDlg, TRNOT_NEXT), FALSE);
	else
		EnableWindow(GetDlgItem(hDlg, TRNOT_NEXT), TRUE);

    SendDlgItemMessage(hDlg, TRNOT_DST, WM_SETTEXT, 0, (LPARAM)GetSpecifiedStringFromMultiString(Pat->Set.Dst, Pat->Set.NextDstNum));
	SendDlgItemMessage(hDlg, TRNOT_SRCLIST, LB_RESETCONTENT, 0, 0);
	Pos = Pat->Set.Src;
	while(*Pos != NUL)
	{
		SendDlgItemMessage(hDlg, TRNOT_SRCLIST, LB_ADDSTRING, 0, (LPARAM)Pos);
		Pos = _tcschr(Pos, NUL) + 1;
	}
	if(Pat->Set.ChkVolLabel == 0)
		SendDlgItemMessage(hDlg, TRNOT_VOL, WM_SETTEXT, 0, (LPARAM)MSGJPN_112);
	else
		SendDlgItemMessage(hDlg, TRNOT_VOL, WM_SETTEXT, 0, (LPARAM)Pat->Set.VolLabel);
	return;
}


/*----- メインダイアログのサイズを保存 ----------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
void SaveMainDlgSize(void)
{
	AskDlgSize(&MainDlgSizeInfo, &MainDlgSize);
}


/*----- メインダイアログの最小サイズを返す ------------------------------------
*
*	Parameter
*		POINT *Point : 最小サイズを返すワーク
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
void AsdMainDlgMinSize(POINT *Point)
{
	AskDlgMinSize(&MainDlgSizeInfo, Point);
	return;
}


/*----- コメントウインドウにコメントを表示 ------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		int Num : 項目番号
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
static void DispCommentToWin(HWND hDlg)
{
	LPTSTR Str;
	int Num;
	HWND hItem;

	if(ShowComment == 2)
	{
		hItem = GetDlgItem(hDlg, MAIN_COMMENT);
		if(SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETSELCOUNT, 0,  0) == 0)
			SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)_T(""));
		else
		{
			Num = SendDlgItemMessage(hDlg, MAIN_LIST, LB_GETCARETINDEX, 0, 0);
			if(hItem != NULL)
			{
				Str = GetPatComment(Num);
				if(Str != NULL)
					SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)Str);
				else
					SendMessage(hItem, WM_SETTEXT, 0, (LPARAM)_T(""));
				free(Str);
			}
		}
	}
	return;
}



/*----- コメント表示ウインドウのコールバック関数 ------------------------------
*
*	Parameter
*		HWND hDlg : ウインドウハンドル
*		UINT message : メッセージ番号
*		WPARAM wParam : メッセージの WPARAM 引数
*		LPARAM lParam : メッセージの LPARAM 引数
*
*	Return Value
*		BOOL TRUE/FALSE
*----------------------------------------------------------------------------*/

static BOOL CALLBACK ShowCommentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int	Num;
	LPTSTR Str;

	switch (message)
	{
		case WM_INITDIALOG :
			PlaySound(_T("SystemQuestion"), NULL, SND_ALIAS|SND_ASYNC);
			Num = SendDlgItemMessage(hWndMainDlg, MAIN_LIST, LB_GETCARETINDEX, 0, 0);
			Str = GetPatComment(Num);
			if(Str != NULL)
			{
				SendDlgItemMessage(hDlg, SHOWCOM_TEXT, WM_SETTEXT, 0, (LPARAM)Str);
				free(Str);
			}
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
			}
			return(TRUE);
	}
    return(FALSE);
}


/*----- バックアップ先番号をインクリメントする ------------------------------
*
*	Parameter
*		int PatNum : パターン番号
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
void IncrementDstNum(int PatNum)
{
    COPYPATLIST *Pat = PatListTop;
    for(; PatNum > 0; PatNum--)
    {
        Pat = Pat->Next;
        if(Pat == NULL)
        {
            break;
        }
    }
    if(Pat != NULL)
    {
        Pat->Set.NextDstNum += 1;
        if(Pat->Set.NextDstNum >= StrMultiCount(Pat->Set.Dst))
        {
            Pat->Set.NextDstNum = 0;
        }
    }
}


