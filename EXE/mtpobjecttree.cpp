/*===========================================================================
/
/                                   Backup
/                       MTPデバイスのオブジェクトツリー
/
/       MTPデバイスのファイル検索等を高速に行うために、あらかじめデバイス上の
/       オブジェクトをリストアップし、ツリー構造で残しておく。
/       FindFirstFile / FindNextFile などは、このオブジェクトのツリーを
/       利用して行う。
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
#include <tchar.h>
#include <portabledeviceapi.h>
#include <portabledevice.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>

#include "common.h"
#include "mtp_common.h"


/* プロトタイプ */
static PWSTR SplitUrl(PCWSTR url, int part);
static int RecursiveMakeMtpObjectTree(PWSTR deviceId, PWSTR filter, MTP_OBJECT_TYPE objectType, MTP_OBJECT_TREE* anchor);


/*----- MTPデバイスのオブジェクトのツリーを作成 ------------------------------------
*
*   Parameter
*       PWSTR url : バックアップ先のURL
*       MTP_OBJECT_TYPE objectType : ツリーに含めるオブジェクトのタイプ
*       MTP_OBJECT_TREE** top : ツリー構造体を返す変数へのポインタ
*       MTP_MAKE_OBJECT_TREE_ERROR_INFO* ErrorInfo : エラー情報
*
*   Return Value
*       int ステータス
*
*   Note
*       ステータス==FAILの時、ErrorInfo->ObjectName はfree()で削除すること
*
*       例）URLが mtp://devive/folder1/folder2/ の場合、以下のようなツリーを返す
*          （device, folder1, folder2 に兄弟がいても、URLで指定されているフォルダの兄弟は返さない）
*           top ---> device ---> folder1 ---> folder2 ---> child1 ---> grandchild1
*                                                      +           +-> grandchild2 ---> grandchild3
*                                                      +-> child2
*                                                      +-> child3 ---> grandchild4
*----------------------------------------------------------------------------*/
int MakeMtpObjectTree(PWSTR url, MTP_OBJECT_TYPE objectType, MTP_OBJECT_TREE** top, MTP_MAKE_OBJECT_TREE_ERROR_INFO* ErrorInfo)
{
    int status = FAIL;
    MTP_DEVICE_LIST* deviceList = NULL;
    PWSTR deviceName = NULL;
    int deviceNumber;
    MTP_OBJECT_TREE* treeTop = NULL;
    MTP_OBJECT_TREE* treeCur;
    PWSTR filter = L"";

    deviceName = SplitUrl(url, 0);
    if (deviceName != NULL)
    {
        /* デバイスの一覧を取得 */
        deviceNumber = -1;
        if (EnumerateMtpDevices(&deviceList) == SUCCESS)
        {
            /* URLで指定されたデバイスを検索 */
            for (DWORD i = 0; i < deviceList->NumberOfDevices; i++)
            {
                if (wcscmp(deviceList->Info[i].DeviceDescription, deviceName) == 0)
                {
                    deviceNumber = i;
                    break;
                }
            }

            if (deviceNumber != -1)
            {
                /* ツリーの先頭にデバイスの情報を追加 */
                treeTop = new MTP_OBJECT_TREE;
                treeTop->Child = NULL;
                treeTop->Sibling = NULL;
                treeTop->ThisIsBackupRoot = FALSE;
                treeTop->Deleted = FALSE;
                treeTop->Info.ObjectID = new WCHAR[wcslen(deviceList->Info[deviceNumber].DeviceID) + 1];
                treeTop->Info.ObjectName = new WCHAR[wcslen(deviceList->Info[deviceNumber].DeviceDescription) + 1];
                wcscpy(treeTop->Info.ObjectID, deviceList->Info[deviceNumber].DeviceID);
                wcscpy(treeTop->Info.ObjectName, deviceList->Info[deviceNumber].DeviceDescription);
                treeTop->Info.ObjectType = ObjectTypeDevice;
                treeTop->Info.ObjectModifiedTime.dwLowDateTime = 0;
                treeTop->Info.ObjectModifiedTime.dwHighDateTime = 0;
                treeTop->Info.ObjectSize = 0;
                treeCur = treeTop;

                /* URLで指定されているオブジェクトを子に追加 */
                status = SUCCESS;
                for (DWORD i = 1; filter != NULL; i++)
                {
                    filter = SplitUrl(url, i);
                    if (filter != NULL)
                    {
                        if (RecursiveMakeMtpObjectTree(treeTop->Info.ObjectID, filter, objectType, treeCur) == SUCCESS)
                        {
                            treeCur = treeCur->Child;
                        }
                        else
                        {
                            /* URLで指定されたオブジェクトが見つからなかった */
                            ErrorInfo->ErrorId = ErrorFolderNotFound;
                            ErrorInfo->ObjectName = (PWSTR)malloc(sizeof(WCHAR) * (wcslen(filter) + 1));
                            if (ErrorInfo->ObjectName != NULL)
                            {
                                wcscpy(ErrorInfo->ObjectName, filter);
                            }
                            status = FAIL;
                            break;
                        }
                        delete[] filter;
                    }
                }

                if (status == SUCCESS)
                {
                    treeCur->ThisIsBackupRoot = TRUE;
                    RecursiveMakeMtpObjectTree(treeTop->Info.ObjectID, NULL, objectType, treeCur);
                    *top = treeTop;

                    /* debug */
                    DoPrintf(_T("------------------------------\r\n"));
                    DispMtpObjectTree(treeTop, 0);
                    DoPrintf(_T("------------------------------\r\n"));
                }
            }
        }

        if (deviceNumber == -1)
        {
            /* 指定されたデバイスが見つからなかった */
            ErrorInfo->ErrorId = ErrorDeviceNotFound;
            ErrorInfo->ObjectName = (PWSTR)malloc(sizeof(WCHAR) * (wcslen(deviceName) + 1));
            if (ErrorInfo->ObjectName != NULL)
            {
                wcscpy(ErrorInfo->ObjectName, deviceName);
            }
        }
    }
    else
    {
        /* URLにデバイスの指示がない */
        ErrorInfo->ErrorId = ErrorInvalidUrl;
        ErrorInfo->ObjectName = NULL;
    }

    /* クリーンアップ */
    delete[] deviceName;
    ReleaseMtpDevices(deviceList);
    if (status == FAIL)
    {
        ReleaseMtpObjectTree(treeTop);
        *top = NULL;
    }

    return status;
}


