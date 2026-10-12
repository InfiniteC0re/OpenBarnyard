#include "pch.h"
#include "AStaticUnit.h"
#include "AUnitType.h"
#include "AUnitTransform.h"
#include "AUnitModelStaticData.h"
#include "ARandom.h"
#include "Animation/AModelRepos.h"
#include "UnitMovement/Steering/ASteer.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

#include <Toshi/TSceneObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00418ea0, CreateObject 004191f0, CreateObjectInPlace 00419280 (the constructor is inlined in both)
TDEFINE_CLASS_INIT( AStaticUnit, 1, 1 );

AFunctionTable AStaticUnit::ms_oFunctionTable( &TGetClass( AStaticUnit ), &AVisibleUnit::ms_oFunctionTable );

// $Barnyard: FUNCTION 00419300
void AStaticUnit::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	// NOTE: SetVisible is virtual, retail binds the compiler's vcall thunk (00414f80 is +0x90)
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AStaticUnit, TBOOL>::Create( "SetVisible", &AStaticUnit::SetVisible ) );
}

// $Barnyard: FUNCTION 00418eb0
void AStaticUnit::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

AStaticUnit::AStaticUnit()
    : m_pModelInstance( TNULL )
    , m_fModelYOffset( 0.0f )
{
}

// $Barnyard: FUNCTION 00419070
// $Barnyard: FUNCTION 00419330
AStaticUnit::~AStaticUnit()
{
	// NOTE: only releases the reference, the instance is destroyed in OnDestroy
	m_pModelInstance = TNULL;
}

// $Barnyard: FUNCTION 00419050
AFunctionTable* AStaticUnit::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 004191c0
void AStaticUnit::SetSize( TFLOAT a_fSize )
{
	m_pModelInstance->SetScale( TVector4( a_fSize, a_fSize, a_fSize, 1.0f ) );
}

// $Barnyard: FUNCTION 00419520
TBOOL AStaticUnit::Create( AUnitTransform* a_pTransform )
{
	CreateModelInstance( a_pTransform->m_iModelIndex );

	m_eFlags |= FLAGS_EXTERNAL_UPDATE;
	m_eFlags |= FLAGS_THINK_ENABLED;

	TBOOL bResult = AVisibleUnit::Create( a_pTransform );
	SetVisible( TTRUE );

	return bResult;
}

// $Barnyard: FUNCTION 00419350
void AStaticUnit::CreateModelInstance( TINT a_iModelIndex )
{
	AUnitModelStaticData* pModelData = TSTATICCAST( AUnitModelStaticData, m_pType->FindStaticData( &TGetClass( AUnitModelStaticData ) ) );

	TINT      iIndex       = ( a_iModelIndex < 0 ) ? ARandom::GetSingleton()->m_oRandom.GetInt( pModelData->GetNumModels() ) : a_iModelIndex % pModelData->GetNumModels();
	TPString8 strModelName = pModelData->GetModelName( iIndex );

	if ( m_pModelInstance )
	{
		AModelInstance* pOldInstance = m_pModelInstance;
		m_pModelInstance             = TNULL;

		g_pModelRepos->DestroyModelInstance( pOldInstance );
	}

	TString8 strPath;
	strPath.Format( "%s%s", "data\\models\\", strModelName.GetString() );

	m_pModelInstance = g_pModelRepos->InstantiateNewModel( TPString8( strPath.GetString() ), TNULL );
	m_iModelIndex    = iIndex;

	// NOTE: retail doesn't check for a missing model
	TSceneObject* pSceneObject = m_pModelInstance->GetSceneObject();
	pSceneObject->EnableOpacity();
	pSceneObject->GetFadeParams().y = 1.0f; // opacity

	ConnectModelInstance( m_pModelInstance );
}

// $Barnyard: FUNCTION 00419110
void AStaticUnit::OnDestroy()
{
	if ( m_pModelInstance )
	{
		AModelInstance* pModelInstance = m_pModelInstance;
		m_pModelInstance               = TNULL;

		g_pModelRepos->DestroyModelInstance( pModelInstance );
	}

	AVisibleUnit::OnDestroy();
}

// $Barnyard: FUNCTION 00418f30
TBOOL AStaticUnit::Think( TFLOAT a_fDeltaTime )
{
	TBOOL bResult = m_pSteer->Update( a_fDeltaTime );
	m_fThinkTimer = 0.0f;

	return bResult;
}

// $Barnyard: FUNCTION 00418ec0
void AStaticUnit::UpdateComponents( TFLOAT a_fDeltaTime )
{
	if ( m_pSteer )
	{
		TVector4 vecPosition;
		m_pSteer->CopyPosition( vecPosition );

		TVector4 vecDirection;
		m_pSteer->CopyDirection( vecDirection );

		vecPosition.y += m_fModelYOffset;
		GetModelInstance()->SetTransform( vecPosition, vecDirection );
	}

	AUnit::UpdateComponents( a_fDeltaTime );
}

// $Barnyard: FUNCTION 004190f0
void AStaticUnit::SetVisible( TBOOL a_bVisible )
{
	m_pModelInstance->SetSkeletonUpdating( a_bVisible );
}

// $Barnyard: FUNCTION 00419100
TBOOL AStaticUnit::IsVisible()
{
	return m_pModelInstance->IsUpdatingSkeleton();
}

// $Barnyard: FUNCTION 00419160
void AStaticUnit::UpdateBoundingSphere()
{
	TVector4 vecPosition;
	m_pSteer->CopyPosition( vecPosition );
	vecPosition.y += m_fModelYOffset;

	TFLOAT fRadius = m_pModelInstance->GetModel()->GetManagedModel()->GetModel()->GetRenderDistance();
	m_oBoundingSphere.Set( vecPosition.x, vecPosition.y, vecPosition.z, fRadius );
}

// NOTE: shares AAnimatedUnit::GetAnimatableObject's body (00415b30), returns the 0x94 member
AModelInstance* AStaticUnit::GetModelInstance()
{
	return m_pModelInstance;
}
