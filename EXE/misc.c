/*===========================================================================
/
/                                   Backup
/                           �e��̔ėp�T�u���[�`��
/
/============================================================================
/ Copyright (C) 1997-2017 Sota. All rights reserved.
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
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <stdarg.h>
#include <winsock.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlobj.h>

#include "common.h"
#include "resource.h"



/*----- ������̍Ō�� "\" ��t����i""�ɂ͕t���Ȃ��j--------------------------
*
*   Parameter
*       LPTSTR Str : ������
*
*   Return Value
*       �Ȃ�
*
*   Note
*       �I���W�i���̕����� LPTSTR Str ���ύX����܂��B
*----------------------------------------------------------------------------*/

void SetYenTail(LPTSTR Str)
{
    SetCharTail(Str, _T("\\"));
    return;
}


/*----- ������̍Ō�ɕ������t����i""�ɂ͕t���Ȃ��j-------------------------
*
*   Parameter
*       LPTSTR Str : ������
*       LPTSTR Ch : �ǉ����镶����
*
*   Return Value
*       �Ȃ�
*
*   Note
*       �I���W�i���̕����� LPTSTR Str ���ύX����܂��B
*----------------------------------------------------------------------------*/

void SetCharTail(LPTSTR Str, LPTSTR Ch)
{
    if((_tcslen(Str) > 0) && (_tcslen(Ch) > 0))
    {
        if(_tcscmp(_tcsninc(Str, _tcslen(Str) - _tcslen(Ch)), Ch) != 0)
            _tcscat(Str, Ch);
    }
    return;;
}


/*----- ������̍Ō�� "\" ����菜�� -----------------------------------------
*
*   Parameter
*       LPTSTR Str : ������
*
*   Return Value
*       �Ȃ�
*
*   Note
*       �I���W�i���̕����� LPTSTR Str ���ύX����܂��B
*----------------------------------------------------------------------------*/

void RemoveYenTail(LPTSTR Str)
{
    LPTSTR Pos;

    if(_tcslen(Str) > 0)
    {
        Pos = _tcsninc(Str, _tcslen(Str) - 1);
        if(_tcscmp(Pos, _T("\\")) == 0)
            *Pos = NUL;
    }
    return;;
}


/*----- �����񂩂���s�R�[�h����菜�� ----------------------------------------
*
*   Parameter
*       LPTSTR Str : ������
*
*   Return Value
*       �Ȃ�
*
*   Note
*       �I���W�i���̕����� LPTSTR Str ���ύX����܂��B
*----------------------------------------------------------------------------*/

void RemoveReturnCode(LPTSTR Str)
{
    LPTSTR Pos;

    if((Pos = _tcschr(Str, 0x0D)) != NULL)
        *Pos = NUL;

    if((Pos = _tcschr(Str, 0x0A)) != NULL)
        *Pos = NUL;
    return;
}


/*----- ��������̓���̕����̐��𐔂��� --------------------------------------
*
*   Parameter
*       LPTSTR Str : ������
*       _TCHAR Ch : ����
*
*   Return Value
*       �����̐�
*----------------------------------------------------------------------------*/

int CountChar(LPTSTR Str, _TCHAR Ch)
{
    int Ret;

    Ret = 0;
    while((Str = _tcschr(Str, Ch)) != NULL)
    {
        Str++;
        Ret++;
    }
    return(Ret);
}


/*----- ��������̓���̕�����S�Ēu�������� ----------------------------------
*
*   Parameter
*       LPTSTR Str : ������ (�����񒆂�'\0'������)
*       int Len : ������̒���
*       _TCHAR Src : ��������
*       _TCHAR Dst : �u������
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void ReplaceAll(LPTSTR Str, int Len, _TCHAR Src, _TCHAR Dst)
{
    for(; Len > 0; Len--)
    {
        if(*Str == Src)
            *Str = Dst;
        Str++;
    }
    return;
}


/*----- �Q�̕�����̐擪�����v���镔���݂̂��c�� --------------------------
*
*   Parameter
*       LPTSTR Str1 : ������1
*       LPTSTR Str2 : ������2
*
*   Return Value
*       �Ȃ�
*
*   Note
*       �I���W�i���̕����� LPTSTR Str1 ���ύX����܂��B
*----------------------------------------------------------------------------*/

