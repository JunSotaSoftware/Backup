/*===========================================================================
/
/                                   Backup
/                               ���W�X�g������
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

static void SetRegType(int Type);
static int OpenReg(LPTSTR Name, void **Handle);
static int CreateReg(LPTSTR Name, void **Handle);
static int CloseReg(void *Handle);
static int OpenSubKey(void *Parent, LPTSTR Name, void **Handle);
static int CreateSubKey(void *Parent, LPTSTR Name, void **Handle);
static int CloseSubKey(void *Handle);
static int DeleteSubKey(void *Handle, LPTSTR Name);
static int DeleteValue(void *Handle, LPTSTR Name);
static int ReadIntValueFromReg(void *Handle, LPTSTR Name, int *Value);
static int WriteIntValueToReg(void *Handle, LPTSTR Name, int Value);
static int ReadStringFromReg(void *Handle, LPTSTR Name, LPTSTR Str, DWORD Size);
static int WriteStringToReg(void *Handle, LPTSTR Name, LPTSTR Str);
static int ReadMultiStringFromReg(void *Handle, LPTSTR Name, LPTSTR Str, DWORD Size);
static int WriteMultiStringToReg(void *Handle, LPTSTR Name, LPTSTR Str);
static int ReadBinaryFromReg(void *Handle, LPTSTR Name, void *Bin, DWORD Size);
static int WriteBinaryToReg(void *Handle, LPTSTR Name, void *Bin, int Len);

/*===== �O���[�o���ȃ��[�N ======*/

/* �ݒ� */
extern int LogSwitch;
extern int LogLimit;
extern int LogUnicode;
extern int LogVerbose;
extern _TCHAR LogFname[MY_MAX_PATH+1];
extern _TCHAR ViewerName[MY_MAX_PATH+1];
extern int SaveWinPos;
extern int MainPosX;
extern int MainPosY;
extern int TrayIcon;
extern int RegType;
extern SIZE MainDlgSize;
extern SIZE TransDlgSize;
extern SIZE NotifyDlgSize;
extern int ExitOnEsc;
extern int ShowComment;     /* 0=�\�����Ȃ�,1=�c�[���`�b�v�ŕ\���A2=�E�C���h�E�ŕ\�� */
extern int AuthDialog;
extern _TCHAR LastWroteLogFname[MY_MAX_PATH+10+1];
extern _TCHAR LastErrorLogFname[MY_MAX_PATH+1];



/*----- ���W�X�g���ɕۑ� ------------------------------------------------------
*
*   Parameter
*       �Ȃ�
*
*   Return Value
*       int �X�e�[�^�X
*----------------------------------------------------------------------------*/

int SaveRegistory(void)
{
    void *hKey3;
    void *hKey4;
    void *hKey5;
    _TCHAR Str[20];
    int Count;
    int i;
    int Tmp;
    COPYPAT Pat;


    SetRegType(RegType);
    if(CreateReg(_T("Backup"), &hKey3) == SUCCESS)
    {
        WriteIntValueToReg(hKey3, _T("Version"), PROGRAM_VERSION_NUM);

        if(CreateSubKey(hKey3, _T("Option"), &hKey4) == SUCCESS)
        {
            WriteIntValueToReg(hKey4, _T("Log"), LogSwitch);
            WriteIntValueToReg(hKey4, _T("LogLimit"), LogLimit);
            WriteIntValueToReg(hKey4, _T("LogUnicode"), LogUnicode);
            WriteIntValueToReg(hKey4, _T("LogVerbose"), LogVerbose);
            WriteStringToReg(hKey4, _T("LogFile"), LogFname);
            WriteStringToReg(hKey4, _T("Viewer"), ViewerName);
            WriteIntValueToReg(hKey4, _T("SavePos"), SaveWinPos);
            WriteIntValueToReg(hKey4, _T("MainX"), MainPosX);
            WriteIntValueToReg(hKey4, _T("MainY"), MainPosY);
            WriteIntValueToReg(hKey4, _T("Tray"), TrayIcon);
            WriteIntValueToReg(hKey4, _T("Reg"), RegType);
            WriteIntValueToReg(hKey4, _T("ExitEsc"), ExitOnEsc);
            WriteIntValueToReg(hKey4, _T("ShowComment"), ShowComment);
            WriteIntValueToReg(hKey4, _T("AuthDialog"), AuthDialog);

            WriteBinaryToReg(hKey4, _T("MainSize"), &MainDlgSize, sizeof(SIZE));
            WriteBinaryToReg(hKey4, _T("TransSize"), &TransDlgSize, sizeof(SIZE));
            WriteBinaryToReg(hKey4, _T("NotifySize"), &NotifyDlgSize, sizeof(SIZE));

			WriteStringToReg(hKey4, _T("LastWroteLogFile"), LastWroteLogFname);
			WriteStringToReg(hKey4, _T("LastErrorLogFile"), LastErrorLogFname);

            /* �Â��`���̃��W�X�g�����폜 */
            DeleteValue(hKey4, _T("IntTime"));

            CloseSubKey(hKey4);
        }

        if(CreateSubKey(hKey3, _T("Patterns"), &hKey4) == SUCCESS)
        {
            Count = GetPatterns();
            WriteIntValueToReg(hKey4, _T("Num"), Count);

            for(i = 0; i < Count; i++)
            {
                if(CopyPatFromList(i, &Pat) == SUCCESS)
                {
                    _stprintf(Str, _T("Pat%d"), i);
                    if(CreateSubKey(hKey4, Str, &hKey5) == SUCCESS)
                    {
                        WriteStringToReg(hKey5, _T("Name"), Pat.Name);
                        WriteStringToReg(hKey5, _T("Comment"), Pat.Comment);
                        WriteMultiStringToReg(hKey5, _T("Src"), Pat.Src);
                        WriteMultiStringToReg(hKey5, _T("Dst"), Pat.Dst);
                        WriteMultiStringToReg(hKey5, _T("IgnDir"), Pat.IgnDir);
                        WriteMultiStringToReg(hKey5, _T("IgnFile"), Pat.IgnFile);
                        Tmp = 0;
                        Tmp |= (Pat.ForceCopy == YES)			? OPT_FORCE            : 0;
                        Tmp |= (Pat.DelDir == YES)				? OPT_RMDIR            : 0;
                        Tmp |= (Pat.DelFile == YES)				? OPT_RMFILE           : 0;
                        Tmp |= (Pat.IgnoreErr == YES)			? OPT_NOERROR          : 0;
                        Tmp |= (Pat.NotifyDel == YES)			? OPT_NOTIFY_DEL       : 0;
                        Tmp |= (Pat.IgnNoDel == YES)			? OPT_IGNNODEL         : 0;
                        Tmp |= (Pat.NewOnly == YES)				? OPT_NEWONLY          : 0;
                        Tmp |= (Pat.ChkVolLabel == YES)			? OPT_CHK_LABEL        : 0;
                        Tmp |= (Pat.UseTrashCan == YES)			? OPT_TRASHCAN         : 0;
                        Tmp |= (Pat.IgnSystemFile == YES)		? OPT_IGN_SYSTEM       : 0;
                        Tmp |= (Pat.IgnHiddenFile == YES)		? OPT_IGN_HIDDEN       : 0;
                        Tmp |= (Pat.IgnAttr == YES)				? OPT_IGN_ATTR         : 0;
                        Tmp |= (Pat.Sound == YES)				? OPT_PLAY_SOUND       : 0;
                        Tmp |= (Pat.NoMakeTopDir == YES)		? OPT_NO_TOPDIR        : 0;
                        Tmp |= (Pat.IgnTime == YES)				? OPT_IGN_TIME         : 0;
                        Tmp |= (Pat.NotifyOvw == YES)			? OPT_NOTIFY_OVW       : 0;
                        Tmp |= (Pat.ShowComment == YES)			? OPT_SHOW_COMMENT     : 0;
                        Tmp |= (Pat.IgnBigFile == YES)			? OPT_IGN_BIG_FILE     : 0;
						Tmp |= (Pat.DstDropbox == YES)			? OPT_DST_DROPBOX      : 0;
						Tmp |= (Pat.MoveInsteadDelete == YES)	? OPT_MOVE_INSTEAD_DEL : 0;
                        WriteIntValueToReg(hKey5, _T("Flags"), Tmp);
                        WriteIntValueToReg(hKey5, _T("Wait"), Pat.Wait);
                        WriteStringToReg(hKey5, _T("Label"), Pat.VolLabel);
                        WriteIntValueToReg(hKey5, _T("Tole"), Pat.Tolerance);
                        WriteIntValueToReg(hKey5, _T("Close"), Pat.AutoClose);
                        WriteStringToReg(hKey5, _T("Sound"), Pat.SoundFile);
                        WriteIntValueToReg(hKey5, _T("Interval"), Pat.IntervalTime);
                        WriteIntValueToReg(hKey5, _T("Big"), Pat.IgnBigSize);
                        WriteIntValueToReg(hKey5, _T("NextDstNum"), Pat.NextDstNum);
						WriteStringToReg(hKey5, _T("MoveToFolder"), Pat.MoveToFolder);

                        CloseSubKey(hKey5);
                    }
                }
            }

            /* �Ȃ��Ȃ����G���g�����폜 */
            for(; ; i++)
            {
                _stprintf(Str, _T("Pat%d"), i);
                if(DeleteSubKey(hKey4, Str) != SUCCESS)
                    break;
            }
            CloseSubKey(hKey4);
        }

        /* �Â��`���̃��W�X�g�����폜 */
        DeleteValue(hKey3, _T("Jobs"));
        for(i = 0; ; i++)
        {
            _stprintf(Str, _T("Job%d"), i);
            if(DeleteSubKey(hKey3, Str) != SUCCESS)
                break;
        }
        CloseReg(hKey3);
    }
    return(SUCCESS);
}


