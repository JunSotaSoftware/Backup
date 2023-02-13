/*===========================================================================
/
/                                   Backup
/                                   MTPサポート
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
static void GetClientInformation(IPortableDeviceValues** ppClientInformation);
static int ReadMtpContentsInfo(IPortableDeviceContent* pContent, PWSTR objectId, MTP_OBJECT_TYPE objectType, MTP_OBJECT_INFO* objectInfo);
static int ReadMtpContentsDateAndSize(IPortableDeviceValues* pObjectProperties, MTP_OBJECT_INFO* objectInfo);
static void InsertSort(MTP_OBJECT_LIST** head);
static void SortedInsert(MTP_OBJECT_LIST** head, MTP_OBJECT_LIST* newNode);


/* 定義 */
#define CLIENT_NAME         L"Backup Application"
#define CLIENT_MAJOR_VER    1
#define CLIENT_MINOR_VER    21
#define CLIENT_REVISION     0

#define MTP_MAX_OBJECT_INFO     10


/*----- MTPデバイスの数を取得する -----------------------------------------------
*
*   Parameter
*       なし
*
*   Return Value
*       DWORD MTPデバイスの数
*----------------------------------------------------------------------------*/
DWORD CountMtpDevices(void)
{
    CComPtr<IPortableDeviceManager> pPortableDeviceManager;
    DWORD numberOfDevices = 0;

    /* PortableDeviceManagerインスタンスを作成 */
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pPortableDeviceManager));
    if (SUCCEEDED(hr))
    {
        /* MTPデバイスの数を取得 */
        hr = pPortableDeviceManager->GetDevices(NULL, &numberOfDevices);
        if (FAILED(hr))
        {
            DoPrintf(_T("Error: Failed to get number of MTP devices. hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to create PortableDeviceManager instance. hr = 0x%lx\r\n"), hr);
    }
    SetWin32LastError(hr);
    return numberOfDevices;
}