void GetSamePartOfString(LPTSTR Str1, LPTSTR Str2)
{
    if((_tcslen(Str1) != 0) && (_tcslen(Str2) != 0))
    {
        while((*Str1 != _T('\0')) && (*Str2 != _T('\0')))
        {
            if(_tcsncmp(Str1, Str2, 1) != 0)
                break;
            Str1 = _tcsinc(Str1);
            Str2 = _tcsinc(Str2);
        }
    }
    *Str1 = _T('\0');
    return;;
}


/*----- �����񒆂̂��镶�����ʂ̕�����ɒu�������� --------------------------
*
*   Parameter
*       LPTSTR Str : ������
*       LPTSTR Find : ����������
*       LPTSTR Repl : �u��������
*       int Scan : �����݂̂����Ȃ�
*
*   Return Value
*       int �Y��������̂������������ǂ���
*
*   Note
*       �I���W�i���̕����� LPTSTR Str ���ύX����܂��B
*----------------------------------------------------------------------------*/

int ReplaceAllStr(LPTSTR Str, LPTSTR Find, LPTSTR Repl, int Scan)
{
    LPTSTR Pos;
    _TCHAR Tmp[MY_MAX_PATH+1];
    int Ret;

    Ret = NO;
    if((_tcslen(Str) != 0) && (_tcslen(Find) != 0) && (_tcscmp(Find, Repl) != 0))
    {
        Pos = _tcsstr(Str, Find);
        if(Pos != NULL)
        {
            Ret = YES;
            if(Scan == NO)
            {
                _tcscpy(Tmp, Pos + _tcslen(Find));
                if((Pos - Str) + _tcslen(Tmp) + _tcslen(Repl) <= MY_MAX_PATH)
                {
                    _tcscpy(Pos, Repl);
                    _tcscat(Str, Tmp);
                }
            }
        }
    }
    return(Ret);
}


/*----- �p�X���̒��̃t�@�C�����̐擪��Ԃ� ------------------------------------
*
*   Parameter
*       LPTSTR Path : �p�X��
*
*   Return Value
*       LPTSTR �t�@�C�����̐擪
*
*   Note
*       �f�B���N�g���̋�؂�L���� "\" �� "/" �̗������L��
*----------------------------------------------------------------------------*/

LPTSTR GetFileName(LPTSTR Path)
{
    LPTSTR Pos;

/* 2017/2/13 �E�o�b�N�A�b�v���t�H���_�Ƃ��āuC:\tmp�v�Ǝw�肵���ꍇ�ƁA�uC:\tmp\�v�̂悤��
�@�Ō�Ɂu\�v���w�肵���ꍇ�ŁA�������Ⴄ�̂��C�����܂����B*/
#if 0
	if((Pos = _tcschr(Path, _T(':'))) != NULL)
        Path = Pos + 1;

    if((Pos = _tcsrchr(Path, _T('\\'))) != NULL)
        Path = Pos + 1;

    if((Pos = _tcsrchr(Path, _T('/'))) != NULL)
        Path = Pos + 1;
#else
	if ((_tcslen(Path) == 3) &&
		((_tcscmp(Path + 1, _T(":\\")) == 0) ||
		 (_tcscmp(Path + 1, _T(":/")) == 0)))							/* D:\ �̂悤�Ȏw��̎� */
	{
		Path = Path + 3;
	}
	else if ((_tcslen(Path) == 2) && (_tcscmp(Path + 1, _T(":")) == 0))	/* D: �̂悤�Ȏw��̎� */
	{
		Path = Path + 2;
	}
	else
	{
		do
		{
			Pos = _tcschr(Path, _T('\\'));
			if (Pos == NULL)
			{
				Pos = _tcschr(Path, _T('/'));
			}
			if (Pos != NULL)
			{
				if (*(Pos + 1) == _T('\0'))
				{
					break;
				}
				Path = Pos + 1;
			}
		} while (Pos != NULL);
	}
#endif

    return(Path);
}


