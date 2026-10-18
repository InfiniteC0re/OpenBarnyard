#include "ToshiPCH.h"
#include "PGUIOwnerDraw.h"

#include <TGUI/TGUIManager.h>
#include <TGUI/TGUIWidgetInfo.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT( PGUIOwnerDraw );

TGUIDeserialisable::PropertySet* PGUIOwnerDraw::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* PGUIOwnerDraw::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  PGUIOwnerDraw::ms_pWidgetInfo         = TNULL;

// $JPOG: FUNCTION PGUICore.dll+00011fa0
void PGUIOwnerDraw::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();
	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *PGUIPanel::ms_oPropertySet );

	ms_pWidgetInfo->SetWidgetName( TPString8( "PGUIOwnerDraw" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Owner-Draw Panel" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( PGUIPanel::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TTRUE );

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $JPOG: FUNCTION PGUICore.dll+000122c0
void PGUIOwnerDraw::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// $JPOG: FUNCTION PGUICore.dll+00012590
PGUIOwnerDraw::PGUIOwnerDraw()
    : m_pCallBack( TNULL )
{
}

// The callback isn't copied
// $JPOG: FUNCTION PGUICore.dll+00012460
PGUIOwnerDraw::PGUIOwnerDraw( const PGUIOwnerDraw& a_rcOther )
    : PGUIPanel( a_rcOther )
    , m_pCallBack( TNULL )
{
}

// $JPOG: FUNCTION PGUICore.dll+00012580
PGUIOwnerDraw::~PGUIOwnerDraw()
{
}

// $JPOG: FUNCTION PGUICore.dll+00012520
TBOOL PGUIOwnerDraw::Deserialise( const PProperties* a_pProperties )
{
	return PGUIPanel::Deserialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+000124f0
TBOOL PGUIOwnerDraw::Serialise( PProperties* a_pProperties )
{
	return PGUIPanel::Serialise( a_pProperties );
}

// $JPOG: FUNCTION PGUICore.dll+000125d0
TGUIDeserialisable::PropertySet* PGUIOwnerDraw::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $JPOG: FUNCTION PGUICore.dll+000125c0
TGUIDeserialisable::PropertySet* PGUIOwnerDraw::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

const TGUIWidgetInfo* PGUIOwnerDraw::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $JPOG: FUNCTION PGUICore.dll+00012550
PGUIOwnerDraw* PGUIOwnerDraw::Clone() const
{
	return new PGUIOwnerDraw( *this );
}

// $JPOG: FUNCTION PGUICore.dll+000124d0
void PGUIOwnerDraw::Pack()
{
	if ( m_pCallBack )
		m_pCallBack->Pack( this );

	m_bValidated = TTRUE;
}

// $JPOG: FUNCTION PGUICore.dll+00012380
void PGUIOwnerDraw::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;

	if ( ( m_eSizeModeHoriz != SizeMode_Fixed || m_eSizeModeVert != SizeMode_Fixed ) && m_pCallBack )
		m_pCallBack->Layout( this );
}

// $JPOG: FUNCTION PGUICore.dll+000124b0
void PGUIOwnerDraw::Paint( TGUIDisplayContext* a_pDisplayContext )
{
	if ( m_pCallBack )
		m_pCallBack->Paint( this, a_pDisplayContext );
}

// $JPOG: FUNCTION PGUICore.dll+00012490
PGUIOwnerDraw::CallBack* PGUIOwnerDraw::SetCallBack( CallBack* a_pCallBack )
{
	CallBack* pOldCallBack = m_pCallBack;
	m_pCallBack            = a_pCallBack;

	return pOldCallBack;
}
