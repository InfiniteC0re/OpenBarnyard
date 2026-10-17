#include "ToshiPCH.h"
#include "PPropertyReader.h"
#include "PProperty.h"
#include "PPropertyValueArray.h"

#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( PPropertyReader );

using Token = TFileLexerUTF8::Token;

// $NicktoonsPS2: FUNCTION 0038acb8
// $JPOG: FUNCTION PPropertyParser.dll+000032b0
PPropertyReader::PPropertyReader()
    : m_pFile( TNULL )
    , m_pLexer( TNULL )
    , m_aDefinitionBlocks( 10, 0 )
    , m_bLoadComments( TFALSE )
    , m_bAssertOnError( TTRUE )
    , m_aBlockLines( 10, 0 )
    , m_oEmitter( this )
{
}

// $NicktoonsPS2: FUNCTION 0038ad50
// $JPOG: FUNCTION PPropertyParser.dll+00003310
PPropertyReader::~PPropertyReader()
{
	if ( m_pFile )
		m_pFile->Destroy();

	if ( m_pLexer )
		delete m_pLexer;

	m_oLexerListener.Disconnect();

	// NOTE: Nicktoons' TEmitter destructor disconnects the listeners, ours doesn't
	m_oEmitter.Destroy();

	m_aBlockLines.FreeData();

	// NOTE: TArray doesn't destroy its elements
	for ( TINT i = m_aDefinitionBlocks.Size() - 1; i >= 0; i-- )
		m_aDefinitionBlocks[ i ].~DefinitionBlock();

	m_aDefinitionBlocks.FreeData();
}

// $NicktoonsPS2: FUNCTION 0038ae08
// $JPOG: FUNCTION PPropertyParser.dll+00003380
TBOOL PPropertyReader::Open( const TString8& a_rcFileName, TINT a_iTokenLookahead )
{
	m_strFileName = a_rcFileName;

	if ( m_pFile )
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}

	if ( m_pLexer )
	{
		delete m_pLexer;
		m_pLexer = TNULL;
	}

	m_pFile = TFile::Create( a_rcFileName, TFILEMODE_READ );

	if ( m_pFile )
	{
		m_pLexer = new TFileLexerUTF8( m_pFile, a_iTokenLookahead );
		m_pLexer->SetOutputComments( m_bLoadComments );
		m_oLexerListener.Connect( *m_pLexer->GetParseErrorEmitter(), this, OnLexerParseError, 0 );

		return TTRUE;
	}

	// NOTE: the file name is passed to Format but the format has no %s
	TString8 strError;
	strError.Format( "Can't open file", a_rcFileName.GetString() );
	Error( strError );

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 0038af28
// $JPOG: FUNCTION PPropertyParser.dll+00003480
TBOOL PPropertyReader::Open( const TString8& a_rcFileName, TFile* a_pFile, TINT a_iTokenLookahead )
{
	m_strFileName = a_rcFileName;

	if ( m_pFile )
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}

	if ( m_pLexer )
	{
		delete m_pLexer;
		m_pLexer = TNULL;
	}

	m_pFile  = a_pFile;
	m_pLexer = new TFileLexerUTF8( m_pFile, a_iTokenLookahead );
	m_pLexer->SetOutputComments( m_bLoadComments );
	m_oLexerListener.Connect( *m_pLexer->GetParseErrorEmitter(), this, OnLexerParseError, 0 );

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0038aff8
// $JPOG: FUNCTION PPropertyParser.dll+00003530
void PPropertyReader::Close()
{
	if ( m_pFile )
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}

	if ( m_pLexer )
	{
		delete m_pLexer;
		m_pLexer = TNULL;
	}
}

