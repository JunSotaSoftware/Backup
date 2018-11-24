/*===========================================================================
/
/									Backup
/								更新風鐸対応
/
/============================================================================
/ Copyright (C) 2003-2007 Sota. All rights reserved.
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
#include <stdarg.h>
#include <windowsx.h>
#include <commctrl.h>

#include "common.h"



#define MAIL_ADDRESS	_T("sota@mwa.biglobe.ne.jp")
#define SOFTWARE		_T("Backup")
#define MYNAME			_T("曽田")
#define MYURL			_T("http://www2.biglobe.ne.jp/~sota/")
#define INFOFILE		_T("http://www2.biglobe.ne.jp/~sota/product/update.inf")


/*----- 更新風鐸のローカル情報を書き込む --------------------------------------
*
*	Parameter
*		なし
*
*	Return Value
*		なし
*----------------------------------------------------------------------------*/

void SaveUpdateBellInfo(void)
{
	HKEY hKey1;
	HKEY hKey2;
	HKEY hKey3;
	DWORD Dispos;
	int Tmp;

	if(RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Upcheck"), 0, KEY_CREATE_SUB_KEY, &hKey1) == ERROR_SUCCESS)
	{
		if(RegCreateKeyEx(hKey1, MAIL_ADDRESS, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY|KEY_SET_VALUE, NULL, &hKey2, &Dispos) == ERROR_SUCCESS)
		{
			RegSetValueEx(hKey2, _T("Author"), 0, REG_SZ, (CONST BYTE *)MYNAME, (_tcslen(MYNAME)+1) * sizeof(_TCHAR));
			RegSetValueEx(hKey2, _T("InfoURL"), 0, REG_SZ, (CONST BYTE *)INFOFILE, (_tcslen(INFOFILE)+1) * sizeof(_TCHAR));
			RegSetValueEx(hKey2, _T("DisplayHost"), 0, REG_SZ, (CONST BYTE *)MYURL, (_tcslen(MYURL)+1) * sizeof(_TCHAR));

			if(RegCreateKeyEx(hKey2, SOFTWARE, 0, _T(""), REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY|KEY_SET_VALUE, NULL, &hKey3, &Dispos) == ERROR_SUCCESS)
			{
				Tmp = PROGRAM_VERSION_NUM;
				RegSetValueEx(hKey3, _T("Build"), 0, REG_DWORD, (CONST BYTE *)&Tmp, sizeof(Tmp));
				RegSetValueEx(hKey3, _T("DistinctName"), 0, REG_SZ, (CONST BYTE *)SOFTWARE, (_tcslen(SOFTWARE)+1) * sizeof(_TCHAR));

				RegCloseKey(hKey3);
			}
			RegCloseKey(hKey2);
		}
		RegCloseKey(hKey1);
	}
	return;
}


