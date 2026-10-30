#include "PrimeDebugPCH.h"
#include "PCVar.h"
#include "PConsole.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TSTATICASSERT( sizeof( PCVarValue ) == 0x10 );
TSTATICASSERT( sizeof( PCVar ) == 0x30 );

static TBOOL PCVar_Vector4Equals( const TFLOAT* a_pVector1, const TFLOAT* a_pVector2 )
{
	for ( TINT i = 0; i < 4; i++ )
	{
		if ( a_pVector1[ i ] != a_pVector2[ i ] )
			return TFALSE;
	}

	return TTRUE;
}

static TBOOL PCVar_MatrixEquals( const TFLOAT* a_pMatrix1, const TFLOAT* a_pMatrix2 )
{
	return PCVar_Vector4Equals( a_pMatrix1, a_pMatrix2 ) && PCVar_Vector4Equals( a_pMatrix1 + 4, a_pMatrix2 + 4 ) &&
	    PCVar_Vector4Equals( a_pMatrix1 + 8, a_pMatrix2 + 8 ) && PCVar_Vector4Equals( a_pMatrix1 + 12, a_pMatrix2 + 12 );
}

static void PCVar_MatrixCopy( TFLOAT* a_pDest, const TFLOAT* a_pSource )
{
	for ( TINT i = 0; i < 16; i++ )
		a_pDest[ i ] = a_pSource[ i ];
}

// $deBlob2: FUNCTION 0080bba7
TBOOL PCVar::Create( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT a_iType, const PCVarValue* a_pDefault, void* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData )
{
	if ( strchr( a_szName, ' ' ) )
	{
		PConsole::Error( "[crCVar_Create] ERROR: Variable names cannot contain spaces.\n" );
		return TFALSE;
	}

	if ( ( a_uiFlags & FLAGS_VALIDATERANGE ) && a_iType != TYPE_INT && a_iType != TYPE_FLOAT )
	{
		PConsole::Error( "[crCVar_Create] ERROR: ValidateRange only valid for int and float types.\n" );
		return TFALSE;
	}

	if ( ( a_uiFlags & FLAGS_BOOL ) && !( a_uiFlags & FLAGS_VALIDATERANGE ) )
	{
		PConsole::Error( "[crCVar_Create] ERROR: Boolean type must have ValidateRange specified.\n" );
		return TFALSE;
	}

	if ( ( a_uiFlags & FLAGS_BOOL ) && a_iType != TYPE_INT )
	{
		PConsole::Error( "[crCVar_Create] ERROR: Boolean type must have have int type.\n" );
		return TFALSE;
	}

	if ( PConsole::HasVariable( this ) )
	{
		PConsole::Error( "[crCVar_Create] ERROR: Command variable already added.\n" );
		return TFALSE;
	}

	if ( strchr( a_szDescription, ';' ) )
	{
		PConsole::Error( "[crCVar_Create] ERROR: Descriptions cannot contain ';' symbols.\n" );
		return TFALSE;
	}

	if ( PConsole::FindVariableUnlocked( a_szName ) )
	{
		PConsole::Error( "[crCVar_Create] ERROR: Command variable for with name '%s' already added.\n", a_szName );
		return TFALSE;
	}

	if ( PConsole::FindCommandUnlocked( a_szName ) )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Command callback for with name '%s' already added.\n", a_szName );
		return TFALSE;
	}

	m_szDescription = a_szDescription;
	m_szName        = a_szName;
	m_uiType        = TUINT16( a_iType );
	m_uiFlags       = a_uiFlags;
	m_oValue        = *a_pDefault;
	m_pNext         = TNULL;
	m_pPrev         = TNULL;
	m_pValue        = a_pStorage ? a_pStorage : this;
	m_fnOnChanged   = a_fnOnChanged;
	m_pUserData     = a_pUserData;
	PConsole::AddVariable( this );

	return TTRUE;
}

