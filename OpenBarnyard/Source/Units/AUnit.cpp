#include "pch.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "AUnitComponent.h"
#include "AUnitComponentManager.h"
#include "AInteractionComponent.h"
#include "AInteractionComponentData.h"
#include "AInteractionManager.h"
#include "AInteraction.h"
#include "Render/AShadowManager.h"
#include "AUnitTransform.h"
#include "AKinematicsObstructionUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerPool.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ff6f0, CreateObject 00413ef0, CreateObjectInPlace 00413f50
TDEFINE_CLASS_INIT( AUnit, 2, 0 );

AFunctionTable AUnit::ms_oFunctionTable( &TGetClass( AUnit ), TNULL );

// FSM fallback: resolves the unit an AUnit native is called on.
// $Barnyard: FUNCTION 00412f40
static TObject* __stdcall AUnitFunctionFallback( void* a_pUnk1, const AFunctionCaller* a_pCaller )
{
	return a_pCaller ? a_pCaller->GetObject() : TNULL;
}

// $Barnyard: FUNCTION 00413f90
void AUnit::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AUnitFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AUnit>::Create( "DoesHaveUnitCountIdentifier", &AUnit::DoesHaveUnitCountIdentifier ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TUINT, AUnit>::Create( "GetUnitCountIdentifier", &AUnit::GetUnitCountIdentifier ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TUINT, AUnit>::Create( "GetUnitCountMaximum", &AUnit::GetUnitCountMaximum ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TBOOL, AUnit>::Create( "HasInteraction", &AUnit::HasInteraction ) );
	ms_oFunctionTable.AddFunction( AMethodPointer3ArgsRT<TBOOL, AUnit, AUnit*, TFLOAT, TFLOAT>::Create( "CanPerceiveUnit", &AUnit::CanPerceiveUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer3ArgsRT<TBOOL, AUnit, AUnit*, TFLOAT, TFLOAT>::Create( "CanPerceiveUnitMe", &AUnit::CanPerceiveUnitMe ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TFLOAT, AUnit, AUnit*>::Create( "GetDistanceSqToUnit", &AUnit::GetDistanceSqToUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TBOOL, AUnit, TBOOL>::Create( "SetMovement", &AUnit::SetMovement ) );
}

// $Barnyard: FUNCTION 00412f60
void AUnit::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 00413860
AUnit::AUnit()
    : m_pObstruction( TNULL )
    , m_eFlags( FLAGS_NONE )
    , m_pSteer( TNULL )
    , m_bUnk20( TTRUE )
    , m_pType( TNULL )
    , m_pHashNode( TNULL )
    , m_iUnk34( 1 )
    , m_uiQueryStamp( 0 )
    , m_iUnk3c( 0 )
{
	m_pComponents = new T2SList<AUnitComponent>();
	m_eFlags |= FLAGS_HASHED;
}

// $Barnyard: FUNCTION 00414090
// $Barnyard: FUNCTION 00413950
AUnit::~AUnit()
{
	if ( m_pComponents )
	{
		m_pComponents->Clear();
		delete m_pComponents;
	}

	// NOTE: m_oSelfProxy releases here via TRef's destructor - it drops this
	// unit's reference on its AUIDMap proxy and recycles it if it was the last.
}

// $Barnyard: FUNCTION 0040d8f0
TRef<AUnitProxy> AUnit::GetSelfProxy() const
{
	return m_oSelfProxy;
}

// $Barnyard: FUNCTION 00413900
AFunctionTable* AUnit::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00413bd0
TBOOL AUnit::SetMovement( TBOOL a_bEnabled )
{
	TBOOL bWasEnabled = ( m_eFlags & FLAGS_THINK_ENABLED ) != 0;

	if ( bWasEnabled != a_bEnabled )
	{
		if ( a_bEnabled == TTRUE )
			m_eFlags |= FLAGS_THINK_ENABLED;
		else
			m_eFlags &= ~FLAGS_THINK_ENABLED;
	}

	return bWasEnabled;
}

// $Barnyard: FUNCTION 00413940
void AUnit::Update( TFLOAT a_fDeltaTime )
{
	UpdateComponents( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00413a90
const ASteerTraits* AUnit::GetSteerTraits()
{
	AUnitStaticData* pTraits = m_pType->FindStaticData( &TGetClass( ASteerTraits ) );

	// Units whose type has no traits share a default-constructed instance
	static ASteerTraits s_oDefaultTraits;

	return pTraits ? TSTATICCAST( ASteerTraits, pTraits ) : &s_oDefaultTraits;
}

// $Barnyard: FUNCTION 004131f0
void AUnit::OnEnterSpecialPathArea()
{
	if ( m_pSteer->IsA( &TGetClass( AInteriorSteer ) ) )
		m_pSteer->Command_CancelCurrentCommand();
}

// $Barnyard: FUNCTION 00412fb0
TINT AUnit::SetUnk34( TINT a_iValue )
{
	TINT iOldValue = m_iUnk34;
	m_iUnk34       = a_iValue;

	return iOldValue;
}

// $Barnyard: FUNCTION 00412fa0
TUINT AUnit::GetUnitCountMaximum()
{
	return m_pType->GetInstanceCount();
}

// $Barnyard: FUNCTION 004139e0
TBOOL AUnit::Create( AUnitTransform* a_pTransform )
{
	Unk_0x84();
	m_pSteer = ASteerPool::GetSingleton()->CreateSteer( GetSteerClass() );

	if ( !m_pSteer->Create( this, a_pTransform->m_vecPosition, a_pTransform->m_vecDirection, GetSimLOD() ) )
	{
		ASteerPool::GetSingleton()->DestroySteer( m_pSteer );
		m_pSteer = TNULL;

		return TFALSE;
	}

	for ( auto it = m_pType->GetStaticData().Begin(); it != m_pType->GetStaticData().End(); it++ )
		it->OnUnitCreate( this );

	return TTRUE;
}

// $Barnyard: FUNCTION 004140b0
void AUnit::OnDestroy()
{
	for ( auto it = m_pType->GetStaticData().Begin(); it != m_pType->GetStaticData().End(); it++ )
		it->OnUnitDestroy( this );

	m_eFlags |= FLAGS_DYING;
	DestroyComponents();

	if ( m_eFlags & FLAGS_AI_ENABLED )
		m_eFlags &= ~FLAGS_AI_ENABLED;

	SetMovement( TFALSE );

	if ( m_eFlags & FLAGS_EXTERNAL_UPDATE )
		m_eFlags &= ~FLAGS_EXTERNAL_UPDATE;

	if ( m_pSteer )
	{
		ASteerPool::GetSingleton()->DestroySteer( m_pSteer );
		m_pSteer = TNULL;
	}

	if ( m_pObstruction )
	{
		delete m_pObstruction;
		m_pObstruction = TNULL;
	}

	// NOTE: retail inlines KillAttachedEffects here
	KillAttachedEffects();
}

// $Barnyard: FUNCTION 00413db0
void AUnit::UpdateAI( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
	{
		if ( it->IsAIEnabled() == TTRUE )
			it->UpdateAI( a_fDeltaTime );
	}
}

// $Barnyard: FUNCTION 00413e10
void AUnit::UpdateComponents( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
		it->Update( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00412f90
const TWCHAR* AUnit::GetScreenName()
{
	return m_pType->GetScreenName();
}

// $Barnyard: FUNCTION 004141a0
Toshi::TClass* AUnit::GetSteerClass()
{
	TPString8 strSteer = m_pType->GetSteerName();

	// Units without a (known) steer stand still
	TClass* pClass = &TGetClass( AStationarySteer );

	if ( strSteer.GetPooledString() && strSteer.GetString8().Length() != 0 )
	{
		TClass* pFound = const_cast<TClass*>( TClass::Find( strSteer.GetString(), &TGetClass( ASteer ) ) );

		if ( pFound )
			pClass = pFound;
	}

	return pClass;
}

// $Barnyard: FUNCTION 00413140
TBOOL AUnit::ChangeSteer( Toshi::TClass* a_pSteerClass )
{
	ASteer* pNewSteer = ASteerPool::GetSingleton()->CreateSteer( a_pSteerClass );

	// The new steer takes over from the old one
	if ( pNewSteer->CreateFrom( m_pSteer, GetSimLOD() ) == TTRUE )
	{
		ASteerPool::GetSingleton()->DestroySteer( m_pSteer );
		m_pSteer = pNewSteer;

		return TTRUE;
	}

	// NOTE: retail leaks pNewSteer here
	return TFALSE;
}

// $Barnyard: FUNCTION 00412f70
void AUnit::SetSteerPosition( const TVector4& a_rcPosition )
{
	m_pSteer->SetPosition( a_rcPosition );
}

// $Barnyard: FUNCTION 00412f80
void AUnit::SetSteerDirection( const TVector4& a_rcDirection )
{
	m_pSteer->SetDirection( a_rcDirection );
}

// $Barnyard: FUNCTION 00412ff0
TBOOL AUnit::CanPerceiveUnit( AUnit* a_pOther, TFLOAT a_fRange, TFLOAT a_fMinDot )
{
	TVector4 vecPos      = *m_pSteer->GetPosition();
	TVector4 vecOtherPos = *a_pOther->m_pSteer->GetPosition();

	if ( a_fRange * a_fRange > TVector4::DistanceSq( vecOtherPos, vecPos ) )
	{
		// NOTE: retail dots this steer's direction with itself; the direction to
		// a_pOther is never used, so for unit vectors this is just 1.0f > a_fMinDot.
		const TVector4* pDir1 = m_pSteer->GetDirection();
		const TVector4* pDir2 = m_pSteer->GetDirection();

		if ( TVector4::DotProduct3( *pDir2, *pDir1 ) > a_fMinDot )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 004130b0
TBOOL AUnit::CanPerceiveUnitMe( AUnit* a_pOther, TFLOAT a_fRange, TFLOAT a_fMinDot )
{
	return a_pOther->CanPerceiveUnit( this, a_fRange, a_fMinDot );
}

// $Barnyard: FUNCTION 00412fc0
TFLOAT AUnit::GetDistanceSqToUnit( AUnit* a_pOther )
{
	// NOTE: this unit's steer position is fetched first
	const TVector4* pPos = m_pSteer->GetPosition();

	return TVector4::DistanceSq( *a_pOther->m_pSteer->GetPosition(), *pPos );
}

// $Barnyard: FUNCTION 00413e70
TBOOL AUnit::HasInteraction() const
{
	return ( m_eFlags & FLAGS_HAS_INTERACTION ) != 0;
}

// $Barnyard: FUNCTION 00414240
void AUnit::SetHasInteraction( TINT a_iValue )
{
	if ( HasInteraction() == ( a_iValue != 0 ) )
		return;

	if ( a_iValue != 0 )
		m_eFlags |= FLAGS_HAS_INTERACTION;
	else
		m_eFlags &= ~FLAGS_HAS_INTERACTION;

	m_pSteer->SetHasInteraction( a_iValue );

	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
		it->OnHasInteractionChanged( a_iValue );
}

// $Barnyard: FUNCTION 00413e80
TBOOL AUnit::CanInteract()
{
	if ( !m_pType->FindStaticData( &TGetClass( AInteractionComponentData ) ) )
		return TFALSE;

	if ( !AInteractionManager::GetSingleton()->CanAssignInteraction() )
		return TFALSE;

	AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, FindComponent( &TGetClass( AInteractionComponent ) ) );

	// NOTE: retail doesn't check pComponent for TNULL
	if ( pComponent->IsInteractionsDisabled() )
		return TFALSE;

	if ( !HasInteraction() )
		return TTRUE;

	// Units can leave interactions that can be interrupted
	if ( pComponent->GetSequence() )
		return pComponent->GetSequence()->GetInteraction()->IsInterruptible();

	return TFALSE;
}

TPSTRING8_DECLARE( player );

// $Barnyard: FUNCTION 004131d0
TBOOL AUnit::IsPlayer() const
{
	return m_pType->GetName() == TPS8( player );
}

// $Barnyard: FUNCTION 00413190
void AUnit::AddComponent( AUnitComponent* a_pComponent )
{
	a_pComponent->SetOwner( this );
	m_pComponents->PushBack( a_pComponent );
	OnComponentAdded( a_pComponent );
}

// $Barnyard: FUNCTION 00413cd0
AUnitComponent* AUnit::FindComponent( Toshi::TClass* a_pClass )
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
	{
		if ( it->GetClass()->IsA( a_pClass ) )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00413d40
AUnitComponent* AUnit::FindComponentExact( Toshi::TClass* a_pClass )
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
	{
		if ( it->GetClass() == a_pClass )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00413c30
void AUnit::DestroyComponents()
{
	for ( auto it = m_pComponents->Begin(); it != m_pComponents->End(); it++ )
		OnComponentRemoved( it );

	while ( !m_pComponents->IsEmpty() )
	{
		AUnitComponent*        pComponent = m_pComponents->PopFront();
		AUnitComponentManager* pManager   = pComponent->GetManager();

		if ( pManager )
			pManager->ReleaseComponent( pComponent );
		else
			delete pComponent;
	}
}

// $Barnyard: FUNCTION 00413130
void AUnit::KillAttachedEffects()
{
	// NOTE: retail calls this unconditionally; OpenBarnyard doesn't create the
	// shadow manager yet (retail only creates it through its TClass)
	if ( AShadowManager::IsSingletonCreated() )
		AShadowManager::GetSingleton()->RemoveShadow( this );
}

// $Barnyard: FUNCTION 00413b10
void AUnit::SetNoQuery( TBOOL a_bNoQuery )
{
	if ( ( ( m_eFlags & FLAGS_NO_QUERY ) != 0 ) != a_bNoQuery )
	{
		if ( a_bNoQuery == TTRUE )
			m_eFlags |= FLAGS_NO_QUERY;
		else
			m_eFlags &= ~FLAGS_NO_QUERY;
	}
}

// $Barnyard: FUNCTION 00413b40
void AUnit::SetHashed( TBOOL a_bHashed )
{
	if ( ( ( m_eFlags & FLAGS_HASHED ) != 0 ) != a_bHashed )
	{
		if ( a_bHashed == TTRUE )
			m_eFlags |= FLAGS_HASHED;
		else
			m_eFlags &= ~FLAGS_HASHED;
	}
}

// $Barnyard: FUNCTION 00413ba0
void AUnit::SetAIEnabled( TBOOL a_bEnabled )
{
	if ( ( ( m_eFlags & FLAGS_AI_ENABLED ) != 0 ) != a_bEnabled )
	{
		if ( a_bEnabled == TTRUE )
			m_eFlags |= FLAGS_AI_ENABLED;
		else
			m_eFlags &= ~FLAGS_AI_ENABLED;
	}
}

// $Barnyard: FUNCTION 004150f0
void AUnit::SetObstructionFlags( TUINT8 a_uiFlags )
{
	TSTATICCAST( AKinematicsObstructionUnit, m_pObstruction )->SetFlags( a_uiFlags, TTRUE );
}

// $Barnyard: FUNCTION 00413c00
void AUnit::SetExternalUpdate( TBOOL a_bExternal )
{
	if ( ( ( m_eFlags & FLAGS_EXTERNAL_UPDATE ) != 0 ) != a_bExternal )
	{
		if ( a_bExternal == TTRUE )
			m_eFlags |= FLAGS_EXTERNAL_UPDATE;
		else
			m_eFlags &= ~FLAGS_EXTERNAL_UPDATE;
	}
}

// $Barnyard: FUNCTION 00413b70
void AUnit::SetPendingRemoval( TBOOL a_bPending )
{
	if ( ( ( m_eFlags & FLAGS_PENDING_REMOVAL ) != 0 ) != a_bPending )
	{
		if ( a_bPending == TTRUE )
			m_eFlags |= FLAGS_PENDING_REMOVAL;
		else
			m_eFlags &= ~FLAGS_PENDING_REMOVAL;
	}
}
