/**
 * @file T2Locale.h
 * Localization system for the Toshi engine
 * 
 * This file provides localization functionality including:
 * - Language and region management
 * - String localization
 * - Culture-specific formatting
 * - Localization resource handling
 */

#pragma once
#include "Toshi/TSingleton.h"
#include "File/TTRB.h"

TOSHI_NAMESPACE_START

using T2LocalisedString = TWCHAR*;

class T2Locale : public Toshi::TSingleton<T2Locale>
{
public:
	using Lang = TINT32;

	struct LocaleStrings
	{
		TINT32             m_numstrings;
		T2LocalisedString* Strings;
	};

public:
	T2Locale( const T2Locale& )  = delete;
	T2Locale( const T2Locale&& ) = delete;

	virtual ~T2Locale();
	virtual const TCHAR* GetLanguageFilename( Lang lang ) = 0;

	void SetLanguage( Lang langid );
	Lang GetLanguage() const;

	TINT              GetNumStrings() const;
	T2LocalisedString GetString( TINT a_iNumString );

protected:
	T2Locale( TINT langCount, size_t bufferSize, void* buffer );

	static void* TRBAllocator( TTRB::AllocType alloctype, size_t size, short unk, size_t unk2, void* userData );
	static void  TRBDeallocator( TTRB::AllocType alloctype, void* ptr, short unk, size_t unk2, void* userData );

	void* TRBAlloc( size_t size )
	{
		void* ptr   = m_BufferPos;
		m_BufferPos = static_cast<TCHAR*>( ptr ) + size;
		return ptr;
	}

protected:
	TTRB           m_Locale;      // 0x04
	void*          m_pBuffer;     // 0x18
	void*          m_BufferPos;   // 0x1C
	LocaleStrings* m_StringTable; // 0x20
	Lang           m_LangId;      // 0x24
	TBOOL          m_bOwnsBuffer; // 0x28
};

TOSHI_NAMESPACE_END
