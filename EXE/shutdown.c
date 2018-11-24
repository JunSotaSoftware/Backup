/*===========================================================================
/
/									Backup
/							Windows�̃V���b�g�_�E��
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


/*===== �v���g�^�C�v =====*/
BOOL CALLBACK CountDownDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


/*-----------------------------------------------------------------------------
 ���O	:	ChangeSystemPowerMode
 ����	:	Windows�̃p���[�X�e�[�g��ύX����
 ����	:	State	�X�e�[�g (2=�V���b�g�_�E��, 3=�X�^���o�C, 4=�x�~, 5=�X�^���o�C, 6=�x�~)
 �߂�l :	BOOL	�X�e�[�^�X (TRUE=����)
-----------------------------------------------------------------------------*/
BOOL ChangeSystemPowerMode(int State)
{
	OSVERSIONINFO		osvi;
	HANDLE				hToken;
	TOKEN_PRIVILEGES	TokenPri;
	BOOL				Sts;
	DWORD				Err;
	LPTSTR 				lpBuffer;

	Sts = FALSE;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == TRUE)
		{
			if(LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &TokenPri.Privileges[0].Luid) == TRUE)
			{
				TokenPri.PrivilegeCount = 1;
				TokenPri.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
				AdjustTokenPrivileges(hToken, FALSE, &TokenPri, 0, NULL, 0);
				/* AdjustTokenPrivileges �͏�� TRUE ��Ԃ�? */
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
	}
	else
	{
		Sts = TRUE;
	}

	if(Sts == TRUE)
	{
		switch(State)
		{
			case 2 :
				if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
				{
					Sts = ExitWindowsEx(EWX_POWEROFF, 0);
				}
				else
				{
					/* Windows98,Me�ł�EWX_POWEROFF���Ƃ��܂������Ȃ� */
					Sts = ExitWindowsEx(EWX_SHUTDOWN, 0);
				}
				break;

			case 3 :
			case 5 :
				if(SetSystemPowerState(TRUE, FALSE) == 0)
				{
					Sts = FALSE;
				}
				break;

			case 4 :
			case 6 :
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
 ���O	:	DoCountDown
 ����	:	�V���b�g�_�E���O�̃J�E���g�_�E���E�C���h�E
 ����	:	State	�X�e�[�g�i2���V���b�g�_�E��, 3=�X�^���o�C, 4=�x�~, 5=�X�^���o�C, 6=�x�~)
 �߂�l :	int	�X�e�[�^�X (YES=�V���b�g�_�E������)
-----------------------------------------------------------------------------*/
int DoCountDown(int State)
{
	return(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(countdown_dlg), GetDesktopWindow(), CountDownDialogProc, State));
}


/*-----------------------------------------------------------------------------
 ���O	:	DoCountDown
 ����	:	�J�E���g�_�E���E�C���h�E�̃R�[���o�b�N
 ����	:	hDlg	�E�C���h�E�n���h��
			message	���b�Z�[�W�ԍ�
			wParam	���b�Z�[�W�� WPARAM ����
			lParam	���b�Z�[�W�� LPARAM ����
 �߂�l :	BOOL	TRUE/FALSE
-----------------------------------------------------------------------------*/
BOOL CALLBACK CountDownDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int		Count;
	static UINT		TimerID;
	_TCHAR			Tmp[20];
	OSVERSIONINFO	osvi;

	switch (message)
	{
		case WM_INITDIALOG :
			Count = SHUTDOWN_PERIOD;
			if(lParam == 2)
				SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_98);
			else if((lParam == 3) || (lParam == 5))
			{
				GetVersionEx(&osvi);
				if(osvi.dwMajorVersion >= WINDOWS_VISTA_VERSION)
					SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_122);
				else
					SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_99);
			}
			else if((lParam == 4) || (lParam == 6))
				SendDlgItemMessage(hDlg, CDOWN_MSG2, WM_SETTEXT, 0, (LPARAM)MSGJPN_100);
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


