#include "pch.h"
#include "AAnimationBreakpointAttachment.h"
#include "AAnimatableObject.h"
#include "AAnimatableObjectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00583e20, CreateObject 005842c0, CreateObjectInPlace 00584320
TDEFINE_CLASS( AAnimationBreakpointAttachment );

TPSTRING8_DECLARE( Mode );
TPSTRING8_DECLARE( CreateChild );
TPSTRING8_DECLARE( DestroyChild );
TPSTRING8_DECLARE( DetachChild );
TPSTRING8_DECLARE( HideChild );
TPSTRING8_DECLARE( ShowChild );
TPSTRING8_DECLARE( SetChildTransform );
TPSTRING8_DECLARE( SetParentTransform );
TPSTRING8_DECLARE( ModelName );
TPSTRING8_DECLARE( BoneName );
TPSTRING8_DECLARE( ChildBoneName );
TPSTRING8_DECLARE( ParentBoneName );
TPSTRING8_DECLARE( Translate );
TPSTRING8_DECLARE( Rotation );
TPSTRING8_DECLARE( RotationAxis );
TPSTRING8_DECLARE( RotationAngle );

// 0x00775098
static TPString8* s_apModeNames[ AAnimationBreakpointAttachment::MODE_NUMOF ] = {
	&TPS8( CreateChild ),
	&TPS8( DestroyChild ),
	&TPS8( DetachChild ),
	&TPS8( HideChild ),
	&TPS8( ShowChild ),
	&TPS8( SetChildTransform ),
	&TPS8( SetParentTransform ),
};

// $Barnyard: FUNCTION 00584070
AAnimationBreakpointAttachment::AAnimationBreakpointAttachment()
    : m_vecTranslation( TVector4::VEC_ZERO )
    , m_quatRotation( TQuaternion::IDENTITY )
    , m_bEnabled( TTRUE )
{
}

// $Barnyard: FUNCTION 00584380
// $Barnyard: FUNCTION 00584360
AAnimationBreakpointAttachment::~AAnimationBreakpointAttachment()
{
}

// $Barnyard: FUNCTION 005840e0
TBOOL AAnimationBreakpointAttachment::Create( const PBProperties* a_pProperties )
{
	TBOOL bResult = AAnimationBreakpoint::Create( a_pProperties );

	TPString8 strMode;
	a_pProperties->GetOptionalPropertyValue( strMode, TPS8( Mode ) );

	TINT i = 0;
	for ( ; i < MODE_NUMOF; i++ )
	{
		if ( strMode == *s_apModeNames[ i ] )
			break;
	}

	m_eMode = i;

	if ( m_eMode == MODE_HIDE_CHILD || m_eMode == MODE_SHOW_CHILD || m_eMode == MODE_DESTROY_CHILD || m_eMode == MODE_DETACH_CHILD || m_eMode == MODE_SET_CHILD_TRANSFORM )
	{
		a_pProperties->GetOptionalPropertyValue( m_strModelName, TPS8( ModelName ) );
		a_pProperties->GetOptionalPropertyValue( m_strBoneName, TPS8( BoneName ) );
	}
	else if ( m_eMode == MODE_CREATE_CHILD )
	{
		a_pProperties->GetOptionalPropertyValue( m_strModelName, TPS8( ModelName ) );
		a_pProperties->GetOptionalPropertyValue( m_strChildBoneName, TPS8( ChildBoneName ) );
		a_pProperties->GetOptionalPropertyValue( m_strParentBoneName, TPS8( ParentBoneName ) );
	}
	else
	{
		// NOTE: MODE_SET_CHILD_TRANSFORM doesn't read the transform, it uses the default one
		TVector3 vecTranslation;
		if ( a_pProperties->GetOptionalVector3( vecTranslation, TPS8( Translate ) ) )
			m_vecTranslation = vecTranslation;

		a_pProperties->GetOptionalQuaternion( m_quatRotation, TPS8( Rotation ) );

		TVector3 vecAxis;
		TFLOAT   fAngle;
		if ( a_pProperties->GetOptionalVector3( vecAxis, TPS8( RotationAxis ) ) &&
		     a_pProperties->GetOptionalPropertyValue( fAngle, TPS8( RotationAngle ) ) )
		{
			// NOTE: the original calls TVector4::Normalise on the TVector3, which does the same
			vecAxis.Normalize();
			fAngle *= 0.017453292f;

			m_quatRotation = TQuaternion::IDENTITY;
			m_quatRotation.RotateAroundAxis( vecAxis, fAngle );
		}
	}

	return bResult;
}

// $Barnyard: FUNCTION 00583f00
void AAnimationBreakpointAttachment::Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime )
{
	if ( !m_bEnabled )
		return;

	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	switch ( m_eMode )
	{
		case MODE_HIDE_CHILD:
		case MODE_SHOW_CHILD:
			SetChildSkeletonUpdating( a_pObject, m_eMode == MODE_SHOW_CHILD );
			break;
		case MODE_CREATE_CHILD:
		{
			AAnimatableObjectType* pType = pManager->FindType( m_strModelName );

			if ( pType )
			{
				AAnimatableObject* pChild = pManager->CreateAnimatableObject( pType, a_pObject->GetOwner(), 0 );
				pChild->SetSkeletonUpdating( TTRUE, TTRUE );
				pChild->SetVisible( TTRUE );

				pManager->AttachObject( pChild, a_pObject, m_strChildBoneName, m_strParentBoneName, m_vecTranslation, m_quatRotation, 1.0f );
			}

			break;
		}
		case MODE_DETACH_CHILD:
		{
			AAnimatableObject* pChild = FindChild( a_pObject );

			while ( pChild )
			{
				pManager->DetachObject( pChild, a_pObject );
				pChild->SetSkeletonUpdating( TFALSE, TTRUE );

				pChild = FindChild( a_pObject );
			}

			break;
		}
		case MODE_DESTROY_CHILD:
		{
			AAnimatableObject* pChild = FindChild( a_pObject );

			while ( pChild )
			{
				pManager->DestroyAnimatableObject( pChild );
				pChild = FindChild( a_pObject );
			}

			break;
		}
		case MODE_SET_CHILD_TRANSFORM:
		{
			AAnimatableObject* pChild = FindChild( a_pObject );

			if ( pChild )
				pChild->SetAttachmentTransform( m_vecTranslation, m_quatRotation );

			break;
		}
		case MODE_SET_PARENT_TRANSFORM:
			// NOTE: sets the transform of the object itself (relative to its parent)
			if ( a_pObject->GetParentObject() )
				a_pObject->SetAttachmentTransform( m_vecTranslation, m_quatRotation );

			break;
	}
}

// $Barnyard: FUNCTION 00583e30
AAnimatableObject* AAnimationBreakpointAttachment::FindChild( AAnimatableObject* a_pObject )
{
	if ( !m_strModelName.IsEmpty() )
		return a_pObject->FindAttachedObjectOfType( m_strModelName );

	if ( !m_strBoneName.IsEmpty() )
		return a_pObject->FindAttachedObject( m_strBoneName );

	return TNULL;
}

// $Barnyard: FUNCTION 00583e90
void AAnimationBreakpointAttachment::SetChildSkeletonUpdating( AAnimatableObject* a_pObject, TBOOL a_bUpdating )
{
	if ( !m_strModelName.IsEmpty() )
		a_pObject->SetAttachedSkeletonUpdatingOfType( m_strModelName, a_bUpdating, TTRUE );
	else if ( !m_strBoneName.IsEmpty() )
		a_pObject->SetAttachedSkeletonUpdatingOnBone( m_strBoneName, a_bUpdating, TTRUE );
}