// Adds a handler of "[name]" blocks, sorted by the pooled name pointers (after the ones with the same name)
// $NicktoonsPS2: FUNCTION 0038b050
// $JPOG: FUNCTION PPropertyParser.dll+00003570
void PPropertyReader::AddDefinitionBlock( TRef<PPropertyBlock> a_pBlock )
{
	TPString8 strName = a_pBlock->GetName();

	TINT iLow   = 0;
	TINT iHigh  = m_aDefinitionBlocks.Size() - 1;
	TINT iIndex = -1;

	while ( iHigh >= iLow )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = -m_aDefinitionBlocks[ iMiddle ].strName.Compare( strName );

		if ( iCompare < 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare > 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			// Insert after the blocks with the same name
			iIndex = iMiddle + 1;
			while ( iHigh >= iIndex && m_aDefinitionBlocks[ iIndex ].strName.Compare( strName ) == 0 )
				iIndex++;

			break;
		}
	}

	if ( iIndex == -1 )
		iIndex = iLow;

	if ( m_aDefinitionBlocks.SizeAllocated() < m_aDefinitionBlocks.Size() + 1 )
	{
		TINT iGrowSize = ( m_aDefinitionBlocks.GetGrowSize() > 1 ) ? m_aDefinitionBlocks.GetGrowSize() : 1;
		m_aDefinitionBlocks.Resize( m_aDefinitionBlocks.SizeAllocated() + iGrowSize );
	}

	TINT iSize = m_aDefinitionBlocks.Size();
	m_aDefinitionBlocks.SetSize( iSize + 1 );
	new ( &m_aDefinitionBlocks[ iSize ] ) DefinitionBlock();

	for ( TINT i = iSize - 1; i >= iIndex; i-- )
		m_aDefinitionBlocks[ i + 1 ] = m_aDefinitionBlocks[ i ];

	m_aDefinitionBlocks[ iIndex ].strName = strName;
	m_aDefinitionBlocks[ iIndex ].pBlock  = a_pBlock;
}

