#pragma once
#include "Toshi/Typedefs.h"

TBOOL TDebug_IsValidAddress(const void* a_pPtr);

void TDebug_AcquireMutex();
void TDebug_ReleaseMutex();

void TDebug_FinalPrintf(const char* a_szFormat, ...);
void TDebug_FinalVPrintf(const char* a_szFormat, va_list a_Args);

void TDebug_FinalSPrintf(const char* a_szFormat, ...);
void TDebug_FinalVSPrintf(const char* a_szFormat, va_list a_Args);

void TDebug_OutputDebugString(const char* a_szString);
void TDebug_BroadcastDebugString(const char* a_szString);

TINT TDebug_AssertHandler(const char* a_szExpression, const char* a_szFileName, TUINT a_uiLineNumber, const char* a_szDescription = "TASSERT");
