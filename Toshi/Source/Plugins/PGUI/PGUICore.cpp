#include "ToshiPCH.h"
#include "PGUICore.h"
#include "PGUIPanel.h"
#include "PGUIButton.h"
#include "PGUIPushButton.h"
#include "PGUIToggleButton.h"
#include "PGUILabel.h"
#include "PGUIDialog.h"
#include "PGUILayoutManager.h"
#include "PGUIStackLayout.h"
#include "PGUIGridLayout.h"
#include "PGUIOverlayLayout.h"
#include "PGUIDiscLayout.h"
#include "PGUISliderBar.h"
#include "PGUITextBox.h"
#include "PGUIMenuBar.h"
#include "PGUIMenuDialog.h"
#include "PGUIDiscMenuDialog.h"
#include "PGUIMenuTree.h"
#include "PGUIListBox.h"
#include "PGUIListItem.h"
#include "PGUITableBox.h"
#include "PGUIHorizontalSplit.h"
#include "PGUIVerticalSplit.h"
#include "PGUIOwnerDraw.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIScreen.h>
#include <TGUI/TGUISoundFactory.h>
#include <TGUI/TGUIAudioContext.h>
#include <Plugins/PPropertyParser/PPropertyName.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

// The classes the definition files create by name, the linker would strip them from the static library otherwise
static const TClass* const s_apClasses[] = {
	&TGetClass( PGUIPanel ),
	&TGetClass( PGUIButton ),
	&TGetClass( PGUIPushButton ),
	&TGetClass( PGUIToggleButton ),
	&TGetClass( PGUILabel ),
	&TGetClass( PGUIDialog ),
	&TGetClass( PGUILayoutManager ),
	&TGetClass( PGUIStackLayout ),
	&TGetClass( PGUIGridLayout ),
	&TGetClass( PGUIOverlayLayout ),
	&TGetClass( PGUIDiscLayout ),
	&TGetClass( PGUISliderBar ),
	&TGetClass( PGUITextBox ),
	&TGetClass( PGUIMenuBar ),
	&TGetClass( PGUIMenuDialog ),
	&TGetClass( PGUIDiscMenuDialog ),
	&TGetClass( PGUIMenuTree ),
	&TGetClass( PGUIListBox ),
	&TGetClass( PGUIListItem ),
	&TGetClass( PGUITableBox ),
	&TGetClass( PGUIHorizontalSplit ),
	&TGetClass( PGUIVerticalSplit ),
	&TGetClass( PGUIOwnerDraw ),
};

void PGUIInitialise()
{
	// The TClass objects register themselves to their parents, TGUIManager initialises them with the other GUI classes
	for ( const TClass* pClass : s_apClasses )
		TASSERT( pClass->GetName() != TNULL );
}

const TClass* const* PGUIGetClasses( TINT& a_rNumClasses )
{
	a_rNumClasses = TARRAYSIZE( s_apClasses );
	return s_apClasses;
}

void PGUIPlaySound( TGUIScreen* a_pScreen, const TCHAR* a_szClass, const TCHAR* a_szSound )
{
	TGUIAudioContext* pAudioContext = a_pScreen->GetAudioContext();
	pAudioContext->PlaySound( TGUIManager::GetSingleton()->GetSoundFactory()->GetSoundID( PPropertyName( TPString8( a_szClass ), TPString8( a_szSound ) ) ) );
}
