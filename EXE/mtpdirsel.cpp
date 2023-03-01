/*===========================================================================
/
/                                   Backup
/                      MTPデバイスのフォルダー選択ダイアログ
/
/============================================================================
/ Copyright (C) 2022-2023 Sota. All rights reserved.
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
#include <tchar.h>
#include <windowsx.h>
#include <commctrl.h>
#include <portabledeviceapi.h>
#include <portabledevice.h>
#include <string>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>

#include "common.h"
#include "mtp_common.h"
#include "resource.h"


/* 構造体 */

/*==== ダイアログボックスに渡す情報 ====*/
typedef struct {
    LPTSTR DialogMessage;
    LPTSTR OperationDesription;
    LPTSTR url;
} DIALOG_PARAM;


/*===== MTPフォルダ選択TreeViewのLPARAM情報 =====*/
typedef struct {
    DWORD DeviceNumber;
    PWSTR ObjectId;
} MTP_FOLDER_TREEVIEW_PARAM;


/* プロトタイプ */
static BOOL CALLBACK MtpFolderSelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK DeviceTreeViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void SetDevicesToTreeView(HWND hDlg, MTP_DEVICE_LIST** deviceList);
static void SetFoldersToTreeView(HWND hDlg, HTREEITEM hItem, MTP_DEVICE_LIST* deviceList);
static void RemoveAllParamFromTreeView(HWND hDlg);
static void RemoveChildParamFromTreeView(HWND hDlg, HTREEITEM hParent);
static PWSTR MakeUrlFromTreeView(HWND hDlg);


/* 静的変数 */
static HINSTANCE hInstance = NULL;
static WNDPROC DeviceTreeViewProcPtr = NULL;


/*----- MTPデバイスのフォルダー選択ダイアログ ------------------------------------
*
*   Parameter
*       HINSTANCE hInst : インスタンス
*       HWND hWndParent : 親ウインドウのハンドル
*       LPTSTR* url : URLを返す変数へのポインタ
*
*   Return Value
*       int ステータス
*
*   Note
*       urlは free(url) で開放すること
*----------------------------------------------------------------------------*/
int ShowMtpFolderSelectDialog(HINSTANCE hInst, HWND hWndParent, LPTSTR* url)
{
    int status = FAIL;
    DIALOG_PARAM param;

    hInstance = hInst;

    if (CountMtpDevices() == 0)
    {
        /* MTPデバイスがないことを表示 */
        DialogBoxParam(hInstance, MAKEINTRESOURCE(common_msg_dlg), hWndParent, ExeEscDialogProc, (LPARAM)MSGJPN_142);
    }
    else
    {
        param.DialogMessage = MSGJPN_143;
        param.OperationDesription = MSGJPN_144;
        param.url = NULL;
        if (DialogBoxParam(hInst, MAKEINTRESOURCE(mtp_folder_select_dlg), hWndParent, MtpFolderSelectDlgProc, (LPARAM)&param) == YES)
        {
            if (param.url != NULL)
            {
                status = SUCCESS;
                *url = param.url;
            }
        }
    }
    return status;
}


