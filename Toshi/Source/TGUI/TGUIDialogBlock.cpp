#include "ToshiPCH.h"
#include "TGUIDialogBlock.h"
#include "TGUIManager.h"
#include "TGUIWidget.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons. The pooled string "skufunction" of Save is created by its
// static initialiser (003823b0)
TDEFINE_CLASS_NORUNTIME( TGUIDialogBlock );

// $NicktoonsPS2: FUNCTION 00381fc8
// $JPOG: FUNCTION TGuiInterface.dll+0002b0c0
TGUIDialogBlock::TGUIDialogBlock()
    : PPropertyBlock( TPString8( "dialog" ), PPropertyBlock::Priority( 3 ) )
{
}

// $NicktoonsPS2: FUNCTION 00382050
// $JPOG: FUNCTION TGuiInterface.dll+0002b4a0
TGUIDialogBlock::~TGUIDialogBlock()
{
}

// NOTE: the SKU condition of the dialog is always empty (true), "skufunction" isn't read.
// An object created that isn't a TGUIWidget is leaked
// $NicktoonsPS2: FUNCTION 00382078
// $JPOG: FUNCTION TGuiInterface.dll+0002b1b0
TBOOL TGUIDialogBlock::Load( PPropertyReader* a_pReader )
{
	PProperties* pProperties = a_pReader->LoadPropertyBlock();

	if ( !pProperties )
		return TTRUE;

	TGUIDeserialisable* pObject = TGUIDeserialisable::Create( pProperties );

	if ( pObject && pObject->IsA( &TGetClass( TGUIWidget ) ) )
	{
		TGUIWidget* pWidget = TSTATICCAST( TGUIWidget, pObject );
		pWidget->Deserialise( pProperties );

		TSumOfMinterms oSkuFunction;
		TGUIManager::GetSingleton()->AddTopLevelInstance( pWidget, oSkuFunction );

		delete pProperties;
		return TTRUE;
	}

	a_pReader->Warning( "Top-level dialog is not a TGUIWidget!" );
	delete pProperties;

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00382210
// $JPOG: FUNCTION TGuiInterface.dll+0002b2c0
TBOOL TGUIDialogBlock::Save( PPropertyWriter* a_pWriter )
{
	TGUIManager*                          pManager   = TGUIManager::GetSingleton();
	TNodeList<TGUIManager::TopLevelInfo>& rInstances = pManager->GetTopLevelInstanceList();

	for ( auto it = rInstances.Begin(); it != rInstances.End(); it++ )
	{
		if ( !a_pWriter->PutBlockHeader( GetName() ) )
			return TFALSE;

		PProperties oProperties;

		if ( it->oSkuFunction.Size() != 0 )
		{
			TPString8 strSkuFunction = it->oSkuFunction.MakeFunctionString( TMinterm::ms_aEnglishOpNames, pManager->GetSkuArray() );
			oProperties.AddProperty( TPString8( "skufunction" ), PPropertyValue( strSkuFunction ) );
		}

		it->pWidget->Serialise( &oProperties );

		if ( !a_pWriter->PutPropertyBlock( &oProperties ) )
			return TFALSE;
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
