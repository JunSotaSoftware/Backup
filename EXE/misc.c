/*===========================================================================
/
/									Backup
/							各種の汎用サブルーチン
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



/*----- 文字列の最後に "\" を付ける（""には付けない）--------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*
*	Return Value
*		なし
*
*	Note
*		オリジナルの文字列 LPTSTR Str が変更されます。
*----------------------------------------------------------------------------*/

void SetYenTail(LPTSTR Str)
{
	SetCharTail(Str, _T("\\"));
	return;
}


/*----- 文字列の最後に文字列を付ける（""には付けない）-------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*		LPTSTR Ch : 追加する文字列
*
*	Return Value
*		なし
*
*	Note
*		オリジナルの文字列 LPTSTR Str が変更されます。
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


/*----- 文字列の最後の "\" を取り除く -----------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*
*	Return Value
*		なし
*
*	Note
*		オリジナルの文字列 LPTSTR Str が変更されます。
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


/*----- 文字列から改行コードを取り除く ----------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*
*	Return Value
*		なし
*
*	Note
*		オリジナルの文字列 LPTSTR Str が変更されます。
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


/*----- 文字列内の特定の文字の数を数える --------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*		_TCHAR Ch : 文字
*
*	Return Value
*		文字の数
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


/*----- 文字列内の特定の文字を全て置き換える ----------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列 (文字列中の'\0'も許可)
*		int Len : 文字列の長さ
*		_TCHAR Src : 検索文字
*		_TCHAR Dst : 置換文字
*
*	Return Value
*		なし
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


/*----- ２つの文字列の先頭から一致する部分のみを残す --------------------------
*
*	Parameter
*		LPTSTR Str1 : 文字列1
*		LPTSTR Str2 : 文字列2
*
*	Return Value
*		なし
*
*	Note
*		オリジナルの文字列 LPTSTR Str1 が変更されます。
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


/*----- 文字列中のある文字列を別の文字列に置き換える --------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*		LPTSTR Find : 検索文字列
*		LPTSTR Repl : 置換文字列
*		int Scan : 検索のみおこなう
*
*	Return Value
*		int 該当するものが見つかったかどうか
*
*	Note
*		オリジナルの文字列 LPTSTR Str が変更されます。
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


/*----- パス名の中のファイル名の先頭を返す ------------------------------------
*
*	Parameter
*		LPTSTR Path : パス名
*
*	Return Value
*		LPTSTR ファイル名の先頭
*
*	Note
*		ディレクトリの区切り記号は "\" と "/" の両方が有効
*----------------------------------------------------------------------------*/

LPTSTR GetFileName(LPTSTR Path)
{
	LPTSTR Pos;

	if((Pos = _tcschr(Path, _T(':'))) != NULL)
		Path = Pos + 1;

	if((Pos = _tcsrchr(Path, _T('\\'))) != NULL)
		Path = Pos + 1;

	if((Pos = _tcsrchr(Path, _T('/'))) != NULL)
		Path = Pos + 1;

	return(Path);
}


/*----- パス名の中の拡張子の先頭を返す ----------------------------------------
*
*	Parameter
*		LPTSTR Path : パス名
*
*	Return Value
*		LPTSTR 拡張子の先頭
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


/*----- 文字列アレイの長さを求める --------------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列アレイ (末尾はNUL２つ)
*
*	Return Value
*		int 長さ
*
*	Note
*		終端の2つのNULのうちの最後の物は数えない
*			StrMultiLen("") = 0
*			StrMultiLen("abc\0xyz\0") = 8
*			StrMultiLen("abc") = 終端が２つのNULでないので求められない
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


/*----- 文字列アレイ中の文字列の数を数える -------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列アレイ (末尾はNUL２つ)
*
*	Return Value
*		int 文字列の個数
*
*	Note
*		終端の2つのNULのうちの最後の物は数えない
*			StrMultiLen("") = 0
*			StrMultiLen("abc\0xyz\0") = 2
*			StrMultiLen("abc") = 終端が２つのNULでないので求められない
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


/*----- 文字列アレイ中から1つの文字列を取得する --------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列アレイ (末尾はNUL２つ)
*       int Num : 何番目の文字列を返すか
*
*	Return Value
*		int 文字列
*
*	Note
*		文字列アレイの中に指定番号の文字列がない時は、最初の文字列を返す
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


/*----- ［実行］と［取消］だけのダイアログの共通コールバック関数 --------------
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


/*----- ファイル選択 ----------------------------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		LPTSTR Fname : ファイル名を返すバッファ
*		LPTSTR Title : タイトル
*		LPTSTR Filters : フィルター文字列
*		LPTSTR Ext : デフォルト拡張子
*		int Flags : 追加するフラグ
*		int Save : 「開く」か「保存」か (0=開く, 1=保存)
*		LPTSTR Dir : デフォルトフォルダ
*
*	Return Value
*		int ステータス
*			TRUE/FALSE=取消
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


/*----- ディレクトリを選択 ----------------------------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		LPTSTR Buf : ディレクトリ名を返すバッファ（初期ディレクトリ名）
*		int MaxLen : バッファのサイズ
*		LPTSTR Title : タイトル
*
*	Return Value
*		int ステータス
*			TRUE/FALSE=取消
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
Name    :	BrowseCallbackProc
Desc    :	Callback function of the directory selecting dialog
Param   :	hwnd		[in] Window handle
			uMsg		[in] Message
			lParam		[in] LPARAM
			lpData		[in] data
Return  :	status
-------------------------------------------------------------------------------*/
int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)
{
	if(uMsg==BFFM_INITIALIZED)
	{
		SendMessage(hwnd,BFFM_SETSELECTION, (WPARAM)TRUE, (LPARAM)CurFolder);
	}
	return(0);
}


