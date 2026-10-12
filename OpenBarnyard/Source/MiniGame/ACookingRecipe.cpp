#include "pch.h"
#include "ACookingRecipe.h"
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

// $Barnyard: FUNCTION 004fff80
ACookingRecipe::ACookingRecipe()
    : m_fScale( 1.0f )
    , m_vOffset( TVector4::VEC_ZERO )
    , m_vFacing( TVector4::VEC_POSZ )
    , m_pObject( TNULL )
    , m_iPrice( 0 )
    , m_eType( 3 )
    , m_uiNumMade( 0 )
    , m_uiNumTotal( 0 )
{
}

// $Barnyard: FUNCTION 00500000
ACookingRecipe::~ACookingRecipe()
{
}

// $Barnyard: FUNCTION 00500320
ACookingRecipe& ACookingRecipe::operator=( const ACookingRecipe& a_rcOther )
{
	m_fScale       = a_rcOther.m_fScale;
	m_vOffset      = a_rcOther.m_vOffset;
	m_vFacing      = a_rcOther.m_vFacing;
	m_pObject      = a_rcOther.m_pObject;
	m_iNameString  = a_rcOther.m_iNameString;
	m_oIngredients = a_rcOther.m_oIngredients;
	m_iPrice       = a_rcOther.m_iPrice;
	m_iUnk40       = a_rcOther.m_iUnk40;
	m_eType        = a_rcOther.m_eType;
	m_strName      = a_rcOther.m_strName;
	m_strQuest     = a_rcOther.m_strQuest;
	m_uiNumMade    = a_rcOther.m_uiNumMade;

	return *this;
}

// $Barnyard: FUNCTION 00500080
void ACookingRecipe::Create( TINT a_iNameString, const TCHAR* a_szName )
{
	m_iNameString = a_iNameString;

	if ( !a_szName )
		return;

	m_strName = TPString8( a_szName );

	// The dish is shown the way it's held
	TPString8  strModel;
	AUnitType* pUnitType = AUnitManager::GetSingleton()->FindUnit( a_szName );

	if ( pUnitType )
	{
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pUnitType->FindStaticData( &TGetClass( AObjectTraits ) ) );
		m_fScale               = pTraits->m_fItemScale;
		m_vOffset              = pTraits->m_vecItemOffset;
		m_vFacing              = pTraits->m_vecItemFacing;

		strModel = TSTATICCAST( AUnitModelStaticData, pUnitType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) )->GetModelName( 0 );
	}
	else
	{
		strModel = m_strName;
	}

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();
	m_pObject                          = pManager->CreateAnimatableObject( pManager->FindType( strModel ), TNULL, 0 );

	m_pObject->GetModelInstance()->SetTranslation( m_vOffset.AsVector3() );
	m_pObject->GetModelInstance()->SetDirection( m_vFacing );

	AModelInstance* pModelInstance = m_pObject->GetModelInstance();
	pModelInstance->DisableDrawWithDefaultMaterials();
	pModelInstance->SetUnlit();
}

// $Barnyard: FUNCTION 004ffe50
void ACookingRecipe::Destroy()
{
	if ( m_pObject )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pObject );
		m_pObject = TNULL;
	}
}

// $Barnyard: FUNCTION 005003f0
void ACookingRecipe::SetQuest( const TCHAR* a_szQuest )
{
	m_strQuest = TPString8( a_szQuest );
}

// $Barnyard: FUNCTION 004ffe70
TINT ACookingRecipe::AddIngredient( ACookingIngredient* a_pIngredient )
{
	return m_oIngredients.PushBack( a_pIngredient );
}

// $Barnyard: FUNCTION 004ffe90
ACookingIngredient* ACookingRecipe::PopIngredient()
{
	return m_oIngredients.PopBack();
}

// $Barnyard: FUNCTION 004ffec0
TBOOL ACookingRecipe::HasSameIngredients( const ACookingRecipe& a_rcOther ) const
{
	TINT iNumIngredients = m_oIngredients.Size();

	if ( iNumIngredients != a_rcOther.m_oIngredients.Size() )
		return TFALSE;

	// Every ingredient has to be there as many times in both
	for ( TINT i = 0; i < iNumIngredients; i++ )
	{
		TINT iCount = 0;

		for ( TINT k = 0; k < iNumIngredients; k++ )
		{
			if ( m_oIngredients[ k ]->GetName().GetPooledString() == m_oIngredients[ i ]->GetName().GetPooledString() )
				iCount++;
		}

		TINT iOtherCount = 0;

		for ( TINT k = 0; k < a_rcOther.m_oIngredients.Size(); k++ )
		{
			if ( m_oIngredients[ i ]->GetName().GetPooledString() == a_rcOther.m_oIngredients[ k ]->GetName().GetPooledString() )
				iOtherCount++;
		}

		if ( iCount != iOtherCount )
			return TFALSE;
	}

	return TTRUE;
}
