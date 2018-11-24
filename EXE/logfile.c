/*===========================================================================
/
/                                   Backup
/                               ���O�t�@�C��
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
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>
#include <windowsx.h>
#include <commctrl.h>

#include "common.h"
#include "resource.h"


/*===== �v���g�^�C�v =====*/

static void MakeLogDir(LPTSTR Path);
static void CheckLogFileSize(void);
static LPTSTR GetTimeString(void);
static void MakeLogFileName(LPTSTR Fname, LPTSTR Buf);
static LPTSTR GetTimeStringForFname(void);

/*===== ���[�J���ȃ��[�N ======*/

static LPTSTR   RealLogFname = NULL;
static FILE     *LogStrm = NULL;
static int      DeleteLog = NO;
static LPTSTR   ErrorLogFname = NULL;
static FILE     *ErrorStrm = NULL;

/*===== �O���Q�� =====*/

/* �ݒ�l */
extern int LogSwitch;
extern int LogLimit;
extern int LogUnicode;
extern _TCHAR LogFname[MY_MAX_PATH+1];
extern _TCHAR ViewerName[MY_MAX_PATH+1];
extern _TCHAR LastWroteLogFname[MY_MAX_PATH+10+1];
extern _TCHAR LastErrorLogFname[MY_MAX_PATH+1];



