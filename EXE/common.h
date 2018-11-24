/*=============================================================================
/                           �a�����������̋��ʐݒ�
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

#ifndef ENGLISH
#include "mesg-jpn.h"
#else
#include "mesg-eng.h"
#endif

#include "helpid.h"


#define FAIL        0
#define SUCCESS     1

#define NO          0
#define YES         1
#define YES_ALL     2
#define NO_ALL      3
#define GO_ABORT    4

#define NUL         _T('\0')

#define SIZING

#define PROGRAM_VERSION         _T("1.16")      /* �o�[�W���� */
#define PROGRAM_VERSION_NUM     0x01100000      /* �o�[�W���� */

#define TIMER_INTERVAL      1
#define TIMER_ANIM          2
#define TIMER_TIPS          3
#define TIMER_COUNTDOWN     4


#define MYWEB_URL   _T("http://www2.biglobe.ne.jp/~sota/backup-qa.html")


/*===== ���[�U�R�}���h =====*/

#define WM_BACKUP_END   (WM_USER+1)
#define WM_BACKUP_ERROR (WM_USER+2)
#define WM_BACKUP_START (WM_USER+3)
#define WM_RETURN_MAIN  (WM_USER+4)
#define WM_CLICK_ICON   (WM_USER+5)
#define WM_ADD_SRCLIST  (WM_USER+6)
#define WM_ADD_DSTLIST  (WM_USER+7)
#define WM_ADD_DIRLIST  (WM_USER+8)
#define WM_ADD_FILELIST (WM_USER+9)
#define WM_FORMAT_TEXT  (WM_USER+10)
#define WM_SIZE_CHANGE  (WM_USER+11)

/*===== �I�v�V���� =====*/

#define OPT_FORCE				0x00000001
#define OPT_RMDIR				0x00000002
#define OPT_RMFILE				0x00000004
#define OPT_NOERROR				0x00000008
#define OPT_START				0x00000010
#define OPT_CLOSE				0x00000020
#define OPT_NAME				0x00000040
#define OPT_NOTIFY_DEL			0x00000080
#define OPT_ALL					0x00000100
#define OPT_IGNNODEL			0x00000200
#define OPT_NEWONLY				0x00000400
#define OPT_CHK_LABEL			0x00000800
#define OPT_TRASHCAN			0x00001000
#define OPT_MINIMIZED			0x00002000
#define OPT_IGN_SYSTEM			0x00004000
#define OPT_IGN_HIDDEN			0x00008000
#define OPT_INI_FILE			0x00010000
#define OPT_IGN_ATTR			0x00020000
#define OPT_PLAY_SOUND			0x00040000
#define OPT_NO_TOPDIR			0x00080000
#define OPT_IGN_TIME			0x00100000
#define OPT_NOTIFY_OVW			0x00200000
#define OPT_SHOW_COMMENT		0x00400000
#define OPT_IGN_BIG_FILE		0x00800000
#define OPT_NO_NOTIFY_DEL		0x01000000
#define OPT_SHOW_AUTHDIALOG		0x02000000
#define OPT_HIDE_AUTHDIALOG		0x04000000
#define OPT_DST_DROPBOX			0x08000000
#define OPT_MOVE_INSTEAD_DEL	0x10000000

/*===== �g���C�A�C�R������ =====*/

#define TICON_NEW       0       /* �V�����ǉ� */
#define TICON_CHANGE    1       /* �w��ԍ��ɕύX */
#define TICON_NEXT      2       /* ���� */
#define TICON_DELETE    3       /* �폜 */

/*===== �^�X�N���b�Z�[�W =====*/

#define TASKMSG_NOR     0       /* �ʏ�̃��b�Z�[�W */
#define TASKMSG_ERR     1       /* �G���[���b�Z�[�W */

/*===== ���O =====*/

#define LOG_SW_OFF      0       /* ���O�����Ȃ� */
#define LOG_SW_APPEND   1       /* ���O��ǉ��������� */
#define LOG_SW_NEW      2       /* ���O���㏑�� */

