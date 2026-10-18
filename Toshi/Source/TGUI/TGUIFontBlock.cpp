#include "ToshiPCH.h"
#include "TGUIFontBlock.h"
#include "TGUIFont.h"
#include "TGUIManager.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TGUIFontBlock );

// $JPOG: FUNCTION TGuiInterface.dll+0002b580
TGUIFontBlock::TGUIFontBlock()
    : PPropertyBlock( TPString8( "font" ), PPropertyBlock::Priority( 1 ) )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0002b880
TGUIFontBlock::~TGUIFontBlock()
{
}

// NOTE: the object leaks if it isn't a TGUIFont
// $JPOG: FUNCTION TGuiInterface.dll+0002b5e0
TBOOL TGUIFontBlock::Load( PPropertyReader* a_pReader )
{
	PProperties* pProperties = a_pReader->LoadPropertyBlock();

	if ( pProperties )
	{
		TGUIDeserialisable* pObject = TGUIDeserialisable::Create( pProperties );

		if ( !pObject )
		{
			delete pProperties;
			return TFALSE;
		}

		if ( !pObject->GetClass()->IsA( &TGetClass( TGUIFont ) ) )
		{
			a_pReader->Error( "Font is not a TGUIFont!" );
			delete pProperties;
			return TFALSE;
		}

		TGUIFont* pFont = TSTATICCAST( TGUIFont, pObject );

		if ( pFont->Deserialise( pProperties ) == TTRUE )
		{
			TGUIManager* pManager = TGUIManager::GetSingleton();

			if ( pManager->FindFont( pFont->GetName() ) )
			{
				TString8 strWarning;
				strWarning.Format( "TGUIFontBlock::Load(): ignoring duplicate font '%s'", pFont->GetName().GetString() );
				a_pReader->Warning( strWarning );

				delete pFont;
			}
			else
			{
				pManager->AddFont( pFont );
			}
		}
		else
		{
			delete pFont;
		}

		delete pProperties;
	}

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+0002b770
TBOOL TGUIFontBlock::Save( PPropertyWriter* a_pWriter )
{
	TGUIManager* pManager = TGUIManager::GetSingleton();

	for ( auto it = pManager->GetFontList().Begin(); it != pManager->GetFontList().End(); it++ )
	{
		if ( !a_pWriter->PutBlockHeader( GetName() ) )
			return TFALSE;

		PProperties oProperties;
		it->Serialise( &oProperties );

		if ( !a_pWriter->PutPropertyBlock( &oProperties ) )
			return TFALSE;
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
