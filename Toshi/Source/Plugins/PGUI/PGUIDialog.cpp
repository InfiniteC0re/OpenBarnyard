#include "ToshiPCH.h"
#include "PGUIDialog.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIScreen.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIDialog );

TGUIDeserialisable::PropertySet* PGUIDialog::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIDialog::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIDialog::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+00004540
void PGUIDialog::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWindow::ms_oPropertySet );

	ms_pWidgetInfo->AddComponentName( TPString8( "background" ) );
	ms_pWidgetInfo->AddComponentName( TPString8( "border" ) );
	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIDialog" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Dialog" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWindow::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+00004940
void PGUIDialog::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00004db0
PGUIDialog::PGUIDialog()
    : m_bDragging( TFALSE )
{
}

// $JPOG: FUNCTION PGUICore.dll+00004ce0
PGUIDialog::PGUIDialog( const PGUIDialog& a_rcOther )
    : TGUIWindow( a_rcOther )
    , m_bDragging( TFALSE )
{
}

// $JPOG: FUNCTION PGUICore.dll+00004da0
PGUIDialog::~PGUIDialog()
{
}

// $JPOG: FUNCTION PGUICore.dll+00004d30
TBOOL PGUIDialog::Deserialise( const PProperties* a_pProperties )
{
	return TGUIWindow::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00004d10
TBOOL PGUIDialog::Serialise( PProperties* a_pProperties )
{
	return TGUIWindow::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00004df0
TGUIDeserialisable::PropertySet* PGUIDialog::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00004de0
TGUIDeserialisable::PropertySet* PGUIDialog::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIDialog::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00004d70
PGUIDialog* PGUIDialog::Clone() const
{
	return new PGUIDialog( *this );
}

// $JPOG: FUNCTION PGUICore.dll+00004e90
TBOOL PGUIDialog::AllowsChildren() const
{
	return TTRUE;
}

// A click starts dragging the dialog
// $JPOG: FUNCTION PGUICore.dll+00004b40
TBOOL PGUIDialog::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( !( a_iActions & 0x4000 ) )
		return TGUIWidget::OnButtonPress( a_pEvent, a_iActions );

	m_bDragging = TTRUE;

	TGUIScreen* pScreen = GetScreen();
	m_iDragPointerX     = pScreen->GetPointerPositionX();
	m_iDragPointerY     = pScreen->GetPointerPositionY();

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00004b90
TBOOL PGUIDialog::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( !( a_iActions & 0x4000 ) || !m_bDragging )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		return TGUIWidget::OnButtonRelease( a_pEvent, a_iActions );
#else
		// The original handles the release as a press
		return TGUIWidget::OnButtonPress( a_pEvent, a_iActions );
#endif
	}

	m_bDragging = TFALSE;
	GetScreen();

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00004cc0
TBOOL PGUIDialog::OnPointerMove( TINT a_iX, TINT a_iY )
{
	return TGUIWidget::OnPointerMove( a_iX, a_iY );
}

// $JPOG: FUNCTION PGUICore.dll+00004a00
void PGUIDialog::Pack()
{
	PackChildren();

	if ( m_pStyle )
	{
		SetComponentBBox( 0, 0, 0, m_iWidth, m_iHeight );
		SetComponentBBox( 1, 0, 0, m_iWidth, m_iHeight );
	}

	m_bValidated = TTRUE;
}

// The children are laid out first, the size that isn't fixed fits them and the border
// $JPOG: FUNCTION PGUICore.dll+00004a50
void PGUIDialog::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;
	LayoutChildren( a_iMaxWidth, a_iMaxHeight );

	if ( m_eSizeModeHoriz == SizeMode_Fixed && m_eSizeModeVert == SizeMode_Fixed )
		return;

	TINT iWidth  = m_iWidth;
	TINT iHeight = m_iHeight;

	if ( m_pStyle )
	{
		TINT  iPreferredWidth  = 0;
		TINT  iPreferredHeight = 0;
		TBOOL bAllKnown;

		if ( m_pStyle->GetComponentPreferredSize( 1, m_pParameters, iPreferredWidth, iPreferredHeight, a_iMaxWidth, a_iMaxHeight, bAllKnown ) )
		{
			iWidth  = TMath::Max( iWidth, iPreferredWidth );
			iHeight = TMath::Max( iHeight, iPreferredHeight );
		}
	}

	if ( m_eSizeModeHoriz != SizeMode_Fixed )
		SetWidth( iWidth );

	if ( m_eSizeModeVert != SizeMode_Fixed )
		SetHeight( iHeight );
}