/*----- ���W�X�g������ďo ----------------------------------------------------
*
*   Parameter
*       �Ȃ�
*
*   Return Value
*       int �X�e�[�^�X
*----------------------------------------------------------------------------*/

int LoadRegistory(void)
{
    void *hKey3;
    void *hKey4;
    void *hKey5;
    _TCHAR Str[20];
    int Count;
    int i;
    int Tmp;
    COPYPAT Pat;
    int OldIntervalTimeFlg;
    int IntervalTime;
    int Version;

    SetRegType(REGTYPE_INI);
    if((i = OpenReg(_T("Backup"), &hKey3)) != SUCCESS)
    {
        SetRegType(REGTYPE_REG);
        i = OpenReg(_T("Backup"), &hKey3);
    }

    if(i == SUCCESS)
    {
        if(ReadIntValueFromReg(hKey3, _T("Version"), &Version) != SUCCESS)
        {
            Version = 0x010a0100;
        }

        if(OpenSubKey(hKey3, _T("Option"), &hKey4) == SUCCESS)
        {
            ReadIntValueFromReg(hKey4, _T("Log"), &LogSwitch);
            ReadIntValueFromReg(hKey4, _T("LogLimit"), &LogLimit);
            if(ReadIntValueFromReg(hKey4, _T("LogUnicode"), &LogUnicode) == FAIL)
            {
                /* Turn off the unicode-log flag for older version compatibility. */
                LogUnicode = NO;
            }
            ReadIntValueFromReg(hKey4, _T("LogVerbose"), &LogVerbose);
            ReadStringFromReg(hKey4, _T("LogFile"), LogFname, MY_MAX_PATH+1);
            ReadStringFromReg(hKey4, _T("Viewer"), ViewerName, MY_MAX_PATH+1);
            ReadIntValueFromReg(hKey4, _T("SavePos"), &SaveWinPos);
            ReadIntValueFromReg(hKey4, _T("MainX"), &MainPosX);
            ReadIntValueFromReg(hKey4, _T("MainY"), &MainPosY);
            ReadIntValueFromReg(hKey4, _T("Tray"), &TrayIcon);
            ReadIntValueFromReg(hKey4, _T("Reg"), &RegType);
            ReadIntValueFromReg(hKey4, _T("ExitEsc"), &ExitOnEsc);
            ReadIntValueFromReg(hKey4, _T("ShowComment"), &ShowComment);
            ReadIntValueFromReg(hKey4, _T("AuthDialog"), &AuthDialog);

            ReadBinaryFromReg(hKey4, _T("MainSize"), &MainDlgSize, sizeof(SIZE));
            ReadBinaryFromReg(hKey4, _T("TransSize"), &TransDlgSize, sizeof(SIZE));
            ReadBinaryFromReg(hKey4, _T("NotifySize"), &NotifyDlgSize, sizeof(SIZE));

			ReadStringFromReg(hKey4, _T("LastWroteLogFile"), LastWroteLogFname, MY_MAX_PATH + 10 + 1);
			ReadStringFromReg(hKey4, _T("LastErrorLogFile"), LastErrorLogFname, MY_MAX_PATH + 1);

            OldIntervalTimeFlg = ReadIntValueFromReg(hKey4, _T("IntTime"), &IntervalTime);

            CloseSubKey(hKey4);
        }

        if(OpenSubKey(hKey3, _T("Patterns"), &hKey4) == SUCCESS)
        {
            Count = 0;
            ReadIntValueFromReg(hKey4, _T("Num"), &Count);
            for(i = 0; i < Count; i++)
            {
                _stprintf(Str, _T("Pat%d"), i);
                if(OpenSubKey(hKey4, Str, &hKey5) == SUCCESS)
                {
                    CopyDefaultPat(&Pat);
                    Pat.PatNum = i;
                    ReadStringFromReg(hKey5, _T("Name"), Pat.Name, PATNAME_LEN+1);
                    ReadStringFromReg(hKey5, _T("Comment"), Pat.Comment, COMMENT_LEN+1);
                    ReadMultiStringFromReg(hKey5, _T("Src"), Pat.Src, SRC_PATH_LEN+1);
                    memset(Pat.Dst, 0, DST_PATH_LEN+1);
                    if(Version < 0x010b0000)
                    {
                        ReadStringFromReg(hKey5, _T("Dst"), Pat.Dst, DST_PATH_LEN+1);
                    }
                    else
                    {
                        ReadMultiStringFromReg(hKey5, _T("Dst"), Pat.Dst, DST_PATH_LEN+1);
                    }
                    ReadMultiStringFromReg(hKey5, _T("IgnDir"), Pat.IgnDir, IGN_PATH_LEN+1);
                    ReadMultiStringFromReg(hKey5, _T("IgnFile"), Pat.IgnFile, IGN_PATH_LEN+1);
                    ReadIntValueFromReg(hKey5, _T("Flags"), &Tmp);
                    Pat.ForceCopy         = (Tmp & OPT_FORCE)            ? YES : NO;
                    Pat.DelDir            = (Tmp & OPT_RMDIR)            ? YES : NO;
                    Pat.DelFile           = (Tmp & OPT_RMFILE)           ? YES : NO;
                    Pat.IgnoreErr         = (Tmp & OPT_NOERROR)          ? YES : NO;
                    Pat.NotifyDel         = (Tmp & OPT_NOTIFY_DEL)       ? YES : NO;
                    Pat.IgnNoDel          = (Tmp & OPT_IGNNODEL)         ? YES : NO;
                    Pat.NewOnly           = (Tmp & OPT_NEWONLY)          ? YES : NO;
                    Pat.ChkVolLabel       = (Tmp & OPT_CHK_LABEL)        ? YES : NO;
                    Pat.UseTrashCan       = (Tmp & OPT_TRASHCAN)         ? YES : NO;
                    Pat.IgnSystemFile     = (Tmp & OPT_IGN_SYSTEM)       ? YES : NO;
                    Pat.IgnHiddenFile     = (Tmp & OPT_IGN_HIDDEN)       ? YES : NO;
                    Pat.IgnAttr           = (Tmp & OPT_IGN_ATTR)         ? YES : NO;
                    Pat.Sound             = (Tmp & OPT_PLAY_SOUND)       ? YES : NO;
                    Pat.NoMakeTopDir      = (Tmp & OPT_NO_TOPDIR)        ? YES : NO;
                    Pat.IgnTime           = (Tmp & OPT_IGN_TIME)         ? YES : NO;
                    Pat.NotifyOvw         = (Tmp & OPT_NOTIFY_OVW)       ? YES : NO;
                    Pat.ShowComment       = (Tmp & OPT_SHOW_COMMENT)     ? YES : NO;
                    Pat.IgnBigFile        = (Tmp & OPT_IGN_BIG_FILE)     ? YES : NO;
					Pat.DstDropbox        = (Tmp & OPT_DST_DROPBOX)      ? YES : NO;
					Pat.MoveInsteadDelete = (Tmp & OPT_MOVE_INSTEAD_DEL) ? YES : NO;
                    ReadIntValueFromReg(hKey5, _T("Wait"), &Pat.Wait);
                    ReadStringFromReg(hKey5, _T("Label"), Pat.VolLabel, MY_MAX_PATH+1);
                    ReadIntValueFromReg(hKey5, _T("Tole"), &Pat.Tolerance);
                    ReadIntValueFromReg(hKey5, _T("Close"), &Pat.AutoClose);
                    ReadStringFromReg(hKey5, _T("Sound"), Pat.SoundFile, MY_MAX_PATH+1);
                    if((ReadIntValueFromReg(hKey5, _T("Interval"), &Pat.IntervalTime) == FAIL) &&
                       (OldIntervalTimeFlg == SUCCESS))
                    {
                        Pat.IntervalTime = IntervalTime;
                    }
                    ReadIntValueFromReg(hKey5, _T("Big"), &Pat.IgnBigSize);
                    if(ReadIntValueFromReg(hKey5, _T("NextDstNum"), &Pat.NextDstNum) == FAIL)
                    {
                        Pat.NextDstNum = 0;
                    }
					ReadStringFromReg(hKey5, _T("MoveToFolder"), Pat.MoveToFolder, MY_MAX_PATH+1);
                    AddPatToList(&Pat);

                    CloseSubKey(hKey5);
                }
            }
            CloseSubKey(hKey4);
        }
        else
        {
            /* �Â����W�X�g���p�^�[�� */

            Count = 0;
            ReadIntValueFromReg(hKey3, _T("Jobs"), &Count);
            for(i = 0; i < Count; i++)
            {
                _stprintf(Str, _T("Job%d"), i);
                if(OpenSubKey(hKey3, Str, &hKey4) == SUCCESS)
                {
                    CopyDefaultPat(&Pat);
                    ReadStringFromReg(hKey4, _T("Src"), Pat.Src, SRC_PATH_LEN+1);
                    *(Pat.Src + _tcslen(Pat.Src) + 1) = NUL;
                    memset(Pat.Name, NUL, (PATNAME_LEN+1) * sizeof(_TCHAR));
                    _tcsncpy(Pat.Name, Pat.Src, PATNAME_LEN);
                    ReadStringFromReg(hKey4, _T("Dst"), Pat.Dst, DST_PATH_LEN+1);

                    ReadIntValueFromReg(hKey4, _T("All"), &Pat.ForceCopy);
                    ReadIntValueFromReg(hKey4, _T("RmDir"), &Pat.DelDir);
                    ReadIntValueFromReg(hKey4, _T("RmFile"), &Pat.DelFile);
                    ReadIntValueFromReg(hKey4, _T("VoidErr"), &Pat.IgnoreErr);
                    AddPatToList(&Pat);

                    CloseSubKey(hKey4);
                }
            }

            MessageBox(NULL, MSGJPN_39
                             MSGJPN_40
                             MSGJPN_41,
                        _T("Backup"), MB_OK);
        }
        CloseReg(hKey3);
    }
    return(SUCCESS);
}