/*----- MTPデバイスリスト情報を作成する -----------------------------------------
*
*   Parameter
*       MTP_DEVICE_LIST** deviceList : デバイスのリストを返す変数へのポインタ
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int EnumerateMtpDevices(MTP_DEVICE_LIST** deviceList)
{
    CComPtr<IPortableDeviceManager> pPortableDeviceManager;
    DWORD i = 0;
    int status = FAIL;
    DWORD cchDescription = 0;
    MTP_DEVICE_LIST* list = NULL;
    PWSTR* deviceIdList = NULL;

    list = new MTP_DEVICE_LIST;
    list->NumberOfDevices = 0;
    list->Info = NULL;

    /* PortableDeviceManagerインスタンスを作成 */
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceManager, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pPortableDeviceManager));
    if (SUCCEEDED(hr))
    {
        /* MTPデバイスの数を取得 */
        hr = pPortableDeviceManager->GetDevices(NULL, &list->NumberOfDevices);
        if (SUCCEEDED(hr))
        {
            DoPrintf(_T("%d Windows Portable Device(s) found on the system.\r\n"), list->NumberOfDevices);
            if (list->NumberOfDevices > 0)
            {
                list->Info = new MTP_DEVICE_INFO[list->NumberOfDevices];
                for (i = 0; i < list->NumberOfDevices; i++)
                {
                    list->Info[i].DeviceDescription = NULL;
                    list->Info[i].DeviceID = NULL;
                }
                deviceIdList = new PWSTR[list->NumberOfDevices];

                /* MTPデバイスのIDリストを取得 */
                hr = pPortableDeviceManager->GetDevices(deviceIdList, &list->NumberOfDevices);
                if (SUCCEEDED(hr))
                {
                    status = SUCCESS;
                    for (i = 0; i < list->NumberOfDevices; i++)
                    {
                        list->Info[i].DeviceID = deviceIdList[i];
                        list->Info[i].DeviceDescription = NULL;

                        /* MTPデバイスの説明の文字数を取得 */
                        cchDescription = 0;
                        hr = pPortableDeviceManager->GetDeviceDescription(deviceIdList[i], NULL, &cchDescription);
                        if (SUCCEEDED(hr) && (cchDescription > 0))
                        {
                            /* MTPデバイスの説明取得 */
                            list->Info[i].DeviceDescription = new WCHAR[cchDescription];
                            hr = pPortableDeviceManager->GetDeviceDescription(deviceIdList[i], list->Info[i].DeviceDescription, &cchDescription);
                            if (FAILED(hr))
                            {
                                DoPrintf(_T("Error: Failed to get MTP device description. hr = 0x%lx\r\n"), hr);
                                status = FAIL;
                                break;
                            }
                        }
                        else
                        {
                            DoPrintf(_T("Error: Failed to get the MTP device description. hr = 0x%lx\r\n"), hr);
                            status = FAIL;
                            break;
                        }
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to get the MTP device list. hr = 0x%lx\r\n"), hr);
                }
                delete[] deviceIdList;
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to get number of MTP devices. hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to create PortableDeviceManager instance. hr = 0x%lx\r\n"), hr);
    }
    SetWin32LastError(hr);
    *deviceList = list;
    return status;
}


/*----- MTPデバイスリスト情報を削除する ----------------------------------------
*
*   Parameter
*       MTP_DEVICE_LIST* deviceList : デバイスのリスト
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
void ReleaseMtpDevices(MTP_DEVICE_LIST* deviceList)
{
    if (deviceList != NULL)
    {
        for (DWORD i = 0; i < deviceList->NumberOfDevices; i++)
        {
            if (deviceList->Info[i].DeviceID != NULL)
            {
                CoTaskMemFree(deviceList->Info[i].DeviceID);
                deviceList->Info[i].DeviceID = NULL;
            }

            delete[] deviceList->Info[i].DeviceDescription;
            deviceList->Info[i].DeviceDescription = NULL;
        }

        delete[] deviceList->Info;
        deviceList->Info = NULL;

        delete[] deviceList;
        deviceList = NULL;
    }
}


/*----- MTPデバイスをオープンする -----------------------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       IPortableDevice** ppDevice : インターフェースを返す変数へのポインタ
*
*   Return Value
*       int ステータス
*----------------------------------------------------------------------------*/
int OpenMtpDevice(PWSTR deviceId, IPortableDevice** ppDevice)
{
    int status = FAIL;
    HRESULT hr;
    CComPtr<IPortableDeviceValues> pClientInformation;

    /* クライアント情報を作成する */
    GetClientInformation(&pClientInformation);

    hr = CoCreateInstance(CLSID_PortableDeviceFTM, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppDevice));
    if (SUCCEEDED(hr))
    {
        hr = (*ppDevice)->Open(deviceId, pClientInformation);
        if (SUCCEEDED(hr))
        {
            status = SUCCESS;
        }
        else
        {
            (*ppDevice)->Release();
            *ppDevice = NULL;
            DoPrintf(_T("Error: Failed to create PortableDeviceManager instance, hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to create PortableDeviceManager instance, hr = 0x%lx\r\n"), hr);
    }
    SetWin32LastError(hr);
    return status;
}


/*----- クライアント情報を作成する ------------------------------------------------
*
*   Parameter
*       IPortableDeviceValues** ppClientInformation : クライアント情報を返す変数へのポインタ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void GetClientInformation(IPortableDeviceValues** ppClientInformation)
{
    HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(ppClientInformation));
    if (SUCCEEDED(hr))
    {
        hr = (*ppClientInformation)->SetStringValue(WPD_CLIENT_NAME, CLIENT_NAME);
        if (SUCCEEDED(hr))
        {
            hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, CLIENT_MAJOR_VER);
            if (SUCCEEDED(hr))
            {
                hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, CLIENT_MINOR_VER);
                if (SUCCEEDED(hr))
                {
                    hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, CLIENT_REVISION);
                    if (SUCCEEDED(hr))
                    {
                        hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
                        if (FAILED(hr))
                        {
                            DoPrintf(_T("Error: Failed to set WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to set WPD_CLIENT_REVISION, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to set WPD_CLIENT_MINOR_VERSION, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to set WPD_CLIENT_MAJOR_VERSION, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to set WPD_CLIENT_NAME, hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to CoCreateInstance CLSID_PortableDeviceValues, hr = 0x%lx\r\n"), hr);
    }
    SetWin32LastError(hr);
    return;
}


/*----- MTPオブジェクトリスト情報を作成する --------------------------------------
*
*   Parameter
*       PWSTR deviceId : デバイスID
*       PWSTR objectId : 親オブジェクトのオブジェクトID
*       MTP_OBJECT_TYPE objectType : リストアップするオブジェクトのタイプ (ObjectTypeFolder/ObjectTypeFile/ObjectTypeBoth)
*       int sort : ソートするかどうか (YES/NO)
*       MTP_OBJECT_LIST** objectList : オブジェクトリストを返す変数へのポインタ
*
*   Return Value
*       int ステータス (SUCCESS/FAIL)
*           リストの個数が0の場合でもSUCCESSである。その場合objectListにはNULLが入る。
*----------------------------------------------------------------------------*/
int EnumerateMtpObject(PWSTR deviceId, PWSTR objectId, MTP_OBJECT_TYPE objectType, int sort, MTP_OBJECT_LIST** objectList)
{
    int status = FAIL;
    int statusTmp;
    DWORD cFetched = 0;
    PWSTR  szObjectIDArray[MTP_MAX_OBJECT_INFO] = { 0 };
    HRESULT hr = S_OK;
    DWORD i;
    MTP_OBJECT_LIST* objectListTop = NULL;
    MTP_OBJECT_LIST* objectListTmp = NULL;
    MTP_OBJECT_INFO info;
    CComPtr<IPortableDeviceContent> pContent;
    CComPtr<IPortableDevice> pIPortableDevice;
    CComPtr<IEnumPortableDeviceObjectIDs> pEnumObjectIDs;

    /* MTPデバイスをオープン */
    if (OpenMtpDevice(deviceId, &pIPortableDevice) == SUCCESS)
    {
        /* IPortableDeviceContentインターフェースを取得 */
        hr = pIPortableDevice->Content(&pContent);
        if (SUCCEEDED(hr))
        {
            /* IEnumPortableDeviceObjectIDsインターフェースをEnumObjectsをコールすることで取得 */
            hr = pContent->EnumObjects(0, objectId, NULL, &pEnumObjectIDs);
            if (SUCCEEDED(hr))
            {
                status = SUCCESS;
                /*
                * IEnumPortableDeviceObjectIDs->Next() は指定した数(MTP_MAX_FOLDER_INFO)個の情報を返した時は S_OK を、
                * 指定した数より少ない数の情報を返した時は S_FALSE を返す。
                * S_FALSE の時でも cFetched が1以上なら情報を返している。ただし、次は Next() を呼ばない。
                */
                while (hr == S_OK)
                {
                    /* 項目を列挙する */
                    cFetched = 0;
                    hr = pEnumObjectIDs->Next(MTP_MAX_OBJECT_INFO, szObjectIDArray, &cFetched);
                    if (SUCCEEDED(hr))  /* S_FALSE でも SUCCEEDED(hr) はTRUEとなる */
                    {
                        for (i = 0; i < cFetched; i++)
                        {
                            /* コンテンツ情報を作成して格納する */
                            statusTmp = ReadMtpContentsInfo(pContent, szObjectIDArray[i], objectType, &info);
                            if (statusTmp == SUCCESS)
                            {
                                objectListTmp = new MTP_OBJECT_LIST;
                                objectListTmp->Next = objectListTop;
                                objectListTop = objectListTmp;

                                objectListTmp->Info.ObjectID = info.ObjectID;
                                objectListTmp->Info.ObjectName = info.ObjectName;
                                objectListTmp->Info.ObjectType = info.ObjectType;
                                objectListTmp->Info.ObjectModifiedTime = info.ObjectModifiedTime;
                                objectListTmp->Info.ObjectSize = info.ObjectSize;
                            }
                            else if (statusTmp != SKIP)
                            {
                                status = FAIL;
                                hr = S_FALSE;
                                break;
                            }
                        }
                    }
                }

                if (status == SUCCESS)
                {
                    if (sort == YES)
                    {
                        InsertSort(&objectListTop);
                    }
                }
                else
                {
                    ReleaseMtpObject(objectListTop);
                    objectListTop = NULL;
                }
                *objectList = objectListTop;
            }
            else
            {
                DoPrintf(_T("Error: Failed to enumuration object, hr = 0x%lx\r\n"), hr);
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


/*----- MTPコンテンツ情報を取得し格納する ---------------------------------------
*
*   Parameter
*       IPortableDeviceContent* pContent : IPortableDeviceContentインターフェース
*       PWSTR objectId : コンテンツのID
*       MTP_OBJECT_TYPE objectType : オブジェクトのタイプ
*       MTP_FOLDER_INFO* objectInfo : 情報を格納する変数へのポインタ
*
*   Return Value
*       int ステータス (SUCCESS / FAIL / SKIP=タイプが違う)
*----------------------------------------------------------------------------*/
static int ReadMtpContentsInfo(IPortableDeviceContent* pContent, PWSTR objectId, MTP_OBJECT_TYPE objectType, MTP_OBJECT_INFO* objectInfo)
{
    int status = FAIL;
    PWSTR pszValue = NULL;
    GUID guidValue = GUID_NULL;
    HRESULT hr;
    CComPtr<IPortableDeviceProperties> pProperties;
    CComPtr<IPortableDeviceKeyCollection> pPropertiesToRead;
    CComPtr<IPortableDeviceValues> pObjectProperties;

    objectInfo->ObjectID = NULL;
    objectInfo->ObjectName = NULL;
    objectInfo->ObjectType = ObjectTypeNone;
    objectInfo->ObjectModifiedTime.dwLowDateTime = 0;
    objectInfo->ObjectModifiedTime.dwHighDateTime = 0;
    objectInfo->ObjectSize = 0;

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
                /* 名前を取得する */
                hr = pPropertiesToRead->Add(WPD_OBJECT_NAME);
                if (SUCCEEDED(hr))
                {
                    /* タイプを取得する */
                    hr = pPropertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
                    if (SUCCEEDED(hr))
                    {
                        /* タイムスタンプを取得する */
                        hr = pPropertiesToRead->Add(WPD_OBJECT_DATE_MODIFIED);
                        if (SUCCEEDED(hr))
                        {
                            /* サイズを取得する */
                            hr = pPropertiesToRead->Add(WPD_OBJECT_SIZE);
                            if (SUCCEEDED(hr))
                            {
                                /* コンテント情報を取得 */
                                hr = pProperties->GetValues(objectId, pPropertiesToRead, &pObjectProperties);
                                if (SUCCEEDED(hr))
                                {
                                    /* 名前を抽出 */
                                    hr = pObjectProperties->GetStringValue(WPD_OBJECT_NAME, &pszValue);
                                    if (SUCCEEDED(hr))
                                    {
                                        /* タイプを抽出 */
                                        hr = pObjectProperties->GetGuidValue(WPD_OBJECT_CONTENT_TYPE, &guidValue);
                                        if (SUCCEEDED(hr))
                                        {
                                            status = SKIP;
                                            GUID folderType = { 0x27E2E392, 0xA111, 0x48E0, 0xAB, 0x0C, 0xE1, 0x77, 0x05, 0xA0, 0x5F, 0x85 };
                                            GUID functionalType = { 0x99ED0160, 0x17FF, 0x4C44, 0x9D, 0x98, 0x1D, 0x7A, 0x6F, 0x94, 0x19, 0x21 };
                                            if (IsEqualGUID(guidValue, functionalType))
                                            {
                                                if ((objectType == ObjectTypeFolder) || (objectType == ObjectTypeBoth))
                                                {
                                                    objectInfo->ObjectID = objectId;
                                                    objectInfo->ObjectName = pszValue;
                                                    objectInfo->ObjectType = ObjectTypeFolder;
                                                    status = SUCCESS;
                                                }
                                            }
                                            else if (IsEqualGUID(guidValue, folderType))
                                            {
                                                if ((objectType == ObjectTypeFolder) || (objectType == ObjectTypeBoth))
                                                {
                                                    status = ReadMtpContentsDateAndSize(pObjectProperties, objectInfo);
                                                    if (status == SUCCESS)
                                                    {
                                                        objectInfo->ObjectID = objectId;
                                                        objectInfo->ObjectName = pszValue;
                                                        objectInfo->ObjectType = ObjectTypeFolder;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if ((objectType == ObjectTypeFile) || (objectType == ObjectTypeBoth))
                                                {
                                                    status = ReadMtpContentsDateAndSize(pObjectProperties, objectInfo);
                                                    if (status == SUCCESS)
                                                    {
                                                        objectInfo->ObjectID = objectId;
                                                        objectInfo->ObjectName = pszValue;
                                                        objectInfo->ObjectType = ObjectTypeFile;
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            DoPrintf(_T("Error: Failed to GetGuidValue, hr = 0x%lx\r\n"), hr);
                                        }
                                    }
                                    else
                                    {
                                        DoPrintf(_T("Error: Failed to GetStringValue, hr = 0x%lx\r\n"), hr);
                                    }
                                }
                                else
                                {
                                    DoPrintf(_T("Error: Failed to GetValue, hr = 0x%lx\r\n"), hr);
                                }
                            }
                            else
                            {
                                DoPrintf(_T("Error: Failed to add WPD_OBJECT_SIZE to IPortableDeviceKeyCollection, hr = 0x%lx\r\n"), hr);
                            }
                        }
                        else
                        {
                            DoPrintf(_T("Error: Failed to add WPD_OBJECT_DATE_MODIFIED to IPortableDeviceKeyCollection, hr = 0x%lx\r\n"), hr);
                        }
                    }
                    else
                    {
                        DoPrintf(_T("Error: Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr = 0x%lx\r\n"), hr);
                    }
                }
                else
                {
                    DoPrintf(_T("Error: Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr = 0x%lx\r\n"), hr);
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
    SetWin32LastError(hr);

    return status;
}


/*----- MTPコンテンツ情報のうちタイムスタンプとサイズを取得して格納する --------------
*
*   Parameter
*       IPortableDeviceValues *pObjectProperties : IPortableDeviceValuesインターフェース
*       MTP_FOLDER_INFO* objectInfo : 情報を格納する変数へのポインタ
*
*   Return Value
*       int ステータス (SUCCESS / FAIL)
*----------------------------------------------------------------------------*/
static int ReadMtpContentsDateAndSize(IPortableDeviceValues *pObjectProperties, MTP_OBJECT_INFO* objectInfo)
{
    PROPVARIANT propVariant;
    VARIANT variant;
    FILETIME filetime;
    ULONGLONG size;
    HRESULT hr;
    int status = FAIL;

    /* タイムスタンプを抽出 */
    PropVariantInit(&propVariant);
    hr = pObjectProperties->GetValue(WPD_OBJECT_DATE_MODIFIED, &propVariant);
    if (SUCCEEDED(hr))
    {
        hr = PropVariantToVariant(&propVariant, &variant);
        if (SUCCEEDED(hr))
        {
            hr = VariantToFileTime(variant, PSTF_UTC, &filetime);
            if (SUCCEEDED(hr))
            {
                /* サイズを抽出 */
                hr = pObjectProperties->GetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, &size);
                if (SUCCEEDED(hr))
                {
                    objectInfo->ObjectModifiedTime.dwLowDateTime = filetime.dwLowDateTime;
                    objectInfo->ObjectModifiedTime.dwHighDateTime = filetime.dwHighDateTime;
                    objectInfo->ObjectSize = size;
                    status = SUCCESS;
                }
                else
                {
                    DoPrintf(_T("Error: Failed to GetUnsignedLargeIntegerValue, hr = 0x%lx\r\n"), hr);
                }
            }
            else
            {
                DoPrintf(_T("Error: Failed to VariantToFileTime, hr = 0x%lx\r\n"), hr);
            }
        }
        else
        {
            DoPrintf(_T("Error: Failed to PropVariantToVariant, hr = 0x%lx\r\n"), hr);
        }
    }
    else
    {
        DoPrintf(_T("Error: Failed to GetValue, hr = 0x%lx\r\n"), hr);
    }
    PropVariantClear(&propVariant);
    SetWin32LastError(hr);

    return status;
}


/*----- MTPオブジェクトリスト情報を削除する --------------------------------------
*
*   Parameter
*       MTP_OBJECT_LIST* objectList : オブジェクトのリスト
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
void ReleaseMtpObject(MTP_OBJECT_LIST* objectList)
{
    MTP_OBJECT_LIST* objectListTmp;

    while (objectList != NULL)
    {
        if (objectList->Info.ObjectID != NULL)
        {
            CoTaskMemFree(objectList->Info.ObjectID);
            objectList->Info.ObjectID = NULL;
        }

        if (objectList->Info.ObjectName != NULL)
        {
            CoTaskMemFree(objectList->Info.ObjectName);
            objectList->Info.ObjectName = NULL;
        }
        objectListTmp = objectList->Next;

        delete objectList;
        objectList = objectListTmp;
    }
}


/*----- オブジェクトリストのリンクリストをソート -----------------------------------
*
*   Parameter
*       MTP_OBJECT_LIST** head : リンクの先頭を指し示す変数へのポインタ
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void InsertSort(MTP_OBJECT_LIST** head)
{
    MTP_OBJECT_LIST* result = NULL;     //ここで答えを作成します
    MTP_OBJECT_LIST* current = *head;   //元のリストを繰り返し処理します
    MTP_OBJECT_LIST* next;

    while (current != NULL)
    {
        next = current->Next;
        SortedInsert(&result, current);
        current = next;
    }

    *head = result;
}


/*----- 指定されたノードを正しいソート位置で指定されたものに挿入する -----------------
*
*   Parameter
*       MTP_OBJECT_LIST** head : リンクの先頭を指し示す変数へのポインタ
*       MTP_OBJECT_LIST* newNode : ノード
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
static void SortedInsert(MTP_OBJECT_LIST** head, MTP_OBJECT_LIST* newNode)
{
    MTP_OBJECT_LIST dummy;
    MTP_OBJECT_LIST* current = &dummy;

    dummy.Next = *head;

    while ((current->Next != NULL) && wcscmp(current->Next->Info.ObjectName, newNode->Info.ObjectName) < 0)
    {
        current = current->Next;
    }

    newNode->Next = current->Next;
    current->Next = newNode;
    *head = dummy.Next;
}


/*----- URLがMTPデバイスを差しているかチェック ------------------------------------
*
*   Parameter
*       PCWSTR url : バックアップ先のURL
*
*   Return Value
*       int ステータス (YES/NO)
*
* Note
*       URLの区切り文字は / と \\ を受け付ける
*       transfer.c : GoMakeDir() の都合で mtp:// ではなく mtp:/ でチェックする
*----------------------------------------------------------------------------*/
int IsMtpDevice(PCWSTR url)
{
    int status = NO;
    if ((wcsncmp(url, L"mtp:/", 5) == 0) || (wcsncmp(url, L"mtp:\\", 5) == 0))
    {
        status = YES;
    }
    return status;
}


/*----- HRESULTからWIN32エラーコードを取得しSetLastErrorでセット ------------------
*
*   Parameter
*       HRESULT hr : HRESULTコード
*
*   Return Value
*       なし
*----------------------------------------------------------------------------*/
void SetWin32LastError(HRESULT hr)
{
    if (FAILED(hr))
    {
        if ((hr & 0xFFFF0000) == MAKE_HRESULT(SEVERITY_ERROR, FACILITY_WIN32, 0))
        {
            DWORD dwError = HRESULT_CODE(hr);
            SetLastError(dwError);
            DoPrintf(_T("Error: SetLastError hr = 0x%lx, Win32Error = 0x%lx\r\n"), hr, dwError);
        }
    }
}

