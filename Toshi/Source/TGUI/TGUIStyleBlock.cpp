#include "ToshiPCH.h"
#include "TGUIStyleBlock.h"
#include "TGUIManager.h"
#include "TGUIStyle.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons. Its static initialiser (00382790) also creates the
// pooled string "TGUIStyle"
TDEFINE_CLASS_NORUNTIME( TGUIStyleBlock );

// $NicktoonsPS2: FUNCTION 003824d0
// $JPOG: FUNCTION TGuiInterface.dll+0002bdf0
TGUIStyleBlock::TGUIStyleBlock()
    : PPropertyBlock( TPString8( "style" ), PPropertyBlock::Priority( 2 ) )
{
}

// $NicktoonsPS2: FUNCTION 00382558
// $JPOG: FUNCTION TGuiInterface.dll+0002c0a0
TGUIStyleBlock::~TGUIStyleBlock()
{
}

// $NicktoonsPS2: FUNCTION 00382580
// $JPOG: FUNCTION TGuiInterface.dll+0002be50
TBOOL TGUIStyleBlock::Load( PPropertyReader* a_pReader )
{
	PProperties* pProperties = a_pReader->LoadPropertyBlock();

	if ( !pProperties )
		return TTRUE;

	// NOTE: the object leaks if it isn't a TGUIStyle
	TGUIDeserialisable* pObject = TGUIDeserialisable::Create( pProperties );

	if ( !pObject || !pObject->GetClass()->IsA( &TGetClass( TGUIStyle ) ) )
	{
		a_pReader->Warning( "Top-level style is not a TGUIStyle!" );
		delete pProperties;

		return TFALSE;
	}

	TGUIStyle* pStyle = TSTATICCAST( TGUIStyle, pObject );

	if ( pStyle->Deserialise( pProperties ) == TTRUE )
		TGUIManager::GetSingleton()->AddStyleInstance( pStyle );
	else
		delete pStyle;

	delete pProperties;
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 003826b0
// $JPOG: FUNCTION TGuiInterface.dll+0002bf90
TBOOL TGUIStyleBlock::Save( PPropertyWriter* a_pWriter )
{
	TNodeList<TGUIStyle>& rStyles = TGUIManager::GetSingleton()->GetStyleInstanceList();

	for ( auto it = rStyles.Begin(); it != rStyles.End(); it++ )
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
