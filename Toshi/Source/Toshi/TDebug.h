#pragma once
#include "Toshi/Typedefs.h"

TBOOL TDebug_IsValidAddress( const void* a_pPtr );

void TDebug_AcquireMutex();
void TDebug_ReleaseMutex();

void TDebug_FinalPrintf( const TCHAR* a_szFormat, ... );
void TDebug_FinalVPrintf( const TCHAR* a_szFormat, va_list a_Args );

void TDebug_FinalSPrintf( const TCHAR* a_szFormat, ... );
void TDebug_FinalVSPrintf( const TCHAR* a_szFormat, va_list a_Args );

void TDebug_OutputDebugString( const TCHAR* a_szString );
void TDebug_BroadcastDebugString( const TCHAR* a_szString );

TINT TDebug_AssertHandler( const TCHAR* a_szExpression, const TCHAR* a_szFileName, TUINT a_uiLineNumber, const TCHAR* a_szDescription = "TASSERT" );
