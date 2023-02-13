/*===========================================================================
/
/                                   Backup
/                           MTPデバイスのファイル操作
/
/
/ WPD API sample code is comes from Microsoft.
/ Original sample code is located at:
/  https://learn.microsoft.com/en-us/samples/microsoft/windows-classic-samples/wpd-sample/
/ Copyright (c) Microsoft Corporation. All rights reserved.
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
#include <propvarutil.h>

#include "common.h"
#include "mtp_common.h"


/* プロトタイプ */
static HRESULT DeleteObjectFromMtpDeviceInner(IPortableDeviceContent* pContent, PWSTR objectId);
static HRESULT GetRequiredPropertiesForFolder(PCWSTR parentObjectID, PCWSTR folderName, IPortableDeviceValues** ppObjectProperties);
static HRESULT GetRequiredPropertiesForContentType(PWSTR parentObjectId, PWSTR destinationFileName, IStream* pFileStream, IPortableDeviceValues** ppObjectProperties, ULONGLONG* fileSize, FILETIME* modifiedTime);
static HRESULT StreamCopy(IStream* pDestStream, IStream* pSourceStream, DWORD cbTransferSize, ULONGLONG fileSize, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data);


/*----- MTPデバイスからオブジェクトを削除する ------------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR objectId : オブジェクトID
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int DeleteObjectFromMtpDevice(PWSTR deviceId, PWSTR objectId)
{
    int status = FAIL;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IPortableDeviceContent> pContent;
    HRESULT hr = S_OK;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* 削除する */
            hr = DeleteObjectFromMtpDeviceInner(pContent, objectId);
            if (SUCCEEDED(hr))
            {
                status = SUCCESS;
            }
            else
            {
                DoPrintf(_T("Error: Delete object failed, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get IPortableDeviceContent interface, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return status;
}


/*----- MTPデバイスからオブジェクトを削除する（内部サブルーチン） -------------------
*
*   Parameter
*       IPortableDeviceContent *pContent : IPortableDeviceContentインターフェース
*       PWSTR objectId : オブジェクトID
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
static HRESULT DeleteObjectFromMtpDeviceInner(IPortableDeviceContent* pContent, PWSTR objectId)
{
    CComPtr<IPortableDevicePropVariantCollection> pObjectsToDelete;
    HRESULT hr = S_OK;

    /* IPortableDevicePropVariantCollectionインターフェースを作成 */
    hr = CoCreateInstance(CLSID_PortableDevicePropVariantCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pObjectsToDelete));
    if (SUCCEEDED(hr))
    {
        if (pObjectsToDelete != NULL)
        {
            /* 削除するオブジェクトのIDをセット */
            PROPVARIANT pv = { 0 };
            PropVariantInit(&pv);
            pv.vt = VT_LPWSTR;
            pv.pwszVal = AtlAllocTaskWideString(objectId);
            if (pv.pwszVal != NULL)
            {
                hr = pObjectsToDelete->Add(&pv);
                if (SUCCEEDED(hr))
                {
                    /* 削除実行 */
                    hr = pContent->Delete(PORTABLE_DEVICE_DELETE_NO_RECURSION, pObjectsToDelete, NULL);
                    if (FAILED(hr))
                    {
                        DoPrintf(_T("Error: Failed to delete object from device, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to add object to IPortableDevicePropVariantCollection, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Out of memory\r\n"));
                hr = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
            }
            PropVariantClear(&pv);
        }
        else
        {
            hr = E_UNEXPECTED;
            DoPrintf(_T("Error: Failed to delete an object from the device because we were returned a NULL IPortableDevicePropVariantCollection interface pointer, hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to create IPortableDevicePropVariantCollection interface, hr = 0x%lx\r\n"), hr);
    }
    return hr;
}


/*----- MTPデバイスにフォルダを作成する -------------------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR parentObjectId : 親オブジェクトのオブジェクトID
*       PWSTR folderName : 作成するフォルダ名
*       PWSTR* objectId : 作成したフォルダ（オブジェクト）のIDを返す変数のポインタ（使い終わったら free() で削除すること）
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int CreateFolderOnMtpDevice(PWSTR deviceId, PWSTR parentObjectId, PWSTR folderName, PWSTR* objectId)
{
    int status = FAIL;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IPortableDeviceContent> pContent;
    CComPtr<IPortableDeviceValues> pFinalObjectProperties;
    HRESULT hr = S_OK;
    PWSTR newObjectId = NULL;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* MTPデバイスにフォルダを作成するためのIPortableDeviceValuesインターフェースを取得 */
            hr = GetRequiredPropertiesForFolder(parentObjectId, folderName, &pFinalObjectProperties);
            if (SUCCEEDED(hr))
            {
                /* オブジェクト（フォルダー）を作成 */
                hr = pContent->CreateObjectWithPropertiesOnly(pFinalObjectProperties, &newObjectId);
                if (SUCCEEDED(hr))
                {
                    if (newObjectId != NULL)
                    {
                        /* 作成したフォルダーのオブジェクトIDを返す */
                        *objectId = (PWSTR)malloc(sizeof(WCHAR) * (wcslen(newObjectId) + 1));
                        wcscpy(*objectId, newObjectId);
                        CoTaskMemFree(newObjectId);
                        newObjectId = NULL;
                        status = SUCCESS;
                    }
                    else
                    {
                        hr = E_UNEXPECTED;
                        DoPrintf(_T("Error: newObjectId is NULL, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to create a object, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: GetRequiredPropertiesForFolder returns failed, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get IPortableDeviceContent interface, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return status;
}


/*----- MTPデバイスにフォルダを作成するためのIPortableDeviceValuesインターフェースを返す --
*
*   Parameter
*       PWSTR parentObjectId : 親オブジェクトのオブジェクトID
*       PWSTR folderName : 作成するフォルダ名
*       IPortableDeviceValues** ppObjectProperties : IPortableDeviceValuesインターフェースを返す変数へのポインター
*
*   Return Value
*       HRESULT エラーコード
*----------------------------------------------------------------------------*/
static HRESULT GetRequiredPropertiesForFolder(PCWSTR parentObjectID, PCWSTR folderName, IPortableDeviceValues** ppObjectProperties)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;
    HRESULT hr;

    /* IPortableDeviceValuesインターフェースを作成 */
    hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pObjectProperties));
    if (SUCCEEDED(hr))
    {
        if (pObjectProperties != NULL)
        {
            /* 親オブジェクトのIDをセット */
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjectID);
            if (SUCCEEDED(hr))
            {
                /* 作成するフォルダの名前をセット */
                hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, folderName);
                if (SUCCEEDED(hr))
                {
                    /* フォルダーを作るので WPD_CONTENT_TYPE_FOLDER をセット */
                    hr = pObjectProperties->SetGuidValue(WPD_OBJECT_CONTENT_TYPE, WPD_CONTENT_TYPE_FOLDER);
                    if (SUCCEEDED(hr))
                    {
                        /* IPortableDeviceValuesインターフェースを返す */
                        hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
                        if (SUCCEEDED(hr))
                        {
                            /* nothing to do */
                        }
                        else
                        {
                            DoPrintf(_T("Error: Failed to QueryInterface for IPortableDeviceValues, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to set content type, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to set folder name, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to set parent object ID, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            DoPrintf(_T("Error: Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return hr;
}


/*----- MTPデバイスのオブジェクトのタイムスタンプ（変更時刻）を変更する -------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR objectId : オブジェクトID
*       FILETIME* time : タイムスタンプ
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int ChangeObjectTimeStampOnMtpDevice(PWSTR deviceId, PWSTR objectId, FILETIME* time)
{
    int status = FAIL;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IPortableDeviceProperties> pProperties;
    CComPtr<IPortableDeviceContent> pContent;
    CComPtr<IPortableDeviceValues> pObjectPropertiesToWrite;
    CComPtr<IPortableDeviceValues> pPropertyWriteResults;
    CComPtr<IPortableDeviceValues> pAttributes;
    BOOL bCanWrite = FALSE;
    PROPVARIANT propvar;
    HRESULT hr = S_OK;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* IPortableDevicePropertiesインターフェースを取得 */
            hr = pContent->Properties(&pProperties);
            if (SUCCEEDED(hr))
            {
                /* 変更時刻アトリビュートを取得 */
                hr = pProperties->GetPropertyAttributes(objectId, WPD_OBJECT_DATE_MODIFIED, &pAttributes);
                if (SUCCEEDED(hr))
                {
                    /* アトリビュートが変更可能か取得 */
                    hr = pAttributes->GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE, &bCanWrite);
                    if (SUCCEEDED(hr))
                    {
                        if (bCanWrite)
                        {
                            /* IPortableDeviceValuesインターフェースを作成 */
                            hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pObjectPropertiesToWrite));
                            if (SUCCEEDED(hr))
                            {
                                if (pObjectPropertiesToWrite != NULL)
                                {
                                    /* FILETIMEからPROPVARIANTを作成 */
                                    hr = InitPropVariantFromFileTime(time, &propvar);
                                    if (SUCCEEDED(hr))
                                    {
                                        /* 変更時刻を設定 */
                                        hr = pObjectPropertiesToWrite->SetValue(WPD_OBJECT_DATE_MODIFIED, &propvar);
                                        if (SUCCEEDED(hr))
                                        {
                                            /* 変更時刻をオブジェクトに書き込む */
                                            hr = pProperties->SetValues(objectId, pObjectPropertiesToWrite, &pPropertyWriteResults);
                                            if (SUCCEEDED(hr))
                                            {
                                                status = SUCCESS;
                                            }
                                            else
                                            {
                                                DoPrintf(_T("Error: Failed to SetValue for IPortableDeviceProperties, hr = 0x%lx\r\n"), hr);
                                            }
                                        }
                                        else
                                        {
                                            DoPrintf(_T("Error: Failed to SetValue for IPortableDeviceValues, hr = 0x%lx\r\n"), hr);
                                        }
                                        PropVariantClear(&propvar);
                                    }
                                    else
                                    {
                                        DoPrintf(_T("Error: InitPropVariantFromFileTime returns failed, hr = 0x%lx\r\n"), hr);
                                    }
                                }
                                else
                                {
                                    hr = E_UNEXPECTED;
                                    DoPrintf(_T("Error: Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\r\n"), hr);
                                }
                            }
                            else
                            {
                                DoPrintf(_T("Error: Failed to create IPortableDeviceValues interface, hr = 0x%lx\r\n"), hr);
                            }
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                            DoPrintf(_T("Error: Modified time cannot to be write, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE) returns failed, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: GetPropertyAttributes returns failed, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to get IPortableDeviceProperties interface, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get IPortableDeviceContent interface, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return status;
}


/*----- MTPデバイスにファイルを転送する -------------------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR parentObjectId : 親オブジェクトのオブジェクトID
*       PWSTR destinationFileName : 転送先ファイル名
*       PWSTR sourcePathName : 転送元パス名（フルパス）
*       PWSTR* objectId : 作成したフォルダ（オブジェクト）のIDを返す変数のポインタ（使い終わったら free() で削除すること）
*       ULONGLONG* fileSize : ファイルのサイズを返す変数へのポインタ
*       FILETIME* modifiedTime : ファイルのタイムスタンプを返す変数へのポインタ
*       COPY_PROGRESS_ROUTINE progressCallback : 進捗状況を知らせるコールバック関数へのポインタ
*       LPVOID data : コールバック関数に渡される引数
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int TransferFileToMtpDevice(PWSTR deviceId, PWSTR parentObjectId, PWSTR destinationFileName, PWSTR sourcePathName, PWSTR* objectId, ULONGLONG* fileSize, FILETIME* modifiedTime, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data)
{
    int status = FAIL;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IPortableDeviceContent> pContent;
    CComPtr<IStream> pFileStream;
    CComPtr<IStream> pTempStream;
    CComPtr<IPortableDeviceValues> pFinalObjectProperties;
    CComPtr<IPortableDeviceDataStream> pFinalObjectDataStream;
    DWORD cbOptimalTransferSize = 0;
    HRESULT hr = S_OK;
    PWSTR newObjectId = NULL;
    BOOL cancelled = FALSE;

    *objectId = NULL;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* コピー元ファイルをオープンする */
            hr = SHCreateStreamOnFile(sourcePathName, STGM_READ, &pFileStream);
            if (SUCCEEDED(hr))
            {
                /* 作成するオブジェクトのプロパティを作成する */
                hr = GetRequiredPropertiesForContentType(parentObjectId, destinationFileName, pFileStream, &pFinalObjectProperties, fileSize, modifiedTime);
                if (SUCCEEDED(hr))
                {
                    /* MTPデバイスにオブジェクトを作成する */
                    hr = pContent->CreateObjectWithPropertiesAndData(pFinalObjectProperties, &pTempStream, &cbOptimalTransferSize, NULL);
                    if (SUCCEEDED(hr))
                    {
                        /* 転送用データストリームを取得 */
                        hr = pTempStream->QueryInterface(IID_PPV_ARGS(&pFinalObjectDataStream));
                        if (SUCCEEDED(hr))
                        {
                            /* ストリームをコピー */
                            hr = StreamCopy(pFinalObjectDataStream, pFileStream, cbOptimalTransferSize, *fileSize, progressCallback, data);
                            if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
                            {
                                /* ユーザーによってキャンセルされた */
                                cancelled = TRUE;
                                hr = S_OK;
                            }
                            if (SUCCEEDED(hr))
                            {
                                /* デバイスへコミット */
                                hr = pFinalObjectDataStream->Commit(STGC_DEFAULT);
                                if (SUCCEEDED(hr))
                                {
                                    hr = pFinalObjectDataStream->GetObjectID(&newObjectId);
                                    if (SUCCEEDED(hr))
                                    {
                                        if (!cancelled)
                                        {
                                            /* 作成したフォルダーのオブジェクトIDを返す */
                                            *objectId = (PWSTR)malloc(sizeof(WCHAR) * (wcslen(newObjectId) + 1));
                                            wcscpy(*objectId, newObjectId);
                                            status = SUCCESS;
                                        }
                                        else
                                        {
                                            /*
                                            * ユーザーによってキャンセルされた
                                            *
                                            * StreamCopyの処理途中（ストリームが全部コピーされていない状態）でキャンセルすると
                                            * ここまで来ずに上の IStream::Commit() が hr=0x800704c7(ERROR_CANCELLED) でエラーになる。
                                            * ストリームを全てコピーした後でキャンセルするとここまで来る。
                                            */

                                            /* ユーザーによってキャンセルされたのでファイルを削除 */
                                            hr = DeleteObjectFromMtpDeviceInner(pContent, newObjectId);
                                            if (SUCCEEDED(hr))
                                            {
                                                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                                            }
                                        }
                                        CoTaskMemFree(newObjectId);
                                        newObjectId = NULL;
                                    }
                                    else
                                    {
                                        DoPrintf(_T("Error: Failed to get created object ID, hr = 0x%lx\r\n"), hr);
                                    }
                                }
                                else
                                {
                                    DoPrintf(_T("Error: Failed to commit object to device, hr = 0x%lx\r\n"), hr);
                                }
                            }
                            else
                            {
                                DoPrintf(_T("Error: Failed to transfer file to device, hr = 0x%lx\r\n"), hr);
                            }
                        }
                        else
                        {
                            DoPrintf(_T("Error: Failed to get stream to transfer, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to create new object on device, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: GetRequiredPropertiesForContentType returns failed, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to open file named (%s) to transfer to device, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get IPortableDeviceContent interface, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return status;
}


/*----- MTPデバイスにファイルを書き込むためのIPortableDeviceValuesインターフェースを返す --
*
*   Parameter
*       PWSTR parentObjectId : 親オブジェクトのオブジェクトID
*       PWSTR destinationFileName : ファイル名
*       IStream* pFileStream : 入力ファイルストリーム
*       IPortableDeviceValues** ppObjectProperties : IPortableDeviceValuesインターフェースを返す変数へのポインター
*       ULONGLONG* fileSize : ファイルのサイズを返す変数へのポインタ
*       FILETIME* modifiedTime : ファイルのタイムスタンプを返す変数へのポインタ
*
*   Return Value
*       HRESULT エラーコード
*----------------------------------------------------------------------------*/
static HRESULT GetRequiredPropertiesForContentType(PWSTR parentObjectId, PWSTR destinationFileName, IStream* pFileStream, IPortableDeviceValues** ppObjectProperties, ULONGLONG* fileSize, FILETIME* modifiedTime)
{
    CComPtr<IPortableDeviceValues> pObjectProperties;
    PROPVARIANT propvar;
    HRESULT hr;
    FILETIME localTime;

    /* IPortableDeviceValuesインターフェースを作成 */
    hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pObjectProperties));
    if (SUCCEEDED(hr))
    {
        if (pObjectProperties != NULL)
        {
            /* 親オブジェクトのIDをセット */
            hr = pObjectProperties->SetStringValue(WPD_OBJECT_PARENT_ID, parentObjectId);
            if (SUCCEEDED(hr))
            {
                /* コピー元ファイルの情報を取得 */
                STATSTG statstg = { 0 };
                hr = pFileStream->Stat(&statstg, STATFLAG_NONAME);
                if (SUCCEEDED(hr))
                {
                    /* ファイルサイズをセット */
                    *fileSize = statstg.cbSize.QuadPart;
                    hr = pObjectProperties->SetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, statstg.cbSize.QuadPart);
                    if (SUCCEEDED(hr))
                    {
                        /* FILETIMEからPROPVARIANTを作成 */
                        FileTimeToLocalFileTime(&statstg.mtime, &localTime);
                        *modifiedTime = localTime;
                        hr = InitPropVariantFromFileTime(&localTime, &propvar);
                        if (SUCCEEDED(hr))
                        {
                            /* 変更時刻をセット */
                            hr = pObjectProperties->SetValue(WPD_OBJECT_DATE_MODIFIED, &propvar);
                            if (SUCCEEDED(hr))
                            {
                                /* オリジナルファイル名をセット */
                                hr = pObjectProperties->SetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, destinationFileName);
                                if (SUCCEEDED(hr))
                                {
                                    /* オブジェクト名をセット */
                                    hr = pObjectProperties->SetStringValue(WPD_OBJECT_NAME, destinationFileName);
                                    if (SUCCEEDED(hr))
                                    {
                                        /* IPortableDeviceValuesインターフェースを取得 */
                                        hr = pObjectProperties->QueryInterface(IID_PPV_ARGS(ppObjectProperties));
                                        if (FAILED(hr))
                                        {
                                            DoPrintf(_T("Error: Failed to get IPortableDeviceValues interface, hr = 0x%lx\r\n"), hr);
                                        }
                                    }
                                    else
                                    {
                                        DoPrintf(_T("Error: Failed to set WPD_OBJECT_NAME, hr = 0x%lx\r\n"), hr);
                                    }
                                }
                                else
                                {
                                    DoPrintf(_T("Error: Failed to set WPD_OBJECT_ORIGINAL_FILE_NAME, hr = 0x%lx\r\n"), hr);
                                }
                            }
                            else
                            {
                                DoPrintf(_T("Error: Failed to set WPD_OBJECT_DATE_MODIFIED, hr = 0x%lx\r\n"), hr);
                            }
                            PropVariantClear(&propvar);
                        }
                        else
                        {
                            DoPrintf(_T("Error: Failed to get PROPVARIANT from FILETIME, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to set WPD_OBJECT_SIZE, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to get file's total size, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to set WPD_OBJECT_PARENT_ID, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            hr = E_UNEXPECTED;
            DoPrintf(_T("Error: Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to create IPortableDeviceValues interface, hr = 0x%lx\r\n"), hr);
    }
    return hr;
}


/*----- 入力ストリームから出力ストリームへデータをコピー ---------------------------
*
*   Parameter
*       IStream* pDestStream :出力ストリーム
*       IStream* pSourceStream : 入力ストリーム
*       DWORD cbTransferSize : バッファサイズ
*       ULONGLONG fileSize : 入力ストリームのサイズ
*       COPY_PROGRESS_ROUTINE progressCallback : 進捗状況を知らせるコールバック関数へのポインタ
*       LPVOID data : コールバック関数に渡される引数
*
*   Return Value
*       HRESULT エラーコード
*----------------------------------------------------------------------------*/
static HRESULT StreamCopy(IStream* pDestStream, IStream* pSourceStream, DWORD cbTransferSize, ULONGLONG fileSize, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data)
{
    HRESULT hr = S_OK;

    BYTE* pObjectData = new BYTE[cbTransferSize];
    DWORD cbBytesRead = 0;
    DWORD cbBytesWritten = 0;
    LARGE_INTEGER TotalFileSize;
    LARGE_INTEGER TotalBytesTransferred;
    LARGE_INTEGER StreamSize;
    LARGE_INTEGER StreamBytesTransferred;
    DWORD dwStreamNumber;
    DWORD dwCallbackReason;
    HANDLE hSourceFile;
    HANDLE hDestinationFile;
    DWORD progressReturn = PROGRESS_CONTINUE;
    COPY_PROGRESS_ROUTINE callbackFunc = progressCallback;

    TotalFileSize.QuadPart = fileSize;
    TotalBytesTransferred.QuadPart = 0;
    StreamSize.QuadPart = 0;
    StreamBytesTransferred.QuadPart = 0;
    dwStreamNumber = 0;
    dwCallbackReason = CALLBACK_STREAM_SWITCH;  /* 最初 */
    hSourceFile = NULL;
    hDestinationFile = NULL;

    /* 進捗コールバック関数をコール */
    progressReturn = PROGRESS_CONTINUE;
    if (callbackFunc != NULL)
    {
        progressReturn = (*callbackFunc)(TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred, dwStreamNumber, dwCallbackReason, hSourceFile, hDestinationFile, data);
    }
    dwCallbackReason = CALLBACK_CHUNK_FINISHED;     /* 2回目以降 */

    if (progressReturn == PROGRESS_CONTINUE)
    {
        do
        {
            /* 入力ストリームから読み込む */
            hr = pSourceStream->Read(pObjectData, cbTransferSize, &cbBytesRead);
            if (SUCCEEDED(hr))
            {
                if (cbBytesRead > 0)
                {
                    /* 出力ストリームに書き込む */
                    hr = pDestStream->Write(pObjectData, cbBytesRead, &cbBytesWritten);
                    if (SUCCEEDED(hr))
                    {
                        TotalBytesTransferred.QuadPart += cbBytesWritten;

                        /* 進捗コールバック関数をコール */
                        progressReturn = PROGRESS_CONTINUE;
                        if (callbackFunc != NULL)
                        {
                            progressReturn = (*callbackFunc)(TotalFileSize, TotalBytesTransferred, StreamSize, StreamBytesTransferred, dwStreamNumber, dwCallbackReason, hSourceFile, hDestinationFile, data);
                        }
                        if (progressReturn == PROGRESS_CANCEL)
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
                            DoPrintf(_T("Warning: StreamCopy is cancelled, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to write object, hr = 0x%lx\r\n"), hr);
                    }
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to read object, hr = 0x%lx\r\n"), hr);
            }
        } while ((SUCCEEDED(hr)) && (cbBytesRead > 0));
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }

    delete[] pObjectData;
    pObjectData = NULL;

    return hr;
}


/*----- MTPデバイスのオブジェクトの名前を変更する --------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR objectId : オブジェクトID
*       PWSTR name : 変更後の名前
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int ChangeObjectNameOnMtpDevice(PWSTR deviceId, PWSTR objectId, PWSTR name)
{
    int status = FAIL;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IPortableDeviceProperties> pProperties;
    CComPtr<IPortableDeviceContent> pContent;
    CComPtr<IPortableDeviceValues> pObjectPropertiesToWrite;
    CComPtr<IPortableDeviceValues> pPropertyWriteResults;
    CComPtr<IPortableDeviceValues> pAttributes;
    BOOL bCanWrite = FALSE;
    HRESULT hr = S_OK;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* IPortableDevicePropertiesインターフェースを取得 */
            hr = pContent->Properties(&pProperties);
            if (SUCCEEDED(hr))
            {
                /* 名前アトリビュートを取得 */
                hr = pProperties->GetPropertyAttributes(objectId, WPD_OBJECT_NAME, &pAttributes);
                if (SUCCEEDED(hr))
                {
                    /* アトリビュートが変更可能か取得 */
                    hr = pAttributes->GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE, &bCanWrite);
                    if (SUCCEEDED(hr))
                    {
                        if (bCanWrite)
                        {
                            /* IPortableDeviceValuesインターフェースを作成 */
                            hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pObjectPropertiesToWrite));
                            if (SUCCEEDED(hr))
                            {
                                if (pObjectPropertiesToWrite != NULL)
                                {
                                    /* 変更する名前を設定 */
                                    hr = pObjectPropertiesToWrite->SetStringValue(WPD_OBJECT_NAME, name);
                                    if (SUCCEEDED(hr))
                                    {
                                        /* 名前をオブジェクトに書き込む */
                                        hr = pProperties->SetValues(objectId, pObjectPropertiesToWrite, &pPropertyWriteResults);
                                        if (SUCCEEDED(hr))
                                        {
                                            status = SUCCESS;
                                        }
                                        else
                                        {
                                            DoPrintf(_T("Error: Failed to SetValue for IPortableDeviceProperties, hr = 0x%lx\r\n"), hr);
                                        }
                                    }
                                    else
                                    {
                                        DoPrintf(_T("Error: Failed to SetValue for IPortableDeviceValues, hr = 0x%lx\r\n"), hr);
                                    }
                                }
                                else
                                {
                                    hr = E_UNEXPECTED;
                                    DoPrintf(_T("Error: Failed to create property information because we were returned a NULL IPortableDeviceValues interface pointer, hr = 0x%lx\r\n"), hr);
                                }
                            }
                            else
                            {
                                DoPrintf(_T("Error: Failed to create IPortableDeviceValues interface, hr = 0x%lx\r\n"), hr);
                            }
                        }
                        else
                        {
                            hr = HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
                            DoPrintf(_T("Error: Object name cannot to be write, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: GetBoolValue(WPD_PROPERTY_ATTRIBUTE_CAN_WRITE) returns failed, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: GetPropertyAttributes returns failed, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to get IPortableDeviceProperties interface, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get IPortableDeviceContent interface, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return status;
}


/*----- MTPデバイスからPCにファイルを転送する -----------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR objectId : オブジェクトID
*       PCWSTR destinationPathName : 転送先パス名（PC上）
*       COPY_PROGRESS_ROUTINE progressCallback : 進捗状況を知らせるコールバック関数へのポインタ
*       LPVOID data : コールバック関数に渡される引数
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int TransferFileFromMtpDevice(PWSTR deviceId, PWSTR objectId, PCWSTR destinationPathName, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data)
{
    int status = FAIL;
    HRESULT hr = S_OK;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IPortableDeviceContent> pContent;
    CComPtr<IPortableDeviceResources> pResources;
    CComPtr<IStream> pObjectDataStream;
    CComPtr<IStream> pFinalFileStream;
    DWORD cbOptimalTransferSize = 0;
    CComPtr<IPortableDeviceProperties> pProperties;
    CComPtr<IPortableDeviceKeyCollection> pPropertiesToRead;
    CComPtr<IPortableDeviceValues> pObjectProperties;
    ULONGLONG fileSize;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* IPortableDeviceResourcesインターフェースを取得 */
            hr = pContent->Transfer(&pResources);
            if (SUCCEEDED(hr))
            {
                /* 転送元のストリームとバッファサイズを取得 */
                hr = pResources->GetStream(objectId, WPD_RESOURCE_DEFAULT, STGM_READ, &cbOptimalTransferSize, &pObjectDataStream);
                if (SUCCEEDED(hr))
                {
                    /* IPortableDevicePropertiesインターフェースを取得 */
                    hr = pContent->Properties(&pProperties);
                    if (SUCCEEDED(hr))
                    {
                        /* IPortableDeviceKeyCollectionインターフェースを作成 */
                        hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pPropertiesToRead));
                        if (SUCCEEDED(hr))
                        {
                            if (pPropertiesToRead != NULL)
                            {
                                /* オブジェクトのサイズを取得するように指示 */
                                hr = pPropertiesToRead->Add(WPD_OBJECT_SIZE);
                                if (SUCCEEDED(hr))
                                {
                                    /* オブジェクトのプロパティを読み込む */
                                    hr = pProperties->GetValues(objectId, pPropertiesToRead, &pObjectProperties);
                                    if (SUCCEEDED(hr))
                                    {
                                        /* オブジェクトのサイズを取得 */
                                        hr = pObjectProperties->GetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, &fileSize);
                                        if (SUCCEEDED(hr))
                                        {
                                            /* 出力先ストリームを作成 */
                                            hr = SHCreateStreamOnFile(destinationPathName, STGM_CREATE | STGM_WRITE, &pFinalFileStream);
                                            if (SUCCEEDED(hr))
                                            {
                                                /* ストリームをコピー */
                                                hr = StreamCopy(pFinalFileStream, pObjectDataStream, cbOptimalTransferSize, fileSize, progressCallback, data);
                                                if (SUCCEEDED(hr))
                                                {
                                                    status = SUCCESS;
                                                }
                                                else
                                                {
                                                    DoPrintf(_T("Error: Failed to transfer file from device, hr = 0x%lx\r\n"), hr);
                                                }
                                            }
                                            else
                                            {
                                                DoPrintf(_T("Error: Failed to create a file named (%ws) to transfer object, hr = 0x%lx\r\n"), destinationPathName, hr);
                                            }
                                        }
                                        else
                                        {
                                            DoPrintf(_T("Error: Failed to get object size, hr = 0x%lx\r\n"), hr);
                                        }
                                    }
                                    else
                                    {
                                        DoPrintf(_T("Error: Failed to GetValue from IPortableDeviceProperties interface, hr = 0x%lx\r\n"), hr);
                                    }
                                }
                                else
                                {
                                    DoPrintf(_T("Error: Failed to add WPD_OBJECT_SIZE to IPortableDeviceKeyCollection infetface, hr = 0x%lx\r\n"), hr);
                                }
                            }
                            else
                            {
                                hr = E_UNEXPECTED;
                                DoPrintf(_T("Error: Failed to create property information because we were returned a NULL PortableDeviceKeyCollection interface pointer, hr = 0x%lx\r\n"), hr);
                            }
                        }
                        else
                        {
                            DoPrintf(_T("Error: Failed to create IPortableDeviceKeyCollection interface, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to get IPortableDeviceProperties interface, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to get IStream (representing object data on the device) from IPortableDeviceResources, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to get IPortableDeviceResources interface, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get IPortableDeviceContent interface, hr = 0x%lx\r\n"), hr);
        }
    }
    SetWin32LastError(hr);
    return status;
}

