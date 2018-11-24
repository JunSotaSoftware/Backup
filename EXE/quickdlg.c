/*===========================================================================
/
/									Backup
/							�N�C�b�N�o�b�N�A�b�v
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
#include <malloc.h>
#include <windowsx.h>
#include <commctrl.h>
#include <htmlhelp.h>

#include "common.h"
#include "resource.h"


/*===== �v���g�^�C�v =====*/

static BOOL CALLBACK QuickBaclupDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void MakeCopyPatFromDialog(HWND hDlg, COPYPATLIST *Pat);
static void CheckSemicolon(LPTSTR Str);
static LRESULT CALLBACK QuickSrcWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK QuickDstWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



static HWND hWndQuickPat;
static WNDPROC QuickSrcProcPtr;
static WNDPROC QuickDstProcPtr;
static COPYPAT OrgPat = {
	_T(""),		/* �p�^�[���� */
	_T(""),		/* �R�����g */
	_T("\0"),	/* �o�b�N�A�b�v�� (�}���`������) */
	_T("\0"),	/* �o�b�N�A�b�v�� (�}���`������) */
	_T("\0"),	/* �o�b�N�A�b�v���Ȃ��t�H���_ (�}���`������) */
	_T("\0"),	/* �o�b�N�A�b�v���Ȃ��t�@�C�� (�}���`������) */
	_T(""),		/* �{�����[�����x�� */
	_T(""),		/* �T�E���h�t�@�C�� */
	0,		/* ��Ƀt�@�C�����R�s�[ */
	0,		/* �t�H���_���폜���� */
	0,		/* �t�@�C�����폜���� */
	1,		/* �G���[�𖳎����� */
	1,		/* �폜�̊m�F���s�Ȃ� */
	0,		/* �㏑���̊m�F���s�Ȃ� */
	0,		/* �o�b�N�A�b�v���Ȃ��t�@�C���^�t�H���_���폜���Ȃ� */
	0,		/* �o�b�N�A�b�v�悪�V�������̓R�s�[���Ȃ� */
	0,		/* �t�@�C���R�s�[���̑҂����� */
	0,		/* �{�����[�����x�����`�F�b�N���� */
	0,		/* ���ݔ����g�p���� */
	2,		/* �^�C���X�^���v�̋��e�덷 */
	0,		/* �o�b�N�A�b�v��̏��� (0=�������Ȃ�, 1=�v���O�����I��, 2=Windows�I��) */
	0,		/* �V�X�e���t�@�C���͏��O */
	0,		/* �B���t�@�C���͏��O */
	0,		/* �傫�ȃt�@�C���͏��O */
	100,	/* �傫�ȃt�@�C����臒l */
	0,		/* �����̈Ⴂ�͖��� */
	0,		/* �o�b�N�A�b�v�I����ɃT�E���h��炷���ǂ��� */
	-60,		/* �ăo�b�N�A�b�v�҂����ԁF�}�C�i�X�l�Ȃ�ăo�b�N�A�b�v�Ȃ� */
	0,		/* �o�b�N�A�b�v��̃t�H���_�����Ȃ� */
	0,		/* �^�C���X�^���v�̈Ⴂ�͖��� */
	0,		/* �o�b�N�A�b�v�J�n���ɃR�����g���E�C���h�E�ŕ\������ */
    0,      /* ���̃o�b�N�A�b�v��ԍ� */
	0,		/* �o�b�N�A�b�v���Dropbox */
	0,		/* �폜�̑���Ƀt�@�C�����ړ����� */
	_T(""),	/* �t�@�C���ړ���̃t�H���_�[ */
    NULL,   /* ���̃o�b�N�A�b�v�� */
    0,      /* �p�^�[���ԍ� */
};





/*----- �N�C�b�N�o�b�N�A�b�v�̓��� --------------------------------------------
*
*	Parameter
*		COPYPATLIST **Top : ���X�g�̐擪
*		HWND hWnd : �e�E�C���h�E
*
*	Return Value
*		int �ݒ�̐�
*----------------------------------------------------------------------------*/
int GetQuickBackupParam(COPYPATLIST **Top, HWND hWnd)
{
	int Num;
	COPYPATLIST *Pat;

	Num = 0;
	if((Pat = malloc(sizeof(COPYPATLIST))) != NULL)
	{
		memcpy(&Pat->Set, &OrgPat, sizeof(COPYPAT));
		Pat->Next = NULL;
		if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(quick_dlg), hWnd, QuickBaclupDialogProc, (LPARAM)Pat) == YES)
		{
			memcpy(&OrgPat, &Pat->Set, sizeof(COPYPAT));
			*Top = Pat;
			Num = 1;
		}
		else
			free(Pat);
	}
	return(Num);
}


