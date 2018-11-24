/*===========================================================================
/
/                                   Backup
/                           バックアップパターン設定
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



typedef struct {
    const _TCHAR    *Title;         /* ページのタイトル */
    int             ResourceId;     /* ダイアログのリソースID */
    int             HelpId;         /* ヘルプのID */
    DLGPROC         WndProc;        /* ウインドウプロシージャ */
    int             Level;          /* アイテムのレベル */
    HWND            hWndSheet;      /* ダイアログのウインドウハンドル */
    HTREEITEM       hItem;          /* TreeViewのアイテムID */
} TREEPROPSHEET;


typedef struct {
    _TCHAR  PathFrom[MY_MAX_PATH+1];
    _TCHAR  PathTo[MY_MAX_PATH+1];
} PATHCONVERTINFO;



/*===== プロトタイプ =====*/

static LRESULT CALLBACK PropSheetFrameWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK NameSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK SourceSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void SetDirButtonHide(HWND hDlg, DWORD ListId, DWORD EditId, DWORD UpId, DWORD DownId, DWORD DelId);
static void SrcToolMenu(HWND hDlg, BOOL Conv, DWORD Id);
static int PathConvertDialog(HWND hListBox, PATHCONVERTINFO *PathInfo);
BOOL CALLBACK PathConvertDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void DoPathConvert(HWND hListBox, PATHCONVERTINFO *PathInfo, int Sel);
static int CheckPathConvert(HWND hListBox, PATHCONVERTINFO *PathInfo, int Sel);
static LRESULT CALLBACK DestinationSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK SrcListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK InpFileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void SetAdvancedPage(HWND hDlgSrc, HWND hDlgAdv);
static LRESULT CALLBACK DstWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK IgnoreSetting1Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK IgnoreSetting2Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK DirListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK FileListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK FlagSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK AdvancedSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK TimerSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK SystemSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void DisplayFileSizeDlg(HWND hDlg);
static int SetStrToListBox(LPTSTR Str, HWND hDlg, int CtrlList, int BufSize, int Pos);
static void SetMultiTextToList(HWND hDlg, int CtrlList, LPTSTR Text);
static void GetMultiTextFromList(HWND hDlg, int CtrlList, LPTSTR Buf, int BufSize);

/*===== ローカルなワーク ======*/

static int Apply;
static COPYPAT TmpPat;
static HWND hWndSrcPage;
static HWND hWndDstPage;
static HWND hWndIgnorePage;
static WNDPROC SrcListProcPtr;
static WNDPROC DstProcPtr;
static WNDPROC DirListProcPtr;
static WNDPROC FileListProcPtr;

static TREEPROPSHEET SheetInfo[] = {
    { MSGJPN_27,  patset_name_dlg,   IDH_HELP_TOPIC_0000010, NameSettingProc,        0, NULL, NULL },
    { MSGJPN_28,  patset_dir_dlg,    IDH_HELP_TOPIC_0000011, SourceSettingProc,      0, NULL, NULL },
    { MSGJPN_103, patset_dst_dlg,    IDH_HELP_TOPIC_0000012, DestinationSettingProc, 0, NULL, NULL },
    { MSGJPN_29,  patset_ign1_dlg,   IDH_HELP_TOPIC_0000013, IgnoreSetting1Proc,     0, NULL, NULL },
    { MSGJPN_124, patset_ign2_dlg,   IDH_HELP_TOPIC_0000034, IgnoreSetting2Proc,     0, NULL, NULL },
    { MSGJPN_30,  patset_flg_dlg,    IDH_HELP_TOPIC_0000014, FlagSettingProc,        0, NULL, NULL },
    { MSGJPN_109, patset_adv_dlg,    IDH_HELP_TOPIC_0000015, AdvancedSettingProc,    1, NULL, NULL },
    { MSGJPN_22,  patset_timer_dlg,  IDH_HELP_TOPIC_0000016, TimerSettingProc,       0, NULL, NULL },
    { MSGJPN_91,  patset_system_dlg, IDH_HELP_TOPIC_0000017, SystemSettingProc,      0, NULL, NULL }
};

#define PAGE_NAME       0
#define PAGE_DIR        1
#define PAGE_DST        2
#define PAGE_IGN1       3
#define PAGE_IGN2       4
#define PAGE_FLG        5
#define PAGE_ADV        6
#define PAGE_TIMER      7
#define PAGE_SYSTEM     8


/*===== グローバルなワーク ======*/

extern _TCHAR MediaPath[MY_MAX_PATH+1];







/*----- パターン設定 ----------------------------------------------------------
*
*   Parameter
*       HWND hWnd : 親ウインドウのウインドウハンドル
*       COPYPAT *Pat : パターン
*
*   Return Value
*       int ステータス
*           YES/NO=取り消し
*----------------------------------------------------------------------------*/

int DispHostSetDlg(HWND hWnd, COPYPAT *Pat)
{
    int Sts;

    memcpy(&TmpPat, Pat, sizeof(COPYPAT));
    Sts = DialogBox(GetBupInst(), MAKEINTRESOURCE(patset_frame_dlg), hWnd, PropSheetFrameWndProc);
    if(Sts == YES)
        memcpy(Pat, &TmpPat, sizeof(COPYPAT));

    return(Sts);
}


/*----- パターン設定フレームダイアログのメッセージ処理-------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/
static LRESULT CALLBACK PropSheetFrameWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT    rt;
    LPPOINT pt = (LPPOINT) &rt;
    TV_INSERTSTRUCT TvIns;
    NM_TREEVIEW *tView;
    int i;
    static NMHDR pnmhdr;
    static HWND g_hTabCtrl;
    static int CurPage;
    HTREEITEM hLevelItem[3];    /* SheetInfoで指定されている最大レベル+2個以上に設定すること */

    #define PROP_PAGES  (sizeof(SheetInfo) / sizeof(TREEPROPSHEET))

    switch(message)
    {
        case WM_INITDIALOG :
            CurPage = 0;
            SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)MSGJPN_31);

            hLevelItem[0] = TVI_ROOT;
            for(i = 0; i < PROP_PAGES; i++)
            {
                TvIns.hParent = hLevelItem[SheetInfo[i].Level];
                TvIns.hInsertAfter = TVI_LAST;
                TvIns.item.mask =  TVIF_TEXT | TVIF_PARAM;
                TvIns.item.pszText = (_TCHAR*)SheetInfo[i].Title;
                TvIns.item.lParam = i;
                SheetInfo[i].hItem = (HTREEITEM)SendDlgItemMessage(hDlg, PATSET_TREE, TVM_INSERTITEM, 0, (LPARAM)&TvIns);
                hLevelItem[SheetInfo[i].Level + 1] = SheetInfo[i].hItem;
            }
