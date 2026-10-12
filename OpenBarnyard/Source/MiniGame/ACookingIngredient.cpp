#include "pch.h"
#include "ACookingIngredient.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Units/AObjectTraits.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitModelStaticData.h"
#include "Units/AUnitType.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 004ffc00
void ACookingIngredient::Create( const TCHAR* a_szName, TFLOAT a_fScale, TBOOL a_bCentred )
{
	m_fScale = a_fScale;

	AUnitType* pUnitType = AUnitManager::GetSingleton()->FindUnit( a_szName );
	TPString8  strModel  = TSTATICCAST( AUnitModelStaticData, pUnitType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) )->GetModelName( 0 );
	m_wszScreenName      = pUnitType->GetScreenName();
	m_pTraits            = TSTATICCAST( AObjectTraits, pUnitType->FindStaticData( &TGetClass( AObjectTraits ) ) );
	m_strName            = TPString8( a_szName );

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
	m_pObject                          = pManager->CreateAnimatableObject( pManager->FindType( strModel ), TNULL, 0 );

	AModelInstance* pModelInstance = m_pObject->GetModelInstance();
	pModelInstance->DisableDrawWithDefaultMaterials();
	pModelInstance->SetUnlit();
	pModelInstance->SetReceivesLight( TFALSE );
	pModelInstance->SetDirection( TVector4::VEC_NEGZ );

	TMatrix44 matTransform;
	pModelInstance->GetLocalTransform( matTransform );
	matTransform.Scale( m_fScale, m_fScale, m_fScale );
	pModelInstance->SetTransform( matTransform );

	m_bCentred = a_bCentred;
}

// $Barnyard: FUNCTION 004ffb90
void ACookingIngredient::Destroy()
{
	if ( m_pObject )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pObject );
		m_pObject = TNULL;
	}
}

// $Barnyard: FUNCTION 004ffdc0
ACookingIngredient* ACookingIngredient::Clone() const
{
	ACookingIngredient* pClone = new ACookingIngredient();
	pClone->Create( m_strName.GetString(), m_fScale, m_bCentred );
	pClone->m_iCount  = m_iCount;
	pClone->m_pTraits = m_pTraits;

	return pClone;
}