/*===== ���W�X�g���̃^�C�v =====*/

#define REGTYPE_REG     0       /* ���W�X�g�� */
#define REGTYPE_INI     1       /* INI�t�@�C�� */

#define REG_SECT_MAX    (16*1024)   /* ���W�X�g���̂P�Z�N�V�����̍ő�f�[�^�T�C�Y */

/*===== �F�؃_�C�A���O��\�����邩  =====*/

#define AUTH_DIALOG_HIDE          0       /* �F�؃_�C�A���O��\�����Ȃ� */
#define AUTH_DIALOG_SHOW          1       /* �F�؃_�C�A���O��\������   */

/*===== TreeView�̃f�[�^�^�C�v =====*/

#define TREE_FOLDER     0
#define TREE_FILE       1
#define TREE_ROOT       2
#define TREE_FOLDER_SEL 3
#define TREE_FILE_SEL   4
#define TREE_ERROR      -1

/*===== �\�����Ă���E�C���h�E =====*/

#define WIN_MAIN        0       /* ���C���E�C���h�E */
#define WIN_TRANS       1       /* �]�����E�C���h�E */

/*===== FS�̎�� =====*/

#define FS_OTHER        0       /* ���L�ȊO */
#define FS_FAT          1       /* FAT */

#define MY_MAX_PATH     512
#define MY_MAX_PATH2    (MY_MAX_PATH*2)

#define PATNAME_LEN     160         /* �p�^�[�����̒��� */
#define COMMENT_LEN     640         /* �R�����g�̒��� */
#define SRC_PATH_LEN    8192        /* �o�b�N�A�b�v�� */
#define DST_PATH_LEN    8192        /* �o�b�N�A�b�v�� */
#define IGN_PATH_LEN    8192        /* �o�b�N�A�b�v���Ȃ��t�H���_�̒��� */

#define COPY_SIZE       65536       /* �t�@�C�����R�s�[���鎞�̃o�b�t�@�T�C�Y */

#define WAIT_TIMER      25

/*===== �h���b�O���h���b�v�\�Ȏ�� ======*/

#define SEND_FOLDER     0x01
#define SEND_FILE       0x02


/*===== �V���b�g�_�E���܂ł̎��� ======*/

#define SHUTDOWN_PERIOD 6           /* �b */


/*===== Windows�̃o�[�W���� =====*/

#define WINDOWS_VISTA_VERSION       6

/*===== �o�b�N�A�b�v�p�^�[�� =====*/

