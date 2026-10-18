#include "ToshiPCH.h"
#include "PGUIDiscMenuDialog.h"
#include "PGUIDiscLayout.h"
#include "PGUIMenuTree.h"
#include "PGUIMenuTreeItem.h"
#include "PGUICore.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>
#include <TGUI/TGUIStyle.h>
#include <TGUI/TGUIScreen.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIDiscMenuDialog );

TGUIDeserialisable::PropertySet* PGUIDiscMenuDialog::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIDiscMenuDialog::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIDiscMenuDialog::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+000050b0
void PGUIDiscMenuDialog::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIMenuDialog::ms_oPropertySet );

	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIDiscMenuDialog" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Disc Menu Dialog" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIMenuDialog::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+000053d0
void PGUIDiscMenuDialog::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00005b80
PGUIDiscMenuDialog::PGUIDiscMenuDialog()
    : m_iUnknown( 0 )
{
}

// $JPOG: FUNCTION PGUICore.dll+00005a20
PGUIDiscMenuDialog::PGUIDiscMenuDialog( const PGUIDiscMenuDialog& a_rcOther )
    : PGUIMenuDialog( a_rcOther )
    , m_iUnknown( 0 )
{
}

// $JPOG: FUNCTION PGUICore.dll+00005b70
PGUIDiscMenuDialog::~PGUIDiscMenuDialog()
{
}