//          for(i = 0; i < PROP_PAGES; i++)
//          {
//              SendDlgItemMessage(hDlg, PATSET_TREE, TVM_EXPAND, TVE_EXPAND, (LPARAM)SheetInfo[i].hItem);
//          }
            SendDlgItemMessage(hDlg, PATSET_TREE, TVM_SELECTITEM, TVGN_CARET, (LPARAM)SheetInfo[0].hItem);

            g_hTabCtrl = GetDlgItem(hDlg, PATSET_DIALOG_POS);
            GetClientRect(g_hTabCtrl, &rt);
            MapWindowPoints(g_hTabCtrl, hDlg, pt, 2);
            for(i = 0; i < PROP_PAGES; i++)
            {
                SheetInfo[i].hWndSheet = CreateDialog(GetBupInst(), MAKEINTRESOURCE(SheetInfo[i].ResourceId), hDlg, SheetInfo[i].WndProc);
                MoveWindow(SheetInfo[i].hWndSheet, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, FALSE);
            }
            ShowWindow(SheetInfo[0].hWndSheet, SW_SHOW);
            return FALSE;

        case WM_NOTIFY:
            tView = (NM_TREEVIEW FAR *)lParam;
            switch(tView->hdr.idFrom)
            {
                case PATSET_TREE :
                    switch(tView->hdr.code)
                    {
// for unicows.dll
//                      case TVN_SELCHANGED :
                        case TVN_SELCHANGEDA :
                        case TVN_SELCHANGEDW :
                            CurPage = tView->itemNew.lParam;
                            for(i = 0; i < PROP_PAGES; i++)
                            {
                                if(i == tView->itemNew.lParam)
                                    ShowWindow(SheetInfo[i].hWndSheet, SW_SHOW);
                                else
                                    ShowWindow(SheetInfo[i].hWndSheet, SW_HIDE);
                            }
                    }
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK :
                    for(i = 0; i < PROP_PAGES; i++)
                    {
                        pnmhdr.code = PSN_APPLY;
                        SendMessage(SheetInfo[i].hWndSheet, WM_NOTIFY, 0, (LPARAM)&pnmhdr);
                        EndDialog(SheetInfo[i].hWndSheet, YES);
                    }
                    EndDialog(hDlg, YES);
                    break;

                case IDCANCEL :
                    for(i = 0; i < PROP_PAGES; i++)
                    {
                        EndDialog(SheetInfo[i].hWndSheet, NO);
                    }
                    EndDialog(hDlg, NO);
                    break;

                case IDHELP :
                    HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, SheetInfo[CurPage].HelpId);
                    break;

                case MENU_FILESIZE :
                    DisplayFileSizeDlg(hDlg);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- パターン名設定ウインドウのメッセージ処理 ------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK NameSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, PATSET_NAME, EM_LIMITTEXT, (WPARAM)PATNAME_LEN, 0);
            SendDlgItemMessage(hDlg, PATSET_NAME, WM_SETTEXT, 0, (LPARAM)TmpPat.Name);
            SendDlgItemMessage(hDlg, PATSET_COMMENT, EM_LIMITTEXT, (WPARAM)COMMENT_LEN, 0);
            SendDlgItemMessage(hDlg, PATSET_COMMENT, WM_SETTEXT, 0, (LPARAM)TmpPat.Comment);
            SendDlgItemMessage(hDlg, PATSET_SHOW_COMMENT, BM_SETCHECK, TmpPat.ShowComment, 0);
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    SendDlgItemMessage(hDlg, PATSET_NAME, WM_GETTEXT, PATNAME_LEN+1, (LPARAM)TmpPat.Name);
                    SendDlgItemMessage(hDlg, PATSET_COMMENT, WM_GETTEXT, COMMENT_LEN+1, (LPARAM)TmpPat.Comment);
                    TmpPat.ShowComment = SendDlgItemMessage(hDlg, PATSET_SHOW_COMMENT, BM_GETCHECK, 0, 0);
                    break;
            }
            break;
    }
    return(FALSE);
}


/*----- バックアップ元設定ウインドウのメッセージ処理 --------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK SourceSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[MY_MAX_PATH+1];
    int Cur;
    int Max;
    HWND hWndChild;
    PATHCONVERTINFO PathInfo;

    switch (message)
    {
        case WM_INITDIALOG :
            SetMultiTextToList(hDlg, PATSET_SRCLIST, TmpPat.Src);
            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_SETHORIZONTALEXTENT, 2048, 0);

            hWndSrcPage = hDlg;

            hWndChild = GetDlgItem(hDlg, PATSET_SRCLIST);
            DragAcceptFiles(hWndChild, TRUE);
            SrcListProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)SrcListWndProc);
            SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    GetMultiTextFromList(hDlg, PATSET_SRCLIST, TmpPat.Src, SRC_PATH_LEN+1);
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case PATSET_ADD :
                    _tcscpy(Tmp, _T(""));
                    if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfile_folder_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                    {
                        SetStrToListBox(Tmp, hDlg, PATSET_SRCLIST, SRC_PATH_LEN+1, -1);
                        GetMultiTextFromList(hDlg, PATSET_SRCLIST, TmpPat.Src, SRC_PATH_LEN+1);
                        SetAdvancedPage(hDlg, SheetInfo[PAGE_ADV].hWndSheet);
                        SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                    }
                    break;

                case PATSET_EDIT :
                    if((Cur = SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETCURSEL, 0,  0)) != LB_ERR)
                    {
                        SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                        if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfile_folder_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                        {
                            SetStrToListBox(Tmp, hDlg, PATSET_SRCLIST, SRC_PATH_LEN+1, Cur);
                            GetMultiTextFromList(hDlg, PATSET_SRCLIST, TmpPat.Src, SRC_PATH_LEN+1);
                            SetAdvancedPage(hDlg, SheetInfo[PAGE_ADV].hWndSheet);
                            SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                        }
                    }
                    break;

                case PATSET_DEL :
                    if(SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETSELCOUNT, 0,  0) > 0)
                    {
                        Cur = SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETCOUNT, 0,  0) - 1;
                        for(; Cur >= 0; Cur--)
                        {
                            if(SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETSEL, Cur,  0) != 0)
                            {
                                SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_DELETESTRING, Cur, 0);
                            }
                        }
                        GetMultiTextFromList(hDlg, PATSET_SRCLIST, TmpPat.Src, SRC_PATH_LEN+1);
                        SetAdvancedPage(hDlg, SheetInfo[PAGE_ADV].hWndSheet);
                        SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                    }
                    break;

                case PATSET_UP :
                    if((Cur = SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        if(Cur > 0)
                        {
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_DELETESTRING, Cur, 0);
                            Cur--;
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                        }
                    }
                    break;

                case PATSET_DOWN :
                    if((Cur = SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        Max = SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETCOUNT, 0, 0);
                        if(Cur < Max - 1)
                        {
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_DELETESTRING, Cur, 0);
                            Cur++;
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_SRCLIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                        }
                    }
                    break;

                case PATSET_TOOLMENU :
                    SrcToolMenu(hDlg, TRUE, MENU_SORT);
                    SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                    break;

                case MENU_SORT :
                    SortListBoxItem(GetDlgItem(hDlg, PATSET_SRCLIST));
                    break;

                case MENU_PATHCONVERT :
                    if(PathConvertDialog(GetDlgItem(hDlg, PATSET_SRCLIST), &PathInfo) == YES)
                    {
                        DoPathConvert(GetDlgItem(hDlg, PATSET_SRCLIST), &PathInfo, YES);
                        GetMultiTextFromList(hDlg, PATSET_SRCLIST, TmpPat.Src, SRC_PATH_LEN+1);

                        if((CheckPathConvert(GetDlgItem(hWndIgnorePage, NOBACK_DIR_LIST), &PathInfo, NO) == YES) ||
                           (CheckPathConvert(GetDlgItem(hWndIgnorePage, NOBACK_FILE_LIST), &PathInfo, NO) == YES))
                        {
                            if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(path_convert_ign_dlg), hDlg, ExeEscDialogProc, (LPARAM)_T("")) == YES)
                            {
                                DoPathConvert(GetDlgItem(hWndIgnorePage, NOBACK_DIR_LIST), &PathInfo, NO);
                                DoPathConvert(GetDlgItem(hWndIgnorePage, NOBACK_FILE_LIST), &PathInfo, NO);
                                GetMultiTextFromList(hWndIgnorePage, NOBACK_DIR_LIST, TmpPat.IgnDir, IGN_PATH_LEN+1);
                                GetMultiTextFromList(hWndIgnorePage, NOBACK_FILE_LIST, TmpPat.IgnFile, IGN_PATH_LEN+1);
                            }
                        }
                        SetAdvancedPage(hDlg, SheetInfo[PAGE_ADV].hWndSheet);
                    }
                    break;

                case PATSET_SRCLIST :
                    switch(GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case LBN_DBLCLK :
                        case LBN_SELCHANGE :
                        case LBN_SELCANCEL :
                            SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                            break;
                    }
                    break;
            }
            return(TRUE);

        case WM_ADD_SRCLIST :
            SetStrToListBox((_TCHAR*)lParam, hDlg, PATSET_SRCLIST, SRC_PATH_LEN+1, -1);
            GetMultiTextFromList(hDlg, PATSET_SRCLIST, TmpPat.Src, SRC_PATH_LEN+1);
            SetAdvancedPage(hDlg, SheetInfo[PAGE_ADV].hWndSheet);
            free((_TCHAR*)lParam);
            SetDirButtonHide(hDlg, PATSET_SRCLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
            break;
    }
    return(FALSE);
}


/*-------------------------------------------------------------------------------
Name    :   SetDirButtonHide
Desc    :   Hide/Show buttons of the source window
Param   :   hDlg    [in] Window handle of the dialog
            ListId  [in] Control ID of the LIST view
            EditId  [in] Control ID of the EDIT button
            UpId    [in] Control ID of the UP button
            DownId  [in] Control ID of the DOWN button
            DelId   [in] Control ID of the DELETE button
Return  :   none
-------------------------------------------------------------------------------*/
static void SetDirButtonHide(HWND hDlg, DWORD ListId, DWORD EditId, DWORD UpId, DWORD DownId, DWORD DelId)
{
    int Cur;
    int Sel;
    int Max;

    Max = SendDlgItemMessage(hDlg, ListId, LB_GETCOUNT, 0, 0);
    Sel = SendDlgItemMessage(hDlg, ListId, LB_GETSELCOUNT, 0, 0);
    Cur = SendDlgItemMessage(hDlg, ListId, LB_GETCURSEL, 0, 0);
    if(Sel == 0)
        EnableWindow(GetDlgItem(hDlg, DelId), FALSE);
    else
        EnableWindow(GetDlgItem(hDlg, DelId), TRUE);
    if(Sel == 1)
    {
        EnableWindow(GetDlgItem(hDlg, EditId), TRUE);
        if((Cur != LB_ERR) && (Cur > 0))
            EnableWindow(GetDlgItem(hDlg, UpId), TRUE);
        else
            EnableWindow(GetDlgItem(hDlg, UpId), FALSE);
        if((Cur != LB_ERR) && (Cur < Max - 1))
            EnableWindow(GetDlgItem(hDlg, DownId), TRUE);
        else
            EnableWindow(GetDlgItem(hDlg, DownId), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, EditId), FALSE);
        EnableWindow(GetDlgItem(hDlg, UpId), FALSE);
        EnableWindow(GetDlgItem(hDlg, DownId), FALSE);
    }
    return;
}


