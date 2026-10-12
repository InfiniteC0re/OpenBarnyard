#include "pch.h"
#include "AObjectTraits.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "AVisibleUnit.h"
#include "AInteractionComponent.h"
#include "AInteractionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0041a7d0, CreateObject 0041aea0, CreateObjectInPlace 0041af00
TDEFINE_CLASS( AObjectTraits );

TPSTRING8_DECLARE( Heavy );
TPSTRING8_DECLARE( ObjectUnEquip );
TPSTRING8_DECLARE( Detector );
TPSTRING8_DECLARE( AutoPickup );
TPSTRING8_DECLARE( Obstruction );
TPSTRING8_DECLARE( WhenThrow );
TPSTRING8_DECLARE( WhenUse );
TPSTRING8_DECLARE( WhenArea );
TPSTRING8_DECLARE( WhenEquip );
TPSTRING8_DECLARE( WhenBreak );
TPSTRING8_DECLARE( Restitution );
TPSTRING8_DECLARE( TangentialRestitution );
TPSTRING8_DECLARE( InventoryForUse );
TPSTRING8_DECLARE( InventoryForStore );
TPSTRING8_DECLARE( ItemScale );
TPSTRING8_DECLARE( HoldOffset );
TPSTRING8_DECLARE( HoldScale );
TPSTRING8_DECLARE( MaxCarry );
TPSTRING8_DECLARE( ItemOffset );
TPSTRING8_DECLARE( ItemFacing );
TPSTRING8_DECLARE( ItemOffsetHud );
TPSTRING8_DECLARE( HoldRotate );
TPSTRING8_DECLARE( Strength );
TPSTRING8_DECLARE( UpDirection );
TPSTRING8_DECLARE( BaseThrowSpeed );
TPSTRING8_DECLARE( CanSpin );
TPSTRING8_DECLARE( Destroy );
TPSTRING8_DECLARE( Drop );
TPSTRING8_DECLARE( AreaName );
TPSTRING8_DECLARE( BreakWhenThrown );
TPSTRING8_DECLARE( ObjectThrow );
TPSTRING8_DECLARE( ObjectUse );
TPSTRING8_DECLARE( ObjectAreaUse );
TPSTRING8_DECLARE( ObjectEquip );
TPSTRING8_DECLARE( ObjectBreak );

// Reads an array of 3 or 4 numbers into a_rOutVector (w defaults to 1.0f).
// NOTE: retail has this as a PBProperties method (006b2110, reading through 006b1dc0)
static TBOOL GetOptionalVector4( const PBProperties* a_pProperties, TVector4& a_rOutVector, const TCHAR* a_szName )
{
	const PBPropertyValue* pValue = a_pProperties->GetProperty( a_szName );

	if ( !pValue || pValue->GetType() != PBPropertyValue::Type::Array )
		return TFALSE;

	const PBPropertyValueArray* pArray = pValue->GetArray();

	if ( pArray->GetSize() != 3 && pArray->GetSize() != 4 )
		return TFALSE;

	for ( TUINT32 i = 0; i < pArray->GetSize(); i++ )
	{
		// NOTE: retail checks the type of the first value on every iteration
		if ( pArray->GetValue( 0 )->GetType() != PBPropertyValue::Type::Float )
			return TFALSE;
	}

	// Anything that isn't a float is read as an integer
	auto fnGetFloat = []( const PBPropertyValue* a_pValue ) -> TFLOAT {
		if ( a_pValue->GetType() == PBPropertyValue::Type::Float )
			return a_pValue->GetFloat();

		return TFLOAT( TINT( TREINTERPRETCAST( TUINTPTR, a_pValue->GetRaw() ) ) );
	};

	a_rOutVector.x = fnGetFloat( pArray->GetValue( 0 ) );
	a_rOutVector.y = fnGetFloat( pArray->GetValue( 1 ) );
	a_rOutVector.z = fnGetFloat( pArray->GetValue( 2 ) );
	a_rOutVector.w = 1.0f;

	if ( pArray->GetSize() == 4 )
		a_rOutVector.w = fnGetFloat( pArray->GetValue( 3 ) );

	return TTRUE;
}

// Creates a trigger of type T from the sub-block a_szName, if there is one.
// $Barnyard: FUNCTION 0041a860
// $Barnyard: FUNCTION 0041aa20
// $Barnyard: FUNCTION 0041ae30
// $Barnyard: FUNCTION 0041a8f0
// $Barnyard: FUNCTION 0041aa90
template <class T>
static TBOOL CreateTrigger( const PBProperties* a_pProperties, const TCHAR* a_szName, T*& a_rpOutTrigger )
{
	const PBPropertyValue* pValue = a_pProperties->GetProperty( a_szName );

	if ( pValue && pValue->GetRaw() )
	{
		a_rpOutTrigger = new T();
		a_rpOutTrigger->Create( TSTATICCAST( const PBProperties, pValue->GetRaw() ) );
	}

	return TTRUE;
}

