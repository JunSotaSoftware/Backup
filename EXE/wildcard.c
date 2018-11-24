/*===========================================================================
/
/									Backup
/							ワイルドカードの処理
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
#include <direct.h>
#include <windowsx.h>
#include <commctrl.h>

#include "common.h"
#include "resource.h"


/*===== プロトタイプ =====*/

static int CheckNameMatch(LPTSTR str, LPTSTR regexp);



/*----- ワイルドカードにマッチするかどうかを返す ------------------------------
*
*	Parameter
*		LPTSTR str : 文字列
*		LPTSTR Array : ワイルドカード検索式（マルチ文字列）
*
*	Return Value
*		int ステータス
*			YES/NO=ワイルドカードに合わなかった
*----------------------------------------------------------------------------*/

int CheckFnameWithArray(LPTSTR Fname, LPTSTR Array)
{
	int Sts;
	_TCHAR Tmp[MY_MAX_PATH+1+2];

	Sts = NO;
	while(*Array != NUL)
	{
		/* パス＋名前の形でないものは _T("*\") を前に付ける */
		_tcscpy(Tmp, _T(""));
		if(_tcschr(Array, _T('\\')) == NULL)
			_tcscat(Tmp, _T("*\\"));
		_tcscat(Tmp, Array);

		if((Sts = CheckFname(Fname, Tmp)) == YES)
			break;

		Array += _tcslen(Array) + 1;
	}
	return(Sts);
}


/*----- ワイルドカードにマッチするかどうかを返す ------------------------------
*
*	Parameter
*		LPTSTR str : 文字列
*		LPTSTR regexp : ワイルドカード検索式
*
*	Return Value
*		int ステータス
*			YES/NO=ワイルドカードに合わなかった
*----------------------------------------------------------------------------*/

int CheckFname(LPTSTR str, LPTSTR regexp)
{
	int Sts;
	_TCHAR p1[MY_MAX_PATH+1+2];
	_TCHAR p2[MY_MAX_PATH+1];
	LPTSTR p;

	_tcscpy(p1, regexp);
	_tcscpy(p2, str);

//	if(_tcschr(p2, _T('.')) == NULL)
//		_tcscat(p2,_T("."));

	/* *? とか ** とかを削除 */
	for(p = p1; *p != NUL; p++)
	{
		while((*p == _T('*')) && ((*(p+1) == _T('?')) || (*(p+1) == _T('*'))))
			_tcscpy(p+1, p+2);
	}

	Sts = YES;
	if(_tcscmp(p1, _T("*")) != 0)
	{
		Sts = CheckNameMatch(p2, p1);
	}
	return(Sts);
}


/*----- ワイルドカード検索サブルーチン ----------------------------------------
*
*	Parameter
*		LPTSTR str : 文字列
*		LPTSTR regexp : ワイルドカード検索式
*
*	Return Value
*		int ステータス
*			YES/NO=ワイルドカードに合わなかった
*----------------------------------------------------------------------------*/

static int CheckNameMatch(LPTSTR str, LPTSTR regexp)
{
	LPTSTR p;

	for(p = regexp; (*p != NUL) && (*str != NUL); )
	{
		switch(*p)
		{
			case _T('?'):
				str++;
				p++;
				break;

			case _T('*'):
				/* Look for a character matching the one after the _T('*') */
				p++;
				if(*p == NUL)
					return YES; /* Automatic match */
				while(*str != NUL)
				{
					while((*str != NUL) && (toupper(*p)!=toupper(*str)))
						str++;
					if(CheckNameMatch(str, p))
						return YES;
					if(*str == NUL)
						return NO;
					else
						str++;
				}
				return NO;

			default:
				if(toupper(*str) != toupper(*p))
					return NO;
				str++;
				p++;
				break;
		}
	}

	if((*p == NUL) && (*str == NUL))
		return YES;

	if ((*p != NUL) && (str[0] == _T('.')) && (str[1] == 0))
		return(YES);
  
	if ((*str == NUL) && (*p == _T('?')))
	{
		while (*p == _T('?'))
			p++;
		return(*p == NUL);
	}

	if((*str == NUL) && (*p == _T('*')) && (p[1] == _T('\0')))
		return YES;

	return NO;
}


