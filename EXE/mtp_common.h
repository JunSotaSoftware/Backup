/*===========================================================================
/
/                                   Backup
/                           MTP処理用の共通ヘッダーファイル
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


/* 構造体 */

/*===== MTPデバイス情報 =====*/
typedef struct {
    PWSTR DeviceID;
    WCHAR* DeviceDescription;
} MTP_DEVICE_INFO;


/*===== MTPデバイスリスト =====*/
typedef struct {
    DWORD NumberOfDevices;
    MTP_DEVICE_INFO* Info;
} MTP_DEVICE_LIST;


/*===== MTPオブジェクトリスト =====*/
typedef struct _mtpobjectlist {
    MTP_OBJECT_INFO Info;
    struct _mtpobjectlist* Next;
} MTP_OBJECT_LIST;


/* プロトタイプ */

/* mtpsupport.cpp */
DWORD CountMtpDevices(void);
int EnumerateMtpDevices(MTP_DEVICE_LIST** deviceList);
void ReleaseMtpDevices(MTP_DEVICE_LIST* deviceList);
int OpenMtpDevice(PWSTR deviceId, IPortableDevice** ppIPortableDevice);
int EnumerateMtpObject(IPortableDevice* pIPortableDevice, PWSTR objectId, MTP_OBJECT_TYPE objectType, int sort, MTP_OBJECT_LIST** objectList);
void ReleaseMtpObject(MTP_OBJECT_LIST* folderList);
void SetWin32LastError(HRESULT hr);

/* mtpobjecttree.cpp */
int MakeMtpObjectTree(PWSTR url, MTP_OBJECT_TYPE objectType, MTP_OBJECT_TREE** top, MTP_MAKE_OBJECT_TREE_ERROR_INFO* ErrorInfo, MTP_TREE_PROCESSING_ROUTINE processingCallback);
void DispMtpObjectTree(MTP_OBJECT_TREE* top, int level);
void ReleaseMtpObjectTree(MTP_OBJECT_TREE* top);
MTP_OBJECT_TREE* FindObjectFromTree(PCWSTR url, MTP_OBJECT_TREE* treeTop, MTP_OBJECT_TREE** parent);
MTP_OBJECT_TREE* FindNextSiblingObjectFromTree(MTP_OBJECT_TREE* object);
MTP_OBJECT_TREE* FindSpecifiedChildObjectFromTree(PCWSTR name, MTP_OBJECT_TREE* parent);
int DeleteObjectFromTree(MTP_OBJECT_TREE* object, MTP_OBJECT_TREE* parent);
int AddObjectToTree(MTP_OBJECT_INFO* object, MTP_OBJECT_TREE* parent);

/* mtpfileoperation.cpp */
int DeleteObjectFromMtpDevice(IPortableDevice* pIPortableDevice, PWSTR objectId);
int CreateFolderOnMtpDevice(IPortableDevice* pIPortableDevice, PWSTR parentObjectId, PWSTR folderName, PWSTR* objectId);
int ChangeObjectTimeStampOnMtpDevice(IPortableDevice* pIPortableDevice, PWSTR objectId, FILETIME* time);
int TransferFileToMtpDevice(IPortableDevice* pIPortableDevice, PWSTR parentObjectId, PWSTR destinationFileName, PWSTR sourcePathName, PWSTR* objectId, ULONGLONG* fileSize, FILETIME* modifiedTime, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data);
int ChangeObjectNameOnMtpDevice(IPortableDevice* pIPortableDevice, PWSTR objectId, PWSTR name);
int TransferFileFromMtpDevice(IPortableDevice* pIPortableDevice, PWSTR objectId, PCWSTR destinationPathName, COPY_PROGRESS_ROUTINE progressCallback, LPVOID data);