/*-------------------------------------------------------------------------------
Name    :   SrcToolMenu
Desc    :   SHow tool menu
Param   :   hdlg    [in] Window handle of the dialog box
            Conv    [in] CONVERT menu item on/off flag
            Id      [in] Control ID
Return  :   none
-------------------------------------------------------------------------------*/
static void SrcToolMenu(HWND hDlg, BOOL Conv, DWORD Id)
{
    HMENU hMenu;
    POINT point;

    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, Id, MSGJPN_113);
    if(Conv)
        AppendMenu(hMenu, MF_STRING, MENU_PATHCONVERT, MSGJPN_110);
    GetCursorPos(&point);
    TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, 0, hDlg, NULL);
    DestroyMenu(hMenu);
    return;
}


/*----- パス名一括変換ダイアログの表示 -----------------------------------------
*
*   Parameter
*       hListBox    リストボックスのウインドウハンドル
*       PathInfo    パス名情報
*
*   Return Value
*       int ダイアログの終了状態 (YES=OK)
*----------------------------------------------------------------------------*/
static int PathConvertDialog(HWND hListBox, PATHCONVERTINFO *PathInfo)
{
//  _TCHAR          TmpPath[MY_MAX_PATH+1];
    int             Cur;
    int             Items;
    BOOL            First;
    int             Ret;

    Ret = NO;
    _tcscpy(PathInfo->PathFrom, _T(""));
    if(SendMessage(hListBox, LB_GETSELCOUNT, 0,  0) == 0)
    {
        SendMessage(hListBox, LB_SETSEL, TRUE, -1);
    }

    if(SendMessage(hListBox, LB_GETSELCOUNT, 0,  0) > 0)
    {
        Items = SendMessage(hListBox, LB_GETCOUNT, 0,  0);
        First = TRUE;
        for(Cur = 0; Cur < Items; Cur++)
        {
            if(SendMessage(hListBox, LB_GETSEL, Cur,  0) != 0)
            {
                SendMessage(hListBox, LB_GETTEXT, Cur, (LPARAM)PathInfo->PathTo);
                if(First)
                {
                    _tcscpy(PathInfo->PathFrom, PathInfo->PathTo);
                    First = FALSE;
                }
                else
                {
                    GetSamePartOfString(PathInfo->PathFrom, PathInfo->PathTo);
                }
            }
        }
        RemoveYenTail(PathInfo->PathFrom);
        if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(path_convert_dlg), hListBox, PathConvertDlgProc, (LPARAM)PathInfo) == YES)
        {
            if((_tcslen(PathInfo->PathFrom) > 0) && (_tcscmp(PathInfo->PathFrom, PathInfo->PathTo) != 0))
            {
                Ret = YES;
            }
        }
    }
    return(Ret);
}


/*-------------------------------------------------------------------------------
Name    :   PathConvertDlgProc
Desc    :   Pathname convert dialog proc
Param   :   hDlg    [in] Window handle of the dialog
            message [in] Message
            wParam  [in] WORD param
            lParam  [in] LONG param
Return  :   status
-------------------------------------------------------------------------------*/
BOOL CALLBACK PathConvertDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static PATHCONVERTINFO  *PathInfo;

    switch (message)
    {
        case WM_INITDIALOG :
            PathInfo = (PATHCONVERTINFO*)lParam;
            SendDlgItemMessage(hDlg, PATHCNV_FROM, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
            SendDlgItemMessage(hDlg, PATHCNV_TO, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
            SendDlgItemMessage(hDlg, PATHCNV_FROM, WM_SETTEXT, 0, (LPARAM)PathInfo->PathFrom);
            SendDlgItemMessage(hDlg, PATHCNV_TO, WM_SETTEXT, 0, (LPARAM)PathInfo->PathFrom);
            return(TRUE);

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK :
                    SendDlgItemMessage(hDlg, PATHCNV_FROM, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)PathInfo->PathFrom);
                    SendDlgItemMessage(hDlg, PATHCNV_TO, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)PathInfo->PathTo);
                    EndDialog(hDlg, YES);
                    break;

                case IDCANCEL :
                    EndDialog(hDlg, NO);
                    break;

                case IDHELP :
                    HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000033);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*-------------------------------------------------------------------------------
Name    :   DoPathConvert
Desc    :   Convert pathnames
Param   :   hListBox    [in] WIndow handle of the list box
            PathInfo    [in] Pathname information
            Sel         [in] Selection flag
Return  :   none
-------------------------------------------------------------------------------*/
static void DoPathConvert(HWND hListBox, PATHCONVERTINFO *PathInfo, int Sel)
{
    _TCHAR          TmpPath[MY_MAX_PATH+1];
    int             Cur;
    int             Items;
//  BOOL            First;

    Items = SendMessage(hListBox, LB_GETCOUNT, 0,  0);
    for(Cur = 0; Cur < Items; Cur++)
    {
        if((Sel == NO) || (SendMessage(hListBox, LB_GETSEL, Cur,  0) != 0))
        {
            SendMessage(hListBox, LB_GETTEXT, Cur, (LPARAM)TmpPath);
            ReplaceAllStr(TmpPath, PathInfo->PathFrom, PathInfo->PathTo, NO);
            SendMessage(hListBox, LB_DELETESTRING, Cur, 0);
            SendMessage(hListBox, LB_INSERTSTRING, Cur, (LPARAM)TmpPath);
            if(Sel == YES)
                SendMessage(hListBox, LB_SETSEL, TRUE, Cur);
        }
    }
    return;
}


/*-------------------------------------------------------------------------------
Name    :   CheckPathConvert
Desc    :   Check pathname conversion target is existing
Param   :   hListBox    [in] WIndow handle of the list box
            PathInfo    [in] Pathname information
            Sel         [in] Selection flag
Return  :   int     status
-------------------------------------------------------------------------------*/
static int CheckPathConvert(HWND hListBox, PATHCONVERTINFO *PathInfo, int Sel)
{
    _TCHAR          TmpPath[MY_MAX_PATH+1];
    int             Cur;
    int             Items;
    int             Ret;

    Ret = NO;
    Items = SendMessage(hListBox, LB_GETCOUNT, 0,  0);
    for(Cur = 0; Cur < Items; Cur++)
    {
        if((Sel == NO) || (SendMessage(hListBox, LB_GETSEL, Cur,  0) != 0))
        {
            SendMessage(hListBox, LB_GETTEXT, Cur, (LPARAM)TmpPath);
            Ret = ReplaceAllStr(TmpPath, PathInfo->PathFrom, PathInfo->PathTo, YES);
            if(Ret == YES)
                break;
        }
    }
    return(Ret);
}


