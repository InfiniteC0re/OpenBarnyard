#pragma once
#include "PProperties.h"
#include "PPropertyBlock.h"

#include <File/TFileLexerUTF8.h>
#include <Toshi/TArray.h>
#include <Toshi/TEvent.h>
#include <Toshi/TString8.h>

class PPropertyName;
class PPropertyValue;
class PPropertyReader;

//-----------------------------------------------------------------------------
// Thrown with the parse error emitter of PPropertyReader (errors and warnings)
//-----------------------------------------------------------------------------
class PPropertyReaderParseError
{
public:
	// NOTE: JPOG only clears the token, the file name is set to TNULL here
	// $JPOG: FUNCTION PPropertyParser.dll+00005f20
	PPropertyReaderParseError()
	    : m_szFileName( TNULL ), m_szMessage( TNULL )
	{}

	// $JPOG: FUNCTION PPropertyParser.dll+00005ef0
	PPropertyReaderParseError( const PPropertyReaderParseError& a_rcOther )
	    : m_oToken( a_rcOther.m_oToken ), m_szFileName( a_rcOther.m_szFileName ), m_szMessage( a_rcOther.m_szMessage )
	{}

	// $JPOG: FUNCTION PPropertyParser.dll+00005ed0
	~PPropertyReaderParseError() {}

	// $JPOG: FUNCTION PPropertyParser.dll+00005ea0
	PPropertyReaderParseError& operator=( const PPropertyReaderParseError& a_rcOther )
	{
		m_oToken     = a_rcOther.m_oToken;
		m_szFileName = a_rcOther.m_szFileName;
		m_szMessage  = a_rcOther.m_szMessage;
		return *this;
	}

public:
	Toshi::TFileLexerUTF8::Token m_oToken;     // 0x00 the token where the error is (an EOF token with the line for lexer errors)
	const TCHAR*                 m_szFileName; // 0x0C only set by Error/Warning( const TString8& )
	const TCHAR*                 m_szMessage;  // 0x10
};

//-----------------------------------------------------------------------------
// Purpose: reads text properties files (.ini) into PProperties:
//
//   name = value;              values: identifiers, "strings", numbers, true/false,
//   name[subname] = value;     { value, value, ... } arrays, name[subname] references,
//   name { ... }               $123 localised strings, nested blocks
//   name.subblock.name = value;
//   name;                      a property without a value
//   #include "file"            (".ini" is appended when the name has no extension)
//
// and the "[name]" blocks of .gui files with Load() and the definition blocks.
//-----------------------------------------------------------------------------
class PPropertyReader : public Toshi::TObject
{
public:
	TDECLARE_CLASS( PPropertyReader, Toshi::TObject );

	using Emitter = Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError>;

	// A registered "[name]" block handler
	struct DefinitionBlock
	{
		Toshi::TPString8            strName;
		Toshi::TRef<PPropertyBlock> pBlock;
	};

public:
	PPropertyReader();
	~PPropertyReader();

	// NOTE: JPOG has no a_iTokenLookahead argument
	virtual TBOOL Open( const Toshi::TString8& a_rcFileName, TINT a_iTokenLookahead );
	virtual TBOOL Open( const Toshi::TString8& a_rcFileName, Toshi::TFile* a_pFile, TINT a_iTokenLookahead );

	void Close();

	void AddDefinitionBlock( Toshi::TRef<PPropertyBlock> a_pBlock );

	// Loads every "[name]" block of the file with the registered definition blocks
	TBOOL Load();

	// Loads a whole file into a_rProperties (a_bLowerCase: identifiers are converted to lowercase)
	// NOTE: JPOG has no a_bLowerCase argument
	TBOOL Load( const Toshi::TString8& a_rcFileName, PProperties& a_rProperties, TBOOL a_bLowerCase );

	// Loads properties up to the end of the block, the file or a token that can't start a property
	PProperties* LoadPropertyBlock();
	TBOOL        LoadPropertyBlock( PProperties& a_rProperties );

	TBOOL LoadProperty( PProperties* a_pProperties );
	TBOOL LoadPropertyName( Toshi::TPString8& a_rName, Toshi::TPString8& a_rSubName );
	TBOOL LoadBlockHeader( Toshi::TPString8& a_rName );
	TBOOL GetValue( PPropertyValue& a_rValue );

	PPropertyValue Token2Value( const Toshi::TFileLexerUTF8::Token& a_rcToken );

	void Error( const Toshi::TString8& a_rcMessage );
	void Error( const Toshi::TFileLexerUTF8::Token& a_rcToken, const Toshi::TString8& a_rcMessage );
	void Warning( const Toshi::TString8& a_rcMessage );
	void Warning( const Toshi::TFileLexerUTF8::Token& a_rcToken, const Toshi::TString8& a_rcMessage );

	// $JPOG: FUNCTION PPropertyParser.dll+00005e60
	Emitter* GetParseErrorEmitter() { return &m_oEmitter; }

	// $JPOG: FUNCTION PPropertyParser.dll+00005e70
	Toshi::TFileLexerUTF8* GetLexer() { return m_pLexer; }

	// $JPOG: FUNCTION PPropertyParser.dll+00005e80
	void SetAssertOnError( TBOOL a_bAssertOnError ) { m_bAssertOnError = a_bAssertOnError; }

	// $JPOG: FUNCTION PPropertyParser.dll+00005e90
	void SetLoadComments( TBOOL a_bLoadComments ) { m_bLoadComments = a_bLoadComments; }

	const Toshi::TString8& GetFileName() const { return m_strFileName; }

private:
	TBOOL SkipBlock();

	// NOTE: the names are unknown, not in JPOG
	TBOOL ResolveIncludePath( const Toshi::TString8& a_rcIncludeName, Toshi::TString8& a_rOutPath );
	TBOOL LoadInclude( const Toshi::TString8& a_rcPath, PProperties& a_rProperties );

	static TBOOL __stdcall OnLexerParseError( PPropertyReader* a_pReader, Toshi::TFileLexerUTF8* a_pLexer, Toshi::TFileLexerUTF8::ParseError* a_pError );

private:
	Toshi::TString8                                                                             m_strFileName;       // 0x04
	Toshi::TFile*                                                                               m_pFile;             // 0x10
	Toshi::TFileLexerUTF8*                                                                      m_pLexer;            // 0x14
	Toshi::TArray<DefinitionBlock>                                                              m_aDefinitionBlocks; // 0x18 sorted by the pooled name pointers
	TBOOL                                                                                       m_bLoadComments;     // 0x28
	TBOOL                                                                                       m_bAssertOnError;    // 0x2C
	Toshi::TArray<TINT>                                                                         m_aBlockLines;       // 0x30 lines of the '{' of the blocks being read
	Emitter                                                                                     m_oEmitter;          // 0x40
	Toshi::TListener<Toshi::TFileLexerUTF8, Toshi::TFileLexerUTF8::ParseError, PPropertyReader> m_oLexerListener;    // 0x4C
};
