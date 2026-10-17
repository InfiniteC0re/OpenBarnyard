#include "ToshiPCH.h"
#include "PPropertyWriter.h"
#include "PProperty.h"
#include "PPropertyValueArray.h"

#include <Toshi/TString16.h>

#include <ctype.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( PPropertyWriter );

// $NicktoonsPS2: FUNCTION 003902d8
// $JPOG: FUNCTION PPropertyParser.dll+00007fc0
PPropertyWriter::PPropertyWriter()
    : m_pFile( TNULL )
    , m_aDefinitionBlocks( 10, 0 )
    , m_iWriteBufferUsed( 0 )
    , m_bPrintUnsignedAsHex( TTRUE )
{
	m_pWriteBuffer = TSTATICCAST( TCHAR, TMalloc( WRITE_BUFFER_SIZE ) );
}

// $NicktoonsPS2: FUNCTION 00390350
// $JPOG: FUNCTION PPropertyParser.dll+00008010
PPropertyWriter::~PPropertyWriter()
{
	if ( m_pFile )
	{
		FlushWriteBuffer();
		m_pFile->Destroy();
	}

	if ( m_pWriteBuffer )
		TFree( m_pWriteBuffer );

	// NOTE: TArray doesn't destroy its elements
	for ( TINT i = m_aDefinitionBlocks.Size() - 1; i >= 0; i-- )
		m_aDefinitionBlocks[ i ].~TRef();

	m_aDefinitionBlocks.FreeData();
}

// $JPOG: FUNCTION PPropertyParser.dll+00008060
TBOOL PPropertyWriter::Open( const TString8& a_rcFileName )
{
	if ( m_pFile )
	{
		FlushWriteBuffer();
		m_pFile->Destroy();
	}

	m_pFile = TFile::Create( a_rcFileName, TFILEMODE_WRITE | TFILEMODE_CREATENEW );

	if ( m_pFile )
		return TTRUE;

	Error( TString8( "Can't save: " ) + a_rcFileName.GetString() );
	return TFALSE;
}

// $JPOG: FUNCTION PPropertyParser.dll+00008110
TBOOL PPropertyWriter::Open( const TString8& a_rcFileName, TFile* a_pFile )
{
	if ( m_pFile )
	{
		FlushWriteBuffer();
		m_pFile->Destroy();
	}

	m_pFile = a_pFile;

	if ( a_pFile )
		return TTRUE;

	Error( TString8( "Can't save: " ) + a_rcFileName.GetString() );
	return TFALSE;
}

// $JPOG: FUNCTION PPropertyParser.dll+000093d0
void PPropertyWriter::Close()
{
	if ( m_pFile )
	{
		FlushWriteBuffer();
		m_pFile->Destroy();
	}

	m_pFile = TNULL;
}

// Adds a block written by Save, sorted by pointer
// $JPOG: FUNCTION PPropertyParser.dll+00009330
void PPropertyWriter::AddDefinitionBlock( TRef<PPropertyBlock> a_pBlock )
{
	PPropertyBlock* pBlock = a_pBlock;

	TINT iIndex = 0;
	while ( iIndex < m_aDefinitionBlocks.Size() && m_aDefinitionBlocks[ iIndex ].Get() <= pBlock )
		iIndex++;

	TINT iSize = m_aDefinitionBlocks.Size();
	m_aDefinitionBlocks.Push( TRef<PPropertyBlock>() );

	for ( TINT i = iSize - 1; i >= iIndex; i-- )
		m_aDefinitionBlocks[ i + 1 ] = m_aDefinitionBlocks[ i ];

	m_aDefinitionBlocks[ iIndex ] = pBlock;
}