// Loads every "[name]" block of the file with its definition block, unknown blocks are skipped
// $JPOG: FUNCTION PPropertyParser.dll+00003680
TBOOL PPropertyReader::Load()
{
	Token oToken = m_pLexer->PeekNextToken( 0 );

	while ( oToken.GetType() != TFileLexer::TOKEN_EOF )
	{
		TPString8 strName;

		if ( !LoadBlockHeader( strName ) )
			return TFALSE;

		// Find the definition block
		TINT iIndex = -1;
		TINT iLow   = 0;
		TINT iHigh  = m_aDefinitionBlocks.Size() - 1;

		while ( iHigh >= iLow )
		{
			TINT iMiddle  = ( iLow + iHigh ) / 2;
			TINT iCompare = m_aDefinitionBlocks[ iMiddle ].strName.Compare( strName );

			if ( iCompare > 0 )
			{
				iHigh = iMiddle - 1;
			}
			else if ( iCompare < 0 )
			{
				iLow = iMiddle + 1;
			}
			else
			{
				iIndex = iMiddle;
				break;
			}
		}

		if ( iIndex == -1 )
		{
			TString8 strWarning;
			strWarning.Format( "Unknown block labelled '%s' found -- skipping", strName.GetString() );
			Warning( oToken, strWarning );

			SkipBlock();
		}
		else
		{
			// NOTE: the result isn't checked
			TRef<PPropertyBlock> pBlock = m_aDefinitionBlocks[ iIndex ].pBlock;
			pBlock->Load( this );
		}

		oToken = m_pLexer->PeekNextToken( 0 );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0038b730
// $JPOG: FUNCTION PPropertyParser.dll+000038c0
TBOOL PPropertyReader::Load( const TString8& a_rcFileName, PProperties& a_rProperties, TBOOL a_bLowerCase )
{
	if ( !Open( a_rcFileName, 2 ) )
		return TFALSE;

	if ( a_bLowerCase )
		m_pLexer->SetFlags( m_pLexer->GetFlags() | TFileLexerUTF8::FLAGS_LOWERCASE );

	Token oToken = m_pLexer->PeekNextToken( 0 );

	if ( oToken.GetType() != TFileLexer::TOKEN_EOF )
	{
		m_aBlockLines.FreeData();

		if ( LoadProperty( &a_rProperties ) )
		{
			Token oNextToken = m_pLexer->PeekNextToken( 0 );

			// NOTE: the file isn't closed when it succeeds
			if ( oNextToken.GetType() == TFileLexer::TOKEN_EOF )
				return TTRUE;

			Warning( "Unexpected data at end of file!" );
		}
	}

	Close();
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 0038b990
// $JPOG: FUNCTION PPropertyParser.dll+000044f0
TBOOL PPropertyReader::LoadPropertyName( TPString8& a_rName, TPString8& a_rSubName )
{
	Token oCloseToken;
	Token oToken     = m_pLexer->GetNextToken();
	Token oNextToken = m_pLexer->PeekNextToken( 0 );

	if ( oToken.GetType() != TFileLexer::TOKEN_IDENT && oToken.GetType() != TFileLexer::TOKEN_STRING )
	{
		Error( oToken, "Expecting property name to be identifer" );
		return TFALSE;
	}

	a_rName = oToken.GetString();

	if ( oNextToken.GetType() == TFileLexer::TOKEN_OPENSQR )
	{
		m_pLexer->GetNextToken();
		Token oSubNameToken = m_pLexer->GetNextToken();

		if ( m_pLexer->Expect( TFileLexer::TOKEN_CLOSESQR, oCloseToken ) != TTRUE )
		{
			Error( oCloseToken, "Expected close bracket after property sub-name" );
			return TFALSE;
		}

		if ( oSubNameToken.GetType() == TFileLexer::TOKEN_IDENT || oSubNameToken.GetType() == TFileLexer::TOKEN_STRING )
		{
			a_rSubName = oSubNameToken.GetString();
		}
		else if ( oSubNameToken.GetType() == TFileLexer::TOKEN_INTEGER )
		{
			a_rSubName = TPString8( TString8::VarArgs( "%d", oSubNameToken.GetInteger() ).GetString() );
		}
		else
		{
			Error( oSubNameToken, "Expecting property subname to be identifier or integer" );
			return TFALSE;
		}
	}

	return TTRUE;
}

// Converts an identifier, a string or a number to a value (true and false are booleans)
// $NicktoonsPS2: FUNCTION 0038bf58
// $JPOG: FUNCTION PPropertyParser.dll+00004af0
PPropertyValue PPropertyReader::Token2Value( const TFileLexerUTF8::Token& a_rcToken )
{
	// NOTE: Nicktoons creates these when PPropertyReader's TClass is initialised
	static TPString8 s_strTrue( "true" );
	static TPString8 s_strFalse( "false" );

	switch ( a_rcToken.GetType() )
	{
		case TFileLexer::TOKEN_IDENT:
			if ( a_rcToken.GetString() == s_strTrue )
				return PPropertyValue( TTRUE );

			if ( a_rcToken.GetString() == s_strFalse )
				return PPropertyValue( TFALSE );

			return PPropertyValue( a_rcToken.GetString() );
		case TFileLexer::TOKEN_STRING:
			return PPropertyValue( a_rcToken.GetString() );
		case TFileLexer::TOKEN_INTEGER:
			return PPropertyValue( a_rcToken.GetInteger() );
		case TFileLexer::TOKEN_UINTEGER:
			return PPropertyValue( a_rcToken.GetUInteger() );
		case TFileLexer::TOKEN_FLOAT:
			return PPropertyValue( a_rcToken.GetFloat() );
		default:
			return PPropertyValue();
	}
}

// $NicktoonsPS2: FUNCTION 0038c060
// $JPOG: FUNCTION PPropertyParser.dll+000039d0
PProperties* PPropertyReader::LoadPropertyBlock()
{
	Token oToken = m_pLexer->PeekNextToken( 0 );

	if ( oToken.GetType() == TFileLexer::TOKEN_EOF )
		return TNULL;

	PProperties* pProperties = new PProperties();
	m_aBlockLines.FreeData();

	// NOTE: the properties leak if it fails
	if ( !LoadProperty( pProperties ) )
		pProperties = TNULL;

	return pProperties;
}

// $JPOG: FUNCTION PPropertyParser.dll+00003a70
TBOOL PPropertyReader::LoadPropertyBlock( PProperties& a_rProperties )
{
	Token oToken = m_pLexer->PeekNextToken( 0 );

	if ( oToken.GetType() == TFileLexer::TOKEN_EOF )
		return TFALSE;

	m_aBlockLines.FreeData();
	return LoadProperty( &a_rProperties );
}

// Loads properties into a_pProperties until the end of the block ('}', consumed), the end of the file,
// or a token that can't start a property (not consumed)
// $NicktoonsPS2: FUNCTION 0038c168
// $JPOG: FUNCTION PPropertyParser.dll+00003b00
TBOOL PPropertyReader::LoadProperty( PProperties* a_pProperties )
{
	// NOTE: changed by the "name.name" paths, only reset after an assignment
	PProperties* pProperties = a_pProperties;
	Token        oSemiToken;

	while ( TTRUE )
	{
		Token oToken = m_pLexer->PeekNextToken( 0 );

		if ( oToken.GetType() == TFileLexer::TOKEN_CLOSEBRACE )
		{
			// End of the block
			if ( m_aBlockLines.Size() > 0 )
				m_aBlockLines.SetSize( m_aBlockLines.Size() - 1 );

			m_pLexer->GetNextToken();
			return TTRUE;
		}

		if ( oToken.GetType() == TFileLexer::TOKEN_EOF && m_aBlockLines.Size() != 0 )
		{
			TString8 strError;
			strError.Format( "Unexpected end of file in middle of property block (started at line %d)", m_aBlockLines[ m_aBlockLines.Size() - 1 ] );
			Error( oToken, strError );

			if ( m_aBlockLines.Size() > 0 )
				m_aBlockLines.SetSize( m_aBlockLines.Size() - 1 );

			return TFALSE;
		}

		if ( oToken.GetType() == TFileLexer::TOKEN_INCLUDE )
		{
			TPString8 strInclude = oToken.GetString();
			TString8  strPath;

			if ( !ResolveIncludePath( strInclude.GetString8(), strPath ) )
			{
				TString8 strError;
				strError.Format( "Invalid #include filename '%s'", strInclude.GetString() );
				Error( oToken, strError );

				return TFALSE;
			}

			// NOTE: always loaded into the properties of the call, not the ones of a "name.name" path
			if ( !LoadInclude( strPath, *a_pProperties ) )
				return TFALSE;

			m_pLexer->GetNextToken();
			continue;
		}

		if ( oToken.GetType() != TFileLexer::TOKEN_IDENT && oToken.GetType() != TFileLexer::TOKEN_STRING && oToken.GetType() != TFileLexer::TOKEN_COMMENT )
			return TTRUE;

		TPString8 strComment;
		TPString8 strName;
		TPString8 strSubName;

		if ( oToken.GetType() == TFileLexer::TOKEN_COMMENT )
		{
			// Only the last comment before the property is kept
			do
			{
				strComment = oToken.GetString();
				m_pLexer->GetNextToken();
				oToken = m_pLexer->PeekNextToken( 0 );
			} while ( oToken.GetType() == TFileLexer::TOKEN_COMMENT );
		}

		TINT iLine = oToken.GetLine();

		if ( !LoadPropertyName( strName, strSubName ) )
			return TFALSE;

		PPropertyName oName( strName, strSubName );
		Token         oOperatorToken = m_pLexer->GetNextToken();

		switch ( oOperatorToken.GetType() )
		{
			case TFileLexer::TOKEN_DOT:
			{
				// name.name: continue in the properties of the first name, created if needed
				// NOTE: the value isn't checked, GetProperties returns TNULL if it holds something else
				const PPropertyValue* pValue = pProperties->GetProperty( oName );

				if ( pValue == TNULL )
				{
					PProperties*   pNewProperties = new PProperties( pProperties );
					PPropertyValue oValue( pNewProperties );
					PProperties*   pParent = pProperties;

					pProperties = pNewProperties;
					pParent->AddCommentedProperty( oName, oValue, strComment, iLine );
				}
				else
				{
					pProperties = pValue->GetProperties();
				}

				break;
			}
			case TFileLexer::TOKEN_SEMI:
			{
				// A property without a value
				PPropertyValue oValue;
				pProperties->AddCommentedProperty( oName, oValue, strComment, iLine );
				break;
			}
			case TFileLexer::TOKEN_OPENBRACE:
			{
				// Nested block
				m_aBlockLines.Push( oOperatorToken.GetLine() );

				PProperties* pNewProperties = new PProperties( pProperties );

				{
					PPropertyValue oValue( pNewProperties );
					pProperties->AddCommentedProperty( oName, oValue, strComment, iLine );
				}

				if ( !LoadProperty( pNewProperties ) )
					return TFALSE;

				break;
			}
			case TFileLexer::TOKEN_EQUAL:
			{
				PPropertyValue oValue;

				if ( !GetValue( oValue ) )
					return TFALSE;

				pProperties->AddCommentedProperty( oName, oValue, strComment, iLine );

				if ( !m_pLexer->Expect( TFileLexer::TOKEN_SEMI, oSemiToken ) )
				{
					Error( oSemiToken, "Expected semicolon at end of property assignment" );
					return TFALSE;
				}

				pProperties = a_pProperties;
				break;
			}
			default:
			{
				TString8 strError;
				strError.Format( "Unexpected operator after property '%s'", oName.GetString().GetString() );
				Error( oOperatorToken, strError );

				return TFALSE;
			}
		}
	}
}

// Makes the path of an #include relative to the folder of this file and adds ".ini" if it has no extension
// $NicktoonsPS2: FUNCTION 0038cd98
TBOOL PPropertyReader::ResolveIncludePath( const TString8& a_rcIncludeName, TString8& a_rOutPath )
{
	TINT iLength = a_rcIncludeName.Length();

	if ( iLength <= 0 )
		return TFALSE;

	TBOOL bShort       = iLength < 5;
	TBOOL bNoExtension = bShort || a_rcIncludeName[ iLength - 4 ] != '.';
	TBOOL bAbsolute    = !bShort && a_rcIncludeName[ 1 ] == ':';

	TINT iSlash     = m_strFileName.FindReverse( '/', -1 );
	TINT iBackSlash = m_strFileName.FindReverse( '\\', -1 );
	TINT iSeparator = ( iSlash >= iBackSlash ) ? iSlash : iBackSlash;

	if ( iSeparator == -1 || bAbsolute )
	{
		// NOTE: ".ini" isn't added here
		a_rOutPath = a_rcIncludeName;
		return TTRUE;
	}

	a_rOutPath = m_strFileName;
	a_rOutPath.Truncate( iSeparator + 1 );
	a_rOutPath.Concat( a_rcIncludeName );

	if ( bNoExtension )
		a_rOutPath.Concat( ".ini" );

	return TTRUE;
}

// Loads an included file into a_rProperties with a new reader
// $NicktoonsPS2: FUNCTION 0038cee8
TBOOL PPropertyReader::LoadInclude( const TString8& a_rcPath, PProperties& a_rProperties )
{
	if ( a_rcPath == m_strFileName )
	{
		TString8 strWarning;
		strWarning.Format( "Trying to nested include file '%s' inside itself", a_rcPath.GetString() );
		Warning( strWarning );

		return TFALSE;
	}

	PPropertyReader oReader;

	if ( oReader.Load( a_rcPath, a_rProperties, TFALSE ) )
		return TTRUE;

	TString8 strWarning;
	strWarning.Format( "Failed to parse nested include file '%s'", a_rcPath.GetString() );
	Warning( strWarning );

	return TFALSE;
}

// Reads a value: a token, { value, value... }, $123 (localised string) or a name[subname] reference
// $NicktoonsPS2: FUNCTION 0038cff8
// $JPOG: FUNCTION PPropertyParser.dll+00004e00
TBOOL PPropertyReader::GetValue( PPropertyValue& a_rValue )
{
	Token oToken = m_pLexer->PeekNextToken( 0 );

	if ( oToken.GetType() == TFileLexer::TOKEN_OPENBRACE )
	{
		// Array
		PPropertyValueArray* pArray = new PPropertyValueArray( 3 );
		m_pLexer->GetNextToken();

		while ( TTRUE )
		{
			Token oNextToken = m_pLexer->PeekNextToken( 0 );

			if ( oNextToken.GetType() == TFileLexer::TOKEN_CLOSEBRACE )
			{
				m_pLexer->GetNextToken();
				a_rValue = pArray;

				return TTRUE;
			}

			PPropertyValue* pValue = pArray->Push( PPropertyValue() );

			if ( !GetValue( *pValue ) )
				break;

			oNextToken = m_pLexer->GetNextToken();

			if ( oNextToken.GetType() != TFileLexer::TOKEN_COMMA )
			{
				if ( oNextToken.GetType() == TFileLexer::TOKEN_CLOSEBRACE )
				{
					a_rValue = pArray;
					return TTRUE;
				}

				Error( oNextToken, "Expected a comma or close brace inside a list" );
				delete pArray;

				return TFALSE;
			}
		}

		delete pArray;
		return TFALSE;
	}

	if ( oToken.GetType() == TFileLexer::TOKEN_DOLLAR )
	{
		// Localised string
		m_pLexer->GetNextToken();
		oToken = m_pLexer->GetNextToken();

		// NOTE: strings referenced by name ($Name) are accepted but the value is left unchanged
		if ( oToken.GetType() == TFileLexer::TOKEN_IDENT )
			return TTRUE;

		if ( oToken.GetType() == TFileLexer::TOKEN_INTEGER )
		{
			// NOTE: Nicktoons doesn't check the index or the locale
			const TWCHAR* wszString = TNULL;
			T2Locale*     pLocale   = T2Locale::GetSingleton();

			if ( pLocale && oToken.GetInteger() >= 0 && oToken.GetInteger() < pLocale->GetNumStrings() )
				wszString = pLocale->GetString( oToken.GetInteger() );

			if ( wszString )
			{
				a_rValue = wszString;
				return TTRUE;
			}

			if ( !m_bAssertOnError )
			{
				TString8 strWarning;
				strWarning.Format( "Localised string numbered '%d' not found in locale!", oToken.GetInteger() );
				Warning( oToken, strWarning );

				a_rValue = TINT( 0 );
			}

			return TTRUE;
		}

		Error( oToken, "Expected identifier after '$' for localised string name" );
		return TFALSE;
	}

	Token oNextToken = m_pLexer->PeekNextToken( 1 );

	if ( oNextToken.GetType() == TFileLexer::TOKEN_OPENSQR )
	{
		// Reference to a property
		TPString8 strName;
		TPString8 strSubName;

		// NOTE: the result isn't checked
		LoadPropertyName( strName, strSubName );
		a_rValue = PPropertyValue( PPropertyName( strName, strSubName ) );

		return TTRUE;
	}

	m_pLexer->GetNextToken();
	a_rValue = Token2Value( oToken );

	if ( !a_rValue.IsDefined() )
	{
		Error( oToken, "Expected a value" );
		return TFALSE;
	}

	return TTRUE;
}

// $JPOG: FUNCTION PPropertyParser.dll+00004860
TBOOL PPropertyReader::LoadBlockHeader( TPString8& a_rName )
{
	Token oNameToken;
	Token oToken;

	while ( m_bLoadComments && m_pLexer->PeekNextToken( 0 ).GetType() == TFileLexer::TOKEN_COMMENT )
		m_pLexer->GetNextToken();

	if ( !m_pLexer->Expect( TFileLexer::TOKEN_OPENSQR, oToken ) )
	{
		Error( oToken, "Block header not found (expected '[')" );
		return TFALSE;
	}

	if ( !m_pLexer->Expect( TFileLexer::TOKEN_IDENT, oNameToken ) )
	{
		Error( oNameToken, "Block header not found (expected identifier)" );
		return TFALSE;
	}

	if ( !m_pLexer->Expect( TFileLexer::TOKEN_CLOSESQR, oToken ) )
	{
		Error( oToken, "Block header not found (expected ']')" );
		return TFALSE;
	}

	a_rName = oNameToken.GetString();
	return TTRUE;
}

// Skips statements up to the next '[' (or the end of the file)
// NOTE: a '[' is only checked between statements, so a block whose last statement isn't followed by
// a ';' outside of braces also skips the header of the next block
// $JPOG: FUNCTION PPropertyParser.dll+00005440
TBOOL PPropertyReader::SkipBlock()
{
	TINT  iDepth = 0;
	Token oToken = m_pLexer->PeekNextToken( 0 );

	while ( oToken.GetType() != TFileLexer::TOKEN_OPENSQR && oToken.GetType() != TFileLexer::TOKEN_EOF )
	{
		oToken = m_pLexer->GetNextToken();

		while ( ( oToken.GetType() != TFileLexer::TOKEN_SEMI || iDepth > 0 ) && oToken.GetType() != TFileLexer::TOKEN_EOF )
		{
			if ( oToken.GetType() == TFileLexer::TOKEN_OPENBRACE )
				iDepth++;

			if ( oToken.GetType() == TFileLexer::TOKEN_CLOSEBRACE )
				iDepth--;

			oToken = m_pLexer->GetNextToken();
		}

		oToken = m_pLexer->PeekNextToken( 0 );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0038da90
// $JPOG: FUNCTION PPropertyParser.dll+000055c0
TBOOL __stdcall PPropertyReader::OnLexerParseError( PPropertyReader* a_pReader, TFileLexerUTF8* a_pLexer, TFileLexerUTF8::ParseError* a_pError )
{
	PPropertyReaderParseError oError;
	oError.m_oToken    = Token( TFileLexer::TOKEN_EOF, a_pError->m_iLine );
	oError.m_szMessage = a_pError->m_szError;

	a_pReader->m_oEmitter.Throw( &oError );
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0038dde0
// $JPOG: FUNCTION PPropertyParser.dll+00004c80
void PPropertyReader::Error( const TString8& a_rcMessage )
{
	PPropertyReaderParseError oError;
	oError.m_oToken     = Token( TFileLexer::TOKEN_EOF, 0 );
	oError.m_szFileName = m_strFileName.GetString();
	oError.m_szMessage  = a_rcMessage.GetString();

	m_oEmitter.Throw( &oError );
}

// $NicktoonsPS2: FUNCTION 0038dcd0
// $JPOG: FUNCTION PPropertyParser.dll+00004d20
void PPropertyReader::Error( const TFileLexerUTF8::Token& a_rcToken, const TString8& a_rcMessage )
{
	PPropertyReaderParseError oError;
	oError.m_oToken    = a_rcToken;
	oError.m_szMessage = a_rcMessage.GetString();

	m_oEmitter.Throw( &oError );

	if ( m_bAssertOnError == TTRUE )
	{
		// NOTE: Nicktoons formats this and doesn't use it (the output is compiled out)
		TString8 strError;
		strError.Format( "%s(%d) : error : %s\n", m_strFileName.GetString(), a_rcToken.GetLine(), a_rcMessage.GetString() );
		TERROR( "%s", strError.GetString() );
	}
}

// NOTE: same code as Error( const TString8& ) in Nicktoons, empty in JPOG
// $NicktoonsPS2: FUNCTION 0038dbb0
// $JPOG: FUNCTION PPropertyParser.dll+00005850
void PPropertyReader::Warning( const TString8& a_rcMessage )
{
	PPropertyReaderParseError oError;
	oError.m_oToken     = Token( TFileLexer::TOKEN_EOF, 0 );
	oError.m_szFileName = m_strFileName.GetString();
	oError.m_szMessage  = a_rcMessage.GetString();

	m_oEmitter.Throw( &oError );
}

// $NicktoonsPS2: FUNCTION 0038df00
// $JPOG: FUNCTION PPropertyParser.dll+00004d90
void PPropertyReader::Warning( const TFileLexerUTF8::Token& a_rcToken, const TString8& a_rcMessage )
{
	PPropertyReaderParseError oError;
	oError.m_oToken    = a_rcToken;
	oError.m_szMessage = a_rcMessage.GetString();

	m_oEmitter.Throw( &oError );
}