/*----- ���W�X�g������MediaPath��ǂݍ��� -------------------------------------
*
*   Parameter
*       MediaPath : MediaPath�i�[�o�b�t�@
*       Max : �ő�T�C�Y
*
*   Return Value
*       int �X�e�[�^�X
*----------------------------------------------------------------------------*/

int GetMediaPath(LPTSTR MediaPath, int Max)
{
    HKEY hKey1;
    DWORD Size;
    DWORD Type;

    _tcscpy(MediaPath, _T(""));
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"), 0, KEY_READ, &hKey1) == ERROR_SUCCESS)
    {
        Size = Max * sizeof(_TCHAR);
        RegQueryValueEx(hKey1, _T("MediaPath"), NULL, &Type, (BYTE *)MediaPath, &Size);
        if(*(MediaPath + Size - 1) != NUL)
            *(MediaPath + Size) = NUL;

        RegCloseKey(hKey1);
    }
    return(SUCCESS);
}


/*----- ���W�X�g���̐ݒ�l���N���A --------------------------------------------
*
*   Parameter
*       �Ȃ�
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void ClearRegistory(void)
{
    HKEY hKey2;
    HKEY hKey3;
    HKEY hKey4;
    DWORD Dispos;
    _TCHAR Str[20];
    int i;

    if(RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Sota"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY, NULL, &hKey2, &Dispos) == ERROR_SUCCESS)
    {
        if(RegCreateKeyEx(hKey2, _T("Backup"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey3, &Dispos) == ERROR_SUCCESS)
        {
            RegDeleteKey(hKey3, _T("Option"));

            if(RegCreateKeyEx(hKey3, _T("Patterns"), 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey4, &Dispos) == ERROR_SUCCESS)
            {
                for(i = 0; ; i++)
                {
                    _stprintf(Str, _T("Pat%d"), i);
                    if(RegDeleteKey(hKey4, Str) != ERROR_SUCCESS)
                        break;
                }
                RegCloseKey(hKey4);
            }
            RegDeleteKey(hKey3, _T("Patterns"));
            RegCloseKey(hKey3);
        }
        RegDeleteKey(hKey2, _T("Backup"));
        RegCloseKey(hKey2);
    }
    return;
}


/*----- �ݒ���t�@�C���ɕۑ� --------------------------------------------------
*
*   Parameter
*       �Ȃ�
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

void SaveSettingsToFile(void)
{
    _TCHAR Tmp[MY_MAX_PATH*2];
    _TCHAR Fname[MY_MAX_PATH+1];

    if(RegType == REGTYPE_REG)
    {
        _tcscpy(Fname, _T("Backup.reg"));
        if(SelectFile(GetMainHwnd(), Fname, MSGJPN_42, MSGJPN_43, _T("reg"), OFN_EXTENSIONDIFFERENT | OFN_OVERWRITEPROMPT, 1, NULL) == TRUE)
        {
            _stprintf(Tmp, _T("/e \x22%s\x22 HKEY_CURRENT_USER\\Software\\sota\\Backup"), Fname);
            if(ShellExecute(NULL, _T("open"), _T("regedit"), Tmp, _T("."), SW_SHOW) <= (HINSTANCE)32)
            {
                DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_44);
            }
        }
    }
    else
    {
        _tcscpy(Fname, MSGJPN_45);
        if(SelectFile(GetMainHwnd(), Fname, MSGJPN_42, MSGJPN_47, _T("ini"), OFN_EXTENSIONDIFFERENT | OFN_OVERWRITEPROMPT, 1, NULL) == TRUE)
        {
            CopyFile(AskIniFilePath(), Fname, FALSE);
        }
    }
    return;
}


/*----- �ݒ���t�@�C�����畜�� ------------------------------------------------
*
*   Parameter
*       �Ȃ�
*
*   Return Value
*       int ���[�h�������ǂ��� (YES/NO)
*----------------------------------------------------------------------------*/