// Starts the interaction a_rcInteraction with a_pObject as the initiator.
// $Barnyard: FUNCTION 0041ab00
static TBOOL AssignObjectInteraction( const TPString8& a_rcInteraction, AUnit* a_pUser, AUnit* a_pObject )
{
	AInteractionManager* pInteractionManager = AInteractionManager::GetSingleton();

	if ( !AInteractionComponent::IsUnitSuitableForInteraction( a_pObject, TTRUE ) )
		return TFALSE;

	AUnit* apUnits[ 2 ] = { a_pObject, a_pUser };

	return pInteractionManager->AssignInteractionToUnits(
	           apUnits, TARRAYSIZE( apUnits ), a_rcInteraction,
	           AInteractionManager::ASSIGNFLAGS_FIRST_IS_INITIATOR | AInteractionManager::ASSIGNFLAGS_FORCED
	       ) != TNULL;
}

// $Barnyard: FUNCTION 0041a7a0
// $Barnyard: FUNCTION 0041a7b0
// $Barnyard: FUNCTION 0041a9b0
AObjectTraits::Trigger::~Trigger()
{
	// NOTE: 0041a9b0 is the deleting destructor every trigger subclass uses
}

// $Barnyard: FUNCTION 0041a7e0
TBOOL AObjectTraits::ThrowTrigger::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_fStrength, TPS8( Strength ) );
	a_pProperties->GetOptionalPropertyValue( m_fUpDirection, TPS8( UpDirection ) );
	a_pProperties->GetOptionalPropertyValue( m_fBaseThrowSpeed, TPS8( BaseThrowSpeed ) );
	a_pProperties->GetOptionalPropertyValue( m_bCanSpin, TPS8( CanSpin ) );
	a_pProperties->GetOptionalPropertyValue( m_bDestroy, TPS8( Destroy ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 0041ab70
TBOOL AObjectTraits::ThrowTrigger::Execute( AUnit* a_pUser, AUnit* a_pObject )
{
	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, a_pObject->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );
	TFLOAT         fScale  = pTraits->m_fHoldScale;

	TSTATICCAST( AVisibleUnit, a_pObject )->GetModelInstance()->SetScale( TVector4( fScale, fScale, fScale, 1.0f ) );

	return AssignObjectInteraction( TPS8( ObjectThrow ), a_pUser, a_pObject );
}

// $Barnyard: FUNCTION 0041a960
TBOOL AObjectTraits::UseTrigger::Create( const PBProperties* a_pProperties )
{
	TBOOL bDrop = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bDrop, TPS8( Drop ) );

	if ( bDrop )
		m_eFlags |= FLAGS_DROP;
	else
		m_eFlags &= ~FLAGS_DROP;

	return TTRUE;
}

// $Barnyard: FUNCTION 0041abc0
TBOOL AObjectTraits::UseTrigger::Execute( AUnit* a_pUser, AUnit* a_pObject )
{
	return AssignObjectInteraction( TPS8( ObjectUse ), a_pUser, a_pObject );
}