/*----- バックアップ元リストのメッセージ処理 ----------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK SrcListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DROPFILES:
            SendDropFilesToControl(hWndSrcPage, WM_ADD_SRCLIST, wParam, lParam, SEND_FOLDER | SEND_FILE);
            break;

        case WM_KEYDOWN:
            if(wParam == VK_DELETE)
            {
                PostMessage(hWndSrcPage, WM_COMMAND, MAKEWPARAM(PATSET_DEL, 0), (LPARAM)0);
                break;
            }
            return(CallWindowProc(SrcListProcPtr, hWnd, message, wParam, lParam));

        default :
            return(CallWindowProc(SrcListProcPtr, hWnd, message, wParam, lParam));
    }
    return(0L);
}


/*----- ファイル名入力ウインドウのメッセージ処理 ------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK InpFileDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static LPARAM Buf;
    _TCHAR Tmp[MY_MAX_PATH+1];
    _TCHAR Tmp2[MY_MAX_PATH+1];

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, INPFILE_FNAME, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
            SendDlgItemMessage(hDlg, INPFILE_FNAME, WM_SETTEXT, 0, lParam);
            Buf = lParam;
            return(TRUE);

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK :
                    SendDlgItemMessage(hDlg, INPFILE_FNAME, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Buf);
                    EndDialog(hDlg, YES);
                    break;

                case IDCANCEL :
                    EndDialog(hDlg, NO);
                    break;

                case INPFILE_FILE_BR :
                    _tcscpy(Tmp, _T(""));
                    SendDlgItemMessage(hDlg, INPFILE_FNAME, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Tmp2);
                    *GetFileName(Tmp2) = NUL;
                    if(SelectFile(hDlg, Tmp, MSGJPN_34, MSGJPN_26, NULL, OFN_NODEREFERENCELINKS, 0, Tmp2) == TRUE)
                        SendDlgItemMessage(hDlg, INPFILE_FNAME, WM_SETTEXT, 0, (LPARAM)Tmp);
                    break;

                case INPFILE_FOLDER_BR :
                    SendDlgItemMessage(hDlg, INPFILE_FNAME, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Tmp);
                    if(SelectDir(hDlg, Tmp, MY_MAX_PATH, MSGJPN_36) == TRUE)
                        SendDlgItemMessage(hDlg, INPFILE_FNAME, WM_SETTEXT, 0, (LPARAM)Tmp);
                    break;

                case IDHELP :
                    HtmlHelp(NULL, AskHelpFilePath(), HH_HELP_CONTEXT, IDH_HELP_TOPIC_0000011);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- 高度な設定ウインドウのグレイ設定を行なう ------------------------------
*
*   Parameter
*       HWND hDlgSrc : バックアップ元ウインドウのハンドル
*       HWND hDlgAdv : 高度な設定ウインドウのハンドル
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void SetAdvancedPage(HWND hDlgSrc, HWND hDlgAdv)
{
    if(SendDlgItemMessage(hDlgSrc, PATSET_SRCLIST, LB_GETCOUNT, 0, 0) > 1)
    {
        SendDlgItemMessage(hDlgAdv, PATSET_NO_DSTDIR, BM_SETCHECK, 0, 0);
        EnableWindow(GetDlgItem(hDlgAdv, PATSET_NO_DSTDIR), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlgAdv, PATSET_NO_DSTDIR), TRUE);
    }
    return;
}


/*----- バックアップ先設定ウインドウのメッセージ処理 --------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK DestinationSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[MY_MAX_PATH+1];
    _TCHAR Tmp2[MY_MAX_PATH+1];
    HWND hWndChild;
    int Cur;
    int Max;
    PATHCONVERTINFO PathInfo;
    HWND hCtrl;
    HDC hDC;

    switch (message)
    {
        case WM_INITDIALOG :
            SetMultiTextToList(hDlg, PATSET_DSTLIST, TmpPat.Dst);
            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_SETHORIZONTALEXTENT, 2048, 0);

            SendDlgItemMessage(hDlg, PATSET_CHK_LABEL, BM_SETCHECK, TmpPat.ChkVolLabel, 0);
            SendDlgItemMessage(hDlg, PATSET_LABEL, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
            SendDlgItemMessage(hDlg, PATSET_LABEL, WM_SETTEXT, 0, (LPARAM)TmpPat.VolLabel);

            if(TmpPat.ChkVolLabel == NO)
            {
                EnableWindow(GetDlgItem(hDlg, PATSET_LABEL), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_LABEL_GET), FALSE);
            }
            hWndDstPage = hDlg;

            hWndChild = GetDlgItem(hDlg, PATSET_DSTLIST);
            DragAcceptFiles(hWndChild, TRUE);
            DstProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)DstWndProc);
            SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
            return(TRUE);

        case WM_CTLCOLORSTATIC:
            hCtrl = (HWND)lParam;
            if(hCtrl == GetDlgItem(hDlg, PATSET_DST_COMMENT))
            {
                hDC = (HDC)wParam;
                SetTextColor(hDC, RGB(0,0,255));
                SetBkColor(hDC, GetSysColor(COLOR_3DFACE));
                return (BOOL)GetStockObject(NULL_BRUSH);
            }
            break;

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    GetMultiTextFromList(hDlg, PATSET_DSTLIST, TmpPat.Dst, DST_PATH_LEN+1);
                    TmpPat.ChkVolLabel = SendDlgItemMessage(hDlg, PATSET_CHK_LABEL, BM_GETCHECK, 0, 0);
                    SendDlgItemMessage(hDlg, PATSET_LABEL, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)TmpPat.VolLabel);
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case PATSET_ADD :
                    _tcscpy(Tmp, _T(""));
                    if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfolder_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                    {
                        SetStrToListBox(Tmp, hDlg, PATSET_DSTLIST, DST_PATH_LEN+1, -1);
                        GetMultiTextFromList(hDlg, PATSET_DSTLIST, TmpPat.Dst, DST_PATH_LEN+1);
                        SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                        TmpPat.NextDstNum = 0;
                    }
                    break;

                case PATSET_EDIT :
                    if((Cur = SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETCURSEL, 0,  0)) != LB_ERR)
                    {
                        SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                        if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfolder_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                        {
                            SetStrToListBox(Tmp, hDlg, PATSET_DSTLIST, DST_PATH_LEN+1, Cur);
                            GetMultiTextFromList(hDlg, PATSET_DSTLIST, TmpPat.Dst, DST_PATH_LEN+1);
                            SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                            TmpPat.NextDstNum = 0;
                        }
                    }
                    break;

                case PATSET_DEL :
                    if(SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETSELCOUNT, 0,  0) > 0)
                    {
                        Cur = SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETCOUNT, 0,  0) - 1;
                        for(; Cur >= 0; Cur--)
                        {
                            if(SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETSEL, Cur,  0) != 0)
                            {
                                SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_DELETESTRING, Cur, 0);
                            }
                        }
                        GetMultiTextFromList(hDlg, PATSET_DSTLIST, TmpPat.Dst, DST_PATH_LEN+1);
                        SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                        TmpPat.NextDstNum = 0;
                    }
                    break;

                case PATSET_UP :
                    if((Cur = SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        if(Cur > 0)
                        {
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_DELETESTRING, Cur, 0);
                            Cur--;
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                            TmpPat.NextDstNum = 0;
                        }
                    }
                    break;

                case PATSET_DOWN :
                    if((Cur = SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        Max = SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETCOUNT, 0, 0);
                        if(Cur < Max - 1)
                        {
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_DELETESTRING, Cur, 0);
                            Cur++;
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, PATSET_DSTLIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                            TmpPat.NextDstNum = 0;
                        }
                    }
                    break;

                case PATSET_TOOLMENU :
                    SrcToolMenu(hDlg, TRUE, MENU_SORT);
                    SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                    break;

                case MENU_SORT :
                    SortListBoxItem(GetDlgItem(hDlg, PATSET_DSTLIST));
                    TmpPat.NextDstNum = 0;
                    break;

                case MENU_PATHCONVERT :
                    if(PathConvertDialog(GetDlgItem(hDlg, PATSET_DSTLIST), &PathInfo) == YES)
                    {
                        DoPathConvert(GetDlgItem(hDlg, PATSET_DSTLIST), &PathInfo, YES);
                        GetMultiTextFromList(hDlg, PATSET_DSTLIST, TmpPat.Dst, DST_PATH_LEN+1);
                        TmpPat.NextDstNum = 0;
                    }
                    break;

                case PATSET_DSTLIST :
                    switch(GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case LBN_DBLCLK :
                        case LBN_SELCHANGE :
                        case LBN_SELCANCEL :
                            SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
                            break;
                    }
                    break;

                case PATSET_CHK_LABEL :
                    if(SendDlgItemMessage(hDlg, PATSET_CHK_LABEL, BM_GETCHECK, 0, 0) == 1)
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_LABEL), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_LABEL_GET), TRUE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_LABEL), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_LABEL_GET), FALSE);
                    }
                    break;

                case PATSET_LABEL_GET :
                    GetMultiTextFromList(hDlg, PATSET_DSTLIST, Tmp, MY_MAX_PATH+1);
                    if(_tcslen(Tmp) > 0)
                    {
                        if(GetVolumeLabel(Tmp, Tmp2, MY_MAX_PATH+1) == TRUE)
                        {
                            SendDlgItemMessage(hDlg, PATSET_LABEL, WM_SETTEXT, 0, (LPARAM)Tmp2);
                        }
                        else
                            DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_111);
                    }
                    else
                        DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(common_msg_dlg), hDlg, ExeEscDialogProc, (LPARAM)MSGJPN_12);
                    break;
            }
            return(TRUE);

        case WM_ADD_DSTLIST :
            SetStrToListBox((_TCHAR*)lParam, hDlg, PATSET_DSTLIST, DST_PATH_LEN+1, -1);
            GetMultiTextFromList(hDlg, PATSET_DSTLIST, TmpPat.Dst, DST_PATH_LEN+1);
            free((_TCHAR*)lParam);
            SetDirButtonHide(hDlg, PATSET_DSTLIST, PATSET_EDIT, PATSET_UP, PATSET_DOWN, PATSET_DEL);
            TmpPat.NextDstNum = 0;
            break;
    }
    return(FALSE);
}


/*----- バックアップ先ウインドウのメッセージ処理 ------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK DstWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DROPFILES:
            SendDropFilesToControl(hWndDstPage, WM_ADD_DSTLIST, wParam, lParam, SEND_FOLDER);
            break;

        case WM_KEYDOWN:
            if(wParam == VK_DELETE)
            {
                PostMessage(hWndDstPage, WM_COMMAND, MAKEWPARAM(PATSET_DEL, 0), (LPARAM)0);
                break;
            }
            return(CallWindowProc(DstProcPtr, hWnd, message, wParam, lParam));


        default :
            return(CallWindowProc(DstProcPtr, hWnd, message, wParam, lParam));
    }
    return(0L);
}


/*----- 除外設定１ウインドウのメッセージ処理 ------------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK IgnoreSetting1Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[MY_MAX_PATH+1];
    int Cur;
    int Max;
    HWND hWndChild;

    switch (message)
    {
        case WM_INITDIALOG :
            SetMultiTextToList(hDlg, NOBACK_DIR_LIST, TmpPat.IgnDir);
            SetMultiTextToList(hDlg, NOBACK_FILE_LIST, TmpPat.IgnFile);

            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_SETHORIZONTALEXTENT, 2048, 0);
            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_SETHORIZONTALEXTENT, 2048, 0);

            hWndIgnorePage = hDlg;

            hWndChild = GetDlgItem(hDlg, NOBACK_DIR_LIST);
            DragAcceptFiles(hWndChild, TRUE);
            DirListProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)DirListWndProc);

            hWndChild = GetDlgItem(hDlg, NOBACK_FILE_LIST);
            DragAcceptFiles(hWndChild, TRUE);
            FileListProcPtr = (WNDPROC)SetWindowLong(hWndChild, GWL_WNDPROC, (LONG)FileListWndProc);

            SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);

            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    GetMultiTextFromList(hDlg, NOBACK_DIR_LIST, TmpPat.IgnDir, IGN_PATH_LEN+1);
                    GetMultiTextFromList(hDlg, NOBACK_FILE_LIST, TmpPat.IgnFile, IGN_PATH_LEN+1);
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case NOBACK_DIR_ADD :
                    _tcscpy(Tmp, _T(""));
                    if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfolder_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                    {
                        SetStrToListBox(Tmp, hDlg, NOBACK_DIR_LIST, IGN_PATH_LEN+1, -1);
                        GetMultiTextFromList(hDlg, NOBACK_DIR_LIST, TmpPat.IgnDir, IGN_PATH_LEN+1);
                        SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                    }
                    break;

                case NOBACK_DIR_EDIT :
                    if((Cur = SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETCURSEL, 0,  0)) != LB_ERR)
                    {
                        SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                        if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfolder_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                        {
                            SetStrToListBox(Tmp, hDlg, NOBACK_DIR_LIST, IGN_PATH_LEN+1, Cur);
                            GetMultiTextFromList(hDlg, NOBACK_DIR_LIST, TmpPat.IgnDir, IGN_PATH_LEN+1);
                            SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                        }
                    }
                    break;

                case NOBACK_DIR_DEL :
                    if(SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETSELCOUNT, 0,  0) > 0)
                    {
                        Cur = SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETCOUNT, 0,  0) - 1;
                        for(; Cur >= 0; Cur--)
                        {
                            if(SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETSEL, Cur,  0) != 0)
                            {
                                SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_DELETESTRING, Cur, 0);
                            }
                        }
                        GetMultiTextFromList(hDlg, NOBACK_DIR_LIST, TmpPat.IgnDir, IGN_PATH_LEN+1);
                        SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                    }
                    break;

                case NOBACK_DIR_UP :
                    if((Cur = SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        if(Cur > 0)
                        {
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_DELETESTRING, Cur, 0);
                            Cur--;
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                        }
                    }
                    break;

                case NOBACK_DIR_DOWN :
                    if((Cur = SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        Max = SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETCOUNT, 0, 0);
                        if(Cur < Max - 1)
                        {
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_DELETESTRING, Cur, 0);
                            Cur++;
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_DIR_LIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                        }
                    }
                    break;

                case NOBACK_DIR_TOOLMENU :
                    SrcToolMenu(hDlg, FALSE, MENU_SORT);
                    break;

                case MENU_SORT :
                    SortListBoxItem(GetDlgItem(hDlg, NOBACK_DIR_LIST));
                    SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                    break;

                case NOBACK_DIR_LIST :
                    switch(GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case LBN_DBLCLK :
                        case LBN_SELCHANGE :
                        case LBN_SELCANCEL :
                            SetDirButtonHide(hDlg, NOBACK_DIR_LIST, NOBACK_DIR_EDIT, NOBACK_DIR_UP, NOBACK_DIR_DOWN, NOBACK_DIR_DEL);
                            break;
                    }
                    break;

                case NOBACK_FILE_ADD :
                    _tcscpy(Tmp, _T(""));
                    if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfile_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                    {
                        SetStrToListBox(Tmp, hDlg, NOBACK_FILE_LIST, IGN_PATH_LEN+1, -1);
                        GetMultiTextFromList(hDlg, NOBACK_FILE_LIST, TmpPat.IgnFile, IGN_PATH_LEN+1);
                        SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                    }
                    break;

                case NOBACK_FILE_EDIT :
                    if((Cur = SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETCURSEL, 0,  0)) != LB_ERR)
                    {
                        SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                        if(DialogBoxParam(GetBupInst(), MAKEINTRESOURCE(inpfile_dlg), hDlg, InpFileDlgProc, (LPARAM)Tmp) == YES)
                        {
                            SetStrToListBox(Tmp, hDlg, NOBACK_FILE_LIST, IGN_PATH_LEN+1, Cur);
                            GetMultiTextFromList(hDlg, NOBACK_FILE_LIST, TmpPat.IgnFile, IGN_PATH_LEN+1);
                            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                        }
                    }
                    break;

                case NOBACK_FILE_DEL :
                    if(SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETSELCOUNT, 0,  0) > 0)
                    {
                        Cur = SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETCOUNT, 0,  0) - 1;
                        for(; Cur >= 0; Cur--)
                        {
                            if(SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETSEL, Cur,  0) != 0)
                            {
                                SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_DELETESTRING, Cur, 0);
                            }
                        }
                        GetMultiTextFromList(hDlg, NOBACK_FILE_LIST, TmpPat.IgnFile, IGN_PATH_LEN+1);
                        SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                    }
                    break;

                case NOBACK_FILE_UP :
                    if((Cur = SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        if(Cur > 0)
                        {
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_DELETESTRING, Cur, 0);
                            Cur--;
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                        }
                    }
                    break;

                case NOBACK_FILE_DOWN :
                    if((Cur = SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETCURSEL, 0, 0)) != LB_ERR)
                    {
                        Max = SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETCOUNT, 0, 0);
                        if(Cur < Max - 1)
                        {
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_GETTEXT, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_DELETESTRING, Cur, 0);
                            Cur++;
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_INSERTSTRING, Cur, (LPARAM)Tmp);
                            SendDlgItemMessage(hDlg, NOBACK_FILE_LIST, LB_SETSEL, TRUE, Cur);
                            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                        }
                    }
                    break;

                case NOBACK_FILE_TOOLMENU :
                    SrcToolMenu(hDlg, FALSE, MENU_SORT2);
                    break;

                case MENU_SORT2 :
                    SortListBoxItem(GetDlgItem(hDlg, NOBACK_FILE_LIST));
                    SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                    break;

                case NOBACK_FILE_LIST :
                    switch(GET_WM_COMMAND_CMD(wParam, lParam))
                    {
                        case LBN_DBLCLK :
                        case LBN_SELCHANGE :
                        case LBN_SELCANCEL :
                            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
                            break;
                    }
                    break;
            }
            return(TRUE);

        case WM_ADD_DIRLIST :
            SetStrToListBox((_TCHAR*)lParam, hDlg, NOBACK_DIR_LIST, IGN_PATH_LEN+1, -1);
            GetMultiTextFromList(hDlg, NOBACK_DIR_LIST, TmpPat.IgnDir, IGN_PATH_LEN+1);
            free((_TCHAR*)lParam);
            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
            break;

        case WM_ADD_FILELIST :
            SetStrToListBox((_TCHAR*)lParam, hDlg, NOBACK_FILE_LIST, IGN_PATH_LEN+1, -1);
            GetMultiTextFromList(hDlg, NOBACK_FILE_LIST, TmpPat.IgnFile, IGN_PATH_LEN+1);
            free((_TCHAR*)lParam);
            SetDirButtonHide(hDlg, NOBACK_FILE_LIST, NOBACK_FILE_EDIT, NOBACK_FILE_UP, NOBACK_FILE_DOWN, NOBACK_FILE_DEL);
            break;
    }
    return(FALSE);
}


/*----- 無視するフォルダリストのメッセージ処理 --------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK DirListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DROPFILES:
            SendDropFilesToControl(hWndIgnorePage, WM_ADD_DIRLIST, wParam, lParam, SEND_FOLDER);
            break;

        case WM_KEYDOWN:
            if(wParam == VK_DELETE)
            {
                PostMessage(hWndIgnorePage, WM_COMMAND, MAKEWPARAM(NOBACK_DIR_DEL, 0), (LPARAM)0);
                break;
            }
            return(CallWindowProc(DirListProcPtr, hWnd, message, wParam, lParam));

        default :
            return(CallWindowProc(DirListProcPtr, hWnd, message, wParam, lParam));
    }
    return(0L);
}


/*----- 無視するファイルリストのメッセージ処理 --------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK FileListWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_DROPFILES:
            SendDropFilesToControl(hWndIgnorePage, WM_ADD_FILELIST, wParam, lParam, SEND_FILE);
            break;

        case WM_KEYDOWN:
            if(wParam == VK_DELETE)
            {
                PostMessage(hWndIgnorePage, WM_COMMAND, MAKEWPARAM(NOBACK_FILE_DEL, 0), (LPARAM)0);
                break;
            }
            return(CallWindowProc(FileListProcPtr, hWnd, message, wParam, lParam));

        default :
            return(CallWindowProc(FileListProcPtr, hWnd, message, wParam, lParam));
    }
    return(0L);
}


/*----- 除外設定２ウインドウのメッセージ処理 ----------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK IgnoreSetting2Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[10];

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, NOBACK_SYSTEM_FILE, BM_SETCHECK, TmpPat.IgnSystemFile, 0);
            SendDlgItemMessage(hDlg, NOBACK_HIDDEN_FILE, BM_SETCHECK, TmpPat.IgnHiddenFile, 0);
            SendDlgItemMessage(hDlg, NOBACK_BIG_FILE, BM_SETCHECK, TmpPat.IgnBigFile, 0);
            if(TmpPat.IgnBigFile == NO)
            {
                EnableWindow(GetDlgItem(hDlg, NOBACK_BIG_SIZE), FALSE);
            }
            _stprintf(Tmp, _T("%d"), TmpPat.IgnBigSize);
            SendDlgItemMessage(hDlg, NOBACK_BIG_SIZE, EM_LIMITTEXT, (WPARAM)7, 0);
            SendDlgItemMessage(hDlg, NOBACK_BIG_SIZE, WM_SETTEXT, 0, (LPARAM)Tmp);
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    TmpPat.IgnSystemFile = SendDlgItemMessage(hDlg, NOBACK_SYSTEM_FILE, BM_GETCHECK, 0, 0);
                    TmpPat.IgnHiddenFile = SendDlgItemMessage(hDlg, NOBACK_HIDDEN_FILE, BM_GETCHECK, 0, 0);
                    TmpPat.IgnBigFile = SendDlgItemMessage(hDlg, NOBACK_BIG_FILE, BM_GETCHECK, 0, 0);
                    SendDlgItemMessage(hDlg, NOBACK_BIG_SIZE, WM_GETTEXT, 7+1, (LPARAM)Tmp);
                    TmpPat.IgnBigSize = _tstoi(Tmp);
                    if(TmpPat.IgnBigFile && (TmpPat.IgnBigSize == 0))
                    {
                        TmpPat.IgnBigFile = NO;
                    }
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case NOBACK_SYSTEM_FILE :
                    /* 容量計算ボタンが押されたときのために、変更があったらすぐに保存しておく */
                    TmpPat.IgnSystemFile = SendDlgItemMessage(hDlg, NOBACK_SYSTEM_FILE, BM_GETCHECK, 0, 0);
                    break;

                case NOBACK_HIDDEN_FILE :
                    /* 容量計算ボタンが押されたときのために、変更があったらすぐに保存しておく */
                    TmpPat.IgnHiddenFile = SendDlgItemMessage(hDlg, NOBACK_HIDDEN_FILE, BM_GETCHECK, 0, 0);
                    break;

                case NOBACK_BIG_FILE :
                    /* 容量計算ボタンが押されたときのために、変更があったらすぐに保存しておく */
                    TmpPat.IgnBigFile = SendDlgItemMessage(hDlg, NOBACK_BIG_FILE, BM_GETCHECK, 0, 0);
                    if(TmpPat.IgnBigFile == NO)
                    {
                        EnableWindow(GetDlgItem(hDlg, NOBACK_BIG_SIZE), FALSE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, NOBACK_BIG_SIZE), TRUE);
                    }
                    break;

                case NOBACK_BIG_SIZE :
                    /* 容量計算ボタンが押されたときのために、変更があったらすぐに保存しておく */
                    SendDlgItemMessage(hDlg, NOBACK_BIG_SIZE, WM_GETTEXT, 7+1, (LPARAM)Tmp);
                    TmpPat.IgnBigSize = _tstoi(Tmp);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- フラグ設定ウインドウのメッセージ処理 ----------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK FlagSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[10];

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, PATSET_RMDIR, BM_SETCHECK, TmpPat.DelDir, 0);
            SendDlgItemMessage(hDlg, PATSET_RMFILE, BM_SETCHECK, TmpPat.DelFile, 0);
            SendDlgItemMessage(hDlg, PATSET_IGNNODEL, BM_SETCHECK, TmpPat.IgnNoDel, 0);
            SendDlgItemMessage(hDlg, PATSET_NOTIFY_DEL, BM_SETCHECK, TmpPat.NotifyDel, 0);
            SendDlgItemMessage(hDlg, PATSET_NOTIFY_OVERWRITE, BM_SETCHECK, TmpPat.NotifyOvw, 0);
            SendDlgItemMessage(hDlg, PATSET_FORCE, BM_SETCHECK, TmpPat.ForceCopy, 0);
            if(TmpPat.ForceCopy == YES)
            {
                EnableWindow(GetDlgItem(hDlg, PATSET_NEWONLY), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE_SPN), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_IGNATTR), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_IGNTIME), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_NOTIFY_OVERWRITE), FALSE);
            }
            SendDlgItemMessage(hDlg, PATSET_NEWONLY, BM_SETCHECK, TmpPat.NewOnly, 0);
            SendDlgItemMessage(hDlg, PATSET_NOERROR, BM_SETCHECK, TmpPat.IgnoreErr, 0);
            SendDlgItemMessage(hDlg, PATSET_USE_TRASHCAN, BM_SETCHECK, TmpPat.UseTrashCan, 0);
            SendDlgItemMessage(hDlg, PATSET_WAIT, TBM_SETRANGE, TRUE, MAKELPARAM(0, 20));
            SendDlgItemMessage(hDlg, PATSET_WAIT, TBM_SETPOS, TRUE, TmpPat.Wait);
            _stprintf(Tmp, _T("%d"), TmpPat.Tolerance);
            SendDlgItemMessage(hDlg, PATSET_TOLERANCE_SPN, UDM_SETRANGE, 0, MAKELONG(999, 0));
            SendDlgItemMessage(hDlg, PATSET_TOLERANCE, EM_LIMITTEXT, (WPARAM)3, 0);
            SendDlgItemMessage(hDlg, PATSET_TOLERANCE, WM_SETTEXT, 0, (LPARAM)Tmp);
            SendDlgItemMessage(hDlg, PATSET_IGNATTR, BM_SETCHECK, TmpPat.IgnAttr, 0);
            SendDlgItemMessage(hDlg, PATSET_IGNTIME, BM_SETCHECK, TmpPat.IgnTime, 0);
            if(TmpPat.IgnTime == YES)
            {
                EnableWindow(GetDlgItem(hDlg, PATSET_NEWONLY), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE_SPN), FALSE);
            }
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    TmpPat.DelDir = SendDlgItemMessage(hDlg, PATSET_RMDIR, BM_GETCHECK, 0, 0);
                    TmpPat.DelFile = SendDlgItemMessage(hDlg, PATSET_RMFILE, BM_GETCHECK, 0, 0);
                    TmpPat.IgnNoDel = SendDlgItemMessage(hDlg, PATSET_IGNNODEL, BM_GETCHECK, 0, 0);
                    TmpPat.NotifyDel = SendDlgItemMessage(hDlg, PATSET_NOTIFY_DEL, BM_GETCHECK, 0, 0);
                    TmpPat.NotifyOvw = SendDlgItemMessage(hDlg, PATSET_NOTIFY_OVERWRITE, BM_GETCHECK, 0, 0);
                    TmpPat.ForceCopy = SendDlgItemMessage(hDlg, PATSET_FORCE, BM_GETCHECK, 0, 0);
                    TmpPat.NewOnly = SendDlgItemMessage(hDlg, PATSET_NEWONLY, BM_GETCHECK, 0, 0);
                    TmpPat.IgnoreErr = SendDlgItemMessage(hDlg, PATSET_NOERROR, BM_GETCHECK, 0, 0);
                    TmpPat.UseTrashCan = SendDlgItemMessage(hDlg, PATSET_USE_TRASHCAN, BM_GETCHECK, 0, 0);
                    TmpPat.Wait = SendDlgItemMessage(hDlg, PATSET_WAIT, TBM_GETPOS, 0, 0);
                    TmpPat.IgnAttr = SendDlgItemMessage(hDlg, PATSET_IGNATTR, BM_GETCHECK, 0, 0);
                    TmpPat.IgnTime = SendDlgItemMessage(hDlg, PATSET_IGNTIME, BM_GETCHECK, 0, 0);
                    SendDlgItemMessage(hDlg, PATSET_TOLERANCE, WM_GETTEXT, 4, (LPARAM)Tmp);
                    TmpPat.Tolerance = _tstoi(Tmp);
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case PATSET_FORCE :
                    if(SendDlgItemMessage(hDlg, PATSET_FORCE, BM_GETCHECK, 0, 0) == 1)
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_IGNATTR), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_IGNTIME), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_NEWONLY), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE_SPN), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_NOTIFY_OVERWRITE), FALSE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_IGNATTR), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_IGNTIME), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_NOTIFY_OVERWRITE), TRUE);
                        if(SendDlgItemMessage(hDlg, PATSET_IGNTIME, BM_GETCHECK, 0, 0) == 0)
                        {
                            EnableWindow(GetDlgItem(hDlg, PATSET_NEWONLY), TRUE);
                            EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE), TRUE);
                            EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE_SPN), TRUE);
                        }
                    }
                    break;

                case PATSET_IGNTIME :
                    if(SendDlgItemMessage(hDlg, PATSET_IGNTIME, BM_GETCHECK, 0, 0) == 1)
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_NEWONLY), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE_SPN), FALSE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_NEWONLY), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TOLERANCE_SPN), TRUE);
                    }
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- 高度な設定ウインドウのメッセージ処理 ----------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK AdvancedSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, PATSET_NO_DSTDIR, BM_SETCHECK, TmpPat.NoMakeTopDir, 0);
            SetAdvancedPage(SheetInfo[PAGE_DIR].hWndSheet, hDlg);
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    TmpPat.NoMakeTopDir = SendDlgItemMessage(hDlg, PATSET_NO_DSTDIR, BM_GETCHECK, 0, 0);
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case PATSET_ADV_RESET :
                    SendDlgItemMessage(hDlg, PATSET_NO_DSTDIR, BM_SETCHECK, 0, 0);
                    break;
            }
    }
    return(FALSE);
}