int LoadSettingsFromFile(void)
{
    int Ret;
    _TCHAR Tmp[MY_MAX_PATH*2];
    _TCHAR Fname[MY_MAX_PATH+1];

    Ret = NO;
    _tcscpy(Fname, _T(""));
    if(SelectFile(GetMainHwnd(), Fname, MSGJPN_48, MSGJPN_49, _T(""), OFN_FILEMUSTEXIST, 0, NULL) == TRUE)
    {
        if((_tcslen(Fname) >= 5) && (_tcsicmp(&Fname[_tcslen(Fname)-4], _T(".reg")) == 0))
        {
            _stprintf(Tmp, _T("\x22%s\x22"), Fname);

            if(ShellExecute(NULL, _T("open"), _T("regedit"), Tmp, _T("."), SW_SHOW) <= (HINSTANCE)32)
            {
                DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_44);
            }
            else
            {
                Ret = YES;
                /* ���W�X�g���G�f�B�^���I������̂�҂� */
//              WaitForSingleObject(Info.hProcess, INFINITE);
            }
        }
        else if((_tcslen(Fname) >= 5) && (_tcsicmp(&Fname[_tcslen(Fname)-4], _T(".ini")) == 0))
        {
            CopyFile(Fname, AskIniFilePath(), FALSE);
            Ret = YES;
        }
        else
            DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_51);
    }
    return(Ret);
}