typedef struct {
    _TCHAR Name[PATNAME_LEN+1];     /* �p�^�[���� */
    _TCHAR Comment[COMMENT_LEN+1];  /* �R�����g */
    _TCHAR Src[SRC_PATH_LEN+1];     /* �o�b�N�A�b�v�� (�}���`������) */
    _TCHAR Dst[DST_PATH_LEN+1];     /* �o�b�N�A�b�v�� (�}���`������) */
    _TCHAR IgnDir[IGN_PATH_LEN+1];  /* �o�b�N�A�b�v���Ȃ��t�H���_ (�}���`������) */
    _TCHAR IgnFile[IGN_PATH_LEN+1]; /* �o�b�N�A�b�v���Ȃ��t�@�C�� (�}���`������) */
    _TCHAR VolLabel[MY_MAX_PATH+1]; /* �{�����[�����x�� */
    _TCHAR SoundFile[MY_MAX_PATH+1];/* �T�E���h�t�@�C�� */
    int ForceCopy;                  /* ��Ƀt�@�C�����R�s�[ */
    int DelDir;                     /* �t�H���_���폜���� */
    int DelFile;                    /* �t�@�C�����폜���� */
    int IgnoreErr;                  /* �G���[�𖳎����� */
    int NotifyDel;                  /* �폜�̊m�F���s�Ȃ� */
    int NotifyOvw;                  /* �㏑���̊m�F���s�Ȃ� */
    int IgnNoDel;                   /* �o�b�N�A�b�v���Ȃ��t�@�C���^�t�H���_���폜���Ȃ� */
    int NewOnly;                    /* �o�b�N�A�b�v�悪�V�������̓R�s�[���Ȃ� */
    int Wait;                       /* �t�@�C���R�s�[���̑҂����� */
    int ChkVolLabel;                /* �{�����[�����x�����`�F�b�N���� */
    int UseTrashCan;                /* ���ݔ����g�p���� */
    int Tolerance;                  /* �^�C���X�^���v�̋��e�덷 */
    int AutoClose;                  /* �o�b�N�A�b�v��̏��� (0=�������Ȃ�, 1=�v���O�����I��, 2=Windows�I��) */
    int IgnSystemFile;              /* �V�X�e���t�@�C���͏��O */
    int IgnHiddenFile;              /* �B���t�@�C���͏��O */
    int IgnBigFile;                 /* �傫�ȃt�@�C���͏��O */
    int IgnBigSize;                 /* �傫�ȃt�@�C����臒l */
    int IgnAttr;                    /* �����̈Ⴂ�͖��� */
    int Sound;                      /* �o�b�N�A�b�v�I����ɃT�E���h��炷���ǂ��� */
    int IntervalTime;               /* �ăo�b�N�A�b�v�҂����ԁF�}�C�i�X�l�Ȃ�ăo�b�N�A�b�v�Ȃ� */
    int NoMakeTopDir;               /* �o�b�N�A�b�v��̃t�H���_�����Ȃ� */
    int IgnTime;                    /* �^�C���X�^���v�̈Ⴂ�͖��� */
    int ShowComment;                /* �o�b�N�A�b�v�J�n���ɃR�����g���E�C���h�E�ŕ\������ */
    int NextDstNum;                 /* ���̃o�b�N�A�b�v��ԍ� */
	int DstDropbox;					/* �o�b�N�A�b�v���Dropbox */
	int MoveInsteadDelete;			/* �폜�̑���Ƀt�@�C�����ړ����� */
	_TCHAR MoveToFolder[MY_MAX_PATH+1]; /* �t�@�C���ړ���̃t�H���_�[ */
    /* �ȉ��͐ݒ�l�ł͂Ȃ��B���������Ŏg�p����B */
    _TCHAR *NextDst;                /* ���̃o�b�N�A�b�v��ւ̃|�C���^ */
    int PatNum;                     /* �p�^�[���ԍ� */
} COPYPAT;


/*===== �o�b�N�A�b�v�p�^�[�����X�g =====*/

typedef struct copypatlist {
    COPYPAT Set;                /* �p�^�[�� */
    struct copypatlist *Next;
} COPYPATLIST;


/*===== ���W�I�{�^���̐ݒ� =====*/

typedef struct {
    int ButID;      /* �{�^����ID */
    int Value;      /* �l */
} RADIOBUTTON;


/*===== �_�C�A���O�{�b�N�X�ύX�����p =====*/

typedef struct {
    int HorMoveList[20];    /* �����ɓ��������i�̃��X�g */
    int VarMoveList[15];    /* �����ɓ��������i�̃��X�g */
    int ResizeList[10];     /* �T�C�Y�ύX���镔�i�̃��X�g */
    int ResizeHorList[10];  /* ���������ɂ̂݃T�C�Y�ύX���镔�i�̃��X�g */
    int ResizeVarList[10];  /* ���������ɂ̂݃T�C�Y�ύX���镔�i�̃��X�g */
    SIZE MinSize;           /* �ŏ��T�C�Y */
    SIZE CurSize;           /* ���݂̃T�C�Y */
} DIALOGSIZE;