/*----- MTPデバイスのフォルダー選択ダイアログのコールバック ------------------------
*
*   Parameter
*       HWND hDlg : ウインドウハンドル
*       UINT message : メッセージ番号
*       WPARAM wParam : メッセージの WPARAM 引数
*       LPARAM lParam : メッセージの LPARAM 引数
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
static BOOL CALLBACK MtpFolderSelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    TVHITTESTINFO HitInfo;
    HTREEITEM hItem = NULL;
    static DIALOG_PARAM* dialogParam = NULL;
    static MTP_DEVICE_LIST* deviceList = NULL;
    static HIMAGELIST hImage = NULL;

    switch (message)
    {
    case WM_INITDIALOG:
        dialogParam = (DIALOG_PARAM*)lParam;

        /* TreeViewでのダブルクリックをつかまえるため */
        DeviceTreeViewProcPtr = (WNDPROC)SetWindowLong(GetDlgItem(hDlg, MTP_FOLDER_TREE), GWL_WNDPROC, (LONG)DeviceTreeViewWndProc);

        SendDlgItemMessage(hDlg, MTP_FOLDER_MSG, WM_SETTEXT, 0, (LPARAM)dialogParam->DialogMessage);
        SendDlgItemMessage(hDlg, MTP_FOLDER_DESC, WM_SETTEXT, 0, (LPARAM)dialogParam->OperationDesription);
        hImage = ImageList_LoadBitmap(hInstance, MAKEINTRESOURCE(device_bmp), 16, 8, RGB(255, 0, 0));
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImage);
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);

        /* MTPデバイスの一覧をTreeViewにセット */
        SetDevicesToTreeView(hDlg, &deviceList);
        return(TRUE);

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:
            DoPrintf(_T("IDOK\r\n"));
            /* URLを作成して返す */
            dialogParam->url = MakeUrlFromTreeView(hDlg);
            if (hImage != NULL)
            {
                ImageList_Destroy(hImage);
            }
            RemoveAllParamFromTreeView(hDlg);
            ReleaseMtpDevices(deviceList);
            EndDialog(hDlg, YES);
            break;

        case IDCANCEL:
            DoPrintf(_T("IDCANCEL\r\n"));
            dialogParam->url = NULL;
            if (hImage != NULL)
            {
                ImageList_Destroy(hImage);
            }
            RemoveAllParamFromTreeView(hDlg);
            ReleaseMtpDevices(deviceList);
            EndDialog(hDlg, NO);
            break;
        }
        return(TRUE);

    case WM_MTP_TREEVIEW_DCLICK:
        /* TreeViewの項目の上でダブルクリックされたかチェック */
        HitInfo.pt.x = LOWORD(lParam);
        HitInfo.pt.y = HIWORD(lParam);
        HitInfo.flags = TVHT_ONITEM;
        hItem = (HTREEITEM)SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETNEXTITEM, TVGN_CARET, 0);
        HitInfo.hItem = hItem;
        if ((HTREEITEM)SendMessage(GetDlgItem(hDlg, MTP_FOLDER_TREE), TVM_HITTEST, 0, (LPARAM)&HitInfo) == hItem)
        {
            /* フォルダ一覧をTreeViewにセット */
            SetFoldersToTreeView(hDlg, hItem, deviceList);
        }
        break;

    }
    return(FALSE);
}


/*----- TreeViewのメッセージ処理 ------------------------------------------------
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
static LRESULT CALLBACK DeviceTreeViewWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDBLCLK:
        PostMessage(GetParent(hWnd), WM_MTP_TREEVIEW_DCLICK, 0, lParam);
        break;
    }
    return(CallWindowProc(DeviceTreeViewProcPtr, hWnd, message, wParam, lParam));
}


/*----- デバイスの一覧をTreeViewにセット -----------------------------------------
*
*   Parameter
*       HWND hDlg: ダイアログのウインドウハンドル
*       MTP_DEVICE_LIST** deviceList : デバイスリストを格納する変数へのポインタ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void SetDevicesToTreeView(HWND hDlg, MTP_DEVICE_LIST** deviceList)
{
    TVINSERTSTRUCT TvIns;
    MTP_FOLDER_TREEVIEW_PARAM* folderInfoParam;

    /* MTPデバイスを列挙してTreeViewに追加 */
    if (EnumerateMtpDevices(deviceList) == SUCCESS)
    {
        /* ちらつくので再描画禁止 */
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, WM_SETREDRAW, (WPARAM)FALSE, 0);

        for (DWORD i = 0; i < (*deviceList)->NumberOfDevices; i++)
        {
            folderInfoParam = new MTP_FOLDER_TREEVIEW_PARAM;
            folderInfoParam->ObjectId = new WCHAR[wcslen(WPD_DEVICE_OBJECT_ID) + 1];
            folderInfoParam->DeviceNumber = i;
            wcscpy(folderInfoParam->ObjectId, WPD_DEVICE_OBJECT_ID);

            TvIns.hParent = TVI_ROOT;
            TvIns.hInsertAfter = TVI_LAST;
            TvIns.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
            TvIns.item.cChildren = 0;
            TvIns.item.pszText = (*deviceList)->Info[i].DeviceDescription;
            TvIns.item.iImage = MTP_DEVICE;
            TvIns.item.iSelectedImage = MTP_DEVICE;
            TvIns.item.lParam = (LPARAM)folderInfoParam;
            SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_INSERTITEM, 0, (LPARAM)&TvIns);
        }

        /* 再描画 */
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, WM_SETREDRAW, (WPARAM)TRUE, 0);
    }
}