/*----- タイマ設定ウインドウのメッセージ処理 ----------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK TimerSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[5+1];

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, PATSET_TIMER_INT_SW, BM_SETCHECK, (TmpPat.IntervalTime > 0 ? 1 : 0), 0);
            SendDlgItemMessage(hDlg, PATSET_TIMER_INT_TIME, EM_LIMITTEXT, (WPARAM)5, 0);
            _stprintf(Tmp, _T("%d"), abs(TmpPat.IntervalTime));
            SendDlgItemMessage(hDlg, PATSET_TIMER_INT_TIME, WM_SETTEXT, 0, (LPARAM)Tmp);
            SendDlgItemMessage(hDlg, PATSET_TIMER_INT_TIME_SPN, UDM_SETRANGE, 0, MAKELONG(2000, 1));
            PostMessage(hDlg, WM_COMMAND, MAKEWPARAM(PATSET_TIMER_INT_SW, 0), 0);
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    SendDlgItemMessage(hDlg, PATSET_TIMER_INT_TIME, WM_GETTEXT, 5+1, (LPARAM)Tmp);
                    TmpPat.IntervalTime = _tstoi(Tmp);
                    CheckRange2(&TmpPat.IntervalTime, 2000, 1);
                    if(SendDlgItemMessage(hDlg, PATSET_TIMER_INT_SW, BM_GETCHECK, 0, 0) == 0)
                        TmpPat.IntervalTime *= -1;
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case PATSET_TIMER_INT_SW :
                    if(SendDlgItemMessage(hDlg, PATSET_TIMER_INT_SW, BM_GETCHECK, 0, 0) == 1)
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_TIMER_INT_TIME), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TIMER_INT_TIME_SPN), TRUE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_TIMER_INT_TIME), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_TIMER_INT_TIME_SPN), FALSE);
                    }
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- システム設定ウインドウのメッセージ処理 --------------------------------
*
*   Parameter
*       HWND hWnd : ウインドウハンドル
*       UINT message  : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       メッセージに対応する戻り値
*----------------------------------------------------------------------------*/