/*===== �㏑���m�F��� =====*/
typedef struct {
    _TCHAR          *Fname;
    FILETIME        SrcTime;
    FILETIME        DstTime;
    DWORD           SrcSizeHigh;
    DWORD           SrcSizeLow;
    DWORD           DstSizeHigh;
    DWORD           DstSizeLow;
} OVERWRITENOTIFYDATA;


/*===== �v���g�^�C�v =====*/

/* main.c */

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int cmdShow);
HWND GetMainHwnd(void);
HINSTANCE GetBupInst(void);
int AskAutoClose(void);
int AskNoNotify(void);
LPTSTR AskHelpFilePath(void);
LPTSTR AskIniFilePath(void);
char *AskIniFilePathAnsi(void);
void SetTrayIcon(int Ope, int Type, LPTSTR AddMesg);
void SetMenuHide(int Win);
void DispErrorBox(LPTSTR szFormat,...);
void DoPrintf(LPTSTR szFormat,...);
void MakeInitialLogFilename(LPTSTR buf);

/* maindlg.c */

int MakeMainDialog(void);
HWND GetMainDlgHwnd(void);
int AddPatToList(COPYPAT *Set);
int SearchPatList(LPTSTR Name);
int CopyPatFromList(int Num, COPYPAT *Set);
void CopyDefaultPat(COPYPAT *Set);
int GetPatterns(void);
int GetSelectedCount(void);
int NotifyBackup(HWND hWnd, COPYPATLIST *Pat);
void SaveMainDlgSize(void);
void AsdMainDlgMinSize(POINT *Point);
LPTSTR GetPatComment(int Num);
void IncrementDstNum(int PatNum);

/* TransDlg.c */

int MakeTransferDialog(void);
void DeleteTransferDialogResources(void);
HWND GetTransDlgHwnd(void);
int StartBackup(COPYPATLIST *Pat);
DWORD ShowWNetUseConnection(HWND hWnd, LPTSTR lpRemoteName);
int ShowAuthDialogForUNCPaths(HWND hWnd, COPYPATLIST *Pat);
int MyPathIsUNCServerShare(_TCHAR *str);
void SelectPass(int Pass);
void SetPatName(LPTSTR Name);
void SetTaskMsg(int Type, LPTSTR szFormat,...);
void SetFileProgress(LONGLONG Total, LONGLONG Done);
void SaveTransDlgSize(void);

/* Transfer.c */

int MakeBackupThread(void);
void CloseBackupThread(void);
void SetBackupPat(COPYPATLIST *Pat);
void SetBackupAbort(void);
void SetBackupPause(void);
void SetBackupRestart(void);
void MakePathandFile(LPTSTR Path, LPTSTR Fname, int Multi);

BOOL SetCurrentDirectory_My(LPCTSTR lpPathName, int normalization);
HANDLE FindFirstFile_My(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData, int normalization);
DWORD GetFileAttributes_My(LPCTSTR lpFileName, int normalization);
DWORD GetFileAttributes_My2(LPCTSTR lpFileName, DWORD * pLastError);
BOOL SetFileAttributes_My(LPCTSTR lpFileName, DWORD dwFileAttributes, int normalization);
BOOL MoveFile_My(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, int normalization);
HANDLE CreateFile_My(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, int normalization);
BOOL RemoveDirectory_My(LPCTSTR lpPathName, int normalization);
BOOL CreateDirectory_My(LPCTSTR lpPathName, LPSECURITY_ATTRIBUTES lpSecurityAttributes, int normalization);
BOOL DeleteFile_My(LPCTSTR lpFileName, int normalization);

/* Registory.c */

int SaveRegistory(void);
int LoadRegistory(void);
void ClearRegistory(void);
void SaveSettingsToFile(void);
int LoadSettingsFromFile(void);
int GetMediaPath(LPTSTR MediaPath, int Max);

/* Logfile.c */

