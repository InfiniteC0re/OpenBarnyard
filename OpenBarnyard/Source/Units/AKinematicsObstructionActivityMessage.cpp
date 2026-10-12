#include "pch.h"
#include "AKinematicsObstructionActivityMessage.h"
#include "AKinematicsObstructionManager.h"
#include "AKinematicsObstructionInteraction.h"
#include "AInteractionActivityMessageGroups.h"
#include "AInteractionSequence.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c80f0, CreateObject 005c85d0, CreateObjectInPlace 005c8640
TDEFINE_CLASS( AKinematicsObstructionActivityMessage );

// $Barnyard: FUNCTION 005c8420
AKinematicsObstructionActivityMessage::AKinematicsObstructionActivityMessage()
    : m_pObstruction( TNULL )
{
}

// $Barnyard: FUNCTION 005c8440
// $Barnyard: FUNCTION 005c83a0
AKinematicsObstructionActivityMessage::~AKinematicsObstructionActivityMessage()
{
	ReleaseObstruction();
}

// $Barnyard: FUNCTION 005c8310
void AKinematicsObstructionActivityMessage::Reset()
{
	ReleaseObstruction();
	AInteractionActivityMessage::Reset();
}

// $Barnyard: FUNCTION 005c8870
void AKinematicsObstructionActivityMessage::CreateGroup( AInteractionActivityMessageGroups* a_pGroups )
{
	a_pGroups->CreateGroup<AKinematicsObstructionActivityMessage>();
}

// $Barnyard: FUNCTION 005c8130
void AKinematicsObstructionActivityMessage::CreateObstruction()
{
	AUnit*  pFirstUnit  = m_pSequence->GetUnit( 0 );
	ASteer* pFirstSteer = pFirstUnit->GetSteer();

	if ( !pFirstSteer )
		return;

	const TVector4* pFirstPosition = pFirstSteer->GetPosition();

	AKinematicsObstructionInteraction* pObstruction = TSTATICCAST( AKinematicsObstructionInteraction, AKinematicsObstructionManager::GetSingleton()->CreateObstruction( &TGetClass( AKinematicsObstructionInteraction ) ) );
	m_pObstruction                                  = pObstruction;

	// A circle around the middle of the units
	TFLOAT fFirstRadius = pFirstUnit->GetSteer()->GetTraits()->m_fRadius;
	TFLOAT fRadius      = fFirstRadius;

	TVector4 vCenter = TVector4::VEC_ZERO;
	vCenter.x += pFirstPosition->x;
	vCenter.y += pFirstPosition->y;
	vCenter.z += pFirstPosition->z;

	for ( TINT i = 1; i < TINT( m_pSequence->GetComponents().Size() ); i++ )
	{
		AUnit*          pUnit     = m_pSequence->GetUnit( i );
		const TVector4* pPosition = pUnit->GetSteer()->GetPosition();

		vCenter.x += pPosition->x;
		vCenter.y += pPosition->y;
		vCenter.z += pPosition->z;

		// NOTE: retail adds the radius of the first unit to the radius of every other one
		TFLOAT fUnitRadius = fFirstRadius + pUnit->GetSteer()->GetTraits()->m_fRadius;
		fRadius            = TMath::Max( fRadius, fUnitRadius );
	}

	vCenter.Divide( TFLOAT( m_pSequence->GetComponents().Size() ) );

	pObstruction->SetPosition( vCenter );
	pObstruction->SetRadius( fRadius );

	if ( !AKinematicsObstructionManager::GetSingleton()->AddObstruction( m_pObstruction ) )
	{
		AKinematicsObstructionManager::GetSingleton()->DeleteObstruction( m_pObstruction );
		m_pObstruction = TNULL;
	}
}

// $Barnyard: FUNCTION 005c82e0
void AKinematicsObstructionActivityMessage::ReleaseObstruction()
{
	if ( m_pObstruction )
	{
		AKinematicsObstructionManager::GetSingleton()->RemoveObstruction( m_pObstruction );
		AKinematicsObstructionManager::GetSingleton()->DeleteObstruction( m_pObstruction );
		m_pObstruction = TNULL;
	}
}

// AInteractionActivityMessageGroup<AKinematicsObstructionActivityMessage> (template instance):
// pool ctor (message array + free list)
// $Barnyard: FUNCTION 005c8460
// pool dtor
// $Barnyard: FUNCTION 005c8560
// ctor
// $Barnyard: FUNCTION 005c86b0
// Allocate
// $Barnyard: FUNCTION 005c8720
// Free
// $Barnyard: FUNCTION 005c8770
// dtor, scalar deleting dtor
// $Barnyard: FUNCTION 005c87a0
// $Barnyard: FUNCTION 005c8780
// AInteractionActivityMessageGroups::CreateGroup<AKinematicsObstructionActivityMessage>
// $Barnyard: FUNCTION 005c87f0