/*===== ���W�X�g����INI�t�@�C���̃A�N�Z�X���� ============*/


/*===== INI�t�@�C���p�̃��W�X�g���f�[�^ =====*/

typedef struct regdatatbl {
    char    KeyName[80+1];          /* �L�[�� (ANSI) */
    char    ValTbl[REG_SECT_MAX];   /* �l�̃e�[�u�� (ANSI) */
    int     ValLen;                 /* �l�f�[�^�̃o�C�g�� */
    int     Mode;                   /* �L�[�̃��[�h */
    struct regdatatbl *Next;
} REGDATATBL;

/*===== �v���g�^�C�v =====*/

static BOOL WriteOutRegToFile(REGDATATBL *Pos);
static int ReadInReg(LPTSTR Name, REGDATATBL **Handle);
static int Unicode2AnsiCat(char *buf, LPTSTR str);
static int AnsiCat(char *Src, int Len, char *Dst);
static int StrReadIn(char *Src, int Max, LPTSTR Dst, BOOL multi);
static int StrReadInAnsi(char *Src, int Max, char *Dst);
static char *ScanValue(void *Handle, LPTSTR Name);


/*===== ���[�J���ȃ��[�N =====*/

static int TmpRegType;



/*----- ���W�X�g���̃^�C�v��ݒ肷�� ------------------------------------------
*
*   Parameter
*       int Type : �^�C�v (REGTYPE_xxx)
*
*   Return Value
*       int �X�e�[�^�X
*----------------------------------------------------------------------------*/

static void SetRegType(int Type)
{
    TmpRegType = Type;
    return;
}


/*----- ���W�X�g��/INI�t�@�C�����I�[�v������i�ǂݍ��݁j-----------------------
*
*   Parameter
*       LPTSTR Name : ���W�X�g����
*       void **Handle : �n���h����Ԃ����[�N
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int OpenReg(LPTSTR Name, void **Handle)
{
    int Sts;
    _TCHAR Tmp[MY_MAX_PATH+1];

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        _tcscpy(Tmp, _T("Software\\Sota\\"));
        _tcscat(Tmp, Name);
        if(RegOpenKeyEx(HKEY_CURRENT_USER, Tmp, 0, KEY_READ, (HKEY *)Handle) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        if((Sts = ReadInReg(Name, (REGDATATBL **)Handle)) == SUCCESS)
            ((REGDATATBL *)(*Handle))->Mode = 0;
    }
    return(Sts);
}


/*----- ���W�X�g��/INI�t�@�C�����쐬����i�������݁j---------------------------
*
*   Parameter
*       LPTSTR Name : ���W�X�g����
*       void **Handle : �n���h����Ԃ����[�N
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int CreateReg(LPTSTR Name, void **Handle)
{
    int Sts;
    _TCHAR Tmp[MY_MAX_PATH+1];
    DWORD Dispos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        _tcscpy(Tmp, _T("Software\\Sota\\"));
        _tcscat(Tmp, Name);
        if(RegCreateKeyEx(HKEY_CURRENT_USER, Tmp, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY | KEY_SET_VALUE, NULL, (HKEY *)Handle, &Dispos) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        if((*Handle = malloc(sizeof(REGDATATBL))) != NULL)
        {
            strcpy(((REGDATATBL *)(*Handle))->KeyName, "");     /* ANSI */
            Unicode2AnsiCat(((REGDATATBL *)(*Handle))->KeyName, Name);
            ((REGDATATBL *)(*Handle))->ValLen = 0;
            ((REGDATATBL *)(*Handle))->Next = NULL;
            ((REGDATATBL *)(*Handle))->Mode = 1;
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- ���W�X�g��/INI�t�@�C�����N���[�Y���� ----------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int CloseReg(void *Handle)
{
    REGDATATBL *Pos;
    REGDATATBL *Next;
    FILE *Strm;

    if(TmpRegType == REGTYPE_REG)
    {
        RegCloseKey(Handle);

        /* INI�t�@�C�����폜 */
        if((Strm = _tfopen(AskIniFilePath(), _T("rt"))) != NULL)
        {
            fclose(Strm);
            MoveFileToTrashCan(AskIniFilePath());
        }
    }
    else
    {
        if(((REGDATATBL *)Handle)->Mode == 1)
        {
            if(WriteOutRegToFile(Handle) == TRUE)
            {
//              /* ���W�X�g�����N���A */
//              ClearRegistory();
            }
        }
        /* �e�[�u�����폜 */
        Pos = Handle;
        while(Pos != NULL)
        {
            Next = Pos->Next;
            free(Pos);
            Pos = Next;
        }
    }
    return(SUCCESS);
}


/*----- ���W�X�g������INI�t�@�C���ɏ������� ---------------------------------
*
*   Parameter
*       REGDATATBL *Pos : ���W�X�g���f�[�^
*
*   Return Value
*       �Ȃ�
*----------------------------------------------------------------------------*/

static BOOL WriteOutRegToFile(REGDATATBL *Pos)
{
    FILE    *Strm;
    char    *Disp;
    BOOL    Ret;
    char    *fname;

    Ret = FALSE;
    /* do not use unicode since compatibility */
    fname = AskIniFilePathAnsi();
    if((Strm = fopen(fname, "wt")) != NULL) /* ANSI */
    {
        fprintf(Strm, MSGJPN_52);
        while(Pos != NULL)
        {
            fprintf(Strm, "\n[%s]\n", Pos->KeyName);

            Disp = Pos->ValTbl;
            while(Disp < (Pos->ValTbl + Pos->ValLen))
            {
                fprintf(Strm, "%s\n", Disp);
                Disp = Disp + strlen(Disp) + 1; /* ANSI */
            }
            Pos = Pos->Next;
        }
        fclose(Strm);
        Ret = TRUE;
    }
    else
    {
        DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), GetMainHwnd(), ExeEscDialogProc, (LPARAM)MSGJPN_53);
    }
    free(fname);

    return(Ret);
}