/*----- 兄弟オブジェクトと子オブジェクトを再帰的に列挙 ----------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスのID
*       PWSTR filter : リストに追加する子オブジェクトの名前（NULL=指定無し）
*       MTP_OBJECT_TYPE objectType : 列挙するオブジェクトのタイプ
*       MTP_OBJECT_TREE* anchor : リストの追加位置（親）
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
static int RecursiveMakeMtpObjectTree(PWSTR deviceId, PWSTR filter, MTP_OBJECT_TYPE objectType, MTP_OBJECT_TREE* anchor)
{
    int status = FAIL;
    MTP_OBJECT_LIST* listTop = NULL;
    MTP_OBJECT_LIST* listCur;
    MTP_OBJECT_TREE* newObject;
    MTP_OBJECT_TREE* prevObject;
    BOOL add;

    PWSTR parentId = anchor->Info.ObjectID;
    if (anchor->Info.ObjectType == ObjectTypeDevice)
    {
        parentId = WPD_DEVICE_OBJECT_ID;
    }

    if (EnumerateMtpObject(deviceId, parentId, objectType, NO, &listTop) == SUCCESS)
    {
        listCur = listTop;
        while (listCur != NULL)
        {
            add = TRUE;
            if (filter != NULL)
            {
                add = FALSE;
                if (wcscmp(listCur->Info.ObjectName, filter) == 0)
                {
                    add = TRUE;
                }
            }

            if (add)
            {
                /* ツリーにオブジェクトの情報を追加 */
                status = SUCCESS;
                newObject = new MTP_OBJECT_TREE;
                newObject->Child = NULL;
                newObject->Sibling = NULL;
                newObject->ThisIsBackupRoot = FALSE;
                newObject->Deleted = FALSE;
                newObject->Info.ObjectID = new WCHAR[wcslen(listCur->Info.ObjectID) + 1];
                newObject->Info.ObjectName = new WCHAR[wcslen(listCur->Info.ObjectName) + 1];
                wcscpy(newObject->Info.ObjectID, listCur->Info.ObjectID);
                wcscpy(newObject->Info.ObjectName, listCur->Info.ObjectName);
                newObject->Info.ObjectType = listCur->Info.ObjectType;
                newObject->Info.ObjectModifiedTime = listCur->Info.ObjectModifiedTime;
                newObject->Info.ObjectSize = listCur->Info.ObjectSize;

