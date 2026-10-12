#include "pch.h"
#include "AFadeAniEffect.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00630380, CreateObject 00630450, CreateObjectInPlace 006304d0 (the constructor is inlined in both)
TDEFINE_CLASS( AFadeAniEffect );

AFadeAniEffect::AFadeAniEffect()
    : m_fSpeed( 1.0f )
    , m_pObject( TNULL )
{
}

// $Barnyard: FUNCTION 00630550
// $Barnyard: FUNCTION 006303e0
AFadeAniEffect::~AFadeAniEffect()
{
}

// $Barnyard: FUNCTION 006303d0
AUnit* AFadeAniEffect::GetUnit()
{
	return m_oUnit->GetUnit();
}

// $Barnyard: FUNCTION 00630600
TBOOL AFadeAniEffect::Update( TFLOAT a_fDeltaTime )
{
	if ( ( !m_oUnit || !m_oUnit->GetUnit() ) && !m_pObject )
	{
		Finish();
		return TFALSE;
	}

	AAnimatableObject* pObject;

	if ( m_oUnit && m_oUnit->GetUnit() )
		pObject = TSTATICCAST( AAnimatedUnit, m_oUnit->GetUnit() )->GetAnimatableObject();
	else
		pObject = m_pObject;

	TFLOAT fOpacity = pObject->GetModelInstance()->GetSceneObject()->GetFadeParams().y;
	fOpacity -= a_fDeltaTime * m_fSpeed;

	if ( fOpacity <= 0.0f )
	{
		pObject->GetModelInstance()->SetOpacity( 0.0f );
		pObject->SetSkeletonUpdating( TFALSE, TTRUE );
	}
	else if ( fOpacity <= 1.0f )
	{
		pObject->GetModelInstance()->SetOpacity( fOpacity );
		return TTRUE;
	}

	Finish();
	return TFALSE;
}

// $Barnyard: FUNCTION 00630570
void AFadeAniEffect::SetUnit( AUnit* a_pUnit )
{
	m_oUnit = a_pUnit->GetSelfProxy();
}

// $Barnyard: FUNCTION 00630390
void AFadeAniEffect::SetObject( AAnimatableObject* a_pObject )
{
	m_oUnit   = TNULL;
	m_pObject = a_pObject;
}