static LRESULT CALLBACK SystemSettingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    NMHDR *pnmhdr;
    _TCHAR Tmp[MY_MAX_PATH+1];
    OSVERSIONINFO osvi;

    switch (message)
    {
        case WM_INITDIALOG :
            SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_93);
            SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_94);
            SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_95);
            osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionEx(&osvi);
            if(osvi.dwMajorVersion >= WINDOWS_VISTA_VERSION)
            {
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_120);
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_97);
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_121);
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_102);
            }
            else
            {
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_96);
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_97);
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_101);
                SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_ADDSTRING, 0, (LPARAM)MSGJPN_102);
            }
            SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_SETCURSEL, TmpPat.AutoClose, 0);

            SendDlgItemMessage(hDlg, PATSET_SOUND, BM_SETCHECK, TmpPat.Sound, 0);
            SendDlgItemMessage(hDlg, PATSET_SOUNDFILE, EM_LIMITTEXT, (WPARAM)MY_MAX_PATH, 0);
            SendDlgItemMessage(hDlg, PATSET_SOUNDFILE, WM_SETTEXT, 0, (LPARAM)TmpPat.SoundFile);
            if(TmpPat.Sound == NO)
            {
                EnableWindow(GetDlgItem(hDlg, PATSET_SOUNDFILE), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_SOUNDFILE_BR), FALSE);
                EnableWindow(GetDlgItem(hDlg, PATSET_SOUND_TEST), FALSE);
            }
            return(TRUE);

        case WM_NOTIFY:
            pnmhdr = (NMHDR FAR *)lParam;
            switch(pnmhdr->code)
            {
                case PSN_APPLY :
                    TmpPat.AutoClose = SendDlgItemMessage(hDlg, PATSET_SYSTEM, CB_GETCURSEL, 0, 0);
                    TmpPat.Sound = SendDlgItemMessage(hDlg, PATSET_SOUND, BM_GETCHECK, 0, 0);
                    SendDlgItemMessage(hDlg, PATSET_SOUNDFILE, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)TmpPat.SoundFile);
                    break;
            }
            break;

        case WM_COMMAND :
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
                case PATSET_SOUND :
                    if(SendDlgItemMessage(hDlg, PATSET_SOUND, BM_GETCHECK, 0, 0) == 1)
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_SOUNDFILE), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_SOUNDFILE_BR), TRUE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_SOUND_TEST), TRUE);
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, PATSET_SOUNDFILE), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_SOUNDFILE_BR), FALSE);
                        EnableWindow(GetDlgItem(hDlg, PATSET_SOUND_TEST), FALSE);
                    }
                    break;

                case PATSET_SOUNDFILE_BR :
                    _tcscpy(Tmp, _T(""));
                    if(SelectFile(hDlg, Tmp, MSGJPN_104, MSGJPN_105, _T(""), OFN_FILEMUSTEXIST, 0, MediaPath) == TRUE)
                        SendDlgItemMessage(hDlg, PATSET_SOUNDFILE, WM_SETTEXT, 0, (LPARAM)Tmp);
                    break;

                case PATSET_SOUND_TEST :
                    SendDlgItemMessage(hDlg, PATSET_SOUNDFILE, WM_GETTEXT, MY_MAX_PATH+1, (LPARAM)Tmp);
                    PlaySound(Tmp, NULL, SND_ASYNC);
                    break;
            }
            return(TRUE);
    }
    return(FALSE);
}