// $JPOG: FUNCTION PPropertyParser.dll+000081a0
TBOOL PPropertyWriter::Save()
{
	for ( TINT i = 0; i < m_aDefinitionBlocks.Size(); i++ )
	{
		if ( !m_aDefinitionBlocks[ i ]->Save( this ) )
			return TFALSE;
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00390468
// $JPOG: FUNCTION PPropertyParser.dll+000082d0
TBOOL PPropertyWriter::PutBlockHeader( const TPString8& a_rcName )
{
	TString8 strName = a_rcName.GetString();
	return BufferCPrintf( "\n[%s]\n", strName.GetString() ) > 0;
}

// $NicktoonsPS2: FUNCTION 003904d8
// $JPOG: FUNCTION PPropertyParser.dll+00009300
TBOOL PPropertyWriter::PutPropertyBlock( const PProperties* a_pProperties )
{
	return PutPropertyBlock( 0, a_pProperties );
}

// Writes the properties, one per line, indented with a_iIndent tabs
// $NicktoonsPS2: FUNCTION 003904f8
// $JPOG: FUNCTION PPropertyParser.dll+00008340
TBOOL PPropertyWriter::PutPropertyBlock( TINT a_iIndent, const PProperties* a_pProperties )
{
	auto& rProperties = const_cast<PProperties*>( a_pProperties )->GetProperties();

	for ( auto it = rProperties.Begin(); it != rProperties.End(); it++ )
	{
		const PPropertyValue& rcValue = it->GetValue();

		if ( rcValue.IsType( PPropertyValue::TYPE_PROPS ) )
		{
			// name {
			// 	...
			// }
			if ( !Put( '\t', a_iIndent ) || !Put( it->GetName() ) || BufferCPrintf( " {\n" ) < 0 )
				return TFALSE;

			if ( !PutPropertyBlock( a_iIndent + 1, rcValue.GetProperties() ) || !Put( '\t', a_iIndent ) || BufferCPrintf( "}\n" ) < 0 )
				return TFALSE;
		}
		else if ( rcValue.IsType( PPropertyValue::TYPE_UNDEF ) )
		{
			// name;
			if ( !Put( '\t', a_iIndent ) || !Put( it->GetName() ) || BufferCPrintf( ";\n" ) < 0 )
				return TFALSE;
		}
		else
		{
			// name = value;
			if ( !Put( '\t', a_iIndent ) || !Put( it->GetName() ) || BufferCPrintf( " = " ) < 0 )
				return TFALSE;

			if ( !Put( rcValue, a_iIndent ) || BufferCPrintf( ";\n" ) < 0 )
				return TFALSE;
		}
	}

	return TTRUE;
}

// $JPOG: FUNCTION PPropertyParser.dll+00008220
TBOOL PPropertyWriter::PutPropertyName( const TPString8& a_rcName, const TPString8& a_rcSubName )
{
	if ( !Put( a_rcName ) )
		return TFALSE;

	if ( a_rcSubName.GetString8().IsEmpty() )
		return TTRUE;

	return BufferCPrintf( "[" ) >= 0 && Put( a_rcSubName ) && BufferCPrintf( "]" ) >= 0;
}

// $NicktoonsPS2: FUNCTION 003906c8
// $JPOG: FUNCTION PPropertyParser.dll+000092e0
TBOOL PPropertyWriter::PutSpace()
{
	return BufferCPrintf( "\t" ) >= 0;
}

// $NicktoonsPS2: FUNCTION 003906f0
// $JPOG: FUNCTION PPropertyParser.dll+000092c0
TBOOL PPropertyWriter::PutEndLine()
{
	return BufferCPrintf( ";\n" ) >= 0;
}

// $JPOG: FUNCTION PPropertyParser.dll+000092a0
TBOOL PPropertyWriter::PutNewLine()
{
	return BufferCPrintf( "\n" ) >= 0;
}

// $JPOG: FUNCTION PPropertyParser.dll+00008ea0
TBOOL PPropertyWriter::PutLiteral( const TCHAR* a_szLiteral )
{
	TString8 strLiteral = a_szLiteral;
	return BufferCPrintf( "%s", strLiteral.GetString() ) >= 0;
}

// Returns TTRUE unless the string is an identifier the reader can read back without quotes
// (a_iLength -1: zero terminated)
// $NicktoonsPS2: FUNCTION 00390718
// $JPOG: FUNCTION PPropertyParser.dll+00008ef0
TBOOL PPropertyWriter::string_needs_quotes( const TCHAR* a_szString, TINT a_iLength ) const
{
	if ( a_iLength == -1 && *a_szString == '\0' )
		return TTRUE;

	if ( a_iLength == 0 )
		return TTRUE;

	if ( TINT8( *a_szString ) < 0 || !isalpha( *a_szString ) )
		return TTRUE;

	// Would be read back as booleans
	if ( TStringManager::String8Compare( a_szString, "true", -1 ) == 0 )
		return TTRUE;

	if ( TStringManager::String8Compare( a_szString, "false", -1 ) == 0 )
		return TTRUE;

	const TCHAR* pcChar = a_szString;

	if ( a_iLength == -1 )
	{
		while ( TINT8( *pcChar ) >= 0 )
		{
			if ( !isalnum( *pcChar ) && *pcChar != '_' )
				return TTRUE;

			pcChar++;

			if ( *pcChar == '\0' )
				return TFALSE;
		}

		return TTRUE;
	}

	TINT iLength = a_iLength;

	while ( iLength > 0 )
	{
		if ( TINT8( *pcChar ) < 0 )
			return TTRUE;

		if ( !isalnum( *pcChar ) && *pcChar != '_' )
			return TTRUE;

		pcChar++;
		iLength--;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00390868
// $JPOG: FUNCTION PPropertyParser.dll+00008500
TBOOL PPropertyWriter::Put( const PPropertyValue& a_rcValue, TINT a_iIndent )
{
	const TClass* pType = a_rcValue.GetType();

	if ( pType == PPropertyValue::TYPE_INT )
		return Put( a_rcValue.GetInteger() );

	if ( pType == PPropertyValue::TYPE_UINT32 )
		return Put( a_rcValue.GetUINT32() );

	if ( pType == PPropertyValue::TYPE_FLOAT )
		return Put( a_rcValue.GetFloat() );

	if ( pType == PPropertyValue::TYPE_BOOL )
		return Put( a_rcValue.GetBoolean() );

	if ( pType == PPropertyValue::TYPE_TPSTRING8 )
		return Put( a_rcValue.GetTPString8() );

	// NOTE: Nicktoons also writes its TPString16 values here, Barnyard's PPropertyValue has no such type

	if ( pType == PPropertyValue::TYPE_STRING16 )
		return Put( a_rcValue.GetString16() );

	if ( pType == PPropertyValue::TYPE_PROPNAME )
		return Put( a_rcValue.GetPropertyName() );

	if ( pType != PPropertyValue::TYPE_ARRAY )
	{
		// NOTE: owned strings, locale IDs and properties (in arrays) can't be written
		return TFALSE;
	}

	// {
	// 	value,
	// 	value
	// }
	if ( !PutToken( TFileLexer::TOKEN_OPENBRACE ) || !Put( '\n', 1 ) )
		return TFALSE;

	PPropertyValueArray* pArray = a_rcValue.GetArray();

	for ( TINT i = 0; i < pArray->GetSize(); i++ )
	{
		if ( i != 0 && ( !PutToken( TFileLexer::TOKEN_COMMA ) || !Put( '\n', 1 ) ) )
			return TFALSE;

		if ( !Put( '\t', a_iIndent + 1 ) || !Put( *pArray->GetValue( i ), a_iIndent + 1 ) )
			return TFALSE;
	}

	return Put( '\n', 1 ) && Put( '\t', a_iIndent ) && PutToken( TFileLexer::TOKEN_CLOSEBRACE );
}

// $NicktoonsPS2: FUNCTION 00390bd0
// $JPOG: FUNCTION PPropertyParser.dll+00008780
TBOOL PPropertyWriter::Put( const PPropertyName& a_rcName )
{
	if ( !Put( a_rcName.GetName() ) )
		return TFALSE;

	if ( a_rcName.GetSubName().GetString8().IsEmpty() )
		return TTRUE;

	if ( BufferCPrintf( "[" ) < 0 )
		return TFALSE;

	if ( Put( a_rcName.GetSubName() ) != TTRUE )
		return TFALSE;

	return BufferCPrintf( "]" ) >= 0;
}

// $NicktoonsPS2: FUNCTION 00390c98
// $JPOG: FUNCTION PPropertyParser.dll+00009270
TBOOL PPropertyWriter::Put( const TPString8& a_rcString )
{
	return Put( a_rcString.GetString8() );
}

// Writes a string, quoted if needed, '"' and '\' are escaped
// $NicktoonsPS2: FUNCTION 00390e08
// $JPOG: FUNCTION PPropertyParser.dll+00008a10
TBOOL PPropertyWriter::Put( const TString8& a_rcString )
{
	TBOOL bQuotes = string_needs_quotes( a_rcString.GetString(), a_rcString.Length() );

	if ( bQuotes )
	{
		TCHAR cQuote = '"';
		BufferCChar( cQuote );
	}

	const TCHAR* pcChar  = a_rcString.GetString();
	TINT         iLength = a_rcString.Length();

	for ( TINT i = 0; i < iLength; i++ )
	{
		TCHAR cChar = *( pcChar++ );

		if ( cChar == '"' || cChar == '\\' )
		{
			TCHAR cEscape = '\\';
			BufferCChar( cEscape );
		}

		BufferCChar( cChar );
	}

	if ( bQuotes )
	{
		TCHAR cQuote = '"';
		BufferCChar( cQuote );
	}

	return TTRUE;
}

// $JPOG: FUNCTION PPropertyParser.dll+00008ae0
TBOOL PPropertyWriter::Put( const TCHAR* a_szString )
{
	TBOOL bQuotes = string_needs_quotes( a_szString, -1 );

	if ( bQuotes )
	{
		TCHAR cQuote = '"';
		BufferCChar( cQuote );
	}

	for ( const TCHAR* pcChar = a_szString; *pcChar != '\0'; pcChar++ )
	{
		if ( *pcChar == '"' || *pcChar == '\\' )
		{
			TCHAR cEscape = '\\';
			BufferCChar( cEscape );
		}

		BufferCChar( *pcChar );
	}

	if ( bQuotes )
	{
		TCHAR cQuote = '"';
		BufferCChar( cQuote );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00390d38
TBOOL PPropertyWriter::Put( const TWCHAR* a_wszString )
{
	if ( a_wszString )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: the original calls itself here and never returns
		TString8 strString;
		strString.Concat( TString16( a_wszString ) );
		return Put( strString );
#else
		return Put( a_wszString );
#endif
	}

	return Put( TString8( "" ) );
}

// $NicktoonsPS2: FUNCTION 00390da0
// $JPOG: FUNCTION PPropertyParser.dll+000089d0
TBOOL PPropertyWriter::Put( TCHAR a_cChar, TINT a_iCount )
{
	for ( TINT i = a_iCount - 1; i >= 0; i-- )
	{
		if ( BufferCChar( a_cChar ) < 0 )
			return TFALSE;
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00390f18
// $JPOG: FUNCTION PPropertyParser.dll+00009240
TBOOL PPropertyWriter::Put( TINT a_iValue )
{
	return BufferCPrintf( "%d", a_iValue ) >= 0;
}

// $NicktoonsPS2: FUNCTION 00390f48
// $JPOG: FUNCTION PPropertyParser.dll+00008b90
TBOOL PPropertyWriter::Put( TUINT32 a_uiValue )
{
	if ( m_bPrintUnsignedAsHex )
		return BufferCPrintf( "0x%08Xu", a_uiValue ) >= 0;

	return BufferCPrintf( "%du", a_uiValue ) >= 0;
}

// $NicktoonsPS2: FUNCTION 00390f90
// $JPOG: FUNCTION PPropertyParser.dll+00009210
TBOOL PPropertyWriter::Put( TFLOAT a_fValue )
{
	return BufferCPrintf( "%gf", a_fValue ) >= 0;
}

// $NicktoonsPS2: FUNCTION 00390fe0
// $JPOG: FUNCTION PPropertyParser.dll+000091d0
TBOOL PPropertyWriter::Put( TBOOL a_bValue )
{
	return BufferCPrintf( a_bValue ? "true" : "false" ) >= 0;
}

// Writes the character of a single character token
// $NicktoonsPS2: FUNCTION 00391020
// $JPOG: FUNCTION PPropertyParser.dll+00008bf0
TBOOL PPropertyWriter::PutToken( TFileLexer::TokenType a_eType )
{
	TCHAR cChar;

	switch ( a_eType )
	{
		case TFileLexer::TOKEN_SEMI: cChar = ';'; break;
		case TFileLexer::TOKEN_COLON: cChar = ':'; break;
		case TFileLexer::TOKEN_COMMA: cChar = ','; break;
		case TFileLexer::TOKEN_DOT: cChar = '.'; break;
		case TFileLexer::TOKEN_ASTERISK: cChar = '*'; break;
		case TFileLexer::TOKEN_DOLLAR: cChar = '$'; break;
		case TFileLexer::TOKEN_OPENSQR: cChar = '['; break;
		case TFileLexer::TOKEN_CLOSESQR: cChar = ']'; break;
		case TFileLexer::TOKEN_OPENBRACE: cChar = '{'; break;
		case TFileLexer::TOKEN_CLOSEBRACE: cChar = '}'; break;
		case TFileLexer::TOKEN_OPENPAREN: cChar = '('; break;
		case TFileLexer::TOKEN_CLOSEPAREN: cChar = ')'; break;
		case TFileLexer::TOKEN_LESSTHAN: cChar = '<'; break;
		case TFileLexer::TOKEN_GREATERTHAN: cChar = '>'; break;
		case TFileLexer::TOKEN_EQUAL: cChar = '='; break;
		case TFileLexer::TOKEN_EXCLAMATION: cChar = '!'; break;
		case TFileLexer::TOKEN_HYPHEN: cChar = '-'; break;
		default: return TFALSE;
	}

	return BufferCChar( cChar ) != -1;
}

// $JPOG: FUNCTION PPropertyParser.dll+000091c0
void PPropertyWriter::Error( const TString8& a_rcMessage )
{
	TERROR( "%s\n", a_rcMessage.GetString() );
}

// $NicktoonsPS2: FUNCTION 003911d0
// $JPOG: FUNCTION PPropertyParser.dll+00008ff0
TINT PPropertyWriter::BufferCChar( const TCHAR& a_rcChar )
{
	TINT iResult = a_rcChar;

	if ( m_iWriteBufferUsed >= WRITE_BUFFER_SIZE )
		iResult = FlushWriteBuffer();

	m_pWriteBuffer[ m_iWriteBufferUsed++ ] = a_rcChar;
	return iResult;
}

// $NicktoonsPS2: FUNCTION 00391288
// $JPOG: FUNCTION PPropertyParser.dll+00009030
TINT PPropertyWriter::BufferCPrintf( const TCHAR* a_szFormat, ... )
{
	FlushWriteBuffer();

	va_list args;
	va_start( args, a_szFormat );
	TINT iResult = m_pFile->VCPrintf( a_szFormat, args );
	va_end( args );

	return iResult;
}

// $NicktoonsPS2: FUNCTION 00391240
// $JPOG: FUNCTION PPropertyParser.dll+00009060
TINT PPropertyWriter::FlushWriteBuffer()
{
	if ( m_iWriteBufferUsed <= 0 )
		return -1;

	TINT iResult       = TINT( m_pFile->Write( m_pWriteBuffer, m_iWriteBufferUsed ) );
	m_iWriteBufferUsed = 0;

	return iResult;
}