/*----- �p�X���̒��̊g���q�̐擪��Ԃ� ----------------------------------------
*
*   Parameter
*       LPTSTR Path : �p�X��
*
*   Return Value
*       LPTSTR �g���q�̐擪
*----------------------------------------------------------------------------*/

LPTSTR GetFileExt(LPTSTR Path)
{
    LPTSTR Ret;

    Ret = _tcschr(Path, NUL);
    if((_tcscmp(Path, _T(".")) != 0) &&
       (_tcscmp(Path, _T("..")) != 0))
    {
        while((Path = _tcschr(Path, _T('.'))) != NULL)
        {
            Path++;
            Ret = Path;
        }
    }
    return(Ret);
}


/*----- ������A���C�̒��������߂� --------------------------------------------
*
*   Parameter
*       LPTSTR Str : ������A���C (������NUL�Q��)
*
*   Return Value
*       int ����
*
*   Note
*       �I�[��2��NUL�̂����̍Ō�̕��͐����Ȃ�
*           StrMultiLen("") = 0
*           StrMultiLen("abc\0xyz\0") = 8
*           StrMultiLen("abc") = �I�[���Q��NUL�łȂ��̂ŋ��߂��Ȃ�
*----------------------------------------------------------------------------*/

int StrMultiLen(LPTSTR Str)
{
    int Len;
    int Tmp;

    Len = 0;
    while(*Str != NUL)
    {
        Tmp = _tcslen(Str) + 1;
        Str += Tmp;
        Len += Tmp;
    }
    return(Len);
}


/*----- ������A���C���̕�����̐��𐔂��� -------------------------------------
*
*   Parameter
*       LPTSTR Str : ������A���C (������NUL�Q��)
*
*   Return Value
*       int ������̌�
*
*   Note
*       �I�[��2��NUL�̂����̍Ō�̕��͐����Ȃ�
*           StrMultiLen("") = 0
*           StrMultiLen("abc\0xyz\0") = 2
*           StrMultiLen("abc") = �I�[���Q��NUL�łȂ��̂ŋ��߂��Ȃ�
*----------------------------------------------------------------------------*/

int StrMultiCount(LPTSTR Str)
{
    int Count;
    int Tmp;

    Count = 0;
    while(*Str != NUL)
    {
        Tmp = _tcslen(Str) + 1;
        Str += Tmp;
        Count += 1;
    }
    return(Count);
}


/*----- ������A���C������1�̕�������擾���� --------------------------------
*
*   Parameter
*       LPTSTR Str : ������A���C (������NUL�Q��)
*       int Num : ���Ԗڂ̕������Ԃ���
*
*   Return Value
*       int ������
*
*   Note
*       ������A���C�̒��Ɏw��ԍ��̕����񂪂Ȃ����́A�ŏ��̕������Ԃ�
*----------------------------------------------------------------------------*/
LPTSTR GetSpecifiedStringFromMultiString(LPTSTR Str, int Num)
{
    int Tmp;
    LPTSTR Result = Str;

    for(; Num > 0; Num--)
    {
        Tmp = _tcslen(Result) + 1;
        Result += Tmp;
        if(*Result == NUL)
        {
            Result = Str;
            break;
        }
    }
    return Result;
}


/*----- �m���s�n�Ɓm����n�����̃_�C�A���O�̋��ʃR�[���o�b�N�֐� --------------
*
*   Parameter
*       HWND hDlg : �E�C���h�E�n���h��
*       UINT message : ���b�Z�[�W�ԍ�
*       WPARAM wParam : ���b�Z�[�W�� WPARAM ����
*       LPARAM lParam : ���b�Z�[�W�� LPARAM ����
*
*   Return Value
*       BOOL TRUE/FALSE
*----------------------------------------------------------------------------*/