/*----- 現在の設定でファイル容量ダイアログを表示する --------------------------
*
*   Parameter
*       HWND hDlg : ダイアログボックスのウインドウハンドル
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void DisplayFileSizeDlg(HWND hDlg)
{
    COPYPATLIST Pat;

    memcpy(&Pat.Set, &TmpPat, sizeof(COPYPAT));
    Pat.Next = NULL;
    FilesSizeDialog(hDlg, &Pat);
    return;
}


/*----- 文字列をリストボックスにセット ----------------------------------------
*
*   Parameter
*       LPTSTR Str : 文字列
*       HWND hDlg : ダイアログボックスのウインドウハンドル
*       int CtrlList : リストボックスのID
*       int BufSize : バッファサイズ
*       int Pos : セット位置 (0～:その位置の文字列と置き換え, -1=最後に追加)
*
*   Return Value
*       int ステータス
*           SUCCESS/FAIL
*----------------------------------------------------------------------------*/

static int SetStrToListBox(LPTSTR Str, HWND hDlg, int CtrlList, int BufSize, int Pos)
{
    _TCHAR Tmp[MY_MAX_PATH+1];
    int Num;
    int i;
    int Len;
    int Sts;

    Sts = SUCCESS;
    Len = _tcslen(Str);
    if(Len > 0)
    {
        Len++;
        Num = SendDlgItemMessage(hDlg, CtrlList, LB_GETCOUNT, 0, 0);
        for(i = 0; i < Num; i++)
        {
            if(i != Pos)
            {
                SendDlgItemMessage(hDlg, CtrlList, LB_GETTEXT, i, (LPARAM)Tmp);
                Len += _tcslen(Tmp) + 1;
            }
        }

        if(Len > (BufSize-1))
        {
            /* 文字列の最大長さを越えた */
            MessageBeep((UINT)-1);
            Sts = FAIL;
        }
        else
        {
            if(Pos == -1)
            {
                SendDlgItemMessage(hDlg, CtrlList, LB_ADDSTRING, 0, (LPARAM)Str);
                Pos = SendDlgItemMessage(hDlg, CtrlList, LB_GETCOUNT, 0, 0) - 1;
            }
            else
            {
                SendDlgItemMessage(hDlg, CtrlList, LB_DELETESTRING, Pos, 0);
                SendDlgItemMessage(hDlg, CtrlList, LB_INSERTSTRING, Pos, (LPARAM)Str);
            }
            SendDlgItemMessage(hDlg, CtrlList, LB_SETCURSEL, Pos, 0);
        }
    }
    return(Sts);
}