//                DoPrintf(_T("MakeTree: %s\r\n"), newObject->Info.ObjectName);

                if (anchor->Child == NULL)
                {
                    /* 最初のオブジェクトは親の子に */
                    anchor->Child = newObject;
                    prevObject = newObject;
                }
                else
                {
                    /* 2つ目以降のオブジェクトは最初のオブジェクトの兄弟に */
                    prevObject->Sibling = newObject;
                    prevObject = newObject;
                }

                /* filter指定がない時は今見つかったオブジェクトの子も検索 */
                if (filter == NULL)
                {
                    if (newObject->Info.ObjectType == ObjectTypeFolder)
                    {
                        RecursiveMakeMtpObjectTree(deviceId, filter, objectType, newObject);
                    }
                }
                else    /* filter指定の時は指定されたもの一つ追加したら終わり */
                {
                    break;
                }
            }
            listCur = listCur->Next;
        }

        ReleaseMtpObject(listTop);
    }
    return status;
}


/*----- URLの部分文字列を返す ----------------------------------------------------
*
*   Parameter
*       PCWSTR url : URL
*       int part : どの部分を返すか（0～）
*           例：mtp://device/folder1/folder2/folder3/
*                       0       1      2       3
*
*   Return Value
*       PWSTR 部分文字列（NULL=ない）　（使い終わったら delete[] で削除すること）
*
* Note
*       URLの区切り文字は / と \\ を受け付ける
*----------------------------------------------------------------------------*/
static PWSTR SplitUrl(PCWSTR url, int part)
{
    PCWSTR bottom;
    PWSTR name = NULL;
    int length;
    PCWSTR tmp;

    if ((wcsncmp(url, L"mtp://", 6) == 0) || (wcsncmp(url, L"mtp:\\\\", 6) == 0))
    {
        url += 6;   /* mtp:// をスキップ */

        /* 取得する部分を探す */
        for (; part > 0; part--)
        {
            tmp = wcschr(url, L'/');
            if (tmp == NULL)
            {
                tmp = wcschr(url, L'\\');
            }
            url = tmp;
            if (url == NULL)
            {
                break;
            }
            else
            {
                url++;
                if (wcslen(url) == 0)
                {
                    url = NULL;
                    break;
                }
            }
        }

        if (url != NULL)
        {
            /* 部分文字列をコピー */
            bottom = wcschr(url, L'/');
            if (bottom == NULL)
            {
                bottom = wcschr(url, L'\\');
            }
            if (bottom != NULL)
            {
                length = bottom - url;
            }
            else
            {
                length = wcslen(url);
            }
            name = new WCHAR[length + 1];
            if (name != NULL)
            {
                ZeroMemory(name, sizeof(WCHAR) * (length + 1));
                wcsncpy(name, url, length);
            }
        }
    }
    return name;
}


