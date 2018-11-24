/*===========================================================================
/
/									Backup
/							���X�g�{�b�N�X�e�B�b�v�X
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

#include "common.h"
#include "resource.h"


/*===== ��` =====*/

/* ���݂̏�� */
typedef enum {
	ERASED,			/* �\�����Ă��Ȃ� */
	PENDING,		/* �\������܂ł̎��ԑ҂� */
	DISPLAYED		/* �\���� */
} TIPSTATUS;


/*===== �v���g�^�C�v =====*/

int InitListBoxTips(HWND hWnd, HINSTANCE hInst);
void DeleteListBoxTips(void);
void EraseListBoxTips(void);
void CheckTipsDisplay(LPARAM lParam);
static void TipsShow(POINT Pos, LPTSTR lpszTitleText, TIPSTATUS Status);
static int CellRectFromPoint(HWND hWnd, POINT point, RECT *cellrect);
static LRESULT CALLBACK TitleTipWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


/*===== ���[�J���ȃ��[�N =====*/

static HWND hWndTips;				/* tips�̃E�C���h�E�n���h�� */
static HWND hWndLbox;				/* LISTBOX�̃E�C���h�E�n���h�� */
static UINT TimerID;				/* �^�C�}ID */
static TIPSTATUS Status = ERASED;	/* ���݂̏�� */
static POINT MousePos;				/* �}�E�X�̈ʒu�ۑ��p */
static int ItemNum;					/* �}�E�X�ʒu��LISTBOX���ڔԍ� */
static RECT cur_rect;


/*-----------------------------------------------------------------------------
 ����	:	���X�g�{�b�N�X�e�B�b�v�X�̃E�C���h�E���쐬
 ����	:	hWnd	�e�E�C���h�E�̃n���h��
			hInst	�C���X�^���X�n���h��
 �߂�l	:	int		�X�e�[�^�X (SUCCESS/FAIL)
 ���l	:
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
 ����	:	���X�g�{�b�N�X�e�B�b�v�X�̃E�C���h�E���폜
 ����	:	�Ȃ�
 �߂�l	:	�Ȃ�
 ���l	:
-----------------------------------------------------------------------------*/
void DeleteListBoxTips(void)
{
	if(hWndTips != NULL)
		DestroyWindow(hWndTips);
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	���X�g�{�b�N�X�e�B�b�v�X�̃E�C���h�E������
 ����	:	�Ȃ�
 �߂�l	:	�Ȃ�
 ���l	:
-----------------------------------------------------------------------------*/
void EraseListBoxTips(void)
{
	ReleaseCapture();
	ShowWindow(hWndTips, SW_HIDE);
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	���X�g�{�b�N�X�e�B�b�v�X�̕\���`�F�b�N
 ����	:	lParam	WM_MOUSEMOVE��LPARAM�l
 �߂�l	:	�Ȃ�
 ���l	:
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
		/* ���ԑ҂��̊ԂɃ}�E�X���������ꂽ����� */
		KillTimer(hWndLbox, TimerID);
		Status = ERASED;
	}

	if(Status == ERASED)
	{
		if(row != -1)
		{
			/* LISTBOX�̍��ڂ̒��ɂ���̂ŁA�\�����ԑ҂��Ɉڍs */
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
			/* �\�����ɍ��ڂ̊O�֏o���̂ŏ��� */
			EraseListBoxTips();
			Status = ERASED;
		}
	}
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	���X�g�{�b�N�X�e�B�b�v�X��\��
 ����	:	Pos				�\���ʒu
			lpszTitleText	������
			Status			���݂̏��
 �߂�l	:	�Ȃ�
 ���l	:
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
			/* ListBox�E�C���h�E�̃t�H���gTIPS�E�C���h�E�ɃR�s�[ */
			dc = GetDC(hWndLbox);
			pFont = (HFONT)SendMessage(hWndLbox, WM_GETFONT, 0, 0);
			ReleaseDC(hWndLbox, dc);

			dc = GetDC(hWndTips);
			pFontDC = SelectObject(dc, pFont);

			/* �\���F��ݒ� */
			SetTextColor(dc, GetSysColor(COLOR_INFOTEXT));
			SetBkMode(dc, TRANSPARENT);

			/* �E�C���h�E�̑傫����ݒ� */
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

			/* �������\�� */
			rectDisplay.right -= rectDisplay.left;
			rectDisplay.bottom -= rectDisplay.top;
			rectDisplay.left = 0;
			rectDisplay.top = 0;
			DrawText(dc, lpszTitleText, _tcslen(lpszTitleText), &rectDisplay, DT_LEFT);

			/* ���ڂ̊O�֏o�����Ƃ��m���Ɍ��o�ł���悤�ɃL���v�`�� */
			SetCapture(hWndLbox);

			SelectObject(dc, pFontDC);
			ReleaseDC(hWndTips, dc);
		}
	}
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	���X�g�{�b�N�X�e�B�b�v�X��\������ʒu��Ԃ�
 ����	:	hWnd		ListBox�̃E�C���h�E�n���h��
			point		�J�[�\���̈ʒu
			cellrect	�A�C�e���̋�`��Ԃ����[�N
 �߂�l	:	int		�s�ԍ� (-1=�Y���Ȃ�)
 ���l	:
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
					/* LISTBOX��ITEM�̈ꕔ���������Ă��Ȃ��Ƃ��̂��߂̏��u */
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
 ����	:	���X�g�{�b�N�X�e�B�b�v�X�E�C���h�E�̃R�[���o�b�N
 ����	:	hWnd	�E�C���h�E�n���h��
			message	���b�Z�[�W�ԍ�
			wParam	���b�Z�[�W�� WPARAM ����
			lParam	���b�Z�[�W�� LPARAM ����
 �߂�l	:	���b�Z�[�W�ɑΉ�����߂�l
 ���l	:
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


