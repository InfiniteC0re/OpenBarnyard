#include "pch.h"
#include "ABreakActivity.h"
#include "AAnimatedUnit.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "AUnitTransform.h"
#include "AUnitModelStaticData.h"
#include "AInteractionSequence.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"
#include "UnitMovement/Steering/ABallSteer2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b3dc0, CreateObject 005b41d0
// NOTE: GetClass 005b3dc0, CreateObject 005b41d0, CreateObjectInPlace 005b4240
TDEFINE_CLASS( ABreakActivity );

TPSTRING8_DECLARE( breaktype );
TPSTRING8_DECLARE( unitname );
TPSTRING8_DECLARE( fallappart );

// 0x00775254
static TPString8* s_apBreakTypeNames[ ABreakActivity::BREAKTYPE_NUMOF ] = {
	&TPS8( fallappart ),
};

// NOTE: retail leaves m_eBreakType and m_fUnk uninitialised
ABreakActivity::ABreakActivity()
{
}

// $Barnyard: FUNCTION 005b3dd0
// $Barnyard: FUNCTION 005b42b0
ABreakActivity::~ABreakActivity()
{
}

// $Barnyard: FUNCTION 005b3e40
TBOOL ABreakActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strBreakType;
	a_pProperties->GetOptionalPropertyValue( strBreakType, TPS8( breaktype ) );

	TINT i = 0;
	for ( ; i < BREAKTYPE_NUMOF; i++ )
	{
		if ( strBreakType == *s_apBreakTypeNames[ i ] )
			break;
	}

	m_eBreakType = i;

	a_pProperties->GetOptionalPropertyValue( m_strUnitName, TPS8( unitname ) );

	// NOTE: retail reads the float from "unitname" too, so it stays 5.0f
	m_fUnk = 5.0f;
	a_pProperties->GetOptionalPropertyValue( m_fUnk, TPS8( unitname ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b3f10
AInteractionActivity::RESULT ABreakActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( m_eBreakType == BREAKTYPE_FALLAPPART )
	{
		AUnitType*            pType       = AUnitManager::GetSingleton()->GetUnitFactory()->Find( m_strUnitName );
		AUnitModelStaticData* pModelData  = TSTATICCAST( AUnitModelStaticData, pType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) );
		AUnit*                pBrokenUnit = a_pSequence->GetUnitByCategory( m_eCategory );

		TVector4 vecPosition  = *pBrokenUnit->GetSteer()->GetPosition();
		TVector4 vecDirection = *pBrokenUnit->GetSteer()->GetDirection();

		// A piece for every model
		for ( TINT i = 0; i < pModelData->GetNumModels(); i++ )
		{
			AUnitTransform oTransform( vecPosition, vecDirection, 0, i, -1, TTRUE );
			AAnimatedUnit* pPiece = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( m_strUnitName, &oTransform ) );

			AAnimatableObject* pAnimObject = pPiece->GetAnimatableObject();
			TPString8          strBone     = pPiece->GetModelName( i );
			TINT               iBone       = pAnimObject->GetModelInstance()->GetBoneIndex( strBone );

			TMatrix44 matBone;
			pAnimObject->GetModelInstance()->GetBoneTransform( iBone, matBone );

			// NOTE: the direction away from the bone isn't used
			TVector4 vecAway( -matBone.m_f41, -matBone.m_f42, -matBone.m_f43, matBone.m_f44 );
			vecAway.Normalise();

			pPiece->ChangeSteer( &TGetClass( ABallSteer2 ) );

			ABallSteer2* pSteer = TSTATICCAST( ABallSteer2, pPiece->GetSteer() );
			pSteer->SetDamping( 1.0f );
			pSteer->SetPosition( vecPosition );
		}

		AUnitManager::GetSingleton()->DestroyUnit( pBrokenUnit );
	}

	return RESULT_DONE;
}