/*----- MTPデバイスのオブジェクトのツリーを削除 ----------------------------------
*
*   Parameter
*       MTP_OBJECT_TREE* top : ツリー
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
void ReleaseMtpObjectTree(MTP_OBJECT_TREE* top)
{
    MTP_OBJECT_TREE* child;
    MTP_OBJECT_TREE* sibling;

    if (top != NULL)
    {
        child = top->Child;
        sibling = top->Sibling;

        delete[] top->Info.ObjectID;
        delete[] top->Info.ObjectName;
        delete top;

        ReleaseMtpObjectTree(child);
        ReleaseMtpObjectTree(sibling);
    }
}


/*----- MTPデバイスのオブジェクトのツリーをデバッグ表示 ----------------------------
*
*   Parameter
*       MTP_OBJECT_TREE* top : ツリー
*       int level : 段下げ (最初は0）
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
void DispMtpObjectTree(MTP_OBJECT_TREE* top, int level)
{
    MTP_OBJECT_TREE* child;
    MTP_OBJECT_TREE* sibling;
    SYSTEMTIME time;

    if (top != NULL)
    {
        child = top->Child;
        sibling = top->Sibling;

        for (int i = 0; i < level; i++)
        {
            DoPrintf(_T("  "));
        }
        FileTimeToSystemTime(&top->Info.ObjectModifiedTime, &time);
        DoPrintf(_T("%s %s (%s) (%d/%d/%d %d:%d:%d) (%llu)\r\n"), top->Info.ObjectName, top->Deleted ? _T("!DELETED!") : _T(""), top->Info.ObjectID, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, top->Info.ObjectSize);

        DispMtpObjectTree(child, level + 1);
        DispMtpObjectTree(sibling, level);
    }
}


/*----- MTPデバイスのオブジェクトツリーから指定のオブジェクトを検索する --------------
*
*   Parameter
*       PCWSTR url : 検索するurl
*       MTP_OBJECT_TREE* treeTop : ツリーの先頭
*       MTP_OBJECT_TREE** parent : 親オブジェクトを返す変数へのポインタ（NULL=親がない）
*
*   Return Value
*       MTP_OBJECT_TREE* 見つかったオブジェクト（NULL=見つからなかった）
*----------------------------------------------------------------------------*/
MTP_OBJECT_TREE* FindObjectFromTree(PCWSTR url, MTP_OBJECT_TREE* treeTop, MTP_OBJECT_TREE** parent)
{
    PWSTR path;
    MTP_OBJECT_TREE* treeCur;
    MTP_OBJECT_TREE* found = NULL;
    MTP_OBJECT_TREE* prev = NULL;

    /* オブジェクト名を探してツリーを一段ずつ探していく */
    treeCur = treeTop;
    *parent = NULL;
    if (treeCur != NULL)
    {
        path = L"";
        for (int part = 0; path != NULL; part++)
        {
            path = SplitUrl(url, part);
            if (path != NULL)
            {
                while (treeCur != NULL)
                {
                    if (wcscmp(path, L"*") == 0)    /* ワイルドカードで指定されたオブジェクト */
                    {
                        *parent = prev;
                        /* 削除済みでなければ見つかった */
                        /* 削除済みなら兄弟を探す */
                        if (!treeCur->Deleted)
                        {
                            break;
                        }
                    }
                    else if (_wcsicmp(path, treeCur->Info.ObjectName) == 0)   /* 名前が指定されているオブジェクト（Windowsに合わせて大文字／小文字同一視） */
                    {
                        *parent = prev;
                        if (treeCur->Deleted)
                        {
                            /* 探していたオブジェクトだが削除済み→見つからなかったことに */
                            treeCur = NULL;
                        }
                        break;
                    }
                    /* 兄弟を探す */
                    treeCur = treeCur->Sibling;
                }

                if (treeCur != NULL)
                {
                    /* 一段目的のものが見つかった */
                    found = treeCur;
                    prev = treeCur;
                    treeCur = treeCur->Child;
                    delete[] path;
                }
                else
                {
                    found = NULL;
                    delete[] path;
                    break;
                }
            }
        }
    }

    if (found == NULL)
    {
        *parent = NULL;
    }

    return found;
}


/*----- MTPデバイスのオブジェクトツリーから次の兄弟オブジェクトを検索する -----------
*
*   Parameter
*       MTP_OBJECT_TREE* object : オブジェクト
*
*   Return Value
*       MTP_OBJECT_TREE* 見つかったオブジェクト（NULL=見つからなかった）
*----------------------------------------------------------------------------*/
MTP_OBJECT_TREE* FindNextSiblingObjectFromTree(MTP_OBJECT_TREE* object)
{
    MTP_OBJECT_TREE* next;

    next = object->Sibling;
    while (next != NULL)
    {
        if (!next->Deleted)
        {
            /* 削除済みでなければ終わり */
            break;
        }
        next = next->Sibling;
    }
    return next;
}