/*----- �N�C�b�N�o�b�N�A�b�v�E�C���h�E�̃��b�Z�[�W���� ------------------------
*
*	Parameter
*		HWND hDlg : �E�C���h�E�n���h��
*		UINT message : ���b�Z�[�W�ԍ�
*		WPARAM wParam : ���b�Z�[�W�� WPARAM ����
*		LPARAM lParam : ���b�Z�[�W�� LPARAM ����
*
*	Return Value
*		BOOL TRUE/FALSE
*----------------------------------------------------------------------------*/
static BOOL CALLBACK QuickBaclupDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static COPYPATLIST *Pat;
	_TCHAR Tmp[MY_MAX_PATH+1];
	HWND hWndChild;
	LPTSTR Tmp2;

	switch (message)
	{
		case WM_INITDIALOG :
			Pat = (COPYPATLIST*)lParam;
			SendDlgItemMessage(hDlg, QUICK_SRC, EM_LIMITTEXT, SRC_PATH_LEN-1, 0);
			SendDlgItemMessage(hDlg, QUICK_DST, EM_LIMITTEXT, DST_PATH_LEN, 0);
			ReplaceAll(Pat->Set.Src, StrMultiLen(Pat->Set.Src), '\0', ';');
			SendDlgItemMessage(hDlg, QUICK_SRC, WM_SETTEXT, 0, (LPARAM)Pat->Set.Src);
			SendDlgItemMessage(hDlg, QUICK_DST, WM_SETTEXT, 0, (LPARAM)Pat->Set.Dst);
			SendDlgItemMessage(hDlg, PATSET_RMDIR, BM_SETCHECK, Pat->Set.DelDir, 0);
			SendDlgItemMessage(hDlg, PATSET_RMFILE, BM_SETCHECK, Pat->Set.DelFile, 0);
			SendDlgItemMessage(hDlg, PATSET_NOTIFY_DEL2, BM_SETCHECK, Pat->Set.NotifyDel, 0);
			SendDlgItemMessage(hDlg, PATSET_USE_TRASHCAN, BM_SETCHECK, Pat->Set.UseTrashCan, 0);
			SendDlgItemMessage(hDlg, PATSET_FORCE, BM_SETCHECK, Pat->Set.ForceCopy, 0);
			SendDlgItemMessage(hDlg, PATSET_NEWONLY, BM_SETCHECK, Pat->Set.NewOnly, 0);
			SendDlgItemMessage(hDlg, PATSET_NOERROR, BM_SETCHECK, Pat->Set.IgnoreErr, 0);

			hWndQuickPat = hDlg;
			hWndChild = GetDlgItem(hDlg, QUICK_SRC);
			DragAcceptFiles(hWndChild, TRUE);
			QuickSrcProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)QuickSrcWndProc);

			hWndChild = GetDlgItem(hDlg, QUICK_DST);
			DragAcceptFiles(hWndChild, TRUE);
			QuickDstProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)QuickDstWndProc);
			return(TRUE);

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK :
					MakeCopyPatFromDialog(hDlg, Pat);
					if((_tcslen(Pat->Set.Dst) > 0) && (_tcslen(Pat->Set.Src) > 0))
						EndDialog(hDlg, YES);
					else
						DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(folder_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_84);
					break;

				case IDCANCEL :
					EndDialog(hDlg, NO);
					break;

				case QUICK_SIZE :
					MakeCopyPatFromDialog(hDlg, Pat);
					if(_tcslen(Pat->Set.Src) > 0)
						FilesSizeDialog(hDlg, Pat);
					else
						DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(folder_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_86);
					break;

				case IDHELP :
					HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000020);
					break;

				case QUICK_SRC_BR :
					if(SelectDir(hDlg, Tmp, MY_MAX_PATH, MSGJPN_36) == TRUE)
						SendDlgItemMessage(hDlg, QUICK_SRC, WM_SETTEXT, 0, (LPARAM)Tmp);
					break;

				case QUICK_DST_BR :
					if(SelectDir(hDlg, Tmp, MY_MAX_PATH, MSGJPN_33) == TRUE)
						SendDlgItemMessage(hDlg, QUICK_DST, WM_SETTEXT, 0, (LPARAM)Tmp);
					break;
			}
			return(TRUE);

		case WM_ADD_SRCLIST :
			Tmp2 = malloc((SRC_PATH_LEN+1) * sizeof(_TCHAR));
			if(Tmp2 != NULL)
			{
				SendDlgItemMessage(hDlg, QUICK_SRC, WM_GETTEXT, SRC_PATH_LEN, (LPARAM)Tmp2);
				if((_tcslen(Tmp2) + _tcslen((_TCHAR*)lParam) + 1) <= SRC_PATH_LEN)
				{
					SetCharTail(Tmp2, _T(";"));
					_tcscat(Tmp2, (_TCHAR*)lParam);
					SendDlgItemMessage(hDlg, QUICK_SRC, WM_SETTEXT, 0, (LPARAM)Tmp2);
					SendDlgItemMessage(hDlg, QUICK_SRC, EM_SETSEL, 0, (LPARAM)-1);
				}
				else
					MessageBeep((UINT)-1);
				free(Tmp2);
			}
			free((_TCHAR*)lParam);
			break;

		case WM_ADD_DSTLIST :
			SendDlgItemMessage(hDlg, QUICK_DST, WM_SETTEXT, 0, (LPARAM)lParam);
			free((_TCHAR*)lParam);
			break;
	}
    return(FALSE);
}