// $JPOG: FUNCTION PGUICore.dll+00005b10
TBOOL PGUIDiscMenuDialog::Deserialise( const PProperties* a_pProperties )
{
	return PGUIMenuDialog::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00005ae0
TBOOL PGUIDiscMenuDialog::Serialise( PProperties* a_pProperties )
{
	return PGUIMenuDialog::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+00005bc0
TGUIDeserialisable::PropertySet* PGUIDiscMenuDialog::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+00005bb0
TGUIDeserialisable::PropertySet* PGUIDiscMenuDialog::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIDiscMenuDialog::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00005b40
PGUIDiscMenuDialog* PGUIDiscMenuDialog::Clone() const
{
	return new PGUIDiscMenuDialog( *this );
}

// The navigation actions are swallowed
// $JPOG: FUNCTION PGUICore.dll+00005a80
TBOOL PGUIDiscMenuDialog::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	return ( a_iActions & 0x1E00 ) || PGUIMenuDialog::OnButtonPress( a_pEvent, a_iActions ) == TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00005a50
TBOOL PGUIDiscMenuDialog::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	return ( a_iActions & 0x1E00 ) || PGUIMenuDialog::OnButtonRelease( a_pEvent, a_iActions );
}

// The stick (pushed far enough, or with an item already selected) selects the item of the slot in its direction
// $JPOG: FUNCTION PGUICore.dll+00005490
TBOOL PGUIDiscMenuDialog::OnAxisMove( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	TINT iDoodad = a_pEvent->GetDoodad();

	if ( iDoodad != 0x10019 && iDoodad != 0x1001B && iDoodad != 0x20001 )
		return TGUIWidget::OnAxisMove( a_pEvent, a_iActions );

	PGUIDiscLayout* pLayout = TSTATICCAST( PGUIDiscLayout, m_pMenuBucket->GetLayoutManager() );

	TVector2 vecStick( a_pEvent->GetMagnitudeFloat( 0 ), a_pEvent->GetMagnitudeFloat( 1 ) );
	TFLOAT   fMagnitude = vecStick.Magnitude();

	if ( m_pSelectedItem && fMagnitude < 0.5f )
	{
		m_iUnknown = 0;
		return TTRUE;
	}

	if ( !m_pSelectedItem && fMagnitude < 0.75f )
		return TTRUE;

	TFLOAT fAngle = TMath::ATan2( a_pEvent->GetMagnitudeFloat( 1 ), a_pEvent->GetMagnitudeFloat( 0 ) );

	if ( fAngle < 0.0f )
		fAngle += TMath::TWO_PI;

	TFLOAT fOffsetAngle = pLayout->GetOffsetAngle();
	TMath::NormaliseAngle( fOffsetAngle );

	if ( fOffsetAngle < 0.0f )
		fOffsetAngle += TMath::TWO_PI;

	TFLOAT fNumSlots  = TFLOAT( ( pLayout->GetSlotCount() > 0 ) ? pLayout->GetSlotCount() : TINT( m_pMenuBucket->GetChildCount() ) );
	TFLOAT fSlotAngle = TMath::TWO_PI / fNumSlots;

	if ( pLayout->GetFlowDirection() == PGUIDiscLayout::FlowDirection_AntiClockwise )
		fSlotAngle = -fSlotAngle;

	TFLOAT fRelativeAngle = fAngle - fOffsetAngle + fSlotAngle * 0.5f;
	TMath::NormaliseAngle( fRelativeAngle );

	if ( fRelativeAngle < 0.0f )
		fRelativeAngle += TMath::TWO_PI;

	TINT iSlot = TINT( fRelativeAngle / fSlotAngle );

	if ( iSlot >= 0 && iSlot < m_pMenuBucket->GetChildCount() )
	{
		PGUIMenuTreeItem* pItem = GetItemAtIndex( iSlot );

		if ( ( !m_pSelectedItem || m_pSelectedItem != pItem ) && pItem->CanSelect() )
		{
			PGUIPlaySound( GetScreen(), "PGUIMenuTree", "navigate" );
			m_pMenuTree->SetSelectedItem( pItem );
		}
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00005ad0
void PGUIDiscMenuDialog::Pack()
{
	PGUIMenuDialog::Pack();
}

// $JPOG: FUNCTION PGUICore.dll+00005ab0
void PGUIDiscMenuDialog::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	PGUIMenuDialog::Layout( a_iMaxWidth, a_iMaxHeight );
}

// $JPOG: FUNCTION PGUICore.dll+00005790
void PGUIDiscMenuDialog::UpdatePosition( TINT& a_rX, TINT& a_rY )
{
	TINT iScreenWidth  = GetScreen()->GetWidth();
	TINT iScreenHeight = GetScreen()->GetHeight();
	TINT iX            = a_rX;
	TINT iY            = a_rY;
	TINT iHalfWidth    = m_iWidth / 2;

	if ( m_pMenuTree->GetScreenMarginLeft() > iX - iHalfWidth )
		iX = m_pMenuTree->GetScreenMarginLeft() + iHalfWidth;

	TINT iRight = iScreenWidth - m_pMenuTree->GetScreenMarginRight();

	if ( iHalfWidth + iX > iRight )
		iX = iRight - iHalfWidth;

	TINT iHalfHeight = m_iHeight / 2;
	TINT iBottom     = iScreenHeight - m_pMenuTree->GetScreenMarginBottom();

	if ( iHalfHeight + iY > iBottom )
		iY = iBottom - iHalfHeight;

	if ( m_pMenuTree->GetScreenMarginTop() > iY - iHalfHeight )
		iY = m_pMenuTree->GetScreenMarginTop() + iHalfHeight;

	if ( m_eDisplayMode == PGUIMenuTreeItemStyle::DisplayLevel_Next )
	{
		SetPosition( iX - iHalfWidth, iY - iHalfHeight );
	}
	else if ( m_eDisplayMode == PGUIMenuTreeItemStyle::DisplayLevel_Current )
	{
		SetPosition( iX - iHalfWidth, iY - iHalfHeight );

		TINT iButtonX, iButtonY;
		m_pSelectedButton->TransformWidgetToDevice( 0, 0, iButtonX, iButtonY );

		a_rX = iButtonX - m_pSelectedButton->GetWidth() / 2;
		a_rY = iButtonY - m_pSelectedButton->GetHeight() / 2;
	}
}

// $JPOG: FUNCTION PGUICore.dll+000058f0
void PGUIDiscMenuDialog::UpdateSelection()
{
	if ( !m_pSelectedButton )
		return;

	m_pSelectedButton->SetState( State_Focused, ( m_uiState & State_Focused ) ? TUINT( State_Focused ) : 0u );
	m_pSelectedButton->SetStyle( m_pSelectedItem->GetStyle()->GetSelectedItemStyle( m_eDisplayMode ) );
	m_pSelectedButton->Layout( -1, -1 );
	m_pSelectedButton->Pack();
}