/*----- MTPデバイスのオブジェクトツリーから指定の名を持つ子オブジェクトを検索する ------
*
*   Parameter
*       PCWSTR name : 子オブジェクトの名前
*       MTP_OBJECT_TREE* parent : 親オブジェクト
*
*   Return Value
*       MTP_OBJECT_TREE* 見つかったオブジェクト（NULL=見つからなかった）
*----------------------------------------------------------------------------*/
MTP_OBJECT_TREE* FindSpecifiedChildObjectFromTree(PCWSTR name, MTP_OBJECT_TREE* parent)
{
    MTP_OBJECT_TREE* child;

    child = parent->Child;
    while (child != NULL)
    {
        if (!child->Deleted && (wcscmp(name, child->Info.ObjectName) == 0))
        {
            /* 削除済みでなく名前が一致 */
            break;
        }
        /* 次の兄弟へうつる */
        child = child->Sibling;
    }
    return child;
}


/*----- MTPデバイスのオブジェクトツリーから指定のオブジェクトを削除する ------------
*
*   Parameter
*       MTP_OBJECT_TREE* object : 削除するオブジェクト
*       MTP_OBJECT_TREE* parent : 親オブジェクト
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int DeleteObjectFromTree(MTP_OBJECT_TREE* object, MTP_OBJECT_TREE* parent)
{
    int status = SUCCESS;
    MTP_OBJECT_TREE* prev = NULL;
    MTP_OBJECT_TREE* child = NULL;

    if (object->Deleted)
    {
        /* 削除しようとしているオブジェクトがすでに削除済みならダメ */
        SetLastError(ERROR_FILE_NOT_FOUND);
        status = FAIL;
    }
    else if (object->Child != NULL)
    {
        /* 削除しようとしているオブジェクトに削除済みでない子（子の兄弟を含む）があったらダメ */
        child = object->Child;
        while (child != NULL)
        {
            if (!child->Deleted)
            {
                SetLastError(ERROR_DIR_NOT_EMPTY);
                status = FAIL;
                break;
            }
            child = child->Sibling;
        }
    }

    if (status == SUCCESS)
    {
        status = FAIL;

        if (parent->Child == object)
        {
            /* 削除するオブジェクトは親の第一子 */
            status = SUCCESS;
        }
        else
        {
            /* 兄弟の中から削除するオブジェクトを探す */
            prev = parent->Child;
            while (prev != NULL)
            {
                if (prev->Sibling == object)
                {
                    status = SUCCESS;
                    break;
                }
                else
                {
                    prev = prev->Sibling;
                }
            }
        }

        /* 見つかったオブジェクトに削除マークを付ける */
        /* （実際に削除してしまうとFindNextFile_Myが正しく動作しなくなる） */
        if (status == SUCCESS)
        {
            object->Deleted = TRUE;
        }
    }
    return status;
}


/*----- MTPデバイスのオブジェクトツリーに指定のオブジェクトを追加する ----------------
*
*   Parameter
*       MTP_OBJECT_INFO* object : 追加するオブジェクト
*       MTP_OBJECT_TREE* parent : 親オブジェクト
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int AddObjectToTree(MTP_OBJECT_INFO* object, MTP_OBJECT_TREE* parent)
{
    int status = SUCCESS;
    MTP_OBJECT_TREE* prev = NULL;
    MTP_OBJECT_TREE* newObject = NULL;

    newObject = new MTP_OBJECT_TREE;
    newObject->Child = NULL;
    newObject->Sibling = NULL;
    newObject->ThisIsBackupRoot = FALSE;
    newObject->Deleted = FALSE;
    newObject->Info.ObjectID = new WCHAR[wcslen(object->ObjectID) + 1];
    newObject->Info.ObjectName = new WCHAR[wcslen(object->ObjectName) + 1];
    wcscpy(newObject->Info.ObjectID, object->ObjectID);
    wcscpy(newObject->Info.ObjectName, object->ObjectName);
    newObject->Info.ObjectType = object->ObjectType;
    newObject->Info.ObjectModifiedTime = object->ObjectModifiedTime;
    newObject->Info.ObjectSize = object->ObjectSize;

    if (parent->Child == NULL)
    {
        /* 追加するオブジェクトは親の第一子 */
        parent->Child = newObject;
    }
    else
    {
        /* 子の一番最後の兄弟として連結 */
        prev = parent->Child;
        while (prev->Sibling != NULL)
        {
            prev = prev->Sibling;
        }
        prev->Sibling = newObject;
    }
    return status;
}

