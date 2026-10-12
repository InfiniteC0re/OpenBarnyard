#include "pch.h"
#include "AUnitBehaviourData.h"
#include "AUnitBehaviour.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00592df0
// NOTE: GetClass 00592df0, CreateObject 00593770, CreateObjectInPlace 005932e0
TDEFINE_CLASS( AUnitBehaviourData );

// $Barnyard: FUNCTION 00593800
// $Barnyard: FUNCTION 005937e0
AUnitBehaviourData::~AUnitBehaviourData()
{
}

// $Barnyard: FUNCTION 00593350
TBOOL AUnitBehaviourData::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strDefaultFSM, "defaultfsm" );

	return AUnitStaticData::Create( a_pProperties );
}

// $Barnyard: FUNCTION 00593de0
void AUnitBehaviourData::OnUnitCreate( AUnit* a_pUnit )
{
	AUnitBehaviour* pBehaviour = new AUnitBehaviour();
	pBehaviour->m_pData        = this;

	if ( !m_strDefaultFSM.IsEmpty() )
	{
		AFSM* pFSM = AUnitBehaviour::FindStateMachine( m_strDefaultFSM );

		if ( pFSM )
			pBehaviour->m_oFSMInstance.Start( pFSM, TNULL );
	}

	a_pUnit->AddComponent( pBehaviour );
}