/*----- INI�t�@�C�����烌�W�X�g������ǂݍ��� -------------------------------
*
*   Parameter
*       Name : ���O
*       Handle : �n���h��
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int ReadInReg(LPTSTR Name, REGDATATBL **Handle)
{
    FILE *Strm;
    char *Buf;      /* ANSI */
    char *Tmp;      /* ANSI */
    char *Data;     /* ANSI */
    REGDATATBL *New;
    REGDATATBL *Pos;
    int Sts;
    char    *fname;

    Sts = FAIL;
    *Handle = NULL;

    /* do not use unicode since compatibility */
    fname = AskIniFilePathAnsi();
    if((Strm = fopen(fname, "rt")) != NULL)     /* ANSI */
    {
        if((Buf = malloc(REG_SECT_MAX)) != NULL)
        {
            while(fgets(Buf, REG_SECT_MAX, Strm) != NULL)   /* ANSI */
            {
                if(*Buf != '#')     /* ANSI */
                {
                    if((Tmp = strchr(Buf, '\n')) != NULL)
                        *Tmp = NUL;

                    if(*Buf == '[')
                    {
                        if((New = malloc(sizeof(REGDATATBL))) != NULL)
                        {
                            if((Tmp = strchr(Buf, ']')) != NULL)    /* ANSI */
                                *Tmp = NUL;
                            strcpy(New->KeyName, Buf+1);            /* ANSI */
                            New->ValLen = 0;
                            New->Next = NULL;
                            Data = New->ValTbl;
                        }
                        if(*Handle == NULL)
                            *Handle = New;
                        else
                        {
                            Pos = *Handle;
                            while(Pos->Next != NULL)
                                Pos = Pos->Next;
                            Pos->Next = New;
                        }
                    }
                    else if(strlen(Buf) > 0)                /* ANSI */
                    {
                        strcpy(Data, Buf);                  /* ANSI */
                        Data += strlen(Buf) + 1;            /* ANSI */
                        New->ValLen += strlen(Buf) + 1;     /* ANSI */
                    }
                }
            }
            Sts = SUCCESS;
            free(Buf);
        }
        fclose(Strm);
    }
    free(fname);
    return(Sts);
}


/*----- �T�u�L�[���I�[�v������ ------------------------------------------------
*
*   Parameter
*       void *Parent : �e�̃n���h��
*       LPTSTR Name : ���O
*       void **Handle : �n���h����Ԃ����[�N
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int OpenSubKey(void *Parent, LPTSTR Name, void **Handle)
{
    int Sts;
    char Key[80];       /* ANSI */
    REGDATATBL *Pos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        if(RegOpenKeyEx(Parent, Name, 0, KEY_READ, (HKEY *)Handle) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        strcpy(Key, ((REGDATATBL *)Parent)->KeyName);   /* ANSI */
        strcat(Key, "\\");                              /* ANSI */
        Unicode2AnsiCat(Key, Name);
        Pos = Parent;
        while(Pos != NULL)
        {
            if(strcmp(Pos->KeyName, Key) == 0)          /* ANSI */
            {
                *Handle = Pos;
                Sts = SUCCESS;
                break;
            }
            Pos = Pos->Next;
        }
    }
    return(Sts);
}


/*----- �T�u�L�[���쐬���� ----------------------------------------------------
*
*   Parameter
*       void *Parent : �e�̃n���h��
*       LPTSTR Name : ���O
*       void **Handle : �n���h����Ԃ����[�N
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int CreateSubKey(void *Parent, LPTSTR Name, void **Handle)
{
    int Sts;
    DWORD Dispos;
    REGDATATBL *Pos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        if(RegCreateKeyEx(Parent, Name, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, (HKEY *)Handle, &Dispos) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        if((*Handle = malloc(sizeof(REGDATATBL))) != NULL)
        {
            strcpy(((REGDATATBL *)(*Handle))->KeyName, "");     /* ANSI */
            strcat(((REGDATATBL *)(*Handle))->KeyName, ((REGDATATBL *)Parent)->KeyName);
            strcat(((REGDATATBL *)(*Handle))->KeyName, "\\");   /* ANSI */
            Unicode2AnsiCat(((REGDATATBL *)(*Handle))->KeyName, Name);
            ((REGDATATBL *)(*Handle))->ValLen = 0;
            ((REGDATATBL *)(*Handle))->Next = NULL;
            Pos = (REGDATATBL *)Parent;
            while(Pos->Next != NULL)
                Pos = Pos->Next;
            Pos->Next = *Handle;
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- �T�u�L�[���N���[�Y���� ------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int CloseSubKey(void *Handle)
{
    if(TmpRegType == REGTYPE_REG)
        RegCloseKey(Handle);
    else
    {
        /* Nothing */
    }
    return(SUCCESS);
}


/*----- �T�u�L�[���폜���� ----------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int DeleteSubKey(void *Handle, LPTSTR Name)
{
    int Sts;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        if(RegDeleteKey(Handle, Name) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        Sts = FAIL;
    }
    return(Sts);
}


/*----- �l���폜���� ----------------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int DeleteValue(void *Handle, LPTSTR Name)
{
    int Sts;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        if(RegDeleteValue(Handle, Name) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        Sts = FAIL;
    }
    return(Sts);
}


/*----- INT�l��ǂݍ��� -------------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       int *Value : INT�l��Ԃ����[�N
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int ReadIntValueFromReg(void *Handle, LPTSTR Name, int *Value)
{
    int Sts;
    DWORD Size;
    char *Pos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        Size = sizeof(int);
        if(RegQueryValueEx(Handle, Name, NULL, NULL, (BYTE *)Value, &Size) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        if((Pos = ScanValue(Handle, Name)) != NULL)
        {
            *Value = atoi(Pos);
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- INT�l���������� -------------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       int Value : INT�l
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int WriteIntValueToReg(void *Handle, LPTSTR Name, int Value)
{
    REGDATATBL *Pos;
    char *Data;
    char Tmp[20];   /* ANSI */

    if(TmpRegType == REGTYPE_REG)
        RegSetValueEx(Handle, Name, 0, REG_DWORD, (CONST BYTE *)&Value, sizeof(int));
    else
    {
        Pos = (REGDATATBL *)Handle;
        Data = Pos->ValTbl + Pos->ValLen;

        strcpy(Data, "");                   /* ANSI */
        Unicode2AnsiCat(Data, Name);
        strcat(Data, "=");                  /* ANSI */
        sprintf(Tmp, "%d", Value);          /* ANSI */
        strcat(Data, Tmp);                  /* ANSI */
        Pos->ValLen += strlen(Data) + 1;    /* ANSI */
    }
    return(SUCCESS);
}


/*----- �������ǂݍ��� ------------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       LPTSTR Str : �������Ԃ����[�N
*       DWORD Size : �ő�T�C�Y
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int ReadStringFromReg(void *Handle, LPTSTR Name, LPTSTR Str, DWORD Size)
{
    int Sts;
    char *Pos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        Size = Size * sizeof(_TCHAR);
        if(RegQueryValueEx(Handle, Name, NULL, NULL, (BYTE *)Str, &Size) == ERROR_SUCCESS)
        {
            Size /= sizeof(_TCHAR);
            if(*(Str + Size - 1) != NUL)
                *(Str + Size) = NUL;
            Sts = SUCCESS;
        }
    }
    else
    {
        if((Pos = ScanValue(Handle, Name)) != NULL)
        {
            Size = StrReadIn(Pos, Size, Str, FALSE);
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- ��������������� ------------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       LPTSTR Str :������
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int WriteStringToReg(void *Handle, LPTSTR Name, LPTSTR Str)
{
    REGDATATBL *Pos;
    char *Data;

    if(TmpRegType == REGTYPE_REG)
        RegSetValueEx(Handle, Name, 0, REG_SZ, (CONST BYTE *)Str, (_tcslen(Str)+1) * sizeof(_TCHAR));
    else
    {
        Pos = (REGDATATBL *)Handle;
        Data = Pos->ValTbl + Pos->ValLen;
        strcpy(Data, "");               /* ANSI */
        Unicode2AnsiCat(Data, Name);
        strcat(Data, "=");              /* ANSI */
        Pos->ValLen += strlen(Data);    /* ANSI */
        Data = Pos->ValTbl + Pos->ValLen;
        Pos->ValLen += Unicode2AnsiCat(Data, Str) + 1;
    }
    return(SUCCESS);
}