// $deBlob2: FUNCTION 0080c751
void PCVar::CreateFloatRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT a_fValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep )
{
	PCVarValue oDefault;
	oDefault.fValue = a_fValue;
	oDefault.fMin   = a_fMin;
	oDefault.fMax   = a_fMax;
	oDefault.fStep  = a_fStep;
	Create( a_szName, a_szDescription, a_uiFlags, TYPE_FLOAT, &oDefault, TNULL, a_fnOnChanged, a_pUserData );
}

// $deBlob2: FUNCTION 0080c710
void PCVar::CreateFloat( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT a_fValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData )
{
	CreateFloatRange( a_szName, a_szDescription, a_uiFlags, a_fValue, a_fnOnChanged, a_pUserData, 0.0f, 1.0f, 0.1f );
}

// $deBlob2: FUNCTION 0080c7f3
void PCVar::CreateFloatRefRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep )
{
	PCVarValue oDefault;
	oDefault.fValue = 0.0f;
	oDefault.fMin   = a_fMin;
	oDefault.fMax   = a_fMax;
	oDefault.fStep  = a_fStep;
	Create( a_szName, a_szDescription, a_uiFlags, TYPE_FLOAT, &oDefault, a_pStorage, a_fnOnChanged, a_pUserData );
}

// $deBlob2: FUNCTION 0080c7ba
void PCVar::CreateFloatRef( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData )
{
	CreateFloatRefRange( a_szName, a_szDescription, a_uiFlags, a_pStorage, a_fnOnChanged, a_pUserData, 0.0f, 1.0f, 0.1f );
}

// $deBlob2: FUNCTION 0080c87f
void PCVar::CreateIntRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT a_iValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TINT a_iMin, TINT a_iMax, TINT a_iStep )
{
	PCVarValue oDefault;
	oDefault.iValue = a_iValue;
	oDefault.iMin   = a_iMin;
	oDefault.iMax   = a_iMax;
	oDefault.iStep  = a_iStep;
	Create( a_szName, a_szDescription, a_uiFlags, TYPE_INT, &oDefault, TNULL, a_fnOnChanged, a_pUserData );
}

// $deBlob2: FUNCTION 0080c857
void PCVar::CreateInt( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT a_iValue, PCVarChangedCallback a_fnOnChanged, void* a_pUserData )
{
	CreateIntRange( a_szName, a_szDescription, a_uiFlags, a_iValue, a_fnOnChanged, a_pUserData, 0, 1, 1 );
}

// $deBlob2: FUNCTION 0080c900
void PCVar::CreateIntRefRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TINT a_iMin, TINT a_iMax, TINT a_iStep )
{
	PCVarValue oDefault;
	oDefault.iValue = 0;
	oDefault.iMin   = a_iMin;
	oDefault.iMax   = a_iMax;
	oDefault.iStep  = a_iStep;
	Create( a_szName, a_szDescription, a_uiFlags, TYPE_INT, &oDefault, a_pStorage, a_fnOnChanged, a_pUserData );
}

// $deBlob2: FUNCTION 0080c8d8
void PCVar::CreateIntRef( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TINT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData )
{
	CreateIntRefRange( a_szName, a_szDescription, a_uiFlags, a_pStorage, a_fnOnChanged, a_pUserData, 0, 1, 1 );
}

// $deBlob2: FUNCTION 0080c9e8
void PCVar::CreateFloatSplatRefRange( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep )
{
	PCVarValue oDefault;
	oDefault.fValue = 0.0f;
	oDefault.fMin   = a_fMin;
	oDefault.fMax   = a_fMax;
	oDefault.fStep  = a_fStep;
	Create( a_szName, a_szDescription, a_uiFlags, TYPE_FLOATSPLAT, &oDefault, a_pStorage, a_fnOnChanged, a_pUserData );
}

