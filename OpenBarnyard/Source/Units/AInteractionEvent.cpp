#include "pch.h"
#include "AInteractionEvent.h"
#include "AUnitGameEventComponent.h"
#include "AUnitManager.h"
#include "AUnit.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c55b0, CreateObject and CreateObjectInPlace are RETURNS_ZERO stubs
TDEFINE_CLASS_INIT_NORUNTIME( AInteractionEvent );

AFunctionTable AInteractionEvent::ms_oFunctionTable( &TGetClass( AInteractionEvent ), TNULL );

// FSM fallback: AInteractionEvent natives run on the object the script refers to.
// NOTE: retail reads the object without checking AFunctionCaller::iKind (shared getter 006d9d10)
// $Barnyard: FUNCTION 005c55c0
static TObject* __stdcall AInteractionEventFunctionFallback( void* a_pUnk1, const AFunctionCaller* a_pCaller )
{
	return a_pCaller->pObject;
}

// $Barnyard: FUNCTION 005c5970
void AInteractionEvent::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AInteractionEventFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AInteractionEvent>::Create( "GetGlobalKnowledgeOfEvent", &AInteractionEvent::GetGlobalKnowledgeOfEvent ) );
}

// $Barnyard: FUNCTION 005c55d0
void AInteractionEvent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 005c5900
AInteractionEvent::AInteractionEvent()
    : m_uiTopicFlags( TOPICFLAGS_GENERAL )
    , m_uiPriority( 0xff )
    , m_uiFadeCurve( 0 )
    , m_fDuration( 60.0f )
    , m_fInterest( 1.0f )
    , m_fTime( 0.0f )
    , m_uiId( 0 )
{
}

// $Barnyard: FUNCTION 005c59a0
// $Barnyard: FUNCTION 005c5940
AInteractionEvent::~AInteractionEvent()
{
}

// $Barnyard: FUNCTION 005c53b0
AFunctionTable* AInteractionEvent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 005c56e0
void AInteractionEvent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteUInt16( m_uiTopicFlags );
	a_pBuffer->WriteUInt8( m_uiPriority );
	a_pBuffer->WriteFloat( m_fDuration );
	a_pBuffer->WriteFloat( m_fInterest );
	a_pBuffer->WriteUInt8( m_uiFadeCurve );
	a_pBuffer->WriteFloat( m_fTime );
	a_pBuffer->WriteInt( TINT( m_uiId ) );
}

// $Barnyard: FUNCTION 005c5750
void AInteractionEvent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadUInt16( m_uiTopicFlags );
	a_pBuffer->ReadUInt8( m_uiPriority );
	a_pBuffer->ReadFloat( m_fDuration );
	a_pBuffer->ReadFloat( m_fInterest );
	a_pBuffer->ReadUInt8( m_uiFadeCurve );
	a_pBuffer->ReadFloat( m_fTime );
	a_pBuffer->ReadUInt( m_uiId );
}

// $Barnyard: FUNCTION 005c55e0
TBOOL AInteractionEvent::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;
	return m_fTime <= m_fDuration;
}

// $Barnyard: FUNCTION 005c59c0
void AInteractionEvent::Reset()
{
	auto& llUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = llUnits.Begin(); it != llUnits.End(); it++ )
	{
		if ( it->IsDying() )
			continue;

		AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, it->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

		if ( pComponent )
			pComponent->RemoveEvent( this );
	}

	m_fInterest = 1.0f;
	m_fTime     = 0.0f;
	m_uiId      = 0;
}

// $Barnyard: FUNCTION 005c5a40
TBOOL AInteractionEvent::CanBeFreed()
{
	TBOOL bWitnessed = TFALSE;
	auto& llUnits    = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = llUnits.Begin(); it != llUnits.End(); it++ )
	{
		AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, it->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

		if ( pComponent )
		{
			// Units that only heard of it forget about it
			if ( pComponent->HasWitnessedEvent( this ) )
				bWitnessed = TTRUE;
			else
				pComponent->RemoveEvent( this );
		}
	}

	return !bWitnessed;
}

// $Barnyard: FUNCTION 005c53c0
AInteractionEvent::TOPICFLAGS AInteractionEvent::GetTopicFlags( void* a_pUnk )
{
	return m_uiTopicFlags;
}

// $Barnyard: FUNCTION 005c5650
TBOOL AInteractionEvent::IsKnownBy( AUnit* a_pUnit )
{
	AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, a_pUnit->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

	if ( pComponent )
		return pComponent->KnowsEvent( this );

	return TFALSE;
}

// Names of the topic flags (TOPICFLAGS_)
static const TCHAR* s_aTopicNames[] = {
	"custom",
	"general",
	"unhappy",
	"happy",
	"love",
	"hate",
	"food",
	"friends",
	"enemies",
	"crime",
	"money",
	"fun",
	"fear",
	"gift",
	"quest",
	"player",
};

// $Barnyard: FUNCTION 005c57b0
TUINT AInteractionEvent::GetTopicFlag( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < TARRAYSIZE( s_aTopicNames ); i++ )
	{
		if ( TStringManager::String8CompareNoCase( a_rcName.GetString(), s_aTopicNames[ i ] ) == 0 )
			return 1 << i;
	}

	return 0;
}

// $Barnyard: FUNCTION 005c5680
void AInteractionEvent::Notify( AUnit* a_pUnit, TUINT8 a_uiUnk )
{
	AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, a_pUnit->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

	if ( pComponent && !pComponent->KnowsEvent( this ) )
	{
		if ( IsWitness( a_pUnit ) )
			pComponent->AddWitnessedEvent( this, a_uiUnk );
		else
			pComponent->AddHeardEvent( this, a_uiUnk );
	}
}

// $Barnyard: FUNCTION 005c5ad0
TFLOAT AInteractionEvent::ComputeGlobalKnowledge()
{
	TINT  iNumUnits     = 0;
	TINT  iNumKnowledge = 0;
	auto& llUnits       = AUnitManager::GetSingleton()->GetUnits();

	if ( llUnits.Begin() == llUnits.End() )
		return 0.0f;

	for ( auto it = llUnits.Begin(); it != llUnits.End(); it++ )
	{
		if ( !it->IsDying() )
		{
			AUnitGameEventComponent* pComponent = TSTATICCAST( AUnitGameEventComponent, it->FindComponent( &TGetClass( AUnitGameEventComponent ) ) );

			if ( pComponent && pComponent->KnowsEvent( this ) )
				iNumKnowledge++;
		}

		iNumUnits++;
	}

	if ( iNumUnits <= 0 )
		return 0.0f;

	return TFLOAT( iNumKnowledge ) / TFLOAT( iNumUnits );
}

// $Barnyard: FUNCTION 005c5600
TFLOAT AInteractionEvent::GetInterest() const
{
	if ( !( m_fTime < m_fDuration ) )
		return 0.0f;

	if ( m_uiFadeCurve == 3 )
		return m_fInterest;

	TFLOAT fLeft = 1.0f - m_fTime / m_fDuration;

	if ( m_uiFadeCurve == 1 )
		return fLeft * fLeft * m_fInterest;

	if ( m_uiFadeCurve == 2 )
	{
		TFLOAT fPassed = 1.0f - fLeft;
		return ( 1.0f - fPassed * fPassed ) * m_fInterest;
	}

	return fLeft * m_fInterest;
}
