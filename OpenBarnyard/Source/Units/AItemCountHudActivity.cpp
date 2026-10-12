#include "pch.h"
#include "AItemCountHudActivity.h"
#include "HUD/AHudManager.h"
#include "HUD/AItemCountHudElement.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b9a10, CreateObject 005b9e90, CreateObjectInPlace 005b9f10
TDEFINE_CLASS( AItemCountHudActivity );

TPSTRING8_DECLARE( inventory );
TPSTRING8_DECLARE( manual );
TPSTRING8_DECLARE( squirt );
TPSTRING8_DECLARE( numleaks );
TPSTRING8_DECLARE( numjunk );
TPSTRING8_DECLARE( numtlparts );
TPSTRING8_DECLARE( numrocks );
TPSTRING8_DECLARE( numfreddyveg );
TPSTRING8_DECLARE( numfreddyfruit );

// NOTE: inlined into CreateObject
AItemCountHudActivity::AItemCountHudActivity()
    : m_uiFlags( FLAGS_NONE )
    , m_iCount( 0 )
    , m_iRequired( -1 )
{
}

// $Barnyard: FUNCTION 005b9a20
// $Barnyard: FUNCTION 005b9f90
AItemCountHudActivity::~AItemCountHudActivity()
{
}

// $Barnyard: FUNCTION 005b9ac0
TBOOL AItemCountHudActivity::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pTexture = a_pProperties->GetProperty( "texture" );

	if ( pTexture )
	{
		m_uiFlags |= FLAGS_TEXTURE;
		m_strTexture = pTexture->GetTPString8();
	}

	const PBPropertyValue* pMode = a_pProperties->GetProperty( "mode" );

	if ( pMode )
	{
		TPString8 strMode = pMode->GetTPString8();

		if ( strMode == TPS8( inventory ) )
			m_uiFlags |= FLAGS_MODE_INVENTORY;
		else if ( strMode == TPS8( manual ) )
			m_uiFlags |= FLAGS_MODE_MANUAL;
		else if ( strMode == TPS8( squirt ) )
			m_uiFlags |= FLAGS_MODE_SQUIRT;
		else if ( strMode == TPS8( numleaks ) )
			m_uiFlags |= FLAGS_MODE_NUMLEAKS;
		else if ( strMode == TPS8( numjunk ) )
			m_uiFlags |= FLAGS_MODE_NUMJUNK;
		else if ( strMode == TPS8( numtlparts ) )
			m_uiFlags |= FLAGS_MODE_NUMTLPARTS;
		else if ( strMode == TPS8( numrocks ) )
			m_uiFlags |= FLAGS_MODE_NUMROCKS;
		else if ( strMode == TPS8( numfreddyveg ) )
			m_uiFlags |= FLAGS_MODE_NUMFREDDYVEG;
		else if ( strMode == TPS8( numfreddyfruit ) )
			m_uiFlags |= FLAGS_MODE_NUMFREDDYFRUIT;
	}

	const PBPropertyValue* pRequired = a_pProperties->GetProperty( "required" );

	if ( pRequired )
	{
		m_uiFlags |= FLAGS_REQUIRED;
		m_strRequired = pRequired->GetTPString8();
	}

	const PBPropertyValue* pSetRequired = a_pProperties->GetProperty( "setrequired" );

	if ( pSetRequired )
	{
		m_iRequired = TINT( TREINTERPRETCAST( TUINTPTR, pSetRequired->GetRaw() ) );
		m_uiFlags |= FLAGS_SETREQUIRED;
	}

	const PBPropertyValue* pSetCount = a_pProperties->GetProperty( "setcount" );

	if ( pSetCount )
	{
		m_iCount = TINT( TREINTERPRETCAST( TUINTPTR, pSetCount->GetRaw() ) );
		m_uiFlags |= FLAGS_SETCOUNT;
	}
	else
	{
		const PBPropertyValue* pAddCount = a_pProperties->GetProperty( "addcount" );

		// NOTE: retail adds the count even if "addcount" isn't set (0 unless the count was set elsewhere)
		m_uiFlags |= FLAGS_ADDCOUNT;

		if ( pAddCount )
			m_iCount = TINT( TREINTERPRETCAST( TUINTPTR, pAddCount->GetRaw() ) );
	}

	const PBPropertyValue* pSetVisible = a_pProperties->GetProperty( "setvisible" );

	if ( pSetVisible && pSetVisible->GetType() == PBPropertyValue::Type::Bool )
		m_uiFlags |= pSetVisible->GetRaw() ? FLAGS_SHOW : FLAGS_HIDE;

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b9d60
AInteractionActivity::RESULT AItemCountHudActivity::Update( AInteractionSequence* a_pSequence )
{
	AItemCountHudElement* pHud = AHudManager::GetSingleton()->FindElement<AItemCountHudElement>();

	if ( m_uiFlags == FLAGS_NONE )
		return RESULT_DONE;

	if ( m_uiFlags & FLAGS_TEXTURE )
		pHud->SetIcon( m_strTexture );

	if ( m_uiFlags & FLAGS_SETREQUIRED )
		pHud->SetMaxCount( m_iRequired );

	if ( m_uiFlags & FLAGS_SETCOUNT )
		pHud->SetCount( m_iCount, TTRUE );
	else if ( m_uiFlags & FLAGS_ADDCOUNT )
		pHud->AddCount( m_iCount );

	// "mode"
	AItemCountHudElement::MODE eMode = 0;

	if ( m_uiFlags & FLAGS_MODE_MANUAL ) eMode = AItemCountHudElement::MODE_MANUAL;
	else if ( m_uiFlags & FLAGS_MODE_INVENTORY ) eMode = AItemCountHudElement::MODE_INVENTORY;
	else if ( m_uiFlags & FLAGS_MODE_SQUIRT ) eMode = AItemCountHudElement::MODE_SQUIRT;
	else if ( m_uiFlags & FLAGS_MODE_NUMJUNK ) eMode = AItemCountHudElement::MODE_NUMJUNK;
	else if ( m_uiFlags & FLAGS_MODE_NUMTLPARTS ) eMode = AItemCountHudElement::MODE_NUMTLPARTS;
	else if ( m_uiFlags & FLAGS_MODE_NUMROCKS ) eMode = AItemCountHudElement::MODE_NUMROCKS;
	else if ( m_uiFlags & FLAGS_MODE_NUMLEAKS ) eMode = AItemCountHudElement::MODE_NUMLEAKS;
	else if ( m_uiFlags & FLAGS_MODE_NUMFREDDYVEG ) eMode = AItemCountHudElement::MODE_NUMFREDDYVEG;
	else if ( m_uiFlags & FLAGS_MODE_NUMFREDDYFRUIT ) eMode = AItemCountHudElement::MODE_NUMFREDDYFRUIT;

	if ( eMode != 0 )
		pHud->SetMode( eMode );

	if ( m_uiFlags & FLAGS_REQUIRED )
		pHud->SetItemName( m_strRequired );

	if ( m_uiFlags & FLAGS_SHOW )
		pHud->SetVisible( TTRUE );
	else if ( m_uiFlags & FLAGS_HIDE )
		pHud->SetVisibleForce( TFALSE );


	return RESULT_DONE;
}