BOOL CALLBACK ExeEscDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, DLG_MSG1, WM_SETTEXT, 0, lParam);
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


/*----- �t�@�C���I�� ----------------------------------------------------------
*
*   Parameter
*       HWND hWnd : �E�C���h�E�n���h��
*       LPTSTR Fname : �t�@�C������Ԃ��o�b�t�@
*       LPTSTR Title : �^�C�g��
*       LPTSTR Filters : �t�B���^�[������
*       LPTSTR Ext : �f�t�H���g�g���q
*       int Flags : �ǉ�����t���O
*       int Save : �u�J���v���u�ۑ��v�� (0=�J��, 1=�ۑ�)
*       LPTSTR Dir : �f�t�H���g�t�H���_
*
*   Return Value
*       int �X�e�[�^�X
*           TRUE/FALSE=���
*----------------------------------------------------------------------------*/

int SelectFile(HWND hWnd, LPTSTR Fname, LPTSTR Title, LPTSTR Filters, LPTSTR Ext, int Flags, int Save, LPTSTR Dir)
{
    OPENFILENAME OpenFile;
    _TCHAR Tmp[MY_MAX_PATH+1];
    int Sts;

    _tcscpy(Tmp, Fname);
    OpenFile.lStructSize = sizeof(OPENFILENAME);
    OpenFile.hwndOwner = hWnd;
    OpenFile.hInstance = 0;
    OpenFile.lpstrFilter = Filters;
    OpenFile.lpstrCustomFilter = NULL;
    OpenFile.nFilterIndex = 1;
    OpenFile.lpstrFile = Tmp;
    OpenFile.nMaxFile = MY_MAX_PATH;
    OpenFile.lpstrFileTitle = NULL;
    OpenFile.nMaxFileTitle = 0;
    OpenFile.lpstrInitialDir = Dir;
    OpenFile.lpstrTitle = Title;
    OpenFile.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | Flags;
    OpenFile.nFileOffset = 0;
    OpenFile.nFileExtension = 0;
    OpenFile.lpstrDefExt = Ext;
    OpenFile.lCustData = 0;
    OpenFile.lpfnHook = NULL;
    OpenFile.lpTemplateName = NULL;

    if(Save == 0)
    {
        if((Sts = GetOpenFileName(&OpenFile)) == TRUE)
            _tcscpy(Fname,Tmp);
    }
    else
    {
        if((Sts = GetSaveFileName(&OpenFile)) == TRUE)
            _tcscpy(Fname,Tmp);
    }
    return(Sts);
}


int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData);
_TCHAR CurFolder[MY_MAX_PATH+1];


/*----- �f�B���N�g����I�� ----------------------------------------------------
*
*   Parameter
*       HWND hWnd : �E�C���h�E�n���h��
*       LPTSTR Buf : �f�B���N�g������Ԃ��o�b�t�@�i�����f�B���N�g�����j
*       int MaxLen : �o�b�t�@�̃T�C�Y
*       LPTSTR Title : �^�C�g��
*
*   Return Value
*       int �X�e�[�^�X
*           TRUE/FALSE=���
*----------------------------------------------------------------------------*/