// $deBlob2: FUNCTION 0080c9af
void PCVar::CreateFloatSplatRef( const TCHAR* a_szName, const TCHAR* a_szDescription, TUINT16 a_uiFlags, TFLOAT* a_pStorage, PCVarChangedCallback a_fnOnChanged, void* a_pUserData )
{
	CreateFloatSplatRefRange( a_szName, a_szDescription, a_uiFlags, a_pStorage, a_fnOnChanged, a_pUserData, 0.0f, 1.0f, 0.1f );
}

// $deBlob2: FUNCTION 0080cb6a
void PCVar::Remove()
{
	if ( !PConsole::ms_bStartedUp )
		return;

	if ( m_uiType == TYPE_STRING )
		SetString( "" );

	PCVar* pNext = m_pNext;
	m_pNext      = TNULL;
	m_uiType     = TYPE_NONE;
	PCVar* pPrev = m_pPrev;
	m_pPrev      = TNULL;

	if ( pNext )
		pNext->m_pPrev = pPrev;

	if ( pPrev )
		pPrev->m_pNext = pNext;
	else if ( this == PConsole::ms_pVariables )
		PConsole::ms_pVariables = pNext;
}

// $deBlob2: FUNCTION 0080cbe1
TINT PCVar::GetEnumNames( TCHAR* a_szBuffer, TUINT a_uiBufferSize, TCHAR** a_ppNames, TINT a_iMaxNames )
{
	if ( a_iMaxNames == 0 )
		return 0;

	const TCHAR* pOpen  = strrchr( m_szDescription, '(' );
	const TCHAR* pClose = strrchr( m_szDescription, ')' );
	TINT         iCount = 0;

	if ( pOpen == TNULL || pClose == TNULL || pOpen >= pClose )
		return iCount;

	PString::Copy( a_szBuffer, a_uiBufferSize, pOpen + 1 );
	a_szBuffer[ pClose - pOpen - 1 ] = '\0';

	for ( TCHAR* p = a_szBuffer; *p != '\0'; p++ )
	{
		while ( isspace( TUINT16( TINT16( *p ) ) ) || *p == ',' )
			p++;

		TCHAR* pEquals = strchr( p, '=' );

		if ( pEquals == TNULL )
			return iCount;

		*pEquals = '\0';

		// The entries must be numbered 0, 1, 2...
		if ( PString::ToInt( p, TNULL, 10 ) != iCount )
			return iCount;

		TCHAR* pName = pEquals + 1;
		p            = pName;

		while ( *p != '\0' && *p != ',' )
			p++;

		*p = '\0';

		for ( TCHAR* pTrim = p - 1; pName < pTrim && *pTrim == ' '; pTrim-- )
			*pTrim = '\0';

		a_ppNames[ iCount ] = pName;
		iCount++;

		if ( iCount == a_iMaxNames )
			return iCount;
	}

	return iCount;
}

