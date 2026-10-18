#include "ToshiPCH.h"
#include "TGUITextureAnimationBlock.h"
#include "TGUITextureAnimation.h"
#include "TGUIManager.h"
#include "TGUITextureFactory.h"

#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons
TDEFINE_CLASS_NORUNTIME( TGUITextureAnimationBlock );

// $NicktoonsPS2: FUNCTION 003828b0
// $JPOG: FUNCTION TGuiInterface.dll+0002c1b0
TGUITextureAnimationBlock::TGUITextureAnimationBlock()
    : PPropertyBlock( TPString8( "textureanimations" ), PPropertyBlock::Priority( 1 ) )
{
}

// $NicktoonsPS2: FUNCTION 00382938
// $JPOG: FUNCTION TGuiInterface.dll+0002d750
TGUITextureAnimationBlock::~TGUITextureAnimationBlock()
{
}

// Reads "name texture frequency x y width height rotation wrap mirrorx mirrory { { x, y, w, h }, ... };" lines
// up to a token that isn't a name
// NOTE: the animation leaks when a frame isn't a set of 4 integers, JPOG reports the errors of the
// lists with Error instead of Warning
// $NicktoonsPS2: FUNCTION 00382960
// $JPOG: FUNCTION TGuiInterface.dll+0002c210
TBOOL TGUITextureAnimationBlock::Load( PPropertyReader* a_pReader )
{
	TGUIManager*    pManager = TGUIManager::GetSingleton();
	TFileLexerUTF8* pLexer   = a_pReader->GetLexer();

	while ( TTRUE )
	{
		TFileLexerUTF8::Token oToken = pLexer->PeekNextToken( 0 );

		if ( oToken.GetType() != TFileLexer::TOKEN_STRING && pLexer->PeekNextToken( 0 ).GetType() != TFileLexer::TOKEN_IDENT )
			return TTRUE;

		PPropertyValue oValue;
		TPString8      strName    = a_pReader->Token2Value( pLexer->GetNextToken() ).GetTPString8();
		TPString8      strTexture = a_pReader->Token2Value( pLexer->GetNextToken() ).GetTPString8();
		TINT16         iTexture   = pManager->FindTexture( strTexture );

		if ( iTexture == -1 )
		{
			a_pReader->Error( oToken, TString8( "Couldn't find texture named '" ) + strTexture.GetString() + "'" );
			return TFALSE;
		}

		if ( pManager->FindTextureSection( strName ) )
		{
			// NOTE: the message shows the texture name instead of the name of the section
			TString8 strWarning;
			strWarning.Format( "Texture section '%s' already declared!", strTexture.GetString() );
			a_pReader->Warning( strWarning );
			return TFALSE;
		}

		TFLOAT fFrequency = a_pReader->Token2Value( pLexer->GetNextToken() ).GetFloat();
		TINT   iX         = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT   iY         = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT   iWidth     = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT   iHeight    = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TINT   iRotation  = a_pReader->Token2Value( pLexer->GetNextToken() ).GetInteger();
		TBOOL  bWrap      = a_pReader->Token2Value( pLexer->GetNextToken() ).GetBoolean();
		TBOOL  bMirrorX   = a_pReader->Token2Value( pLexer->GetNextToken() ).GetBoolean();
		TBOOL  bMirrorY   = a_pReader->Token2Value( pLexer->GetNextToken() ).GetBoolean();

		if ( !a_pReader->GetValue( oValue ) || !oValue.IsType( PPropertyValue::TYPE_ARRAY ) || !oValue.GetArray()->CanBeTypeArray( PPropertyValue::TYPE_ARRAY, -1 ) )
		{
			a_pReader->Warning( "Expected list of texture coordinates!" );
			return TFALSE;
		}

		PPropertyValue oFrames( oValue );

		if ( !pLexer->Expect( TFileLexer::TOKEN_SEMI ) )
			return TFALSE;

		PPropertyValueArray*  pFrames    = oFrames.GetArray();
		TGUITextureAnimation* pAnimation = new TGUITextureAnimation( iTexture, strName, pFrames->GetSize(), fFrequency, iX, iY, iWidth, iHeight, iRotation, bWrap, bMirrorX, bMirrorY );

		for ( TINT i = 0; i < pFrames->GetSize(); i++ )
		{
			oValue = *pFrames->GetValue( i );
			PPropertyValueArray* pFrame = oValue.GetArray();

			if ( !pFrame->CanBeTypeArray( PPropertyValue::TYPE_INT, 4 ) )
			{
				a_pReader->Warning( "Expected a set of texture coordinates!" );
				return TFALSE;
			}

			TGUITextureAnimation::Frame& rFrame = pAnimation->GetFrame( i + 1 );
			rFrame.uiX                          = TUINT16( pFrame->GetValue( 0 )->GetInteger() );
			rFrame.uiY                          = TUINT16( pFrame->GetValue( 1 )->GetInteger() );
			rFrame.uiWidth                      = TUINT16( pFrame->GetValue( 2 )->GetInteger() );
			rFrame.uiHeight                     = TUINT16( pFrame->GetValue( 3 )->GetInteger() );
		}

		pManager->AddTextureSection( pAnimation );
	}
}