int SelectDir(HWND hWnd, LPTSTR Buf, int MaxLen, LPTSTR Title)
{
    _TCHAR Tmp[MY_MAX_PATH+1];
    BROWSEINFO  Binfo;
    LPITEMIDLIST lpIdll;
    int Sts;
    LPMALLOC lpMalloc;

    _tcscpy(CurFolder, Buf);
    Sts = FALSE;
    if(SHGetMalloc(&lpMalloc) == NOERROR)
    {
        Binfo.hwndOwner = hWnd;
        Binfo.pidlRoot = NULL;
        Binfo.pszDisplayName = Tmp;
        Binfo.lpszTitle = Title;
        Binfo.ulFlags = BIF_RETURNONLYFSDIRS;
        Binfo.lpfn = &BrowseCallbackProc;
        Binfo.lParam = 0;
        Binfo.iImage = 0;
        if((lpIdll = SHBrowseForFolder(&Binfo)) != NULL)
        {
            SHGetPathFromIDList(lpIdll, Tmp);
            memset(Buf, NUL, MaxLen * sizeof(_TCHAR));
            _tcsncpy(Buf, Tmp, MaxLen-1);
            Sts = TRUE;
            lpMalloc->lpVtbl->Free(lpMalloc, lpIdll);
        }
        lpMalloc->lpVtbl->Release(lpMalloc);
    }
    return(Sts);
}


/*-------------------------------------------------------------------------------
Name    :   BrowseCallbackProc
Desc    :   Callback function of the directory selecting dialog
Param   :   hwnd        [in] Window handle
            uMsg        [in] Message
            lParam      [in] LPARAM
            lpData      [in] data
Return  :   status
-------------------------------------------------------------------------------*/
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
    if(uMsg==BFFM_INITIALIZED)
    {
        SendMessage(hwnd,BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)CurFolder);
    }
    return(0);
}


/*----- �l�Ɋ֘A�t����ꂽ���W�I�{�^�����`�F�b�N���� --------------------------
*
*   Parameter
*       HWND hDlg : �_�C�A���O�{�b�N�X�̃E�C���h�E�n���h��
*       int Value : �l
*       const RADIOBUTTON *Buttons : ���W�I�{�^���ƒl�̊֘A�t���e�[�u��
*       int Num : �{�^���̐�
*
*   Return Value
*       �Ȃ�
*
*   Note
*       �l�Ɋ֘A�t����ꂽ�{�^�����������́A�e�[�u���̍ŏ��ɓo�^����Ă���{�^
*       �����`�F�b�N����
*----------------------------------------------------------------------------*/

void SetRadioButtonByValue(HWND hDlg, int Value, const RADIOBUTTON *Buttons, int Num)
{
    int i;
    int Def;

    Def = Buttons->ButID;
    for(i = 0; i < Num; i++)
    {
        if(Value == Buttons->Value)
        {
            SendDlgItemMessage(hDlg, Buttons->ButID, BM_SETCHECK, 1, 0);
            /* ���W�I�{�^����ύX�������ɑ��̍��ڂ̃n�C�h�Ȃǂ��s�Ȃ�����   */
            /* ����̂ŁA���̂��߂� WM_COMMAND �𑗂�                       */
            SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(Buttons->ButID, 0), 0);
            break;
        }
        Buttons++;
    }
    if(i == Num)
    {
        SendDlgItemMessage(hDlg, Def, BM_SETCHECK, 1, 0);
        SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(Def, 0), 0);
    }
    return;
}


/*----- �`�F�b�N����Ă���{�^���Ɋ֘A�t����ꂽ�l��Ԃ� ----------------------
*
*   Parameter
*       HWND hDlg : �_�C�A���O�{�b�N�X�̃E�C���h�E�n���h��
*       const RADIOBUTTON *Buttons : ���W�I�{�^���ƒl�̊֘A�t���e�[�u��
*       int Num : �{�^���̐�
*
*   Return Value
*       int �l
*
*   Note
*       �ǂ̃{�^�����`�F�b�N����Ă��Ȃ����́A�e�[�u���̍ŏ��ɓo�^����Ă���{
*       �^���̒l��Ԃ�
*----------------------------------------------------------------------------*/

int AskRadioButtonValue(HWND hDlg, const RADIOBUTTON *Buttons, int Num)
{
    int i;
    int Ret;

    Ret = Buttons->Value;
    for(i = 0; i < Num; i++)
    {
        if(SendDlgItemMessage(hDlg, Buttons->ButID, BM_GETCHECK, 0, 0) == 1)
        {
            Ret = Buttons->Value;
            break;
        }
        Buttons++;
    }
    return(Ret);
}


