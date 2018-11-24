/*===========================================================================
/
/									Backup
/						�_�C�A���O�{�b�N�X�̃T�C�Y�ύX����
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

#define	STRICT
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <windowsx.h>

#include "common.h"
#include "resource.h"


/*---------------------------------------------------------------------------*/
/* �T�C�Y�ύX�\�Ƃ���_�C�A���O�{�b�N�X�� WS_CLIPCHILDREN �X�^�C����ǉ��� */
/* �邱�� */
/*---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 ����	:	�_�C�A���O�{�b�N�X�̏����T�C�Y��ݒ�
 ����	:	hDlg	�E�C���h�E�n���h��
			Dt		�_�C�A���O�T�C�Y�ݒ�p�����[�^
			Size	�_�C�A���O�̃T�C�Y
			Move	MoveWindow���s�Ȃ����ǂ���
 �߂�l :	�Ȃ�
-----------------------------------------------------------------------------*/
void DlgSizeInit(HWND hDlg, DIALOGSIZE *Dt, SIZE *Size, BOOL Move)
{
	RECT	Rect;

	GetWindowRect(hDlg, &Rect);

	Dt->MinSize.cx = Rect.right - Rect.left;
	Dt->MinSize.cy = Rect.bottom - Rect.top;
	Dt->CurSize.cx = Dt->MinSize.cx;
	Dt->CurSize.cy = Dt->MinSize.cy;

	if(Size->cx != -1)
	{
		Rect.right = Rect.left + Size->cx;
		Rect.bottom = Rect.top + Size->cy;

		DlgSizeChange(hDlg, Dt, &Rect, WMSZ_BOTTOMRIGHT);

		if(Move == TRUE)
		{
			GetWindowRect(hDlg, &Rect);
			MoveWindow(hDlg, Rect.left, Rect.top, Dt->CurSize.cx, Dt->CurSize.cy, TRUE);
		}
	}
	if(Move != TRUE)
	{
		MoveWindow(hDlg, 0, 0, Dt->CurSize.cx, Dt->CurSize.cy, TRUE);
	}
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	�_�C�A���O�{�b�N�X�̃T�C�Y��Ԃ�
 ����	:	Dt		�_�C�A���O�T�C�Y�ݒ�p�����[�^
			Size	�_�C�A���O�̃T�C�Y��Ԃ����[�N
 �߂�l :	�Ȃ�
-----------------------------------------------------------------------------*/
void AskDlgSize(DIALOGSIZE *Dt, SIZE *Size)
{
	Size->cx = Dt->CurSize.cx;
	Size->cy = Dt->CurSize.cy;
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	�_�C�A���O�{�b�N�X�̍ŏ��T�C�Y��Ԃ�
 ����	:	Dt		�_�C�A���O�T�C�Y�ݒ�p�����[�^
			Point	�_�C�A���O�̍ŏ��T�C�Y��Ԃ����[�N
 �߂�l :	�Ȃ�
-----------------------------------------------------------------------------*/
void AskDlgMinSize(DIALOGSIZE *Dt, POINT *Point)
{
	Point->x = Dt->MinSize.cx;
	Point->y = Dt->MinSize.cy;
	return;
}


/*-----------------------------------------------------------------------------
 ����	:	�_�C�A���O�{�b�N�X�̃T�C�Y�ύX����
 ����	:	hDlg	�E�C���h�E�n���h��
			Dt		�_�C�A���O�T�C�Y�ݒ�p�����[�^
			New		�V�����_�C�A���O�̃T�C�Y
			Flg		�T�C�Y�ύX���� (WMSZ_xxx)
 �߂�l :	�Ȃ�
 ���l	:	�_�C�A���O�{�b�N�X�� WM_SIZING ���b�Z�[�W���������ɌĂԎ�
-----------------------------------------------------------------------------*/
void DlgSizeChange(HWND hDlg, DIALOGSIZE *Dt, RECT *New, int Flg)
{
	int		*Win;
	RECT	Rect;
	POINT	Point;

	/* �ŏ��T�C�Y��菬�����Ȃ�Ȃ��悤�ɂ��鏈�� */
	if((New->right - New->left) < Dt->MinSize.cx)
	{
//		if((Flg == WMSZ_LEFT) || (Flg == WMSZ_TOPLEFT) || (Flg == WMSZ_BOTTOMLEFT))
//			New->left = New->right - Dt->MinSize.cx;
//		else
			New->right = New->left + Dt->MinSize.cx;
	}
	if((New->bottom - New->top) < Dt->MinSize.cy)
	{
//		if((Flg == WMSZ_TOP) || (Flg == WMSZ_TOPLEFT) || (Flg == WMSZ_TOPRIGHT))
//			New->top = New->bottom - Dt->MinSize.cy;
//		else
			New->bottom = New->top + Dt->MinSize.cy;
	}

	/* ���������Ɉړ����镔�i�̏��� */
	if(Dt->CurSize.cx != New->right - New->left)
	{
		Win = Dt->HorMoveList;
		while(*Win != -1)
		{
			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);
			Point.x = Rect.left + (New->right - New->left) - Dt->CurSize.cx;
			Point.y = Rect.top;
			ScreenToClient(hDlg, &Point);

			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);
			Rect.right -= Rect.left;
			Rect.bottom -= Rect.top;

			Rect.left = Point.x;
			Rect.top = Point.y;
			MoveWindow(GetDlgItem(hDlg, *Win), Rect.left, Rect.top, Rect.right, Rect.bottom, TRUE);

			Win++;
		}
	}

	/* ���������Ɉړ����镔�i�̏��� */
	if(Dt->CurSize.cy != New->bottom - New->top)
	{
		Win = Dt->VarMoveList;
		while(*Win != -1)
		{
			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);
			Point.x = Rect.left;
			Point.y = Rect.top + (New->bottom - New->top) - Dt->CurSize.cy;
			ScreenToClient(hDlg, &Point);

			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);
			Rect.right -= Rect.left;
			Rect.bottom -= Rect.top;

			Rect.left = Point.x;
			Rect.top = Point.y;
			MoveWindow(GetDlgItem(hDlg, *Win), Rect.left, Rect.top, Rect.right, Rect.bottom, TRUE);

			Win++;
		}
	}

	/* �傫����ύX���镔�i�̏��� */
	if((Dt->CurSize.cx != New->right - New->left) ||
	   (Dt->CurSize.cy != New->bottom - New->top))
	{
		Win = Dt->ResizeList;
		while(*Win != -1)
		{
			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);

			Rect.right = (Rect.right - Rect.left) + (New->right - New->left) - Dt->CurSize.cx;
			Rect.bottom = (Rect.bottom - Rect.top) + (New->bottom - New->top) - Dt->CurSize.cy;

			Point.x = Rect.left;
			Point.y = Rect.top;
			ScreenToClient(hDlg, &Point);
			Rect.left = Point.x;
			Rect.top = Point.y;
			MoveWindow(GetDlgItem(hDlg, *Win), Rect.left, Rect.top, Rect.right, Rect.bottom, TRUE);

			Win++;
		}
	}

	/* ���������ɑ傫����ύX���镔�i�̏��� */
	if(Dt->CurSize.cx != New->right - New->left)
	{
		Win = Dt->ResizeHorList;
		while(*Win != -1)
		{
			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);

			Rect.right = (Rect.right - Rect.left) + (New->right - New->left) - Dt->CurSize.cx;
			Rect.bottom = (Rect.bottom - Rect.top);

			Point.x = Rect.left;
			Point.y = Rect.top;
			ScreenToClient(hDlg, &Point);
			Rect.left = Point.x;
			Rect.top = Point.y;
			MoveWindow(GetDlgItem(hDlg, *Win), Rect.left, Rect.top, Rect.right, Rect.bottom, TRUE);

			Win++;
		}
	}

	/* ���������ɑ傫����ύX���镔�i�̏��� */
	if(Dt->CurSize.cx != New->right - New->left)
	{
		Win = Dt->ResizeVarList;
		while(*Win != -1)
		{
			GetWindowRect(GetDlgItem(hDlg, *Win), &Rect);

			Rect.right = (Rect.right - Rect.left);
			Rect.bottom = (Rect.bottom - Rect.top) + (New->bottom - New->top) - Dt->CurSize.cy;

			Point.x = Rect.left;
			Point.y = Rect.top;
			ScreenToClient(hDlg, &Point);
			Rect.left = Point.x;
			Rect.top = Point.y;
			MoveWindow(GetDlgItem(hDlg, *Win), Rect.left, Rect.top, Rect.right, Rect.bottom, TRUE);

			Win++;
		}
	}

	Dt->CurSize.cx = New->right - New->left;
	Dt->CurSize.cy = New->bottom - New->top;
	InvalidateRect(hDlg, NULL, TRUE);

	return;
}