/*----- フォルダの一覧（一階層）をTreeViewにセット ------------------------------
*
*   Parameter
*       HWND hDlg: ダイアログのウインドウハンドル
*       HTREEITEM hItem : 親フォルダのTreeViewアイテム
*       MTP_DEVICE_LIST* deviceList : デバイスリストを格納する変数へのポインタ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void SetFoldersToTreeView(HWND hDlg, HTREEITEM hItem, MTP_DEVICE_LIST* deviceList)
{
    TVINSERTSTRUCT TvIns;
    TVITEM TvItem;
    DWORD childs = 0;
    MTP_FOLDER_TREEVIEW_PARAM* folderInfoParam;
    MTP_FOLDER_TREEVIEW_PARAM* folderInfoParamParent;
    MTP_OBJECT_LIST* objectListCur;
    MTP_OBJECT_LIST* objectListTop;
    CComPtr<IPortableDevice> pIPortableDevice;

    /* TreeViewの項目の上でダブルクリックされた → 子アイテム作成済みかチェック */
    TvItem.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_PARAM;
    TvItem.hItem = hItem;
    SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETITEM, 0, (LPARAM)&TvItem);
    if (TvItem.cChildren == 0)
    {
        folderInfoParamParent = (MTP_FOLDER_TREEVIEW_PARAM*)TvItem.lParam;

        /* MTPデバイスをオープン */
        if (OpenMtpDevice(deviceList->Info[folderInfoParamParent->DeviceNumber].DeviceID, &pIPortableDevice) == SUCCESS)
        {
            /* 子アイテム未作成 → フォルダを検索してTreeViewに追加 */
            if (EnumerateMtpObject(pIPortableDevice, folderInfoParamParent->ObjectId, ObjectTypeFolder, YES, &objectListTop) == SUCCESS)
            {
                if (objectListTop != NULL)
                {
                    /* ちらつくので再描画禁止 */
                    SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, WM_SETREDRAW, (WPARAM)FALSE, 0);

                    childs = 0;
                    objectListCur = objectListTop;
                    while (objectListCur != NULL)
                    {
                        folderInfoParam = new MTP_FOLDER_TREEVIEW_PARAM;
                        folderInfoParam->ObjectId = new WCHAR[wcslen(objectListCur->Info.ObjectID) + 1];
                        wcscpy(folderInfoParam->ObjectId, objectListCur->Info.ObjectID);
                        folderInfoParam->DeviceNumber = folderInfoParamParent->DeviceNumber;

                        TvIns.hParent = hItem;
                        TvIns.hInsertAfter = TVI_LAST;
                        TvIns.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                        TvIns.item.cChildren = 0;
                        TvIns.item.pszText = objectListCur->Info.ObjectName;
                        TvIns.item.iImage = MTP_FOLDER;
                        TvIns.item.iSelectedImage = MTP_FOLDER_SEL;
                        TvIns.item.lParam = (LPARAM)folderInfoParam;
                        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_INSERTITEM, 0, (LPARAM)&TvIns);

                        childs++;
                        objectListCur = objectListCur->Next;
                    }

                    if (childs > 0)
                    {
                        TvItem.cChildren = 1;
                        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_SETITEM, 0, (LPARAM)&TvItem);

                        /* アイテムを開く */
                        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_EXPAND, TVE_EXPAND, (LPARAM)hItem);
                    }

                    /* 再描画 */
                    SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, WM_SETREDRAW, (WPARAM)TRUE, 0);

                    /* フォルダ情報を削除 */
                    ReleaseMtpObject(objectListTop);
                }
            }
        }
    }
}