int OpenLogfile(void);
int CloseLogfile(void);
int DeleteLogFilename(void);
int WriteMsgToLogfile(LPTSTR Msg);
void WriteTitleToLogfile(LPTSTR Name, LPTSTR SrcPath, LPTSTR DstPath);
void WriteEndTimeToLogfile(void);
void DispLogWithViewer(void);
int OpenErrorLogfile(void);
int CloseErrorLogfile(void);
int DeleteErrorLogFilename(void);
int WriteMsgToErrorLogfile(LPTSTR Msg);
void DispErrorLogWithViewer(void);
void OpenLogDir(void);

/* option.c */

int SetOption(HWND hWnd);

/* patman.c */

int DispHostSetDlg(HWND hWnd, COPYPAT *Pat);

/* misc.c */

void SetYenTail(LPTSTR Str);
void SetCharTail(LPTSTR Str, LPTSTR Ch);
void RemoveYenTail(LPTSTR Str);
void RemoveReturnCode(LPTSTR Str);
int CountChar(LPTSTR Str, _TCHAR Ch);
void ReplaceAll(LPTSTR Str, int Len, _TCHAR Src, _TCHAR Dst);
int ReplaceAllStr(LPTSTR Str, LPTSTR Find, LPTSTR Repl, int Scan);
void GetSamePartOfString(LPTSTR Str1, LPTSTR Str2);
LPTSTR GetFileName(LPTSTR Path);
LPTSTR GetFileExt(LPTSTR Path);
int StrMultiLen(LPTSTR Str);
int StrMultiCount(LPTSTR Str);
LPTSTR GetSpecifiedStringFromMultiString(LPTSTR Str, int Num);
BOOL CALLBACK ExeEscDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
int SelectDir(HWND hWnd, LPTSTR Buf, int MaxLen, LPTSTR Title);
int SelectFile(HWND hWnd, LPTSTR Fname, LPTSTR Title, LPTSTR Filters, LPTSTR Ext, int Flags, int Save, LPTSTR Dir);
void SetRadioButtonByValue(HWND hDlg, int Value, const RADIOBUTTON *Buttons, int Num);
int AskRadioButtonValue(HWND hDlg, const RADIOBUTTON *Buttons, int Num);
void MakeSizeString(double Size, LPTSTR Buf);
void BackgrndMessageProc(void);
void SortListBoxItem(HWND hWnd);
void GetRootPath(LPTSTR Path, LPTSTR Buf);
int GetDriveFormat(LPTSTR Path);
BOOL GetVolumeLabel(LPTSTR Path, LPTSTR Buf, int Size);
UINT GetDriveTypeFromPath(LPTSTR Path);
int MoveFileToTrashCan(LPTSTR Path);
void SendDropFilesToControl(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam, int Type);
void ExecViewer(LPTSTR Fname, LPTSTR App);
void CheckRange2(int *Cur, int Max, int Min);
void FileTime2TimeString(FILETIME *Time, LPTSTR Buf);

/* wildcard.c */

int CheckFnameWithArray(LPTSTR Fname, LPTSTR Array);
int CheckFname(LPTSTR str, LPTSTR regexp);

/* filesize.c */

void FilesSizeDialog(HWND hWnd, COPYPATLIST *Pat);

/* dlgsize.c */

void DlgSizeInit(HWND hDlg, DIALOGSIZE *Dt, SIZE *Size, BOOL Move);
void AskDlgSize(DIALOGSIZE *Dt, SIZE *Size);
void AskDlgMinSize(DIALOGSIZE *Dt, POINT *Point);
void DlgSizeChange(HWND hDlg, DIALOGSIZE *Dt, RECT *New, int Flg);

/* quickdlg.c */

int GetQuickBackupParam(COPYPATLIST **Top, HWND hWnd);

/* lbtips.c */

int InitListBoxTips(HWND hWnd, HINSTANCE hInst);
void CheckTipsDisplay(LPARAM lParam);

/* updatebell.c */

void SaveUpdateBellInfo(void);


/* shutdown.c */
BOOL ChangeSystemPowerMode(int State);
int DoCountDown(int State);
