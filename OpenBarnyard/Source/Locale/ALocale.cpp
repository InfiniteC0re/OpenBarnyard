#include "pch.h"
#include "ALocale.h"
#include "ALocaleManager.h"
#include "Input/AInputMapManager.h"

#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

#include <ctype.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TWCHAR* ALocale::ms_wszInputTag = L"%c_";

// DAT_007817f4
ALocaleUnknownData g_oLocaleUnknownData;

// $Barnyard: FUNCTION 0042b320
TINT ALocaleUnknownData_SetUnk1( TINT a_iValue )
{
	g_oLocaleUnknownData.m_iUnk1 = a_iValue;
	return a_iValue;
}

// $Barnyard: FUNCTION 0042c110
ALocaleUnknownData::ALocaleUnknownData()
{
	m_iUnk1    = 0;
	m_iUnk3    = 0;
	m_iUnk2    = 10;
	m_iUnk4    = 0;
	m_pEntries = TNULL;
}

// $Barnyard: FUNCTION 0042c180
ALocaleUnknownData::~ALocaleUnknownData()
{
	if ( m_pEntries )
		delete[] m_pEntries;
}

// $Barnyard: FUNCTION 0042c200
void ALocaleUnknownData::Clear()
{
	if ( m_pEntries )
		delete[] m_pEntries;

	m_pEntries = TNULL;
	m_iUnk4    = 0;
	m_iUnk3    = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_aStrings ); i++ )
		m_aStrings[ i ] = TPString8();
}

// NOTE: shared with other element types (no marker), 005a2160 = TGenericArray::~TGenericArray
ALocaleUnknownData::Entry::~Entry()
{
	if ( pData )
		TFree( pData );
}

// $Barnyard: FUNCTION 0042b330
void ALocale::FormatHashFloat( TFLOAT a_fValue, TString16& a_rOutString )
{
	if ( TMath::Abs( a_fValue ) < TMath::TFLOAT_EPSILON )
		a_rOutString.Format( g_pLocaleManager->GetString( 1130 ), 0 );
	else
		a_rOutString = g_pLocaleManager->GetString( 1135 );
}

// $Barnyard: FUNCTION 0042b390
void ALocale::FormatMoney( TINT a_iMoney, TString16& a_rOutString, TBOOL a_bWithSign )
{
	TINT          iMoney = a_iMoney;
	const TWCHAR* wszSign;

	if ( iMoney >= 0 )
	{
		wszSign = g_pLocaleManager->GetString( 1139 );
	}
	else
	{
		iMoney  = -iMoney;
		wszSign = g_pLocaleManager->GetString( 1138 );
	}

	const TWCHAR* wszSeparator = g_pLocaleManager->GetString( 1140 );

	TINT iBillions  = iMoney / 1000000000 % 1000000;
	TINT iMillions  = iMoney / 1000000 % 1000;
	TINT iThousands = iMoney / 1000 % 1000;
	TINT iUnits     = iMoney % 1000;

	if ( iMoney < 1000 )
	{
		if ( a_bWithSign )
			a_rOutString.Format( L"%s%d", wszSign, iUnits );
		else
			a_rOutString.Format( L"%d", iUnits );
	}
	else if ( iMoney < 1000000 )
	{
		if ( a_bWithSign )
			a_rOutString.Format( L"%s%d%s%03d", wszSign, iThousands, wszSeparator, iUnits );
		else
			a_rOutString.Format( L"%d%s%03d", iThousands, wszSeparator, iUnits );
	}
	else if ( iMoney < 1000000000 )
	{
		if ( a_bWithSign )
			a_rOutString.Format( L"%s%d%s%03d%s%03d", wszSign, iMillions, wszSeparator, iThousands, wszSeparator, iUnits );
		else
			a_rOutString.Format( L"%d%s%03d%s%03d", iMillions, wszSeparator, iThousands, wszSeparator, iUnits );
	}
	else
	{
		if ( a_bWithSign )
			a_rOutString.Format( L"%s%d%s%03d%s%03d%s%03d", wszSign, iBillions, wszSeparator, iMillions, wszSeparator, iThousands, wszSeparator, iUnits );
		else
			a_rOutString.Format( L"%d%s%03d%s%03d%s%03d", iBillions, wszSeparator, iMillions, wszSeparator, iThousands, wszSeparator, iUnits );
	}
}

