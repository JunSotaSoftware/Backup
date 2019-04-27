/*===========================================================================
/
/									Backup
/							クイックバックアップ
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


/*===== プロトタイプ =====*/

static BOOL CALLBACK QuickBaclupDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void MakeCopyPatFromDialog(HWND hDlg, COPYPATLIST *Pat);
static void CheckSemicolon(LPTSTR Str);
static LRESULT CALLBACK QuickSrcWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK QuickDstWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



static HWND hWndQuickPat;
static WNDPROC QuickSrcProcPtr;
static WNDPROC QuickDstProcPtr;
static COPYPAT OrgPat = {
    1,          /* 有効/無効 */
	_T(""),		/* パターン名 */
	_T(""),		/* コメント */
	_T("\0"),	/* バックアップ元 (マルチ文字列) */
	_T("\0"),	/* バックアップ先 (マルチ文字列) */
	_T("\0"),	/* バックアップしないフォルダ (マルチ文字列) */
	_T("\0"),	/* バックアップしないファイル (マルチ文字列) */
	_T(""),		/* ボリュームラベル */
	_T(""),		/* サウンドファイル */
	0,		/* 常にファイルをコピー */
	0,		/* フォルダを削除する */
	0,		/* ファイルを削除する */
	1,		/* エラーを無視する */
	1,		/* 削除の確認を行なう */
	0,		/* 上書きの確認を行なう */
	0,		/* バックアップしないファイル／フォルダを削除しない */
	0,		/* バックアップ先が新しい時はコピーしない */
	0,		/* ファイルコピー時の待ち時間 */
	0,		/* ボリュームラベルをチェックする */
	0,		/* ごみ箱を使用する */
	2,		/* タイムスタンプの許容誤差 */
    {
        AUTOCLOSE_ACTION_DEFAULT_SUCCESS,   /* 成功終了時の処理 */
        AUTOCLOSE_ACTION_DEFAULT_ERROR,     /* エラー終了時の処理 */
    },      /* バックアップ後の処理 */
	0,		/* システムファイルは除外 */
	0,		/* 隠しファイルは除外 */
	0,		/* 大きなファイルは除外 */
	100,	/* 大きなファイルの閾値 */
	0,		/* 属性の違いは無視 */
	0,		/* バックアップ終了後にサウンドを鳴らすかどうか */
	-60,	/* 再バックアップ待ち時間：マイナス値なら再バックアップなし */
	0,		/* バックアップ先のフォルダを作らない */
	0,		/* タイムスタンプの違いは無視 */
	0,		/* バックアップ開始時にコメントをウインドウで表示する */
    0,      /* 次のバックアップ先番号 */
	0,		/* バックアップ先はDropbox */
	0,		/* 削除の代わりにファイルを移動する */
	_T(""),	/* ファイル移動先のフォルダー */
    NULL,   /* 次のバックアップ先 */
    0,      /* パターン番号 */
};





