#pragma once
#include <Toshi/TString8.h>
#include <Toshi/TString16.h>
#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Formatting of the localised strings.
// Names come from JPOG (ALocale::FormatTags, ALocale::FormatValue, ALocale::FormatMoney).
//-----------------------------------------------------------------------------
class ALocale
{
public:
	// Maximum number of ':' separated tokens in a %{...} tag
	static constexpr TINT MAX_NUM_TAG_TOKENS = 4;

	// Prefix of the input tags, e.g. %c_DialogInputMap[OK]
	static const TWCHAR* ms_wszInputTag;

public:
	// Formats a value of a %1 or %{1:spec} tag ("$" - money, "#" - see FormatHashFloat)
	static void FormatValue( const PPropertyValue& a_rcValue, const Toshi::TString8& a_rcSpec, Toshi::TString16& a_rOutString );

	// Formats a number of money with the thousand separators
	static void FormatMoney( TINT a_iMoney, Toshi::TString16& a_rOutString, TBOOL a_bWithSign );

	// Formats a float that has a "#" spec
	static void FormatHashFloat( TFLOAT a_fValue, Toshi::TString16& a_rOutString );

	// Replaces %1-%9, %{N:spec} and %{N:true text:false text} tags with the values of the arguments
	static void FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString, TINT a_iNumArgs, const PPropertyValue* a_pArgs );
	static void FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString, const PPropertyValue& a_rcArg1 );
	static void FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString, const PPropertyValue& a_rcArg1, const PPropertyValue& a_rcArg2 );
	static void FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId );
	static void FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId, const PPropertyValue& a_rcArg1 );
	static void FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId, const PPropertyValue& a_rcArg1, const PPropertyValue& a_rcArg2 );

	// Replaces %c_InputMapName[CommandName] tags with the names of the buttons bound to the commands
	static void FormatInputTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString );
	static void FormatInputTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId );
};

//-----------------------------------------------------------------------------
// NOTE: an unidentified global (DAT_007817f4) next to the ALocale functions.
// Only its first value is ever set (to 0, by AApplication::OnCreate), and it's
// cleared by DestroyEngine, so it's always empty. Names are invented.
//-----------------------------------------------------------------------------
struct ALocaleUnknownData
{
	struct Entry
	{
		~Entry();

		TUINT8 PADDING[ 12 ];
		void*  pData; // 0x0c
	};

	ALocaleUnknownData();
	~ALocaleUnknownData();

	// Deletes the entries and releases the strings
	void Clear();

	TINT             m_iUnk1;         // 0x00
	Toshi::TPString8 m_aStrings[ 2 ]; // 0x04
	TINT             m_iUnk2;         // 0x0c
	TINT             m_iUnk3;         // 0x10
	TINT             m_iUnk4;         // 0x14
	Entry*           m_pEntries;      // 0x18
};

TSTATICASSERT( sizeof( ALocaleUnknownData::Entry ) == 0x10 );
TSTATICASSERT( sizeof( ALocaleUnknownData ) == 0x1c );

extern ALocaleUnknownData g_oLocaleUnknownData;

// Sets the first value of g_oLocaleUnknownData
TINT ALocaleUnknownData_SetUnk1( TINT a_iValue );