/*----- �}���`�������ǂݍ��� ------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       LPTSTR Str : �������Ԃ����[�N
*       DWORD Size : �ő�T�C�Y
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int ReadMultiStringFromReg(void *Handle, LPTSTR Name, LPTSTR Str, DWORD Size)
{
    int Sts;
    char *Pos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        Size = Size * sizeof(_TCHAR);
        if(RegQueryValueEx(Handle, Name, NULL, NULL, (BYTE *)Str, &Size) == ERROR_SUCCESS)
        {
            Size /= sizeof(_TCHAR);
            if(*(Str + Size - 1) != NUL)
                *(Str + Size) = NUL;
            Sts = SUCCESS;
        }
    }
    else
    {
        if((Pos = ScanValue(Handle, Name)) != NULL)
        {
            Size = StrReadIn(Pos, Size, Str, TRUE);
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- �}���`��������������� ------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       LPTSTR Str : ������
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int WriteMultiStringToReg(void *Handle, LPTSTR Name, LPTSTR Str)
{
    REGDATATBL *Pos;
    char *Data;

    if(TmpRegType == REGTYPE_REG)
        RegSetValueEx(Handle, Name, 0, REG_MULTI_SZ, (CONST BYTE *)Str, (StrMultiLen(Str)+1) * sizeof(_TCHAR));
    else
    {
        Pos = (REGDATATBL *)Handle;
        Data = Pos->ValTbl + Pos->ValLen;
        strcpy(Data, "");               /* ANSI */
        Unicode2AnsiCat(Data, Name);
        strcat(Data, "=");              /* ANSI */
        Pos->ValLen += strlen(Data);    /* ANSI */
        Data = Pos->ValTbl + Pos->ValLen;
        if(*Str != NUL)
        {
            for(; *Str != NUL;)
            {
                Pos->ValLen += Unicode2AnsiCat(Data, Str);
                strcat(Data, "\\00");
                Pos->ValLen += 3;
                Str = _tcschr(Str, NUL) + 1;
            }
        }
        Pos->ValLen += 1;
    }
    return(SUCCESS);
}


/*----- �o�C�i����ǂݍ���-----------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       void *Bin : �o�C�i����Ԃ����[�N
*       DWORD Size : �ő�T�C�Y
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int ReadBinaryFromReg(void *Handle, LPTSTR Name, void *Bin, DWORD Size)
{
    int Sts;
    char *Pos;

    Sts = FAIL;
    if(TmpRegType == REGTYPE_REG)
    {
        if(RegQueryValueEx(Handle, Name, NULL, NULL, (BYTE *)Bin, &Size) == ERROR_SUCCESS)
            Sts = SUCCESS;
    }
    else
    {
        if((Pos = ScanValue(Handle, Name)) != NULL)
        {
            Size = StrReadInAnsi(Pos, Size, Bin);
            Sts = SUCCESS;
        }
    }
    return(Sts);
}


/*----- �o�C�i������������ ----------------------------------------------------
*
*   Parameter
*       void *Handle : �n���h��
*       LPTSTR Name : ���O
*       void *Bin : �o�C�i��
*       int Len : ����
*
*   Return Value
*       int �X�e�[�^�X
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int WriteBinaryToReg(void *Handle, LPTSTR Name, void *Bin, int Len)
{
    REGDATATBL *Pos;
    char *Data;

    if(TmpRegType == REGTYPE_REG)
        RegSetValueEx(Handle, Name, 0, REG_BINARY, (CONST BYTE *)Bin, Len);
    else
    {
        Pos = (REGDATATBL *)Handle;
        Data = Pos->ValTbl + Pos->ValLen;
        strcpy(Data, "");               /* ANSI */
        Unicode2AnsiCat(Data, Name);
        strcat(Data, "=");              /* ANSI */
        Pos->ValLen += strlen(Data);    /* ANSI */
        Data = Pos->ValTbl + Pos->ValLen;
        Pos->ValLen += AnsiCat(Bin, Len, Data) + 1;
    }
    return(SUCCESS);
}


/*-------------------------------------------------------------------------------
Name    :   Unicode2AnsiCat
Desc    :   Convert unicode string to ANSI string and concat string
Param   :   buf     [in/out] ANSI string
            str     [in] unicode string
Return  :   int     length of the ANSI atring
-------------------------------------------------------------------------------*/