// $Barnyard: FUNCTION 0041a840
TBOOL AObjectTraits::AreaTrigger::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strAreaName, TPS8( AreaName ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 0041ac60
TBOOL AObjectTraits::AreaTrigger::Execute( AUnit* a_pUser, AUnit* a_pObject )
{
	return AssignObjectInteraction( TPS8( ObjectAreaUse ), a_pUser, a_pObject );
}

// $Barnyard: FUNCTION 0041ac80
TBOOL AObjectTraits::EquipTrigger::Execute( AUnit* a_pUser, AUnit* a_pObject )
{
	return AssignObjectInteraction( TPS8( ObjectEquip ), a_pUser, a_pObject );
}

// $Barnyard: FUNCTION 0041aca0
TBOOL AObjectTraits::EquipTrigger::UnEquip( AUnit* a_pUser, AUnit* a_pObject )
{
	return AssignObjectInteraction( TPS8( ObjectUnEquip ), a_pUser, a_pObject );
}

// $Barnyard: FUNCTION 0041a9d0
TBOOL AObjectTraits::BreakTrigger::Create( const PBProperties* a_pProperties )
{
	TBOOL bBreakWhenThrown = TTRUE;
	a_pProperties->GetOptionalPropertyValue( bBreakWhenThrown, TPS8( BreakWhenThrown ) );

	if ( bBreakWhenThrown )
		m_eFlags |= FLAGS_BREAK_WHEN_THROWN;
	else
		m_eFlags &= ~FLAGS_BREAK_WHEN_THROWN;

	return TTRUE;
}

// $Barnyard: FUNCTION 0041acc0
TBOOL AObjectTraits::BreakTrigger::Execute( AUnit* a_pUser, AUnit* a_pObject )
{
	return AssignObjectInteraction( TPS8( ObjectBreak ), a_pUser, a_pObject );
}

// $Barnyard: FUNCTION 0041ace0
AObjectTraits::AObjectTraits()
    : m_eFlags( FLAGS_NONE )
    , m_pWhenThrow( TNULL )
    , m_pWhenUse( TNULL )
    , m_pWhenArea( TNULL )
    , m_pWhenEquip( TNULL )
    , m_pWhenBreak( TNULL )
    , m_vecHoldOffset( 0.0f, 0.0f, 0.0f, 1.0f )
    , m_fHoldScale( 1.0f )
    , m_fItemScale( 1.0f )
    , m_fRestitution( 0.5f )
    , m_fTangentialRestitution( 0.5f )
    , m_iMaxCarry( 1 )
{
}

// $Barnyard: FUNCTION 0041ad50
// $Barnyard: FUNCTION 0041af40
AObjectTraits::~AObjectTraits()
{
	if ( m_pWhenThrow )
		delete m_pWhenThrow;

	if ( m_pWhenUse )
		delete m_pWhenUse;

	if ( m_pWhenArea )
		delete m_pWhenArea;

	if ( m_pWhenEquip )
		delete m_pWhenEquip;

	if ( m_pWhenBreak )
		delete m_pWhenBreak;
}

// $Barnyard: FUNCTION 0041af60
TBOOL AObjectTraits::Create( const PBProperties* a_pProperties )
{
	TBOOL bHeavy = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bHeavy, TPS8( Heavy ) );

	if ( bHeavy )
		m_eFlags |= FLAGS_HEAVY;
	else
		m_eFlags &= ~FLAGS_HEAVY;

	TBOOL bDetector = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bDetector, TPS8( Detector ) );

	if ( bDetector )
		m_eFlags |= FLAGS_DETECTOR;
	else
		m_eFlags &= ~FLAGS_DETECTOR;

	TBOOL bAutoPickup = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bAutoPickup, TPS8( AutoPickup ) );

	if ( bAutoPickup )
		m_eFlags |= FLAGS_AUTOPICKUP;
	else
		m_eFlags &= ~FLAGS_AUTOPICKUP;

	TBOOL bObstruction = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bObstruction, TPS8( Obstruction ) );

	if ( bObstruction )
		m_eFlags |= FLAGS_OBSTRUCTION;
	else
		m_eFlags &= ~FLAGS_OBSTRUCTION;

	CreateTrigger( a_pProperties, TPS8( WhenThrow ), m_pWhenThrow );
	CreateTrigger( a_pProperties, TPS8( WhenUse ), m_pWhenUse );
	CreateTrigger( a_pProperties, TPS8( WhenArea ), m_pWhenArea );
	CreateTrigger( a_pProperties, TPS8( WhenEquip ), m_pWhenEquip );
	CreateTrigger( a_pProperties, TPS8( WhenBreak ), m_pWhenBreak );

	// NOTE: retail uses the TPString8 overload at 006b1f50, same as GetOptionalFloatOrIntValue
	a_pProperties->GetOptionalFloatOrIntValue( m_fRestitution, TPS8( Restitution ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fTangentialRestitution, TPS8( TangentialRestitution ) );
	a_pProperties->GetOptionalPropertyValue( m_strInventoryForUse, TPS8( InventoryForUse ) );
	a_pProperties->GetOptionalPropertyValue( m_strInventoryForStore, TPS8( InventoryForStore ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fItemScale, TPS8( ItemScale ) );
	GetOptionalVector4( a_pProperties, m_vecHoldOffset, TPS8( HoldOffset ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fHoldScale, TPS8( HoldScale ) );
	a_pProperties->GetOptionalPropertyValue( m_iMaxCarry, TPS8( MaxCarry ) );

	m_vecItemOffset = TVector4::VEC_ZERO;
	GetOptionalVector4( a_pProperties, m_vecItemOffset, TPS8( ItemOffset ) );

	m_vecItemFacing = TVector4::VEC_POSX;
	GetOptionalVector4( a_pProperties, m_vecItemFacing, TPS8( ItemFacing ) );

	m_vecItemOffsetHud = TVector4::VEC_ZERO;
	GetOptionalVector4( a_pProperties, m_vecItemOffsetHud, TPS8( ItemOffsetHud ) );

	m_vecHoldRotate = TVector4::VEC_ZERO;
	GetOptionalVector4( a_pProperties, m_vecHoldRotate, TPS8( HoldRotate ) );

	return TTRUE;
}