// $deBlob2: FUNCTION 0080ccce
TBOOL PCVar::GetMaxInt( TINT* a_pMax )
{
	if ( m_uiFlags & FLAGS_VALIDATERANGE )
	{
		*a_pMax = m_oValue.iMax;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080cceb
TBOOL PCVar::GetMinInt( TINT* a_pMin )
{
	if ( m_uiFlags & FLAGS_VALIDATERANGE )
	{
		*a_pMin = m_oValue.iMin;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080cd08
TBOOL PCVar::GetMaxFloat( TFLOAT* a_pMax )
{
	if ( m_uiFlags & FLAGS_VALIDATERANGE )
	{
		*a_pMax = m_oValue.fMax;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080cd25
TBOOL PCVar::GetMinFloat( TFLOAT* a_pMin )
{
	if ( m_uiFlags & FLAGS_VALIDATERANGE )
	{
		*a_pMin = m_oValue.fMin;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080cdca
void PCVar::FormatValue( TCHAR* a_szBuffer, TINT a_iBufferSize )
{
	switch ( m_uiType )
	{
		case TYPE_INT:
		{
			TINT iValue = *(TINT*)m_pValue;

			if ( m_uiFlags & FLAGS_BOOL )
			{
				PString::Format( a_szBuffer, a_iBufferSize, "%d (%s)", iValue, iValue ? "true" : "false" );
			}
			else if ( m_uiFlags & FLAGS_HEX )
			{
				PString::Format( a_szBuffer, a_iBufferSize, "0x%x", iValue );
			}
			else if ( m_uiFlags & FLAGS_HASH )
			{
				PString::Format( a_szBuffer, a_iBufferSize, "0x%x (%s)", iValue, "???" );
			}
			else
			{
				if ( m_uiFlags & FLAGS_ENUM )
				{
					TCHAR  szNames[ 512 ];
					TCHAR* apNames[ 32 ];
					TINT   iNumNames = GetEnumNames( szNames, sizeof( szNames ), apNames, 32 );

					if ( iValue >= 0 && iValue < iNumNames )
					{
						PString::Format( a_szBuffer, a_iBufferSize, "%d (%s)", iValue, apNames[ iValue ] );
						break;
					}
				}

				PString::Format( a_szBuffer, a_iBufferSize, "%d", iValue );
			}
			break;
		}

		case TYPE_FLOAT:
		case TYPE_FLOATSPLAT:
			PString::Format( a_szBuffer, a_iBufferSize, "%f", *(TFLOAT*)m_pValue );
			break;

		case TYPE_VECTOR4:
		{
			const TFLOAT* pValue = (const TFLOAT*)m_pValue;
			PString::Format( a_szBuffer, a_iBufferSize, "(%f,%f,%f,%f)", pValue[ 0 ], pValue[ 1 ], pValue[ 2 ], pValue[ 3 ] );
			break;
		}

		case TYPE_MATRIX:
		{
			const TFLOAT* m = (const TFLOAT*)m_pValue;
			PString::Format( a_szBuffer, a_iBufferSize, "[%f,%f,%f,%f|%f,%f,%f,%f|%f,%f,%f,%f|%f,%f,%f,%f]", m[ 0 ], m[ 1 ], m[ 2 ], m[ 3 ], m[ 4 ], m[ 5 ], m[ 6 ], m[ 7 ], m[ 8 ], m[ 9 ], m[ 10 ], m[ 11 ], m[ 12 ], m[ 13 ], m[ 14 ], m[ 15 ] );
			break;
		}

		case TYPE_STRING:
			PString::Format( a_szBuffer, a_iBufferSize, "\"%s\"", (const TCHAR*)m_pValue );
			break;
	}
}

// $deBlob2: FUNCTION 0080d032
void PCVar::FormatValueRaw( TCHAR* a_szBuffer, TINT a_iBufferSize )
{
	switch ( m_uiType )
	{
		case TYPE_INT:
			PString::Format( a_szBuffer, a_iBufferSize, "%d", *(TINT*)m_pValue );
			break;

		case TYPE_FLOAT:
		case TYPE_FLOATSPLAT:
			PString::Format( a_szBuffer, a_iBufferSize, "%f", *(TFLOAT*)m_pValue );
			break;

		case TYPE_VECTOR4:
		{
			const TFLOAT* pValue = (const TFLOAT*)m_pValue;
			PString::Format( a_szBuffer, a_iBufferSize, "%f %f %f %f", pValue[ 0 ], pValue[ 1 ], pValue[ 2 ], pValue[ 3 ] );
			break;
		}

		case TYPE_MATRIX:
		{
			const TFLOAT* m = (const TFLOAT*)m_pValue;
			PString::Format( a_szBuffer, a_iBufferSize, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", m[ 0 ], m[ 1 ], m[ 2 ], m[ 3 ], m[ 4 ], m[ 5 ], m[ 6 ], m[ 7 ], m[ 8 ], m[ 9 ], m[ 10 ], m[ 11 ], m[ 12 ], m[ 13 ], m[ 14 ], m[ 15 ] );
			break;
		}

		case TYPE_STRING:
			PString::Format( a_szBuffer, a_iBufferSize, "\"%s\"", (const TCHAR*)m_pValue );
			break;
	}
}

// $deBlob2: FUNCTION 0080d215
void PCVar::GetVector4( TFLOAT* a_pOut )
{
	const TFLOAT* pValue = (const TFLOAT*)m_pValue;
	a_pOut[ 0 ]          = pValue[ 0 ];
	a_pOut[ 1 ]          = pValue[ 1 ];
	a_pOut[ 2 ]          = pValue[ 2 ];
	a_pOut[ 3 ]          = pValue[ 3 ];
}

TBOOL PCVar::ClampFloat( TFLOAT& a_rValue )
{
	if ( m_uiFlags & FLAGS_VALIDATERANGE )
	{
		const PCVarValue& rRange = m_oValue;

		if ( rRange.fMin > a_rValue )
		{
			if ( a_rValue <= rRange.fMin - rRange.fStep )
				return TFALSE;

			a_rValue = rRange.fMin;
		}
		else if ( a_rValue > rRange.fMax )
		{
			if ( rRange.fStep + rRange.fMax <= a_rValue )
				return TFALSE;

			a_rValue = rRange.fMax;
		}
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080d240
TBOOL PCVar::SetFloat( TFLOAT a_fValue, TFLOAT* a_pOldValue )
{
	if ( m_uiFlags & FLAGS_READONLY )
		return TFALSE;

	if ( !ClampFloat( a_fValue ) )
		return TFALSE;

	TFLOAT fOldValue = *(TFLOAT*)m_pValue;

	if ( a_pOldValue )
		*a_pOldValue = fOldValue;

	if ( fOldValue != a_fValue )
	{
		*(TFLOAT*)m_pValue = a_fValue;

		if ( m_fnOnChanged )
			m_fnOnChanged( this );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080d2c3
TBOOL PCVar::SetInt( TINT a_iValue, TINT* a_pOldValue )
{
	if ( m_uiFlags & FLAGS_READONLY )
		return TFALSE;

	if ( ( m_uiFlags & FLAGS_VALIDATERANGE ) && ( a_iValue < m_oValue.iMin || a_iValue > m_oValue.iMax ) )
		return TFALSE;

	TINT iOldValue = *(TINT*)m_pValue;

	if ( a_pOldValue )
		*a_pOldValue = iOldValue;

	if ( iOldValue != a_iValue )
	{
		*(TINT*)m_pValue = a_iValue;

		if ( m_fnOnChanged )
			m_fnOnChanged( this );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080d30f
TBOOL PCVar::SetMatrix( const TFLOAT* a_pValue, TFLOAT* a_pOldValue )
{
	if ( m_uiFlags & FLAGS_READONLY )
		return TFALSE;

	TFLOAT aOldValue[ 16 ];
	PCVar_MatrixCopy( aOldValue, (const TFLOAT*)m_pValue );

	if ( a_pOldValue )
		PCVar_MatrixCopy( a_pOldValue, aOldValue );

	if ( !PCVar_MatrixEquals( aOldValue, a_pValue ) )
	{
		PCVar_MatrixCopy( (TFLOAT*)m_pValue, a_pValue );

		if ( m_fnOnChanged )
			m_fnOnChanged( this );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080d38c
TBOOL PCVar::SetFloatSplat( TFLOAT a_fValue, TFLOAT* a_pOldValue )
{
	if ( m_uiFlags & FLAGS_READONLY )
		return TFALSE;

	if ( !ClampFloat( a_fValue ) )
		return TFALSE;

	TFLOAT fOldValue = *(TFLOAT*)m_pValue;

	if ( a_pOldValue )
		*a_pOldValue = fOldValue;

	if ( fOldValue != a_fValue )
	{
		TFLOAT* pValue = (TFLOAT*)m_pValue;
		pValue[ 0 ]    = a_fValue;
		pValue[ 1 ]    = a_fValue;
		pValue[ 2 ]    = a_fValue;
		pValue[ 3 ]    = a_fValue;

		if ( m_fnOnChanged )
			m_fnOnChanged( this );
	}

	return TTRUE;
}

// Strings of up to 16 bytes are kept in the variable itself, longer ones are allocated.
// NOTE: doesn't call the change callback (as Prime)
// $deBlob2: FUNCTION 0080d443
TBOOL PCVar::SetString( const TCHAR* a_szValue )
{
	if ( m_uiFlags & FLAGS_READONLY )
		return TFALSE;

	// NOTE: Prime frees the old value before copying the new one; here the new one is copied first, so a variable can
	// be set to a string it holds
	TUINT  uiSize    = TUINT( strlen( a_szValue ) + 1 );
	TCHAR* pOldValue = ( m_pValue != this ) ? (TCHAR*)m_pValue : TNULL;

	if ( uiSize <= sizeof( PCVarValue ) )
	{
		TCHAR szValue[ sizeof( PCVarValue ) ];
		memcpy( szValue, a_szValue, uiSize );
		memset( &m_oValue, 0, sizeof( PCVarValue ) );
		m_pValue = this;
		memcpy( m_pValue, szValue, uiSize );
	}
	else
	{
		TCHAR* pValue = new TCHAR[ uiSize ];
		memcpy( pValue, a_szValue, uiSize );
		m_pValue = pValue;
	}

	delete[] pOldValue;
	return TTRUE;
}

// $deBlob2: FUNCTION 0080d4bc
TBOOL PCVar::SetVector4( const TFLOAT* a_pValue, TFLOAT* a_pOldValue )
{
	if ( m_uiFlags & FLAGS_READONLY )
		return TFALSE;

	TFLOAT* pValue = (TFLOAT*)m_pValue;
	TFLOAT  aOldValue[ 4 ];
	aOldValue[ 0 ] = pValue[ 0 ];
	aOldValue[ 1 ] = pValue[ 1 ];
	aOldValue[ 2 ] = pValue[ 2 ];
	aOldValue[ 3 ] = pValue[ 3 ];

	if ( a_pOldValue )
	{
		a_pOldValue[ 0 ] = aOldValue[ 0 ];
		a_pOldValue[ 1 ] = aOldValue[ 1 ];
		a_pOldValue[ 2 ] = aOldValue[ 2 ];
		a_pOldValue[ 3 ] = aOldValue[ 3 ];
	}

	if ( !PCVar_Vector4Equals( aOldValue, a_pValue ) )
	{
		pValue      = (TFLOAT*)m_pValue;
		pValue[ 0 ] = a_pValue[ 0 ];
		pValue[ 1 ] = a_pValue[ 1 ];
		pValue[ 2 ] = a_pValue[ 2 ];
		pValue[ 3 ] = a_pValue[ 3 ];

		if ( m_fnOnChanged )
			m_fnOnChanged( this );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080d52f
void PCVar::StepFloat( TBOOL a_bUp, TFLOAT* a_pOldValue )
{
	TFLOAT fDirection = a_bUp ? 1.0f : -1.0f;
	SetFloat( m_oValue.fStep * fDirection + *(TFLOAT*)m_pValue, a_pOldValue );
}

// $deBlob2: FUNCTION 0080d571
void PCVar::StepInt( TBOOL a_bUp, TINT* a_pOldValue )
{
	TINT iDirection = a_bUp ? 1 : -1;
	SetInt( iDirection * m_oValue.iStep + *(TINT*)m_pValue, a_pOldValue );
}

// $deBlob2: FUNCTION 0080d5a0
void PCVar::StepFloatSplat( TBOOL a_bUp, TFLOAT* a_pOldValue )
{
	TFLOAT fDirection = a_bUp ? 1.0f : -1.0f;
	SetFloatSplat( m_oValue.fStep * fDirection + *(TFLOAT*)m_pValue, a_pOldValue );
}

#endif // PRIMEDEBUG