// $Barnyard: FUNCTION 0042b570
void ALocale::FormatValue( const PPropertyValue& a_rcValue, const TString8& a_rcSpec, TString16& a_rOutString )
{
	const TClass* pType = a_rcValue.GetType();

	if ( pType == PPropertyValue::TYPE_INT || pType == PPropertyValue::TYPE_UINT32 )
	{
		if ( a_rcSpec == "$" )
			FormatMoney( a_rcValue.GetInteger(), a_rOutString, TTRUE );
		else if ( a_rcSpec == "#" )
			a_rOutString = L"*ERROR*";
		else
			a_rOutString.Format( L"%d", a_rcValue.GetInteger() );
	}
	else if ( pType == PPropertyValue::TYPE_FLOAT )
	{
		if ( a_rcSpec == "#" )
			FormatHashFloat( a_rcValue.GetFloat(), a_rOutString );
		else if ( a_rcSpec == "$" )
			FormatMoney( TINT( a_rcValue.GetFloat() ), a_rOutString, TTRUE );
		else
			a_rOutString.Format( L"%f", a_rcValue.GetFloat() );
	}
	else if ( pType == PPropertyValue::TYPE_STRING16 )
	{
		a_rOutString.Copy( a_rcValue.GetString16(), -1 );
	}
	else
	{
		a_rOutString = L"*ERROR*";
	}
}

// $Barnyard: FUNCTION 0042b6b0
void ALocale::FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString, TINT a_iNumArgs, const PPropertyValue* a_pArgs )
{
	// NOTE: a_iNumArgs is not used by the original, the indices of the tags aren't validated
	TString16 strString( a_wszString );
	TString16 strResult;
	TString16 strValue;
	TString16 astrTokens[ MAX_NUM_TAG_TOKENS ];
	TString8  astrTokens8[ MAX_NUM_TAG_TOKENS ];

	TINT iPos = 0;

	for ( TINT iTagPos = strString.Find( L'%', 0 ); iTagPos != -1; iTagPos = strString.Find( L'%', iPos ) )
	{
		// Copy the text that precedes the tag
		if ( iTagPos > iPos )
			strResult.Concat( strString.GetString() + iPos, iTagPos - iPos );

		TWCHAR wcNext = strString.GetString()[ iTagPos + 1 ];

		if ( wcNext == L'{' )
		{
			// %{token0:token1:...}
			TINT iOpen  = iTagPos + 1;
			TINT iClose = strString.Find( L'}', iOpen );

			TINT aiSeparators[ MAX_NUM_TAG_TOKENS + 1 ];
			TINT iNumTokens   = 0;
			aiSeparators[ 0 ] = iOpen;

			do
			{
				TASSERT( iNumTokens < MAX_NUM_TAG_TOKENS );

				TINT iStart = aiSeparators[ iNumTokens ];
				TINT iColon = strString.Find( L':', iStart + 1 );

				aiSeparators[ iNumTokens + 1 ] = ( iColon == -1 ) ? iClose : TMath::Min( iClose, iColon );

				TINT iLength = aiSeparators[ iNumTokens + 1 ] - iStart - 1;
				astrTokens[ iNumTokens ].Copy( strString.GetString() + iStart + 1, iLength );
				astrTokens8[ iNumTokens ].Copy( strString.GetString() + iStart + 1, iLength );

				iNumTokens++;
			} while ( aiSeparators[ iNumTokens ] != iClose );

			if ( astrTokens8[ 0 ].Compare( "command", -1 ) != 0 && astrTokens8[ 0 ].Length() == 1 && isdigit( astrTokens8[ 0 ].GetString()[ 0 ] ) )
			{
				TINT iArg = astrTokens8[ 0 ].GetString()[ 0 ] - '1';

				if ( iNumTokens == 2 )
				{
					// %{N:spec}
					FormatValue( a_pArgs[ iArg ], astrTokens8[ 1 ], strValue );
					strResult.Concat( strValue, -1 );
				}
				else if ( iNumTokens == 3 )
				{
					// %{N:text if 1:text otherwise}
					if ( a_pArgs[ iArg ].GetInteger() == 1 )
						strResult.Concat( astrTokens[ 1 ], -1 );
					else
						strResult.Concat( astrTokens[ 2 ], -1 );
				}
			}

			iPos = iClose + 1;
		}
		else if ( isdigit( wcNext ) )
		{
			// %N
			FormatValue( a_pArgs[ wcNext - L'1' ], TString8(), strValue );
			strResult.Concat( strValue, -1 );
			iPos = iTagPos + 2;
		}
		else
		{
			// %% or a single %
			strResult.Concat( L"%", -1 );
			iPos = ( wcNext == L'%' ) ? iTagPos + 2 : iTagPos + 1;
		}
	}

	// Copy the rest of the string
	if ( iPos < strString.Length() )
		strResult.Concat( strString.GetString() + iPos, strString.Length() - iPos );

	TStringManager::String16Copy( a_wszOutString, strResult.GetString(), a_iOutSize );
}

// $Barnyard: FUNCTION 0042bac0
void ALocale::FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString, const PPropertyValue& a_rcArg1 )
{
	PPropertyValue aArgs[ 1 ];
	aArgs[ 0 ] = a_rcArg1;

	FormatTags( a_wszOutString, a_iOutSize, a_wszString, 1, aArgs );
}

// $Barnyard: FUNCTION 0042bb50
void ALocale::FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString, const PPropertyValue& a_rcArg1, const PPropertyValue& a_rcArg2 )
{
	PPropertyValue aArgs[ 2 ];
	aArgs[ 0 ] = a_rcArg1;
	aArgs[ 1 ] = a_rcArg2;

	FormatTags( a_wszOutString, a_iOutSize, a_wszString, 2, aArgs );
}

