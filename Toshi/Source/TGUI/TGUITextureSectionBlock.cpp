#include "ToshiPCH.h"
#include "TGUITextureSectionBlock.h"
#include "TGUIManager.h"
#include "TGUITextureFactory.h"

#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons
TDEFINE_CLASS_NORUNTIME( TGUITextureSectionBlock );

// $NicktoonsPS2: FUNCTION 00384058
// $JPOG: FUNCTION TGuiInterface.dll+0002dbf0
TGUITextureSectionBlock::TGUITextureSectionBlock()
    : PPropertyBlock( TPString8( "texturesections" ), PPropertyBlock::Priority( 1 ) )
{
}

// $NicktoonsPS2: FUNCTION 003840e0
// $JPOG: FUNCTION TGuiInterface.dll+0002e970
TGUITextureSectionBlock::~TGUITextureSectionBlock()
{
}

// Reads "name texture x y width height rotation wrap mirrorx mirrory;" lines up to a token that isn't a name,
// texture sections that already exist are replaced
// $NicktoonsPS2: FUNCTION 00384108
// $JPOG: FUNCTION TGuiInterface.dll+0002dc50
TBOOL TGUITextureSectionBlock::Load( PPropertyReader* a_pReader )
{
	TGUIManager*    pManager = TGUIManager::GetSingleton();
	TFileLexerUTF8* pLexer   = a_pReader->GetLexer();

	while ( TTRUE )
	{
		TFileLexerUTF8::Token oToken = pLexer->PeekNextToken( 0 );

		if ( oToken.GetType() != TFileLexer::TOKEN_STRING && pLexer->PeekNextToken( 0 ).GetType() != TFileLexer::TOKEN_IDENT )
			return TTRUE;

		TPString8 strName    = a_pReader->Token2Value( pLexer->GetNextToken() ).GetTPString8();
		TPString8 strTexture = a_pReader->Token2Value( pLexer->GetNextToken() ).GetTPString8();
		TINT      iX         = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT      iY         = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT      iWidth     = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT      iHeight    = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT      iRotation  = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TBOOL     bWrap      = a_pReader->Token2Value( pLexer->GetNextToken() ).GetBoolean();
		TBOOL     bMirrorX   = a_pReader->Token2Value( pLexer->GetNextToken() ).GetBoolean();
		TBOOL     bMirrorY   = a_pReader->Token2Value( pLexer->GetNextToken() ).GetBoolean();

		if ( !pLexer->Expect( TFileLexer::TOKEN_SEMI ) )
			return TFALSE;

		TINT16 iTexture = pManager->FindTexture( strTexture );

		if ( iTexture == -1 )
		{
			a_pReader->Error( oToken, TString8( "Couldn't find texture named '" ) + strTexture.GetString() + "'" );
			return TFALSE;
		}

		TGUITextureSection* pTextureSection = pManager->FindTextureSection( strName );

		if ( pTextureSection )
			pTextureSection->Set( iTexture, strName, iX, iY, iWidth, iHeight, iRotation, bWrap, bMirrorX, bMirrorY );
		else
			pManager->AddTextureSection( new TGUITextureSection( iTexture, strName, iX, iY, iWidth, iHeight, iRotation, bWrap, bMirrorX, bMirrorY ) );
	}
}

// Writes the texture sections that aren't animations or glyphs
// $NicktoonsPS2: FUNCTION 00384b10
// $JPOG: FUNCTION TGuiInterface.dll+0002e770
TBOOL TGUITextureSectionBlock::Save( PPropertyWriter* a_pWriter )
{
	if ( !a_pWriter->PutBlockHeader( GetName() ) )
		return TFALSE;

	TGUIManager* pManager = TGUIManager::GetSingleton();

	for ( auto it = pManager->GetTextureSectionList().Begin(); it != pManager->GetTextureSectionList().End(); it++ )
	{
		if ( it->GetClass() != &TGetClass( TGUITextureSection ) )
			continue;

		a_pWriter->Put( it->GetName() );
		a_pWriter->PutSpace();
		a_pWriter->Put( pManager->GetTextureFactory()->GetTextureName( it->GetTexture() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( it->GetX() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( it->GetY() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( it->GetWidth() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( it->GetHeight() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( it->GetRotation() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TBOOL( it->GetWrap() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TBOOL( it->GetMirrorX() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TBOOL( it->GetMirrorY() ) );
		a_pWriter->PutEndLine();
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
