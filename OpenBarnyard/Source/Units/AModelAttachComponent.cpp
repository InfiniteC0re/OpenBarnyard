#include "pch.h"
#include "AModelAttachComponent.h"
#include "AModelAttachComponentData.h"
#include "AGeneralUnitComponentManager.h"
#include "AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058d0f0, CreateObject 0058d420, CreateObjectInPlace 0058d480
TDEFINE_CLASS_INIT( AModelAttachComponent, 1, 0 );

AFunctionTable AModelAttachComponent::ms_oFunctionTable( &TGetClass( AModelAttachComponent ), &AUnitComponent::ms_oFunctionTable );

// $Barnyard: FUNCTION 0058da80
void AModelAttachComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer3Args<AModelAttachComponent, TPString8, TPString8, TPString8>::Create( "AttachModel", &AModelAttachComponent::AttachModel ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AModelAttachComponent, TPString8>::Create( "DetachModel", &AModelAttachComponent::DetachModel ) );
}

// $Barnyard: FUNCTION 0058d100
void AModelAttachComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0058d110
AModelAttachComponent::AModelAttachComponent()
{
}

// $Barnyard: FUNCTION 0058d170
// $Barnyard: FUNCTION 0058d4c0
AModelAttachComponent::~AModelAttachComponent()
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < m_aModels.Size(); i++ )
	{
		if ( m_aModels[ i ] )
		{
			pAnimObjManager->DestroyAnimatableObject( m_aModels[ i ] );
			m_aModels[ i ] = TNULL;
		}
	}

	m_aModels.Resize( 0 );
}

// $Barnyard: FUNCTION 0058d160
AFunctionTable* AModelAttachComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded getter of AGeneralUnitComponentManager::ms_pSingleton (005d2300)
AUnitComponentManager* AModelAttachComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingleton();
}

// NOTE: folded with AModelSwapComponent::CreateModels (0058deb0), the code is identical
TBOOL AModelAttachComponent::CreateModels( AUnit* a_pUnit, AModelAttachComponentData* a_pData )
{
	TINT iNumModels = a_pData->GetNumModels();

	if ( iNumModels > 0 )
	{
		if ( m_aModels.Capacity() < iNumModels )
			m_aModels.Resize( iNumModels );

		m_aModels.SetSize( iNumModels );

		AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

		for ( TINT i = 0; i < iNumModels; i++ )
		{
			m_aModels[ i ] = TNULL;

			AAnimatableObjectType* pType = pAnimObjManager->FindType( a_pData->GetModel( i ) );

			if ( pType )
			{
				m_aModels[ i ] = pAnimObjManager->CreateAnimatableObject( pType, a_pUnit, 0 );
				m_aModels[ i ]->SetAnimControllersDisabled( TTRUE );
			}
		}
	}

	return TTRUE;
}

TINT AModelAttachComponent::FindModel( const TPString8& a_rcModel ) const
{
	for ( TINT i = 0; i < m_aModels.Size(); i++ )
	{
		if ( m_aModels[ i ] && m_aModels[ i ]->GetObjectType()->GetName() == a_rcModel )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 0058d7d0
void AModelAttachComponent::AttachModel( TPString8 a_strModel, TPString8 a_strParentBone, TPString8 a_strChildBone )
{
	TINT iModel = FindModel( a_strModel );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail reads m_aModels[ -1 ] when the unit has no such model
	if ( iModel == -1 )
	{
		TWARN( "AModelAttachComponent::AttachModel: the unit has no model '%s'\n", a_strModel.GetString() );
		return;
	}
#endif

	AAnimatableObject* pModel = m_aModels[ iModel ];

	// Detach it first if it's attached already
	if ( pModel->GetParentObject() )
		DetachModel( a_strModel );

	pModel->SetAnimControllersDisabled( TFALSE );
	pModel->SetSkeletonUpdating( TTRUE, TTRUE );

	AAnimatableObject* pParent = TSTATICCAST( AAnimatedUnit, GetOwner() )->GetAnimatableObject();
	AAnimatableObjectManager::GetSingleton()->AttachObject( pModel, pParent, a_strChildBone, a_strParentBone, TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
}

// $Barnyard: FUNCTION 0058d4e0
void AModelAttachComponent::DetachModel( TPString8 a_strModel )
{
	TINT iModel = FindModel( a_strModel );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail reads m_aModels[ -1 ] when the unit has no such model
	if ( iModel == -1 )
		return;
#endif

	AAnimatableObject* pModel = m_aModels[ iModel ];

	if ( pModel->GetParentObject() )
	{
		AAnimatableObjectManager::GetSingleton()->DetachObject( pModel, TSTATICCAST( AAnimatedUnit, GetOwner() )->GetAnimatableObject() );
		pModel->SetAnimControllersDisabled( TTRUE );
		pModel->SetSkeletonUpdating( TFALSE, TTRUE );
	}
}
