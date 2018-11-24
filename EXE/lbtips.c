/*===========================================================================
/
/									Backup
/							リストボックスティップス
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
#include <windowsx.h>
#include <commctrl.h>

#include "common.h"
#include "resource.h"


/*===== 定義 =====*/

/* 現在の状態 */
typedef enum {
	ERASED,			/* 表示していない */
	PENDING,		/* 表示するまでの時間待ち */
	DISPLAYED		/* 表示中 */
} TIPSTATUS;


/*===== プロトタイプ =====*/

int InitListBoxTips(HWND hWnd, HINSTANCE hInst);
void DeleteListBoxTips(void);
void EraseListBoxTips(void);
void CheckTipsDisplay(LPARAM lParam);
static void TipsShow(POINT Pos, LPTSTR lpszTitleText, TIPSTATUS Status);
static int CellRectFromPoint(HWND hWnd, POINT point, RECT *cellrect);
static LRESULT CALLBACK TitleTipWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


/*===== ローカルなワーク =====*/

static HWND hWndTips;				/* tipsのウインドウハンドル */
static HWND hWndLbox;				/* LISTBOXのウインドウハンドル */
static UINT TimerID;				/* タイマID */
static TIPSTATUS Status = ERASED;	/* 現在の状態 */
static POINT MousePos;				/* マウスの位置保存用 */
static int ItemNum;					/* マウス位置のLISTBOX項目番号 */
static RECT cur_rect;


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスのウインドウを作成
 引数	:	hWnd	親ウインドウのハンドル
			hInst	インスタンスハンドル
 戻り値	:	int		ステータス (SUCCESS/FAIL)
 備考	:	
-----------------------------------------------------------------------------*/
int InitListBoxTips(HWND hWnd, HINSTANCE hInst)
{
	WNDCLASSEX	wClass;
	int			Ret;

	Ret = FAIL;

	hWndLbox = hWnd;

	wClass.cbSize		 = sizeof(WNDCLASSEX);
	wClass.style		 = 0;
	wClass.lpfnWndProc	 = TitleTipWndProc;
	wClass.cbClsExtra	 = 0;
	wClass.cbWndExtra	 = 0;
	wClass.hInstance	 = hInst;
	wClass.hIcon		 = NULL;
	wClass.hCursor		 = NULL;
	wClass.hbrBackground = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_INFOBK));
	wClass.lpszMenuName  = NULL;
	wClass.lpszClassName = _T("BupTitleTip");
	wClass.hIconSm		 = NULL;
	RegisterClassEx(&wClass);

	hWndTips = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
				_T("BupTitleTip"), NULL,
				WS_BORDER | WS_POPUP,
				0, 0, 0, 0,
				hWnd, NULL, hInst, NULL);

	if(hWndTips != NULL)
		Ret = SUCCESS;

	return(Ret);
}


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスのウインドウを削除
 引数	:	なし
 戻り値	:	なし
 備考	:	