/*----- �t�@�C���T�C�Y�𕶎���ɕϊ����� --------------------------------------
*
*   Parameter
*       double Size : �t�@�C���T�C�Y
*       LPTSTR Buf : �������Ԃ��o�b�t�@
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void MakeSizeString(double Size, LPTSTR Buf)
{
    if(Size >= (1024. * 1024. * 1024. * 1024.))
    {
        Size /= (1024. * 1024. * 1024. * 1024.);
        _stprintf(Buf, _T("%.2fT Bytes"), Size);
    }
    else if(Size >= (1024. * 1024. * 1024.))
    {
        Size /= (1024. * 1024. * 1024.);
        _stprintf(Buf, _T("%.2fG Bytes"), Size);
    }
    else if(Size >= (1024 * 1024))
    {
        Size /= (1024 * 1024);
        _stprintf(Buf, _T("%.2fM Bytes"), Size);
    }
    else if (Size >= 1024)
    {
        Size /= 1024;
        _stprintf(Buf, _T("%.2fK Bytes"), Size);
    }
    else
        _stprintf(Buf, _T("%.0f Bytes"), Size);

    return;
}


/*----- ���b�Z�[�W���� --------------------------------------------------------
*
*   Parameter
*       �Ȃ�
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void BackgrndMessageProc(void)
{
    MSG Msg;

    while(PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return;
}


/*----- ���X�g�{�b�N�X�̓��e���\�[�g���� --------------------------------------
*
*   Parameter
*       HWND hWnd : ���X�g�{�b�N�X�̃E�C���h�E�n���h��
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void SortListBoxItem(HWND hWnd)
{
    LPTSTR Buf;
    LPTSTR Tmp;
    int Num;
    int i;

    Num = SendMessage(hWnd, LB_GETCOUNT, 0, 0);
    if(Num > 1)
    {
        if((Buf = malloc(Num * (MY_MAX_PATH+1) * sizeof(_TCHAR))) != NULL)
        {
            Tmp = Buf;
            for(i = 0; i < Num; i++)
            {
                SendMessage(hWnd, LB_GETTEXT, i, (LPARAM)Tmp);
                Tmp += MY_MAX_PATH+1;
            }

            qsort(Buf, Num, (MY_MAX_PATH+1) * sizeof(_TCHAR), _tcscmp);

            SendMessage(hWnd, LB_RESETCONTENT, 0, 0);
            Tmp = Buf;
            for(i = 0; i < Num; i++)
            {
                SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)Tmp);
                Tmp += MY_MAX_PATH+1;
            }

            free(Buf);
        }
    }
    return;
}


/*----- ���[�g�f�B���N�g�������߂� --------------------------------------------
*
*   Parameter
*       LPTSTR Path : �p�X��
*       LPTSTR Buf : �o�b�t�@
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void GetRootPath(LPTSTR Path, LPTSTR Buf)
{
    LPTSTR Pos;

    _tcscpy(Buf, Path);
    Pos = Buf;

    if(_tcsncmp(Pos, _T("\\\\"), 2) == 0)
    {
        Pos += 2;
        if((Pos = _tcschr(Pos, _T('\\'))) != NULL)
        {
            Pos++;
            if((Pos = _tcschr(Pos, _T('\\'))) != NULL)
                *(Pos+1) = NUL;
            else
                _tcscat(Buf, _T("\\"));
        }
    }
    else
    {
        if((Pos = _tcschr(Pos, _T('\\'))) != NULL)
            *(Pos+1) = NUL;
        else
            _tcscat(Buf, _T("\\"));
    }
    return;
}


/*----- FS�̎�ނ�Ԃ� --------------------------------------------------------
*
*   Parameter
*       LPTSTR Path : �p�X��
*
*   Return Value
*       int FS�̎�� (FS_FAT/FS_OTHER)
*----------------------------------------------------------------------------*/