/*----- 値に関連付けられたラジオボタンをチェックする --------------------------
*
*	Parameter
*		HWND hDlg : ダイアログボックスのウインドウハンドル
*		int Value : 値
*		const RADIOBUTTON *Buttons : ラジオボタンと値の関連付けテーブル
*		int Num : ボタンの数
*
*	Return Value
*		なし
*
*	Note
*		値に関連付けられたボタンが無い時は、テーブルの最初に登録されているボタ
*		ンをチェックする
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
			/* ラジオボタンを変更した時に他の項目のハイドなどを行なう事が	*/
			/* あるので、そのために WM_COMMAND を送る 						*/
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


/*----- チェックされているボタンに関連付けられた値を返す ----------------------
*
*	Parameter
*		HWND hDlg : ダイアログボックスのウインドウハンドル
*		const RADIOBUTTON *Buttons : ラジオボタンと値の関連付けテーブル
*		int Num : ボタンの数
*
*	Return Value
*		int 値
*
*	Note
*		どのボタンもチェックされていない時は、テーブルの最初に登録されているボ
*		タンの値を返す
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


/*----- ファイルサイズを文字列に変換する --------------------------------------
*
*	Parameter
*		double Size : ファイルサイズ
*		LPTSTR Buf : 文字列を返すバッファ
*
*	Return Value
*		なし
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


/*----- メッセージ処理 --------------------------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
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


/*----- リストボックスの内容をソートする --------------------------------------
*
*	Parameter
*		HWND hWnd : リストボックスのウインドウハンドル
*
*	Return Value
*		なし
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


/*----- ルートディレクトリを求める --------------------------------------------
*
*	Parameter
*		LPTSTR Path : パス名
*		LPTSTR Buf : バッファ
*
*	Return Value
*		なし
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


/*----- FSの種類を返す --------------------------------------------------------
*
*	Parameter
*		LPTSTR Path : パス名
*
*	Return Value
*		int FSの種類 (FS_FAT/FS_OTHER)
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


/*----- ボリュームラベルを返す ------------------------------------------------
*
*	Parameter
*		LPTSTR Path : パス名
*		LPTSTR Buf : バッファ
*		int Size : バッファのサイズ
*
*	Return Value
*		BOOL ステータス (TRUE=正常)
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


/*----- ドライブの種類を返す --------------------------------------------------
*
*	Parameter
*		LPTSTR Path : パス名
*
*	Return Value
*		UINT ドライブの種類 (DRIVE_xxx)
*----------------------------------------------------------------------------*/

UINT GetDriveTypeFromPath(LPTSTR Path)
{
	_TCHAR Root[MY_MAX_PATH+2];
	UINT Ret;

	GetRootPath(Path, Root);
	Ret = GetDriveType(Root);
	return(Ret);
}


/*----- ファイルをゴミ箱に削除 ------------------------------------------------
*
*	Parameter
*		LPTSTR Path : ファイル名
*
*	Return Value
*		int ステータス (0=正常終了)
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
	FileOp.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
	FileOp.lpszProgressTitle = _T("");
	return(SHFileOperation(&FileOp));
}


/*----- ドラッグ&ドロップされたファイルをページに送る -------------------------
*
*	Parameter
*		HWND hWnd : 送り先のダイアログ
*		UINT message  : 送るメッセージ
*		WPARAM wParam : WM_DROPFILESメッセージの WPARAM 引数
*		LPARAM lParam : WM_DROPFILESメッセージの LPARAM 引数
*		int Type : 許可するタイプ (SEND_xxx)
*
*	Return Value
*		なし
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
			Attr = GetFileAttributes_My(Tmp);
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


/*----- ビューワを起動 --------------------------------------------------------
*
*	Parameter
*		LPTSTR Fname : ファイル名
*		LPTSTR App : 
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void ExecViewer(LPTSTR Fname, LPTSTR App)
{
	PROCESS_INFORMATION Info;
	STARTUPINFO Startup;
	_TCHAR AssocProg[MY_MAX_PATH+1];
	_TCHAR ComLine[MY_MAX_PATH*2+10];

	/* FindExecutable()は関連付けられたプログラムのパス名にスペースが	*/
	/* 含まれている時、間違ったパス名を返す事がある。					*/
	/* そこで、関連付けられたプログラムの起動はShellExecute()を使う。	*/

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


/*----- 設定値の範囲チェック --------------------------------------------------
*
*	Parameter
*		int *Cur : 設定値
*		int Max : 最大値
*		int Min : 最小値
*
*	Return Value
*		なし
*
*	Parameter change
*		int *Cur : 設定値
*----------------------------------------------------------------------------*/

void CheckRange2(int *Cur, int Max, int Min)
{
	if(*Cur < Min)
		*Cur = Min;
	if(*Cur > Max)
		*Cur = Max;
	return;
}


/*----- FILETIME(UTC)を日付文字列(JST)に変換 ----------------------------------
*
*	Parameter
*		FILETIME *Time : ファイルタイム
*		LPTSTR Buf : 日付文字列を返すワーク
*
*	Return Value
*		なし
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