/*----- クイックバックアップの入力 --------------------------------------------
*
*	Parameter
*		COPYPATLIST **Top : リストの先頭
*		HWND hWnd : 親ウインドウ
*
*	Return Value
*		int 設定の数
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
		if(DialogBoxParamI(GetBupInst(), MAKEINTRESOURCE(quick_dlg), hWnd, QuickBaclupDialogProc, (LPARAM)Pat) == YES)
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


/*----- クイックバックアップウインドウのメッセージ処理 ------------------------
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
			SendDlgItemMessageI(hDlg, QUICK_SRC, EM_LIMITTEXT, SRC_PATH_LEN-1, 0);
			SendDlgItemMessageI(hDlg, QUICK_DST, EM_LIMITTEXT, DST_PATH_LEN, 0);
			ReplaceAll(Pat->Set.Src, StrMultiLen(Pat->Set.Src), '\0', ';');
			SendDlgItemMessageI(hDlg, QUICK_SRC, WM_SETTEXT, 0, (LPARAM)Pat->Set.Src);
			SendDlgItemMessageI(hDlg, QUICK_DST, WM_SETTEXT, 0, (LPARAM)Pat->Set.Dst);
			SendDlgItemMessageI(hDlg, PATSET_RMDIR, BM_SETCHECK, Pat->Set.DelDir, 0);
			SendDlgItemMessageI(hDlg, PATSET_RMFILE, BM_SETCHECK, Pat->Set.DelFile, 0);
			SendDlgItemMessageI(hDlg, PATSET_NOTIFY_DEL2, BM_SETCHECK, Pat->Set.NotifyDel, 0);
			SendDlgItemMessageI(hDlg, PATSET_USE_TRASHCAN, BM_SETCHECK, Pat->Set.UseTrashCan, 0);
			SendDlgItemMessageI(hDlg, PATSET_FORCE, BM_SETCHECK, Pat->Set.ForceCopy, 0);
			SendDlgItemMessageI(hDlg, PATSET_NEWONLY, BM_SETCHECK, Pat->Set.NewOnly, 0);
			SendDlgItemMessageI(hDlg, PATSET_NOERROR, BM_SETCHECK, Pat->Set.IgnoreErr, 0);

			hWndQuickPat = hDlg;
			hWndChild = GetDlgItem(hDlg, QUICK_SRC);
			DragAcceptFiles(hWndChild, TRUE);
			QuickSrcProcPtr = (WNDPROC)SetWindowLongPtr(hWndChild, GWLP_WNDPROC, (LONG_PTR)QuickSrcWndProc);

			hWndChild = GetDlgItem(hDlg, QUICK_DST);
			DragAcceptFiles(hWndChild, TRUE);
			QuickDstProcPtr = (WNDPROC)SetWindowLongPtr(hWndChild, GWLP_WNDPROC, (LONG_PTR)QuickDstWndProc);
			return(TRUE);

		case WM_COMMAND :
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK :
					MakeCopyPatFromDialog(hDlg, Pat);
					if((TCSLEN(Pat->Set.Dst) > 0) && (TCSLEN(Pat->Set.Src) > 0))
						EndDialog(hDlg, YES);
					else
						DialogBoxParamI(GetBupInst(), MAKEINTRESOURCE(folder_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_84);
					break;

				case IDCANCEL :
					EndDialog(hDlg, NO);
					break;

				case QUICK_SIZE :
					MakeCopyPatFromDialog(hDlg, Pat);
					if(TCSLEN(Pat->Set.Src) > 0)
						FilesSizeDialog(hDlg, Pat);
					else
						DialogBoxParamI(GetBupInst(), MAKEINTRESOURCE(folder_notify_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_86);
					break;

				case IDHELP :
					HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000020);
					break;

				case QUICK_SRC_BR :
					if(SelectDir(hDlg, Tmp, MY_MAX_PATH, MSGJPN_36) == TRUE)
						SendDlgItemMessageI(hDlg, QUICK_SRC, WM_SETTEXT, 0, (LPARAM)Tmp);
					break;

				case QUICK_DST_BR :
					if(SelectDir(hDlg, Tmp, MY_MAX_PATH, MSGJPN_33) == TRUE)
						SendDlgItemMessageI(hDlg, QUICK_DST, WM_SETTEXT, 0, (LPARAM)Tmp);
					break;
			}
			return(TRUE);

		case WM_ADD_SRCLIST :
			Tmp2 = malloc((SRC_PATH_LEN+1) * sizeof(_TCHAR));
			if(Tmp2 != NULL)
			{
				SendDlgItemMessageI(hDlg, QUICK_SRC, WM_GETTEXT, SRC_PATH_LEN, (LPARAM)Tmp2);
				if((TCSLEN(Tmp2) + TCSLEN((_TCHAR*)lParam) + 1) <= SRC_PATH_LEN)
				{
					SetCharTail(Tmp2, _T(";"));
					_tcscat(Tmp2, (_TCHAR*)lParam);
					SendDlgItemMessageI(hDlg, QUICK_SRC, WM_SETTEXT, 0, (LPARAM)Tmp2);
					SendDlgItemMessageI(hDlg, QUICK_SRC, EM_SETSEL, 0, (LPARAM)-1);
				}
				else
					MessageBeep((UINT)-1);
				free(Tmp2);
			}
			free((_TCHAR*)lParam);
			break;

		case WM_ADD_DSTLIST :
			SendDlgItemMessageI(hDlg, QUICK_DST, WM_SETTEXT, 0, (LPARAM)lParam);
			free((_TCHAR*)lParam);
			break;
	}
    return(FALSE);
}


/*----- ダイアログの情報からパターンデータを作成する --------------------------
*
*	Parameter
*		HWND hDlg : ウインドウハンドル
*		COPYPATLIST *Pat : パターンデータの格納先
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/
static void MakeCopyPatFromDialog(HWND hDlg, COPYPATLIST *Pat)
{
	SendDlgItemMessageI(hDlg, QUICK_SRC, WM_GETTEXT, SRC_PATH_LEN, (LPARAM)Pat->Set.Src);
	CheckSemicolon(Pat->Set.Src);
	Pat->Set.Src[TCSLEN(Pat->Set.Src) + 1] = '\0';
	ReplaceAll(Pat->Set.Src, TCSLEN(Pat->Set.Src), ';', '\0');
	SendDlgItemMessageI(hDlg, QUICK_DST, WM_GETTEXT, DST_PATH_LEN, (LPARAM)Pat->Set.Dst);
	Pat->Set.Dst[TCSLEN(Pat->Set.Dst) + 1] = '\0';
	Pat->Set.DelDir = SendDlgItemMessageI(hDlg, PATSET_RMDIR, BM_GETCHECK, 0, 0);
	Pat->Set.DelFile = SendDlgItemMessageI(hDlg, PATSET_RMFILE, BM_GETCHECK, 0, 0);
	Pat->Set.NotifyDel = SendDlgItemMessageI(hDlg, PATSET_NOTIFY_DEL2, BM_GETCHECK, 0, 0);
	Pat->Set.UseTrashCan = SendDlgItemMessageI(hDlg, PATSET_USE_TRASHCAN, BM_GETCHECK, 0, 0);
	Pat->Set.ForceCopy = SendDlgItemMessageI(hDlg, PATSET_FORCE, BM_GETCHECK, 0, 0);
	Pat->Set.NewOnly = SendDlgItemMessageI(hDlg, PATSET_NEWONLY, BM_GETCHECK, 0, 0);
	Pat->Set.IgnoreErr = SendDlgItemMessageI(hDlg, PATSET_NOERROR, BM_GETCHECK, 0, 0);
	return;
}


/*----- 不要なセミコロンのチェック --------------------------------------------
*
*	Parameter
*		LPTSTR Str : 文字列
*
*	Return Value
*		なし
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


/*----- バックアップ元ウインドウのメッセージ処理 ------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		UINT message  : メッセージ番号
*		WPARAM wParam : メッセージの WPARAM 引数
*		LPARAM lParam : メッセージの LPARAM 引数
*
*	Return Value
*		メッセージに対応する戻り値
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


/*----- バックアップ先ウインドウのメッセージ処理 ------------------------------
*
*	Parameter
*		HWND hWnd : ウインドウハンドル
*		UINT message  : メッセージ番号
*		WPARAM wParam : メッセージの WPARAM 引数
*		LPARAM lParam : メッセージの LPARAM 引数
*
*	Return Value
*		メッセージに対応する戻り値
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