int GetDriveFormat(LPTSTR Path)
{
    _TCHAR Root[MY_MAX_PATH+2];
    _TCHAR Type[20];
    DWORD MaxLen;
    DWORD Flags;
    int Ret;

    Ret = FS_OTHER;
    GetRootPath(Path, Root);
    if(GetVolumeInformation(Root, NULL, 0, NULL, &MaxLen, &Flags, Type, 20) != 0)
    {
        if(_tcsnicmp(Type, _T("FAT"), 3) == 0)
            Ret = FS_FAT;
    }
    return(Ret);
}


/*----- �{�����[�����x����Ԃ� ------------------------------------------------
*
*   Parameter
*       LPTSTR Path : �p�X��
*       LPTSTR Buf : �o�b�t�@
*       int Size : �o�b�t�@�̃T�C�Y
*
*   Return Value
*       BOOL �X�e�[�^�X (TRUE=����)
*----------------------------------------------------------------------------*/

BOOL GetVolumeLabel(LPTSTR Path, LPTSTR Buf, int Size)
{
    _TCHAR Root[MY_MAX_PATH+2];
    DWORD MaxLen;
    DWORD Flags;
    BOOL Ret;

    GetRootPath(Path, Root);
    _tcscpy(Buf, _T(""));
    Ret = GetVolumeInformation(Root, Buf, Size, NULL, &MaxLen, &Flags, NULL, 0);
    return(Ret);
}


/*----- �h���C�u�̎�ނ�Ԃ� --------------------------------------------------
*
*   Parameter
*       LPTSTR Path : �p�X��
*
*   Return Value
*       UINT �h���C�u�̎�� (DRIVE_xxx)
*----------------------------------------------------------------------------*/

UINT GetDriveTypeFromPath(LPTSTR Path)
{
    _TCHAR Root[MY_MAX_PATH+2];
    UINT Ret;

    GetRootPath(Path, Root);
    Ret = GetDriveType(Root);
    return(Ret);
}


/*----- �t�@�C�����S�~���ɍ폜 ------------------------------------------------
*
*   Parameter
*       LPTSTR Path : �t�@�C����
*
*   Return Value
*       int �X�e�[�^�X (0=����I��)
*----------------------------------------------------------------------------*/

int MoveFileToTrashCan(LPTSTR Path)
{
    SHFILEOPSTRUCT FileOp;
    _TCHAR Tmp[MY_MAX_PATH+2];

    memset(Tmp, 0, (MY_MAX_PATH+2) * sizeof(_TCHAR));
    _tcscpy(Tmp, Path);
    FileOp.hwnd = NULL;
    FileOp.wFunc = FO_DELETE;
    FileOp.pFrom = Tmp;
    FileOp.pTo = _T("");
    FileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO | FOF_NO_CONNECTED_ELEMENTS;
    FileOp.lpszProgressTitle = _T("");
    return(SHFileOperation(&FileOp));
}


