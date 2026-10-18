#pragma once
#include "Toshi/Typedefs.h"
#include "Mod/ModAPI.h"

MODAPI TBOOL TDebug_IsValidAddress( const void* a_pPtr );

MODAPI void TDebug_AcquireMutex();
MODAPI void TDebug_ReleaseMutex();

MODAPI void TDebug_FinalPrintf( const TCHAR* a_szFormat, ... );
MODAPI void TDebug_FinalVPrintf( const TCHAR* a_szFormat, va_list a_Args );

MODAPI void TDebug_FinalSPrintf( const TCHAR* a_szFormat, ... );
MODAPI void TDebug_FinalVSPrintf( const TCHAR* a_szFormat, va_list a_Args );

MODAPI void TDebug_OutputDebugString( const TCHAR* a_szString );
MODAPI void TDebug_BroadcastDebugString( const TCHAR* a_szString );

MODAPI TINT TDebug_AssertHandler( const TCHAR* a_szExpression, const TCHAR* a_szFileName, TUINT a_uiLineNumber, const TCHAR* a_szDescription = "TASSERT" );