/*----- TreeViewの全項目からParamを除去 ----------------------------------------
*
*   Parameter
*       HWND hDlg: ダイアログのウインドウハンドル
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void RemoveAllParamFromTreeView(HWND hDlg)
{
    HTREEITEM hItem;

    hItem = (HTREEITEM)SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETNEXTITEM, TVGN_ROOT, 0);
    RemoveChildParamFromTreeView(hDlg, hItem);
}


/*----- TreeViewの項目の子と兄弟のParamを除去 -----------------------------------
*
*   Parameter
*       HWND hDlg: ダイアログのウインドウハンドル
*       HTREEITEM hParent : TreeViewの項目
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void RemoveChildParamFromTreeView(HWND hDlg, HTREEITEM hParent)
{
    HTREEITEM hItem;
    TVITEM TvItem;
    MTP_FOLDER_TREEVIEW_PARAM* folderInfoParam;

    while (hParent != NULL)
    {
        TvItem.mask = TVIF_PARAM | TVIF_HANDLE;
        TvItem.hItem = hParent;
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETITEM, 0, (LPARAM)&TvItem);
        folderInfoParam = (MTP_FOLDER_TREEVIEW_PARAM*)TvItem.lParam;

        TvItem.mask = TVIF_PARAM | TVIF_HANDLE;
        TvItem.lParam = (LPARAM)NULL;
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_SETITEM, 0, (LPARAM)&TvItem);

        /* 自身のParamを削除 */
        delete folderInfoParam->ObjectId;
        delete folderInfoParam;

        /* 子のParamを再帰的に削除 */
        hItem = (HTREEITEM)SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hParent);
        RemoveChildParamFromTreeView(hDlg, hItem);

        /* 兄弟の項目へ移動 */
        hParent = (HTREEITEM)SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hParent);
    }
    return;
}


/*----- TreeViewで選択中の項目のURLを返す ---------------------------------------
*
*   Parameter
*       HWND hDlg: ダイアログのウインドウハンドル
*
*   Return Value
*       PWSTR URL文字列 （呼び出し側で free で削除すること）
*----------------------------------------------------------------------------*/
static PWSTR MakeUrlFromTreeView(HWND hDlg)
{
    struct itemSelectedTree {
        PWSTR Name;
        struct itemSelectedTree* Next;
    };

    #define ITEM_NAME_LENGTH 256

    PWSTR url = NULL;
    DWORD urlLength;
    HTREEITEM hItem;
    TVITEM TvItem;
    struct itemSelectedTree* top = NULL;
    struct itemSelectedTree* current = NULL;
    struct itemSelectedTree* tmp = NULL;

    /* TreeViewで選択されている項目から親に向かって検索していく */
    hItem = (HTREEITEM)SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETNEXTITEM, TVGN_CARET, 0);
    while (hItem != NULL)
    {
        /* 項目のテキストを取得 */
        TvItem.mask = TVIF_TEXT;
        TvItem.hItem = hItem;
        TvItem.pszText = new WCHAR[ITEM_NAME_LENGTH + 1];
        TvItem.cchTextMax = ITEM_NAME_LENGTH;
        SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETITEM, 0, (LPARAM)&TvItem);

        /* リンクリストの先頭に追加 */
        current = new struct itemSelectedTree;
        current->Next = top;
        current->Name = TvItem.pszText;
        top = current;

        /* 親へと遡っていく */
        hItem = (HTREEITEM)SendDlgItemMessage(hDlg, MTP_FOLDER_TREE, TVM_GETNEXTITEM, TVGN_PARENT, (LPARAM)hItem);
    }

    /* URLの最終的な長さを計算 */
    urlLength = 0;
    current = top;
    while (current != NULL)
    {
        urlLength += wcslen(current->Name) + 1;     /* +1は / の分 */
        current = current->Next;
    }

    if (urlLength > 0)
    {
        urlLength += 6 + 1;         /* "mtp://" とターミネータの分 */
        url = (PWSTR)malloc(sizeof(WCHAR) * urlLength);     /* .C ソースに値を返す関係でnewではなくmallocでメモリ確保 */
        if (url != NULL)
        {
            wcscpy(url, L"mtp://");

            /* URL文字列を作成 */
            current = top;
            while (current != NULL)
            {
                wcscat(url, current->Name);
                wcscat(url, L"/");
                delete[] current->Name;
                tmp = current->Next;
                delete current;
                current = tmp;
            }
        }
    }
    return url;
}