/*----- �h���b�O&�h���b�v���ꂽ�t�@�C�����y�[�W�ɑ��� -------------------------
*
*   Parameter
*       HWND hWnd : �����̃_�C�A���O
*       UINT message  : ���郁�b�Z�[�W
*       WPARAM wParam : WM_DROPFILES���b�Z�[�W�� WPARAM ����
*       LPARAM lParam : WM_DROPFILES���b�Z�[�W�� LPARAM ����
*       int Type : ������^�C�v (SEND_xxx)
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void SendDropFilesToControl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int Type)
{
    int Max;
    int i;
    LPTSTR Tmp;
    DWORD Attr;
    int Error;
    _TCHAR Msg[128];

    Error = 0;
    Max = DragQueryFile((HDROP)wParam, 0xFFFFFFFF, NULL, 0);
    for(i = 0; i < Max; i++)
    {
        Tmp = malloc((MY_MAX_PATH+1) * sizeof(_TCHAR));
        if(Tmp != NULL)
        {
            DragQueryFile((HDROP)wParam, i, Tmp, MY_MAX_PATH);
            Attr = GetFileAttributes_My(Tmp, NO);
            if((Attr == 0xFFFFFFFF) ||
               ((Attr & FILE_ATTRIBUTE_DIRECTORY) && (Type & SEND_FOLDER)) ||
               (!(Attr & FILE_ATTRIBUTE_DIRECTORY) && (Type & SEND_FILE)))
            {
                PostMessage(hDlg, message, 0, (LPARAM)Tmp);
            }
            else
            {
                Error++;
                free(Tmp);
            }
        }
    }
    DragFinish((HDROP)wParam);

    if(Error != 0)
    {
        if(Type & SEND_FOLDER)
            _stprintf(Msg, MSGJPN_37, Error);
        else
            _stprintf(Msg, MSGJPN_38, Error);
        DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(dragdrop_err_dlg), hDlg, ExeEscDialogProc, (LPARAM)Msg);
    }
    return;
}


/*----- �r���[�����N�� --------------------------------------------------------
*
*   Parameter
*       LPTSTR Fname : �t�@�C����
*       LPTSTR App :
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void ExecViewer(LPTSTR Fname, LPTSTR App)
{
    PROCESS_INFORMATION Info;
    STARTUPINFO Startup;
    _TCHAR AssocProg[MY_MAX_PATH+1];
    _TCHAR ComLine[MY_MAX_PATH*2+10];

    /* FindExecutable()�͊֘A�t����ꂽ�v���O�����̃p�X���ɃX�y�[�X��   */
    /* �܂܂�Ă��鎞�A�Ԉ�����p�X����Ԃ���������B                   */
    /* �����ŁA�֘A�t����ꂽ�v���O�����̋N����ShellExecute()���g���B   */

    if((_tcslen(App) == 0) && (FindExecutable(Fname, NULL, AssocProg) > (HINSTANCE)32))
    {
        ShellExecute(NULL, _T("open"), Fname, NULL, _T(""), SW_SHOW);
    }
    else
    {
        if(_tcschr(Fname, _T(' ')) == NULL)
            _stprintf(ComLine, _T("%s %s"), App, Fname);
        else
            _stprintf(ComLine, _T("%s \"%s\""), App, Fname);

        memset(&Startup, NUL, sizeof(STARTUPINFO));
        Startup.cb = sizeof(STARTUPINFO);
        Startup.wShowWindow = SW_SHOW;
        if(CreateProcess(NULL, ComLine, NULL, NULL, FALSE, 0, NULL, NULL, &Startup, &Info) == FALSE)
        {
            DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_87);
        }
    }
    return;
}


/*----- �ݒ�l�͈̔̓`�F�b�N --------------------------------------------------
*
*   Parameter
*       int *Cur : �ݒ�l
*       int Max : �ő�l
*       int Min : �ŏ��l
*
*   Return Value
*       �Ȃ�
*
*   Parameter change
*       int *Cur : �ݒ�l
*----------------------------------------------------------------------------*/

void CheckRange2(int *Cur, int Max, int Min)
{
    if(*Cur < Min)
        *Cur = Min;
    if(*Cur > Max)
        *Cur = Max;
    return;
}


/*----- FILETIME(UTC)����t������(JST)�ɕϊ� ----------------------------------
*
*   Parameter
*       FILETIME *Time : �t�@�C���^�C��
*       LPTSTR Buf : ���t�������Ԃ����[�N
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void FileTime2TimeString(FILETIME *Time, LPTSTR Buf)
{
    SYSTEMTIME sTime;
    FILETIME fTime;

    /* _T("yyyy/mm/dd hh:mm") */
    FileTimeToLocalFileTime(Time, &fTime);
    FileTimeToSystemTime(&fTime, &sTime);

    _stprintf(Buf, _T("%04d/%02d/%02d %2d:%02d:%02d"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
    return;
}