/*----- �_�C�A���O�̏�񂩂�p�^�[���f�[�^���쐬���� --------------------------
*
*	Parameter
*		HWND hDlg : �E�C���h�E�n���h��
*		COPYPATLIST *Pat : �p�^�[���f�[�^�̊i�[��
*
*	Return Value
*		�Ȃ�
*----------------------------------------------------------------------------*/
static void MakeCopyPatFromDialog(HWND hDlg, COPYPATLIST *Pat)
{
	SendDlgItemMessage(hDlg, QUICK_SRC, WM_GETTEXT, SRC_PATH_LEN, (LPARAM)Pat->Set.Src);
	CheckSemicolon(Pat->Set.Src);
	Pat->Set.Src[_tcslen(Pat->Set.Src) + 1] = '\0';
	ReplaceAll(Pat->Set.Src, _tcslen(Pat->Set.Src), ';', '\0');
	SendDlgItemMessage(hDlg, QUICK_DST, WM_GETTEXT, DST_PATH_LEN, (LPARAM)Pat->Set.Dst);
	Pat->Set.Dst[_tcslen(Pat->Set.Dst) + 1] = '\0';
	Pat->Set.DelDir = SendDlgItemMessage(hDlg, PATSET_RMDIR, BM_GETCHECK, 0, 0);
	Pat->Set.DelFile = SendDlgItemMessage(hDlg, PATSET_RMFILE, BM_GETCHECK, 0, 0);
	Pat->Set.NotifyDel = SendDlgItemMessage(hDlg, PATSET_NOTIFY_DEL2, BM_GETCHECK, 0, 0);
	Pat->Set.UseTrashCan = SendDlgItemMessage(hDlg, PATSET_USE_TRASHCAN, BM_GETCHECK, 0, 0);
	Pat->Set.ForceCopy = SendDlgItemMessage(hDlg, PATSET_FORCE, BM_GETCHECK, 0, 0);
	Pat->Set.NewOnly = SendDlgItemMessage(hDlg, PATSET_NEWONLY, BM_GETCHECK, 0, 0);
	Pat->Set.IgnoreErr = SendDlgItemMessage(hDlg, PATSET_NOERROR, BM_GETCHECK, 0, 0);
	return;
}


/*----- �s�v�ȃZ�~�R�����̃`�F�b�N --------------------------------------------
*
*	Parameter
*		LPTSTR Str : ������
*
*	Return Value
*		�Ȃ�
*----------------------------------------------------------------------------*/
static void CheckSemicolon(LPTSTR Str)
{
	LPTSTR Put;
	_TCHAR Prev;

	Prev= '\0';
	Put = Str;
	while(*Str != '\0')
	{
		if(*Str != ';')
			break;
		Str++;
	}

	while(*Str != '\0')
	{
		if((*Str != ';') || (Prev != ';'))
		{
			Prev = *Str++;
			*Put++ = Prev;
		}
		else
			Str++;
	}
	*Put = '\0';
	return;
}


/*----- �o�b�N�A�b�v���E�C���h�E�̃��b�Z�[�W���� ------------------------------
*
*	Parameter
*		HWND hWnd : �E�C���h�E�n���h��
*		UINT message  : ���b�Z�[�W�ԍ�
*		WPARAM wParam : ���b�Z�[�W�� WPARAM ����
*		LPARAM lParam : ���b�Z�[�W�� LPARAM ����
*
*	Return Value
*		���b�Z�[�W�ɑΉ�����߂�l
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK QuickSrcWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
        case WM_DROPFILES:
			SendDropFilesToControl(hWndQuickPat, WM_ADD_SRCLIST, wParam, lParam, SEND_FOLDER | SEND_FILE);
			break;

		default :
			return(CallWindowProc(QuickSrcProcPtr, hWnd, message, wParam, lParam));
	}
    return(0L);
}


/*----- �o�b�N�A�b�v��E�C���h�E�̃��b�Z�[�W���� ------------------------------
*
*	Parameter
*		HWND hWnd : �E�C���h�E�n���h��
*		UINT message  : ���b�Z�[�W�ԍ�
*		WPARAM wParam : ���b�Z�[�W�� WPARAM ����
*		LPARAM lParam : ���b�Z�[�W�� LPARAM ����
*
*	Return Value
*		���b�Z�[�W�ɑΉ�����߂�l
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK QuickDstWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
        case WM_DROPFILES:
			SendDropFilesToControl(hWndQuickPat, WM_ADD_DSTLIST, wParam, lParam, SEND_FOLDER);
			break;

		default :
			return(CallWindowProc(QuickDstProcPtr, hWnd, message, wParam, lParam));
	}
    return(0L);
}