-----------------------------------------------------------------------------*/
void DeleteListBoxTips(void)
{
	if(hWndTips != NULL)
		DestroyWindow(hWndTips);
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスのウインドウを消去
 引数	:	なし
 戻り値	:	なし
 備考	:	
-----------------------------------------------------------------------------*/
void EraseListBoxTips(void)
{
	ReleaseCapture();
	ShowWindow(hWndTips, SW_HIDE);
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスの表示チェック
 引数	:	lParam	WM_MOUSEMOVEのLPARAM値
 戻り値	:	なし
 備考	:	
-----------------------------------------------------------------------------*/
void CheckTipsDisplay(LPARAM lParam)
{
	RECT	cellrect;
	int		row;

	MousePos.x = LOWORD(lParam);
	MousePos.y = HIWORD(lParam);
	row = CellRectFromPoint(hWndLbox, MousePos, &cellrect);

	if(Status == PENDING)
	{
		/* 時間待ちの間にマウスが動かされたら解除 */
		KillTimer(hWndLbox, TimerID);
		Status = ERASED;
	}

	if(Status == ERASED)
	{
		if(row != -1)
		{
			/* LISTBOXの項目の中にあるので、表示時間待ちに移行 */
			cur_rect = cellrect;
			TimerID = SetTimer(hWndTips, TIMER_TIPS, 500, NULL);
			Status = PENDING;
			ItemNum = row;
		}
	}
	else if(Status == DISPLAYED)
	{
		if(PtInRect(&cur_rect, MousePos) == FALSE)
		{
			/* 表示中に項目の外へ出たので消去 */
			EraseListBoxTips();
			Status = ERASED;
		}
	}
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスを表示
 引数	:	Pos				表示位置
			lpszTitleText	文字列
			Status			現在の状態
 戻り値	:	なし
 備考	:	
-----------------------------------------------------------------------------*/
static void TipsShow(POINT Pos, LPTSTR lpszTitleText, TIPSTATUS Status)
{
	HDC		dc;
	HFONT	pFont;
	HFONT	pFontDC;
	RECT	rectDisplay;

	if(Status == PENDING)
	{
		if(GetFocus() != NULL)
		{
			/* ListBoxウインドウのフォントTIPSウインドウにコピー */
			dc = GetDC(hWndLbox);
			pFont = (HFONT)SendMessage(hWndLbox, WM_GETFONT, 0, 0);
			ReleaseDC(hWndLbox, dc);

			dc = GetDC(hWndTips);
			pFontDC = SelectObject(dc, pFont);

			/* 表示色を設定 */
			SetTextColor(dc, GetSysColor(COLOR_INFOTEXT));
			SetBkMode(dc, TRANSPARENT);

			/* ウインドウの大きさを設定 */
			ClientToScreen(hWndLbox, &Pos);
#if 0
			rectDisplay.top = Pos.y + GetSystemMetrics(SM_CYCURSOR);
#else
			rectDisplay.top = Pos.y + 21;
#endif
			rectDisplay.left = Pos.x;
			rectDisplay.right = rectDisplay.left + 1024;

			DrawText(dc, lpszTitleText, -1, &rectDisplay, DT_CALCRECT);
			rectDisplay.right += 2;
			rectDisplay.bottom += 2;

			SetWindowPos(hWndTips, HWND_TOPMOST, 
				rectDisplay.left, rectDisplay.top, 
				rectDisplay.right - rectDisplay.left, 
				rectDisplay.bottom - rectDisplay.top, 
				SWP_SHOWWINDOW|SWP_NOACTIVATE );

			/* 文字列を表示 */
			rectDisplay.right -= rectDisplay.left;
			rectDisplay.bottom -= rectDisplay.top;
			rectDisplay.left = 0;
			rectDisplay.top = 0;
			DrawText(dc, lpszTitleText, _tcslen(lpszTitleText), &rectDisplay, DT_LEFT);

			/* 項目の外へ出たことを確実に検出できるようにキャプチャ */
			SetCapture(hWndLbox);

			SelectObject(dc, pFontDC);
			ReleaseDC(hWndTips, dc);
		}
	}
	return;
}


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスを表示する位置を返す
 引数	:	hWnd		ListBoxのウインドウハンドル
			point		カーソルの位置
			cellrect	アイテムの矩形を返すワーク
 戻り値	:	int		行番号 (-1=該当なし)
 備考	:	
-----------------------------------------------------------------------------*/
static int CellRectFromPoint(HWND hWnd, POINT point, RECT *cellrect)
{
	RECT	Rect;
	RECT	RectWin;
	int		row;
	int		max;
	int		Ret;

	Ret = -1;

	GetClientRect(hWnd, &RectWin);
	if(PtInRect(&RectWin, point))
	{
		row = SendMessage(hWnd, LB_GETTOPINDEX, 0, 0);
		max = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
		for(; row < max; row++)
		{
			if(SendMessage(hWnd, LB_GETITEMRECT, row, (LPARAM)&Rect) != LB_ERR)
			{
				if(PtInRect(&Rect, point))
				{
					/* LISTBOXのITEMの一部しか見えていないときのための処置 */
					if(Rect.left < RectWin.left)
						Rect.left = RectWin.left;
					if(Rect.top < RectWin.top)
						Rect.top = RectWin.top;
					if(Rect.right > RectWin.right)
						Rect.right = RectWin.right;
					if(Rect.bottom > RectWin.bottom)
						Rect.bottom = RectWin.bottom;

					*cellrect = Rect;
					Ret = row;
					break;
				}
			}
			else
				break;
		}
	}
	return(Ret);
}


/*-----------------------------------------------------------------------------
 説明	:	リストボックスティップスウインドウのコールバック
 引数	:	hWnd	ウインドウハンドル
			message	メッセージ番号
			wParam	メッセージの WPARAM 引数
			lParam	メッセージの LPARAM 引数
 戻り値	:	メッセージに対応する戻り値
 備考	:	
-----------------------------------------------------------------------------*/
static LRESULT CALLBACK TitleTipWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	_TCHAR	*Text;
	POINT	Point;

	switch(message)
	{
		case WM_TIMER :
			if(Status == PENDING)
			{
				KillTimer(hWndLbox, TimerID);
				GetCursorPos(&Point);
				ScreenToClient(hWndLbox, &Point);
				if(PtInRect(&cur_rect, Point))
				{
					Text = GetPatComment(ItemNum);
					if((Text != NULL) && (_tcslen(Text) > 0))
						TipsShow(MousePos, Text, Status);
					free(Text);
					Status = DISPLAYED;
				}
				else
					Status = ERASED;
			}
			break;

	}
	return(DefWindowProc(hWnd, message, wParam, lParam));
}


