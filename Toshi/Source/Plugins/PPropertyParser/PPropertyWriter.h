#pragma once
#include "PProperties.h"
#include "PPropertyBlock.h"

#include <File/TFileLexer.h>
#include <File/TFile.h>
#include <Toshi/TArray.h>
#include <Toshi/TString8.h>

class PPropertyName;
class PPropertyValue;

//-----------------------------------------------------------------------------
// Purpose: writes PProperties in the text format read by PPropertyReader
// (properties of a block one per line, indented with tabs, arrays one value
// per line) and the "[name]" blocks of the registered definition blocks.
// Comments aren't written.
//-----------------------------------------------------------------------------
class PPropertyWriter : public Toshi::TObject
{
public:
	TDECLARE_CLASS( PPropertyWriter, Toshi::TObject );

	static constexpr TINT WRITE_BUFFER_SIZE = 0x4000;

public:
	PPropertyWriter();
	~PPropertyWriter();

	// NOTE: JPOG takes the file names as TWString
	TBOOL Open( const Toshi::TString8& a_rcFileName );
	TBOOL Open( const Toshi::TString8& a_rcFileName, Toshi::TFile* a_pFile );
	void  Close();

	// $JPOG: FUNCTION PPropertyParser.dll+00009320
	TBOOL IsFileValid() { return m_pFile != TNULL; }

	void AddDefinitionBlock( Toshi::TRef<PPropertyBlock> a_pBlock );

	// Writes every definition block
	TBOOL Save();

	TBOOL PutBlockHeader( const Toshi::TPString8& a_rcName );
	TBOOL PutPropertyBlock( const PProperties* a_pProperties );
	TBOOL PutPropertyBlock( TINT a_iIndent, const PProperties* a_pProperties );
	TBOOL PutPropertyName( const Toshi::TPString8& a_rcName, const Toshi::TPString8& a_rcSubName );

	// NOTE: JPOG has no a_iIndent argument (it writes arrays on one line)
	TBOOL Put( const PPropertyValue& a_rcValue, TINT a_iIndent );
	TBOOL Put( const PPropertyName& a_rcName );
	TBOOL Put( const Toshi::TPString8& a_rcString );
	TBOOL Put( const Toshi::TString8& a_rcString );
	TBOOL Put( const TCHAR* a_szString );
	TBOOL Put( const TWCHAR* a_wszString );
	TBOOL Put( TCHAR a_cChar, TINT a_iCount );
	TBOOL Put( TINT a_iValue );
	TBOOL Put( TUINT32 a_uiValue );
	TBOOL Put( TFLOAT a_fValue );
	TBOOL Put( TBOOL a_bValue );

	TBOOL PutToken( Toshi::TFileLexer::TokenType a_eType );
	TBOOL PutLiteral( const TCHAR* a_szLiteral );
	TBOOL PutNewLine();
	TBOOL PutEndLine();
	TBOOL PutSpace();

	void Error( const Toshi::TString8& a_rcMessage );

	// $JPOG: FUNCTION PPropertyParser.dll+00009850
	void PrintUnsignedAsHex( TBOOL a_bHex ) { m_bPrintUnsignedAsHex = a_bHex; }

protected:
	TBOOL string_needs_quotes( const TCHAR* a_szString, TINT a_iLength ) const;

	TINT BufferCChar( const TCHAR& a_rcChar );
	TINT BufferCPrintf( const TCHAR* a_szFormat, ... );
	TINT FlushWriteBuffer();

private:
	Toshi::TFile*                              m_pFile;               // 0x04
	Toshi::TArray<Toshi::TRef<PPropertyBlock>> m_aDefinitionBlocks;   // 0x08 sorted by pointer
	TCHAR*                                     m_pWriteBuffer;        // 0x18
	TINT                                       m_iWriteBufferUsed;    // 0x1C
	TBOOL                                      m_bPrintUnsignedAsHex; // 0x20
};