static int Unicode2AnsiCat(char *buf, LPTSTR str)
{
    char        *oemStr;    /* not _TCHAR */
    int         length;

    /* convert strings to OEM character set */
    length = WideCharToMultiByte(CP_OEMCP, 0, str, -1, NULL, 0, NULL, NULL);
    oemStr = (char*)malloc(length+1);
    memset(oemStr, NUL, length+1);
    WideCharToMultiByte(CP_OEMCP, 0, str, -1, oemStr, length, NULL, NULL);
    length = AnsiCat(oemStr, strlen(oemStr), buf);      /* ANSI */
    free(oemStr);
    return(length);
}


/*----- �f�[�^���o�b�t�@�ɒǉ��������݂��� ------------------------------------
*
*   Parameter
*       char *Src : ������
*       int len : ������̒���
*       char *Dst : �������݂���o�b�t�@
*
*   Return Value
*       int �ǉ������o�C�g��
*----------------------------------------------------------------------------*/

static int AnsiCat(char *Src, int Len, char *Dst)       /* ANSI */
{
    int Count;

    Dst += strlen(Dst);         /* ANSI */
    Count = 0;
    for(; Len > 0; Len--)
    {
        if(*Src == '\\')        /* ANSI */
        {
            *Dst++ = '\\';      /* ANSI */
            *Dst++ = '\\';      /* ANSI */
            Count += 2;
        }
        else if((*Src >= 0x20) && (*Src <= 0x7E))
        {
            *Dst++ = *Src;
            Count++;
        }
        else
        {
            sprintf(Dst, "\\%02X", *(unsigned char *)Src);  /* ANSI */
            Dst += 3;
            Count += 3;
        }
        Src++;
    }
    *Dst = NUL;
    return(Count);
}


/*----- ��������o�b�t�@�ɓǂݍ��� (ANSI->Unicode�ϊ�) -------------------------
*
*   Parameter
*       LPTSTR Src : ������
*       int Max : �o�b�t�@�̃T�C�Y
*       LPTSTR Dst : �������݂���o�b�t�@
*       multi       �}���`�����񂩂ǂ���
*
*   Return Value
*       int �ǂݍ��񂾃o�C�g��
*----------------------------------------------------------------------------*/
static int StrReadIn(char *Src, int Max, LPTSTR Dst, BOOL multi)
{
    int Count;
    int Tmp;
    char *tmpBuf;
    char *pos;
    LPTSTR uniBuf;
    int length;
    int uniLen;

    Max--;
    uniLen = 0;
    *Dst = NUL;
    if(*Src != NUL)
    {
        tmpBuf = malloc(REG_SECT_MAX);
        pos = tmpBuf;
        Count = 0;
        for(;;)
        {
            if(Count >= REG_SECT_MAX-1)
                break;

            *pos = *Src;
            if(*Src == '\\')
            {
                Src++;
                if(*Src != '\\')
                {
                    sscanf(Src, "%02x", &Tmp);
                    *pos = Tmp;
                    Src++;
                }
            }

            if((*Src == '\0') || (*pos == '\0'))
            {
                length = MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, tmpBuf, -1, NULL, 0);
                uniBuf = (LPTSTR)malloc((length + 1) * sizeof(_TCHAR));
                memset(uniBuf, 0, (length + 1) * sizeof(_TCHAR));
                MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, tmpBuf, -1, uniBuf, length);
                length = _tcslen(uniBuf);
                if(multi)
                {
                    length++;
                }
                if(length > Max)
                {
                    break;
                }
                memcpy(Dst + uniLen, uniBuf, length * sizeof(_TCHAR));
                uniLen += length;
                Max -= length;
                *(Dst + uniLen) = NUL;
                free(uniBuf);

                pos = tmpBuf;
                Count = 0;
                if((*Src == '\0') || (Max <= 0) || (*(Src+1) == '\0'))
                {
                    break;
                }
            }
            else
            {
                pos++;
                Count++;
            }
            Src++;
        }
        free(tmpBuf);
    }
    return(uniLen);
}


/*----- ��������o�b�t�@�ɓǂݍ��� (ANSI�̂܂�) --------------------------------
*
*   Parameter
*       LPTSTR Src : ������
*       int Max : �o�b�t�@�̃T�C�Y
*       LPTSTR Dst : �������݂���o�b�t�@
*
*   Return Value
*       int �ǂݍ��񂾃o�C�g��
*----------------------------------------------------------------------------*/
static int StrReadInAnsi(char *Src, int Max, char *Dst)
{
    int Count;
    int Tmp;

    Count = 0;
    while(*Src != NUL)
    {
        if(Count >= Max)
            break;

        if(*Src == '\\')
        {
            Src++;
            if(*Src == '\\')
                *Dst = '\\';
            else
            {
                sscanf(Src, "%02x", &Tmp);
                *Dst = Tmp;
                Src++;
            }
        }
        else
            *Dst = *Src;

        Count++;
        Dst++;
        Src++;
    }
    return(Count);
}


/*----- �l���������� ----------------------------------------------------------
*
*   Parameter
*       LPTSTR Handle : �n���h��
*       LPTSTR Name : ���O
*
*   Return Value
*       LPTSTR �l�f�[�^�̐擪
*           NULL=�w��̖��O�̒l��������Ȃ�
*----------------------------------------------------------------------------*/
static char *ScanValue(void *Handle, LPTSTR Name)
{
    REGDATATBL *Cur;
    char *Pos;
    char *Ret;
    char        *oemStr;    /* not _TCHAR */
    int         length;

    /* convert strings to OEM character set */
    length = WideCharToMultiByte(CP_OEMCP, 0, Name, -1, NULL, 0, NULL, NULL);
    oemStr = (char*)malloc(length+1);
    memset(oemStr, NUL, length+1);
    WideCharToMultiByte(CP_OEMCP, 0, Name, -1, oemStr, length, NULL, NULL);

    Ret = NULL;
    Cur = Handle;
    Pos = Cur->ValTbl;
    while(Pos < (Cur->ValTbl + Cur->ValLen))
    {
        if((strncmp(oemStr, Pos, strlen(oemStr)) == 0) &&
           (*(Pos + strlen(oemStr)) == '='))
        {
            Ret = Pos + strlen(oemStr) + 1;
            break;
        }
        Pos += strlen(Pos) + 1;
    }
    free(oemStr);
    return(Ret);
}