// $NicktoonsPS2: FUNCTION 00383618
// $JPOG: FUNCTION TGuiInterface.dll+0002d2f0
TBOOL TGUITextureAnimationBlock::Save( PPropertyWriter* a_pWriter )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();

	if ( !a_pWriter->PutBlockHeader( GetName() ) )
		return TFALSE;

	for ( auto it = pManager->GetTextureSectionList().Begin(); it != pManager->GetTextureSectionList().End(); it++ )
	{
		if ( !it->GetClass()->IsExactly( &TGetClass( TGUITextureAnimation ) ) )
			continue;

		TGUITextureAnimation* pAnimation = TSTATICCAST( TGUITextureAnimation, it.Get() );
		TINT                  iNumFrames = pAnimation->GetFrames().Size();

		PPropertyValue oFrames( new PPropertyValueArray() );
		oFrames.GetArray()->SetSize( iNumFrames );

		for ( TINT i = 0; i < iNumFrames; i++ )
		{
			PPropertyValue oFrame( new PPropertyValueArray() );
			oFrame.GetArray()->SetSize( 4 );

			TUINT16 uiX, uiY, uiWidth, uiHeight;
			pAnimation->GetBBox( uiX, uiY, uiWidth, uiHeight, i + 1 );

			*oFrame.GetArray()->GetValue( 0 ) = PPropertyValue( TINT( uiX ) );
			*oFrame.GetArray()->GetValue( 1 ) = PPropertyValue( TINT( uiY ) );
			*oFrame.GetArray()->GetValue( 2 ) = PPropertyValue( TINT( uiWidth ) );
			*oFrame.GetArray()->GetValue( 3 ) = PPropertyValue( TINT( uiHeight ) );

			*oFrames.GetArray()->GetValue( i ) = oFrame;
		}

		a_pWriter->Put( pAnimation->GetName() );
		a_pWriter->PutSpace();
		a_pWriter->Put( pManager->GetTextureFactory()->GetTextureName( pAnimation->GetTexture() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( pAnimation->GetFrequency() );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( pAnimation->GetX() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( pAnimation->GetY() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( pAnimation->GetWidth() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( pAnimation->GetHeight() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TINT( pAnimation->GetRotation() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TBOOL( pAnimation->GetWrap() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TBOOL( pAnimation->GetMirrorX() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( TBOOL( pAnimation->GetMirrorY() ) );
		a_pWriter->PutSpace();
		a_pWriter->Put( PPropertyValue( oFrames ), 0 );
		a_pWriter->PutEndLine();
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