/*----- マルチ文字列をリストボックスにセット ----------------------------------
*
*   Parameter
*       HWND hDlg : ダイアログボックスのウインドウハンドル
*       int CtrlList : リストボックスのID
*       LPTSTR Text : 文字列
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void SetMultiTextToList(HWND hDlg, int CtrlList, LPTSTR Text)
{
    LPTSTR Pos;

    Pos = Text;
    while(*Pos != NUL)
    {
        SendDlgItemMessage(hDlg, CtrlList, LB_ADDSTRING, 0, (LPARAM)Pos);
        Pos += _tcslen(Pos) + 1;
    }
    return;
}


/*----- リストボックスの内容をマルチ文字列にする ------------------------------
*
*   Parameter
*       HWND hDlg : ダイアログボックスのウインドウハンドル
*       int CtrlList : リストボックスのID
*       LPTSTR Buf : 文字列をセットするバッファ
*       int BufSize : バッファのサイズ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/

static void GetMultiTextFromList(HWND hDlg, int CtrlList, LPTSTR Buf, int BufSize)
{
    _TCHAR Tmp[MY_MAX_PATH+1];
    int Num;
    int i;
    int len;

    memset(Buf, NUL, BufSize * sizeof(_TCHAR));
    BufSize -= 1;
    Num = SendDlgItemMessage(hDlg, CtrlList, LB_GETCOUNT, 0, 0);
    for(i = 0; i < Num; i++)
    {
        SendDlgItemMessage(hDlg, CtrlList, LB_GETTEXT, i, (LPARAM)Tmp);
        len = _tcslen(Tmp);
        if(len > 0)
        {
            if(BufSize >= (len + 1))
            {
                _tcscpy(Buf + StrMultiLen(Buf), Tmp);
                BufSize -= (len + 1);
            }
            else
            {
                break;
            }
        }
    }
    return;
}