// $Barnyard: FUNCTION 0042bbf0
void ALocale::FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId )
{
	FormatTags( a_wszOutString, a_iOutSize, g_pLocaleManager->GetString( a_iLocaleId ), 0, TNULL );
}

// $Barnyard: FUNCTION 0042bc20
void ALocale::FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId, const PPropertyValue& a_rcArg1 )
{
	FormatTags( a_wszOutString, a_iOutSize, g_pLocaleManager->GetString( a_iLocaleId ), a_rcArg1 );
}

// $Barnyard: FUNCTION 0042bc50
void ALocale::FormatTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId, const PPropertyValue& a_rcArg1, const PPropertyValue& a_rcArg2 )
{
	FormatTags( a_wszOutString, a_iOutSize, g_pLocaleManager->GetString( a_iLocaleId ), a_rcArg1, a_rcArg2 );
}

// $Barnyard: FUNCTION 0042bc80
void ALocale::FormatInputTags( TWCHAR* a_wszOutString, TINT a_iOutSize, const TWCHAR* a_wszString )
{
	TString16 strString( a_wszString );
	TString16 strResult;

	static TINT s_iInputTagLength = TINT( TStringManager::String16Length( ms_wszInputTag ) );

	TINT iPos    = 0;
	TINT iTagPos = strString.Find( ms_wszInputTag, 0 );

	while ( iTagPos != -1 )
	{
		// Copy the text that precedes the tag
		if ( iTagPos > iPos )
			strResult.Concat( strString.GetString() + iPos, iTagPos - iPos );

		// %c_MapName[CommandName]
		TINT iMapStart     = iTagPos + s_iInputTagLength;
		TINT iCommandEnd   = strString.Find( L']', iMapStart );
		TINT iCommandStart = strString.Find( L'[', iMapStart );

		if ( iCommandEnd == -1 || iCommandStart == -1 )
		{
			iPos = 0;
			break;
		}

		TWCHAR wszName[ 64 ];
		TCHAR  szMapName[ 64 ];
		TCHAR  szCommandName[ 64 ];

		TStringManager::String16CopySafe( wszName, strString.GetString() + iMapStart, iCommandStart - iMapStart + 1 );
		TStringManager::StringUnicodeToChar( szMapName, wszName, -1 );
		TStringManager::String16CopySafe( wszName, strString.GetString() + iCommandStart + 1, iCommandEnd - iCommandStart );
		TStringManager::StringUnicodeToChar( szCommandName, wszName, -1 );

		AInputMapManager* pInputMapManager = AInputMapManager::GetSingleton();
		AInputCommand     eCommand         = pInputMapManager->GetCommandCode( TPString8( szCommandName ) );
		AInputMap*        pInputMap        = pInputMapManager->FindMap( TPString8( szMapName ) );

		if ( pInputMap == TNULL || eCommand == AInputCommand_Unknown )
		{
			// NOTE: the original also converts a_wszString into an unused local buffer here.
			// The whole source string gets appended to the text that is already processed.
			iPos = 0;
			break;
		}

		if ( eCommand == AInputCommand_Move )
		{
			// Move isn't bound to a single button, use a string that describes it
			TWCHAR wszMove[ 128 ];
			FormatInputTags( wszMove, 128, g_pLocaleManager->GetString( 1285 ) );
			strResult.Concat( wszMove, -1 );
		}
		else
		{
			TInputDevice::Doodad iDoodad = pInputMap->GetCommandDoodad( eCommand, TFALSE );

			if ( iDoodad == -1 )
			{
				// NOTE: the original also converts the command name into the (unused) map name buffer here
				iPos = 0;
				break;
			}

			const TWCHAR* wszButtonName = TNULL;

			if ( !pInputMapManager->GetDoodadCustomName( iDoodad, wszButtonName ) )
				pInputMapManager->GetDoodadLocalisedName( iDoodad, wszButtonName );

			if ( wszButtonName == TNULL )
			{
				iPos = 0;
				break;
			}

			strResult.Concat( wszButtonName, -1 );
		}

		iPos    = iCommandEnd + 1;
		iTagPos = strString.Find( ms_wszInputTag, iPos );
	}

	// Copy the rest of the string
	if ( iPos < strString.Length() )
		strResult.Concat( strString.GetString() + iPos, strString.Length() - iPos );

	TStringManager::String16CopySafe( a_wszOutString, strResult.GetString(), a_iOutSize );
}

// $Barnyard: FUNCTION 0042b550
void ALocale::FormatInputTags( TWCHAR* a_wszOutString, TINT a_iOutSize, TINT a_iLocaleId )
{
	FormatInputTags( a_wszOutString, a_iOutSize, g_pLocaleManager->GetString( a_iLocaleId ) );
}
