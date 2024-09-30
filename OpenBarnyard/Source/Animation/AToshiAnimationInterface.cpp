#include "pch.h"
#include "AToshiAnimationInterface.h"
#include "AAnimatableObjectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AToshiAnimationInterface::AToshiAnimationInterface()
    : m_pSkeletonInstance( TNULL )
{
	TIMPLEMENT();
}

AToshiAnimationInterface::~AToshiAnimationInterface()
{
	TIMPLEMENT();

	DestroyAnimReferences();
}

// $Barnyard: FUNCTION 005836e0
void AToshiAnimationInterface::KillAllAnimations()
{
	DestroyAnimReferences();

	TVALIDPTR( m_pSkeletonInstance );
	m_pSkeletonInstance->RemoveAllAnimations();
}

void AToshiAnimationInterface::DestroyAnimReferences()
{
	while ( !m_AnimRefList1.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_AnimRefList1.Begin();
		m_AnimRefList1.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}

	while ( !m_AnimRefList2.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_AnimRefList2.Begin();
		m_AnimRefList2.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}
}
