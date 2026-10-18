#include "ToshiPCH.h"
#include "TGUIStringBlock.h"
#include "TGUIManager.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PProperty.h>
#include <Plugins/PPropertyParser/PPropertyReader.h>
#include <Plugins/PPropertyParser/PPropertyWriter.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TGUIStringBlock );

// $JPOG: FUNCTION TGuiInterface.dll+0002b990
TGUIStringBlock::TGUIStringBlock()
    : PPropertyBlock( TPString8( "strings" ), PPropertyBlock::Priority( 1 ) )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0002bd10
TGUIStringBlock::~TGUIStringBlock()
{
}

// NOTE: JPOG inserts the strings into the TLocale of the system
// $JPOG: FUNCTION TGuiInterface.dll+0002b9f0
TBOOL TGUIStringBlock::Load( PPropertyReader* a_pReader )
{
	PProperties* pProperties = a_pReader->LoadPropertyBlock();

	if ( pProperties )
	{
		TGUIManager* pManager = TGUIManager::GetSingleton();

		for ( auto it = pProperties->GetProperties().Begin(); it != pProperties->GetProperties().End(); it++ )
		{
			TPString8 strValue = it->GetValue().GetTPString8();
			pManager->AddString( it->GetName().GetName(), TString16( strValue.GetString() ) );
		}

		delete pProperties;
	}

	return TTRUE;
}

// NOTE: JPOG writes the strings of the TLocale that are referenced and don't start with "TGUI"
// $JPOG: FUNCTION TGuiInterface.dll+0002bb50
TBOOL TGUIStringBlock::Save( PPropertyWriter* a_pWriter )
{
	if ( !a_pWriter->PutBlockHeader( GetName() ) )
		return TFALSE;

	PProperties oProperties;

	for ( auto it = TGUIManager::GetSingleton()->GetStrings().Begin(); it != TGUIManager::GetSingleton()->GetStrings().End(); it++ )
	{
		if ( TStringManager::String8Compare( it->strName.GetString(), "TGUI", 4 ) != 0 )
		{
			PPropertyValue oValue;
			oValue = it->strValue.GetString();
			oProperties.PutProperty( it->strName, oValue );
		}
	}

	return a_pWriter->PutPropertyBlock( &oProperties );
}

TOSHI_NAMESPACE_END
