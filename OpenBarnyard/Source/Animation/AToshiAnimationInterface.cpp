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

// $Barnyard: FUNCTION 00470970
ANamedAnimationSetRef AToshiAnimationInterface::GetAnimationSet()
{
	return m_pAnimationSet;
}

void AToshiAnimationInterface::DestroyAnimReferences()
{
	while ( !m_llAnimRefs.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_llAnimRefs.Begin();
		m_llAnimRefs.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}

	while ( !m_llOverlayAnimRefs.IsEmpty() )
	{
		T2DList<AToshiAnimationRef>::Iterator itAnimRef = m_llOverlayAnimRefs.Begin();
		m_llOverlayAnimRefs.Erase( itAnimRef );

		g_oAnimationRefPool.DeleteObject( itAnimRef );
	}
}