/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C�����I�[�v������
 ����   :   �Ȃ�
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int OpenLogfile(void)
{
    char    *oemStr;    /* not _TCHAR */
    int     length;
    int     result;

    result = SUCCESS;
    if(RealLogFname == NULL)
        RealLogFname = malloc((MY_MAX_PATH+10+1) * sizeof(_TCHAR));
    MakeLogFileName(LogFname, RealLogFname);
    MakeLogDir(RealLogFname);

	_tcscpy(LastWroteLogFname, RealLogFname);

    if((LogSwitch == LOG_SW_NEW) && (DeleteLog == NO))
        DeleteFile_My(RealLogFname, NO);
    DeleteLog = YES;

    LogStrm = NULL;
    if(LogSwitch != LOG_SW_OFF)
    {
        if(LogUnicode)
        {
            LogStrm = _tfopen(RealLogFname, _T("a+t, ccs=UTF-8"));      /* unicode stream */
        }
        else
        {
            /* convert filename to OEM character set and open ANSI stream */
            length = WideCharToMultiByte(CP_OEMCP, 0, RealLogFname, -1, NULL, 0, NULL, NULL);
            oemStr = (char*)malloc(length+1);
            memset(oemStr, NUL, length+1);
            WideCharToMultiByte(CP_OEMCP, 0, RealLogFname, -1, oemStr, length, NULL, NULL);
            LogStrm = fopen(oemStr, "a+t"); /* ANSI stream */
            free(oemStr);
        }

        if(LogStrm == NULL)
        {
            free(RealLogFname);
            RealLogFname = NULL;
            if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(log_err_notify_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)_T("")) == NO)
            {
                result = FAIL;
            }
        }
    }
    return(result);
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C�����쐬����t�H���_�����
 ����   :   Path : ���O�t�@�C����
 �߂�l :   �Ȃ�
 ���l   :   �����ł̓G���[�`�F�b�N���Ȃ��B���s���Ă�fopen�ŃG���[�ƂȂ�B
-----------------------------------------------------------------------------*/
static void MakeLogDir(LPTSTR Path)
{
    LPTSTR Pos;
    _TCHAR Tmp[MY_MAX_PATH+1];
    HANDLE fHnd;
    WIN32_FIND_DATA FindBuf;

    if((_tcslen(Path) > 3) &&
            ((_tcsncmp(Path+1, _T(":\\"), 2) == 0) || (_tcsncmp(Path, _T("\\\\"), 2) == 0)))
    {
        Pos = Path + 2;
        if(_tcsncmp(Path, _T("\\\\"), 2) == 0)
        {
            Pos = _tcschr(Path+2, _T('\\'));
            if(Pos != NULL)
            {
                Pos = _tcschr(Pos+1, _T('\\'));
            }
        }
        while((Pos != NULL) && ((Pos = _tcschr(Pos+1, _T('\\'))) != NULL))
        {
            _tcsncpy(Tmp, Path, Pos - Path);
            Tmp[Pos - Path] = NUL;
            if((fHnd = FindFirstFile(Tmp, &FindBuf)) == INVALID_HANDLE_VALUE)
            {
                CreateDirectory(Tmp, NULL);
            }
            else
            {
                FindClose(fHnd);
            }
        }
    }
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C�����N���[�Y����
 ����   :   �Ȃ�
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int CloseLogfile(void)
{
    if(LogStrm != NULL)
        fclose(LogStrm);
    LogStrm = NULL;

    return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C���������폜����
 ����   :   �Ȃ�
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int DeleteLogFilename(void)
{
    if(RealLogFname != NULL)
    {
        free(RealLogFname);
        RealLogFname = NULL;
    }
    return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C���Ƀ��b�Z�[�W���L�^
 ����   :   Msg     [in] ���b�Z�[�W
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int WriteMsgToLogfile(LPTSTR Msg)
{
    char    *oemStr;    /* not _TCHAR */
    int     length;

    if(LogStrm != NULL)
    {
        CheckLogFileSize();

        if(LogUnicode)
        {
            _ftprintf(LogStrm, _T("%s\n"), Msg);
        }
        else
        {
            /* convert to OEM character set and write it */
            length = WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, NULL, 0, NULL, NULL);
            oemStr = (char*)malloc(length+1);
            memset(oemStr, NUL, length+1);
            WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, oemStr, length, NULL, NULL);
            fprintf(LogStrm, "%s\n", oemStr);   /* ANSI */
            free(oemStr);
        }
    }
    return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C���̃T�C�Y���`�F�b�N�i�K�v�Ȃ�V�����̂��쐬�j
 ����   :   �Ȃ�
 �߂�l :   �Ȃ�
 ���l   :
-----------------------------------------------------------------------------*/
static void CheckLogFileSize(void)
{
    _TCHAR Tmp[MY_MAX_PATH+1+15];
    WIN32_FIND_DATA FindBuf;
    HANDLE fHnd;
    int Max;

    if((LogSwitch == LOG_SW_APPEND) && (LogLimit > 0))
    {
        if((ftell(LogStrm) / 1024) >= LogLimit)
        {
            CloseLogfile();

            Max = 0;
            _stprintf(Tmp, _T("%s.*"), RealLogFname);
            if((fHnd = FindFirstFile_My(Tmp, &FindBuf, NO)) != INVALID_HANDLE_VALUE)
            {
                do
                {
                    Max = max(Max, _tstoi(GetFileExt(FindBuf.cFileName)));
                }
                while(FindNextFile(fHnd, &FindBuf) == TRUE);
                FindClose(fHnd);
            }

            _stprintf(Tmp, _T("%s.%d"), RealLogFname, Max+1);
            MoveFile_My(RealLogFname, Tmp, NO);

            OpenLogfile();
        }
    }
    return;
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C���Ƀ^�C�g��������������
 ����   :   Name			[in] �p�^�[����
			SrcPath     [in] �o�b�N�A�b�v��
            DstPath     [in] �o�b�N�A�b�v��
 �߂�l :   �Ȃ�
 ���l   :
-----------------------------------------------------------------------------*/
void WriteTitleToLogfile(LPTSTR Name, LPTSTR SrcPath, LPTSTR DstPath)
{
    _TCHAR Tmp[MY_MAX_PATH+20];

    WriteMsgToLogfile(_T(""));
    _stprintf(Tmp, _T("#### Backup Ver %s ####"), PROGRAM_VERSION);
    WriteMsgToLogfile(Tmp);

	_stprintf(Tmp, MSGJPN_133, Name);
	WriteMsgToLogfile(Tmp);
	
	_stprintf(Tmp, MSGJPN_0, GetTimeString());
    WriteMsgToLogfile(Tmp);

    while(*SrcPath != NUL)
    {
        _stprintf(Tmp, MSGJPN_1, SrcPath);
        WriteMsgToLogfile(Tmp);
        SrcPath = _tcschr(SrcPath, NUL) + 1;
    }

    _stprintf(Tmp, MSGJPN_2, DstPath);
    WriteMsgToLogfile(Tmp);
    return;
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C���ɏI����������������
 ����   :   �Ȃ�
 �߂�l :   �Ȃ�
 ���l   :
-----------------------------------------------------------------------------*/
void WriteEndTimeToLogfile(void)
{
    _TCHAR Tmp[256];

    _stprintf(Tmp, MSGJPN_3, GetTimeString());
    WriteMsgToLogfile(Tmp);
    return;
}


/*-----------------------------------------------------------------------------
 ����   :   ���݂̎����������Ԃ�
 ����   :   �Ȃ�
 �߂�l :   ����������
 ���l   :
-----------------------------------------------------------------------------*/
static LPTSTR GetTimeString(void)
{
    static _TCHAR Buf[80];
    static const LPTSTR WeekStr[] = { MSGJPN_4, MSGJPN_5, MSGJPN_6, MSGJPN_7, MSGJPN_8, MSGJPN_9, MSGJPN_10 };
    SYSTEMTIME lTime;

    GetLocalTime(&lTime);
    _stprintf(Buf, MSGJPN_11, lTime.wYear, lTime.wMonth, lTime.wDay, WeekStr[lTime.wDayOfWeek], lTime.wHour, lTime.wMinute);

    return(Buf);
}


/*-----------------------------------------------------------------------------
 ����   :   ���O�t�@�C�������쐬����
 ����   :   Fname   [in] �t�@�C����
            Buf     [out] �o�b�t�@
 �߂�l :   �Ȃ�
 ���l   :
-----------------------------------------------------------------------------*/
static void MakeLogFileName(LPTSTR Fname, LPTSTR Buf)
{
    int Flag;

    Flag = 1;
    while(*Fname != NUL)
    {
        if(*Fname == _T('$'))
        {
            Fname++;
            if(*Fname == NUL)
                break;
            if(*Fname == _T('$'))
                *Buf++ = _T('$');
            else if((toupper(*Fname) == _T('D')) && (Flag))
            {
                Flag = 0;
                _tcscpy(Buf, GetTimeStringForFname());
                Buf = _tcschr(Buf, NUL);
            }
            Fname++;
        }
        else
            *Buf++ = *Fname++;
    }
    *Buf = NUL;
    return;
}


/*-----------------------------------------------------------------------------
 ����   :   �t�@�C�����p�̓��t�������Ԃ�
 ����   :   �Ȃ�
 �߂�l :   ���t������
 ���l   :
-----------------------------------------------------------------------------*/
static LPTSTR GetTimeStringForFname(void)
{
    static _TCHAR Buf[10];
    SYSTEMTIME lTime;

    GetLocalTime(&lTime);
    _stprintf(Buf, _T("%04d%02d%02d"), lTime.wYear, lTime.wMonth, lTime.wDay);

    return(Buf);
}


/*-----------------------------------------------------------------------------
 ����   :   ���O���r���[���ŕ\��
 ����   :   �Ȃ�
 �߂�l :   �Ȃ�
 ���l   :
-----------------------------------------------------------------------------*/
void DispLogWithViewer(void)
{
    if(_tcslen(LastWroteLogFname) > 0)
        ExecViewer(LastWroteLogFname, ViewerName);
    else
        DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_119);
    return;
}


/*-----------------------------------------------------------------------------
 ����   :   �G���[�L�^��p���O�t�@�C�����I�[�v������
 ����   :   �Ȃ�
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int OpenErrorLogfile(void)
{
    int     length;
    int     result;
    char    *oemStr;    /* not _TCHAR */

    result = FAIL;
    if(ErrorLogFname == NULL)
    {
        ErrorLogFname = malloc((MY_MAX_PATH+1) * sizeof(_TCHAR));
        GetTempPath(MY_MAX_PATH, ErrorLogFname);
        if(GetTempFileName(ErrorLogFname, _T("$$$"), 0, ErrorLogFname) == 0)
        {
            free(ErrorLogFname);
            ErrorLogFname = NULL;
			_tcscpy(LastErrorLogFname, _T(""));
		}
		else
		{
			if (_tcslen(LastErrorLogFname) > 0)
			{
				DeleteFile_My(LastErrorLogFname, NO);
			}
			_tcscpy(LastErrorLogFname, ErrorLogFname);
		}
    }

    if(ErrorLogFname != NULL)
    {
        if(LogUnicode)
        {
            ErrorStrm = _tfopen(ErrorLogFname, _T("wt, ccs=UTF-8"));        /* unicode stream */
        }
        else
        {
            /* convert filename to OEM character set and open ANSI stream */
            length = WideCharToMultiByte(CP_OEMCP, 0, ErrorLogFname, -1, NULL, 0, NULL, NULL);
            oemStr = (char*)malloc(length+1);
            memset(oemStr, NUL, length+1);
            WideCharToMultiByte(CP_OEMCP, 0, ErrorLogFname, -1, oemStr, length, NULL, NULL);
            ErrorStrm = fopen(oemStr, "wt");    /* ANSI stream */
            free(oemStr);
        }

        if(ErrorStrm != NULL)
        {
            result = SUCCESS;
        }
        else
        {
            free(ErrorLogFname);
            ErrorLogFname = NULL;
        }
    }
    return(result);
}


/*-----------------------------------------------------------------------------
 ����   :   �G���[�L�^��p���O�t�@�C�����N���[�Y����
 ����   :   �Ȃ�
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int CloseErrorLogfile(void)
{
    if(ErrorStrm != NULL)
    {
        fclose(ErrorStrm);
    }
    ErrorStrm = NULL;

    return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 ����   :   �G���[�L�^��p���O�t�@�C���������폜����
 ����   :   �Ȃ�
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int DeleteErrorLogFilename(void)
{
    if(ErrorLogFname != NULL)
    {
        free(ErrorLogFname);
        ErrorLogFname = NULL;
    }
    return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 ����   :   �G���[��p���O�t�@�C���Ƀ��b�Z�[�W���L�^
 ����   :   Msg     [in] ���b�Z�[�W
 �߂�l :   �X�e�[�^�X
 ���l   :
-----------------------------------------------------------------------------*/
int WriteMsgToErrorLogfile(LPTSTR Msg)
{
    int     length;
    char    *oemStr;    /* not _TCHAR */

    if(ErrorStrm != NULL)
    {
        if(LogUnicode)
        {
            _ftprintf(ErrorStrm, _T("%s\n"), Msg);
        }
        else
        {
            /* convert to OEM character set and write it */
            length = WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, NULL, 0, NULL, NULL);
            oemStr = (char*)malloc(length+1);
            memset(oemStr, NUL, length+1);
            WideCharToMultiByte(CP_OEMCP, 0, Msg, -1, oemStr, length, NULL, NULL);
            fprintf(ErrorStrm, "%s\n", oemStr); /* ANSI */
            free(oemStr);
        }
    }
    return(SUCCESS);
}


/*-----------------------------------------------------------------------------
 ����   :   �G���[��p���O���r���[���ŕ\��
 ����   :   �Ȃ�
 �߂�l :   �Ȃ�
 ���l   :
-----------------------------------------------------------------------------*/
void DispErrorLogWithViewer(void)
{
	if (_tcslen(LastErrorLogFname) > 0)
		ExecViewer(LastErrorLogFname, ViewerName);
    else
        DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_119);
    return;
}


/*-----------------------------------------------------------------------------
����   :   ���O�t�@�C���̃t�H���_���G�N�X�v���[���[�ŊJ��
����   :   �Ȃ�
�߂�l :   �Ȃ�
-----------------------------------------------------------------------------*/
void OpenLogDir(void)
{
	LPTSTR Pos;
	_TCHAR Path[MY_MAX_PATH + 1];

	_tcscpy(Path, LogFname);

	if ((_tcslen(Path) > 3) &&
		((_tcsncmp(Path + 1, _T(":\\"), 2) == 0) || (_tcsncmp(Path, _T("\\\\"), 2) == 0)))
	{
		Pos = Path + 3;
		Pos = _tcsrchr(Path, _T('\\'));
		if (Pos != NULL)
		{
			*Pos = _T('\0');
		}

		ShellExecute(NULL, _T("open"), Path, NULL, NULL, SW_SHOWNORMAL);
	}
}


