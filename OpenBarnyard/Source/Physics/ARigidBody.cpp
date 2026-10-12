#include "pch.h"
#include "ARigidBody.h"
#include "ACollisionObject.h"
#include "AVehicleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

//-----------------------------------------------------------------------------
// ARigidBodyMaterial
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 006582f0
ARigidBodyMaterial::ARigidBodyMaterial( TFLOAT a_fFriction, TFLOAT a_fRestitution, TUINT a_uiCollideTypes, TUINT a_uiResponseTypes, TUINT a_uiIgnoreGroups, TINT a_iUnk14 )
    : m_fFriction( a_fFriction )
    , m_fRestitution( a_fRestitution )
    , m_uiCollideTypes( a_uiCollideTypes )
    , m_uiResponseTypes( a_uiResponseTypes )
    , m_uiIgnoreGroups( a_uiIgnoreGroups )
    , m_iUnk14( a_iUnk14 )
{
}

// $Barnyard: FUNCTION 00658320
ARigidBodyMaterial& ARigidBodyMaterial::operator=( const ARigidBodyMaterial& a_rcOther )
{
	m_fFriction       = a_rcOther.m_fFriction;
	m_fRestitution    = a_rcOther.m_fRestitution;
	m_uiCollideTypes  = a_rcOther.m_uiCollideTypes;
	m_uiResponseTypes = a_rcOther.m_uiResponseTypes;
	m_uiIgnoreGroups  = a_rcOther.m_uiIgnoreGroups;
	m_iUnk14          = a_rcOther.m_iUnk14;

	return *this;
}

//-----------------------------------------------------------------------------
// ARigidBodyShape
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00659740
ARigidBodyShape::ARigidBodyShape()
{
	m_eType           = TYPE_NONE;
	m_iNumPoints      = 0;
	m_vBoundingCentre = TVector4::VEC_ZERO;
	m_fBoundingRadius = 0.0f;
	m_bBoundsDirty    = TTRUE;
}

// $Barnyard: FUNCTION 00658350
ARigidBodyShape& ARigidBodyShape::operator=( const ARigidBodyShape& a_rcOther )
{
	m_eType      = a_rcOther.m_eType;
	m_iNumPoints = a_rcOther.m_iNumPoints;

	for ( TINT i = 0; i < m_iNumPoints; i++ )
		m_aPoints[ i ] = a_rcOther.m_aPoints[ i ];

	m_oBox = a_rcOther.m_oBox;

	return *this;
}

// $Barnyard: FUNCTION 006597a0
void ARigidBodyShape::UpdateBounds()
{
	if ( m_eType == TYPE_POINTS )
	{
		m_vBoundingCentre = TVector4::VEC_ZERO;

		for ( TINT i = 0; i < m_iNumPoints; i++ )
		{
			m_vBoundingCentre.x += m_aPoints[ i ].x;
			m_vBoundingCentre.y += m_aPoints[ i ].y;
			m_vBoundingCentre.z += m_aPoints[ i ].z;
		}

		m_vBoundingCentre.Multiply( 1.0f / m_iNumPoints );

		TFLOAT fMaxDistance = -3.4028235e38f;

		for ( TINT i = 0; i < m_iNumPoints; i++ )
		{
			TVector4 vToPoint;
			vToPoint.Substract( m_vBoundingCentre, m_aPoints[ i ] );

			TFLOAT fDistance = vToPoint.Magnitude();
			fMaxDistance     = ( fMaxDistance >= fDistance ) ? fMaxDistance : fDistance;
		}

		m_fBoundingRadius = fMaxDistance;
	}
	else if ( m_eType == TYPE_SPHERE )
	{
		m_vBoundingCentre.Set( m_aPoints[ 0 ].x, m_aPoints[ 0 ].y, m_aPoints[ 0 ].z, 1.0f );
		m_fBoundingRadius = m_aPoints[ 0 ].w;
	}
	else
	{
		m_vBoundingCentre = m_oBox.GetCentre();
		m_fBoundingRadius = m_oBox.m_fRadius;
	}
}

// $Barnyard: FUNCTION 00659d00
const TVector4& ARigidBodyShape::GetBoundingCentre()
{
	if ( m_bBoundsDirty == TTRUE )
	{
		UpdateBounds();
		m_bBoundsDirty = TFALSE;
	}

	return m_vBoundingCentre;
}

// $Barnyard: FUNCTION 00659d20
TFLOAT ARigidBodyShape::GetBoundingRadius()
{
	if ( m_bBoundsDirty == TTRUE )
	{
		UpdateBounds();
		m_bBoundsDirty = TFALSE;
	}

	return m_fBoundingRadius;
}

//-----------------------------------------------------------------------------
// ARigidBodyContact
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00659da0
ARigidBodyContact::ARigidBodyContact()
{
	m_pOtherMaterial   = TNULL;
	m_pOtherBody       = TNULL;
	m_vPosition        = TVector4::VEC_ZERO;
	m_vNormal          = TVector4::VEC_ZERO;
	m_vImpulse         = TVector4::VEC_ZERO;
	m_vOffset          = TVector4::VEC_ZERO;
	m_fImpulse         = 0.0f;
	m_fTime            = 0.0f;
	m_iSurfaceType     = AVehicleManager::SURFACE_NONE;
	m_vPushOut         = TVector4::VEC_ZERO;
	m_pCollisionObject = TNULL;
}

// $Barnyard: FUNCTION 00659e80
void ARigidBodyContact::Reset()
{
	m_pOtherMaterial   = TNULL;
	m_pOtherBody       = TNULL;
	m_vPosition        = TVector4::VEC_ZERO;
	m_vNormal          = TVector4::VEC_NEGY;
	m_vImpulse         = TVector4::VEC_ZERO;
	m_vOffset          = TVector4::VEC_ZERO;
	m_fImpulse         = 0.0f;
	m_fTime            = 0.0f;
	m_iSurfaceType     = AVehicleManager::SURFACE_NONE;
	m_vPushOut         = TVector4::VEC_ZERO;
	m_pCollisionObject = TNULL;
}

//-----------------------------------------------------------------------------
// ARigidBodyCollision
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0045d980
ARigidBodyCollision::ARigidBodyCollision()
{
	m_vPosition        = TVector4::VEC_ZERO;
	m_vNormal          = TVector4::VEC_NEGY;
	m_vOffset          = TVector4::VEC_NEGY;
	m_iSurfaceType     = AVehicleManager::SURFACE_NONE;
	m_fDepth           = 0.0f;
	m_fTime            = 0.0f;
	m_fImpulse         = 0.0f;
	m_pCollisionObject = TNULL;
}

//-----------------------------------------------------------------------------
// ARigidBody
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00659d40
ARigidBody::ARigidBody()
    : m_oMaterial( 1.0f, 0.2f, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, 0, -1 )
{
	m_iNumContacts = 0;
	Reset();
}

// $Barnyard: FUNCTION 00659980
void ARigidBody::Reset()
{
	m_vAngularMomentum    = TVector4::VEC_ZERO;
	m_vCentreOfMassLocal  = TVector4::VEC_ZERO;
	m_vCentreOfMass       = TVector4::VEC_ZERO;
	m_vAngularVelocity    = TVector4::VEC_ZERO;
	m_vInertia.x          = 1.0f;
	m_vInertia.y          = 1.0f;
	m_vInertia.z          = 1.0f;
	m_fMaxAngularVelocity = 200.0f;
	m_vInertia.w          = 1.0f;
	m_fAngularDamping     = 0.0f;
	m_vTorque             = TVector4::VEC_ZERO;
	m_fMass               = 1.0f;
	m_fMaxVelocity        = 100.0f;
	m_fContactScale       = 1.0f;
	m_fLinearDamping      = 0.0f;
	m_oTransform.Identity();
	m_oPrevTransform.Identity();
	m_qOrientation             = TQuaternion::IDENTITY;
	m_vVelocity                = TVector4::VEC_ZERO;
	m_vForce                   = TVector4::VEC_ZERO;
	m_bEnabled                 = TTRUE;
	m_oMaterial.m_fFriction    = 0.5f;
	m_oMaterial.m_fRestitution = 0.2f;

	for ( TINT i = 0; i < m_iNumContacts; i++ )
		m_apContacts[ i ] = TNULL;

	m_iNumContacts  = 0;
	m_pEventHandler = TNULL;

	UpdateInverseInertia();
}

// $Barnyard: FUNCTION 00658230
TVector4 ARigidBody::GetPointVelocity( const TVector4& a_rcPoint ) const
{
	TVector4 vOffset;
	vOffset.Substract( a_rcPoint, m_vCentreOfMass );

	TVector4 vVelocity;
	vVelocity.CrossProduct( m_vAngularVelocity, vOffset );

	return TVector4( vVelocity.x + m_vVelocity.x, vVelocity.y + m_vVelocity.y, vVelocity.z + m_vVelocity.z, m_vVelocity.w );
}

// $Barnyard: FUNCTION 006583e0
void ARigidBody::SetAngularVelocity( const TVector4& a_rcAngularVelocity )
{
	m_vAngularVelocity = a_rcAngularVelocity;

	// NOTE: retail's TVector4::Magnitude (006c74f0) accepts zero vectors, TMath::Sqrt asserts on them
	TFLOAT fSpeed = sqrtf( m_vAngularVelocity.MagnitudeSq() );

	if ( fSpeed > m_fMaxAngularVelocity )
		m_vAngularVelocity.Multiply( m_fMaxAngularVelocity / fSpeed );

	// Convert to the local space, multiply by the inertia and convert back
	TMatrix44 oInvTransform;
	oInvTransform.Transpose( m_oTransform );

	TVector4 vLocal;
	TMatrix44::RotateVector( vLocal, oInvTransform, m_vAngularVelocity );

	TVector4 vLocalMomentum;
	vLocalMomentum.x = vLocal.x * m_vInertia.x;
	vLocalMomentum.y = vLocal.y * m_vInertia.y;
	vLocalMomentum.z = vLocal.z * m_vInertia.z;
	vLocalMomentum.w = TVector4::VEC_ZERO.w;

	TMatrix44::RotateVector( m_vAngularMomentum, m_oTransform, vLocalMomentum );
}

// $Barnyard: FUNCTION 006584c0
TVector4 ARigidBody::GetVelocityDirection() const
{
	TVector4 vDirection = m_vVelocity;
	vDirection.Normalise();

	return vDirection;
}

// $Barnyard: FUNCTION 00658520
void ARigidBody::AddAngularResponse( TVector4& a_rVector, const TVector4& a_rcPoint, const TVector4& a_rcDirection ) const
{
	TVector4 vOffset;
	vOffset.Substract( a_rcPoint, m_vCentreOfMass );

	TVector4 vResponse;
	vResponse.CrossProduct( vOffset, a_rcDirection );
	TMatrix44::RotateVector( vResponse, m_oInvInertiaWorld, vResponse );
	vResponse.CrossProduct( vResponse, vOffset );

	a_rVector.x += vResponse.x;
	a_rVector.y += vResponse.y;
	a_rVector.z += vResponse.z;
}

// $Barnyard: FUNCTION 006585d0
void ARigidBody::UpdateInverseInertia()
{
	TFLOAT fInvX = 1.0f / m_vInertia.x;

	TMatrix44 oInertia = TMatrix44::IDENTITY;
	TFLOAT    fInvY    = 1.0f / m_vInertia.y;
	TFLOAT    fInvZ    = 1.0f / m_vInertia.z;

	TMatrix44 oRotation = m_oTransform;

	oInertia.m_f11 = fInvX * m_oTransform.m_f11;
	oInertia.m_f12 = fInvY * m_oTransform.m_f21;
	oInertia.m_f13 = fInvZ * m_oTransform.m_f31;
	oRotation.GetTranslation().Set( 0.0f, 0.0f, 0.0f, 0.0f );
	oInertia.m_f21 = fInvX * m_oTransform.m_f12;
	oInertia.m_f22 = fInvY * m_oTransform.m_f22;
	oInertia.m_f23 = fInvZ * m_oTransform.m_f32;
	oInertia.m_f31 = fInvX * m_oTransform.m_f13;
	oInertia.m_f32 = fInvY * m_oTransform.m_f23;
	oInertia.m_f33 = fInvZ * m_oTransform.m_f33;

	TMatrix44 oResult;
	oResult.Multiply( oRotation, oInertia );
	m_oInvInertiaWorld = oResult;

	TMatrix44::RotateVector( m_vAngularVelocity, m_oInvInertiaWorld, m_vAngularMomentum );
}

// $Barnyard: FUNCTION 00658700
void ARigidBody::Integrate( TFLOAT a_fDeltaTime )
{
	m_oPrevTransform = m_oTransform;

	if ( m_fAngularDamping > 0.0f )
		m_vAngularMomentum.Multiply( 1.0f - a_fDeltaTime * m_fAngularDamping );

	if ( m_fLinearDamping > 0.0f )
		m_vVelocity.Multiply( 1.0f - a_fDeltaTime * m_fLinearDamping );

	// Angular movement
	TVector4 vDelta;
	vDelta.Multiply( m_vTorque, a_fDeltaTime );
	m_vAngularMomentum.x += vDelta.x;
	m_vAngularMomentum.y += vDelta.y;
	m_vAngularMomentum.z += vDelta.z;
	m_vTorque = TVector4::VEC_ZERO;

	TMatrix44::RotateVector( m_vAngularVelocity, m_oInvInertiaWorld, m_vAngularMomentum );
	TFLOAT fAngularSpeed = m_vAngularVelocity.Magnitude();

	if ( fAngularSpeed > m_fMaxAngularVelocity )
	{
		TFLOAT fScale = m_fMaxAngularVelocity / fAngularSpeed;
		m_vAngularMomentum.Multiply( fScale );
		m_vAngularVelocity.Multiply( fScale );
		fAngularSpeed = m_fMaxAngularVelocity;
	}

	if ( fAngularSpeed <= 0.0000000099999999f )
	{
		m_vAngularVelocity = TVector4::VEC_ZERO;
		m_vAngularMomentum = TVector4::VEC_ZERO;
	}
	else
	{
		TVector4 vAxis;
		vAxis.Multiply( m_vAngularVelocity, 1.0f / fAngularSpeed );
		vAxis.Normalise();

		TQuaternion qRotation;
		qRotation.SetRotation( vAxis.AsVector3(), fAngularSpeed * a_fDeltaTime );
		m_qOrientation = qRotation * m_qOrientation;
	}

	m_qOrientation.AsVector4().Divide4( TMath::Sqrt( m_qOrientation.x * m_qOrientation.x + m_qOrientation.y * m_qOrientation.y + m_qOrientation.z * m_qOrientation.z + m_qOrientation.w * m_qOrientation.w ) );

	TVector4 vTranslation = m_oTransform.GetTranslation();
	m_oTransform.SetFromQuaternion( m_qOrientation );
	m_oTransform.GetTranslation() = vTranslation;

	UpdateInverseInertia();

	// Linear movement
	vDelta.Multiply( m_vForce, a_fDeltaTime / m_fMass );
	m_vVelocity.x += vDelta.x;
	m_vVelocity.y += vDelta.y;
	m_vVelocity.z += vDelta.z;
	m_vForce = TVector4::VEC_ZERO;

	TFLOAT fSpeed = m_vVelocity.Magnitude();

	if ( fSpeed > m_fMaxVelocity )
		m_vVelocity.Multiply( m_fMaxVelocity / fSpeed );

	vDelta.Multiply( m_vVelocity, a_fDeltaTime );
	m_vCentreOfMass.x += vDelta.x;
	m_vCentreOfMass.y += vDelta.y;
	m_vCentreOfMass.z += vDelta.z;

	TVector4 vCentreOffset = m_vCentreOfMassLocal;
	TMatrix44::RotateVector( vCentreOffset, m_oTransform, vCentreOffset );

	m_oTransform.GetTranslation().Set(
	    m_vCentreOfMass.x - vCentreOffset.x,
	    m_vCentreOfMass.y - vCentreOffset.y,
	    m_vCentreOfMass.z - vCentreOffset.z,
	    m_vCentreOfMass.w
	);
}

// $Barnyard: FUNCTION 00658b30
void ARigidBody::IntegrateVelocity( TFLOAT a_fDeltaTime )
{
	TVector4 vDelta;
	vDelta.Multiply( m_vTorque, a_fDeltaTime );
	m_vAngularMomentum.x += vDelta.x;
	m_vAngularMomentum.y += vDelta.y;
	m_vAngularMomentum.z += vDelta.z;
	m_vTorque = TVector4::VEC_ZERO;

	TMatrix44::RotateVector( m_vAngularVelocity, m_oInvInertiaWorld, m_vAngularMomentum );
	TFLOAT fAngularSpeed = m_vAngularVelocity.Magnitude();

	if ( fAngularSpeed > m_fMaxAngularVelocity )
	{
		TFLOAT fScale = m_fMaxAngularVelocity / fAngularSpeed;
		m_vAngularMomentum.Multiply( fScale );
		m_vAngularVelocity.Multiply( fScale );
	}

	vDelta.Multiply( m_vForce, a_fDeltaTime / m_fMass );
	m_vVelocity.x += vDelta.x;
	m_vVelocity.y += vDelta.y;
	m_vVelocity.z += vDelta.z;
	m_vForce = TVector4::VEC_ZERO;

	TFLOAT fSpeed = m_vVelocity.Magnitude();

	if ( fSpeed > m_fMaxVelocity )
		m_vVelocity.Multiply( m_fMaxVelocity / fSpeed );
}

// $Barnyard: FUNCTION 00658cc0
TFLOAT ARigidBody::CalculateImpulse( const TVector4& a_rcPoint, const TVector4& a_rcDirection, TFLOAT a_fDeltaTime, const TVector4& a_rcTargetVelocity ) const
{
	TVector4 vOffset;
	vOffset.Substract( a_rcPoint, m_vCentreOfMass );

	TVector4 vLinear;
	vLinear.Multiply( a_rcDirection, 1.0f / m_fMass );

	TVector4 vAngular;
	vAngular.CrossProduct( vOffset, a_rcDirection );
	TMatrix44::RotateVector( vAngular, m_oInvInertiaWorld, vAngular );
	vAngular.CrossProduct( vAngular, vOffset );

	TVector4 vResponse( vAngular.x + vLinear.x, vAngular.y + vLinear.y, vAngular.z + vLinear.z, vLinear.w );

	TVector4 vRelVelocity = GetPointVelocity( a_rcPoint );
	vRelVelocity.Substract( vRelVelocity, a_rcTargetVelocity );

	TFLOAT fVelocity = -TVector4::DotProduct3( vRelVelocity, a_rcDirection );
	return fVelocity / ( TVector4::DotProduct3( vResponse, a_rcDirection ) * a_fDeltaTime );
}

// $Barnyard: FUNCTION 00658e50
void ARigidBody::SetPosition( const TVector4& a_rcPosition, TBOOL a_bResetPrevious )
{
	m_oTransform.GetTranslation() = a_rcPosition;
	TMatrix44::TransformVector( m_vCentreOfMass, m_oTransform, m_vCentreOfMassLocal );

	if ( a_bResetPrevious == TTRUE )
		m_oPrevTransform = m_oTransform;
}

// $Barnyard: FUNCTION 00658eb0
void ARigidBody::SetOrientation( const TQuaternion& a_rcOrientation, TBOOL a_bResetPrevious )
{
	m_qOrientation = a_rcOrientation;

	TVector4 vTranslation = m_oTransform.GetTranslation();
	m_oTransform.SetFromQuaternion( m_qOrientation );
	m_oTransform.GetTranslation() = vTranslation;

	TMatrix44::TransformVector( m_vCentreOfMass, m_oTransform, m_vCentreOfMassLocal );

	if ( a_bResetPrevious == TTRUE )
		m_oPrevTransform = m_oTransform;
}

// $Barnyard: FUNCTION 00658f50
void ARigidBody::SetPreviousPosition( const TVector4& a_rcPosition )
{
	m_oPrevTransform.GetTranslation() = a_rcPosition;
}

// $Barnyard: FUNCTION 00658f80
void ARigidBody::SetPreviousOrientation( const TQuaternion& a_rcOrientation )
{
	TVector4 vTranslation = m_oPrevTransform.GetTranslation();
	m_oPrevTransform.SetFromQuaternion( a_rcOrientation );
	m_oPrevTransform.GetTranslation() = vTranslation;
}

// $Barnyard: FUNCTION 00658fd0
void ARigidBody::ApplyForce( const TVector4& a_rcForce, const TVector4& a_rcPoint )
{
	TVector4 vOffset;
	vOffset.Substract( a_rcPoint, m_vCentreOfMass );

	TVector4 vTorque;
	vTorque.CrossProduct( vOffset, a_rcForce );

	m_vTorque.x += vTorque.x;
	m_vTorque.y += vTorque.y;
	m_vTorque.z += vTorque.z;
	m_vForce.x += a_rcForce.x;
	m_vForce.y += a_rcForce.y;
	m_vForce.z += a_rcForce.z;
}

// $Barnyard: FUNCTION 006590a0
void ARigidBody::SetCentreOfMass( const TVector4& a_rcCentreOfMass )
{
	m_vCentreOfMassLocal = a_rcCentreOfMass;
	TMatrix44::TransformVector( m_vCentreOfMass, m_oTransform, a_rcCentreOfMass );
}

// $Barnyard: FUNCTION 006590e0
TFLOAT ARigidBody::CalculateCollisionImpulse( const TVector4& a_rcPoint, const TVector4& a_rcNormal ) const
{
	TVector4 vVelocity = GetPointVelocity( a_rcPoint );

	if ( TVector4::DotProduct3( vVelocity, a_rcNormal ) >= 0.0f )
		return 0.0f;

	TVector4 vResponse;
	vResponse.Multiply( a_rcNormal, 1.0f / m_fMass );
	AddAngularResponse( vResponse, a_rcPoint, a_rcNormal );

	TFLOAT fVelocity = -TVector4::DotProduct3( vVelocity, a_rcNormal );
	return fVelocity / TVector4::DotProduct3( vResponse, a_rcNormal );
}

// $Barnyard: FUNCTION 006591a0
TFLOAT ARigidBody::CalculateCollisionImpulse( const TVector4& a_rcPoint, const TVector4& a_rcNormal, TFLOAT a_fMass, const TVector4& a_rcVelocity, const TVector4& a_rcCentre ) const
{
	TVector4 vVelocity = GetPointVelocity( a_rcPoint );

	TVector4 vRelVelocity;
	vRelVelocity.x = vVelocity.x - a_rcVelocity.x;
	vRelVelocity.y = vVelocity.y - a_rcVelocity.y;
	vRelVelocity.z = vVelocity.z - a_rcVelocity.z;
	vRelVelocity.w = vVelocity.w;

	if ( TVector4::DotProduct3( vRelVelocity, a_rcNormal ) > 0.0f )
		return 0.0f;

	TVector4 vResponse;
	vResponse.Multiply( a_rcNormal, 1.0f / m_fMass + 1.0f / a_fMass );
	AddAngularResponse( vResponse, a_rcPoint, a_rcNormal );

	// The other object is a solid sphere
	TMatrix44 oOtherInvInertia = TMatrix44::IDENTITY;
	oOtherInvInertia.m_f11     = 1.0f / ( a_fMass * 0.40000001f );
	oOtherInvInertia.m_f22     = oOtherInvInertia.m_f11;
	oOtherInvInertia.m_f33     = oOtherInvInertia.m_f11;

	TVector4 vOffset;
	vOffset.Substract( a_rcPoint, a_rcCentre );

	TVector4 vAngular;
	vAngular.CrossProduct( vOffset, a_rcNormal );
	TMatrix44::RotateVector( vAngular, oOtherInvInertia, vAngular );
	vAngular.CrossProduct( vAngular, vOffset );

	vResponse.x = vResponse.x + vAngular.x;
	vResponse.y = vAngular.y + vResponse.y;
	vResponse.z = vAngular.z + vResponse.z;

	TFLOAT fVelocity = -TVector4::DotProduct3( vRelVelocity, a_rcNormal );
	return fVelocity / TVector4::DotProduct3( vResponse, a_rcNormal );
}

// $Barnyard: FUNCTION 006593d0
TFLOAT ARigidBody::CalculateCollisionImpulse( const TVector4& a_rcPoint, const TVector4& a_rcNormal, const ARigidBody* a_pOther ) const
{
	TVector4 vVelocity      = GetPointVelocity( a_rcPoint );
	TVector4 vOtherVelocity = a_pOther->GetPointVelocity( a_rcPoint );

	TVector4 vRelVelocity;
	vRelVelocity.Substract( vVelocity, vOtherVelocity );

	if ( TVector4::DotProduct3( vRelVelocity, a_rcNormal ) > 0.0f )
		return 0.0f;

	TVector4 vResponse;
	vResponse.Multiply( a_rcNormal, 1.0f / a_pOther->m_fMass + 1.0f / m_fMass );
	AddAngularResponse( vResponse, a_rcPoint, a_rcNormal );
	a_pOther->AddAngularResponse( vResponse, a_rcPoint, a_rcNormal );

	TFLOAT fVelocity = -TVector4::DotProduct3( vRelVelocity, a_rcNormal );
	return fVelocity / TVector4::DotProduct3( vResponse, a_rcNormal );
}

// $Barnyard: FUNCTION 00659500
TVector4 ARigidBody::CalculateFrictionImpulse( const TVector4& a_rcPoint, const TVector4& a_rcNormal, TFLOAT a_fNormalImpulse, TFLOAT a_fFriction ) const
{
	if ( 0.0f == m_vVelocity.MagnitudeSq() )
		return TVector4::VEC_ZERO;

	TVector4 vTangent;
	vTangent.CrossProduct( a_rcNormal, m_vVelocity );
	vTangent.CrossProduct( a_rcNormal, vTangent );
	vTangent.Normalise();

	TFLOAT fStopImpulse = CalculateCollisionImpulse( a_rcPoint, vTangent );
	TFLOAT fImpulse     = a_fNormalImpulse * a_fFriction;

	if ( fImpulse > fStopImpulse )
		fImpulse = fStopImpulse;

	TVector4 vImpulse;
	vImpulse.Multiply( vTangent, fImpulse );

	return vImpulse;
}

// $Barnyard: FUNCTION 006595f0
TVector4 ARigidBody::CalculateFrictionImpulse( const TVector4& a_rcPoint, const TVector4& a_rcNormal, const ARigidBody* a_pOther, TFLOAT a_fNormalImpulse, TFLOAT a_fFriction ) const
{
	TVector4 vOtherVelocity = a_pOther->GetPointVelocity( a_rcPoint );
	TVector4 vVelocity      = GetPointVelocity( a_rcPoint );

	TVector4 vRelVelocity;
	vRelVelocity.Substract( vVelocity, vOtherVelocity );

	if ( 0.0f == vRelVelocity.MagnitudeSq() )
		return TVector4::VEC_ZERO;

	TVector4 vTangent;
	vTangent.CrossProduct( a_rcNormal, vRelVelocity );
	vTangent.CrossProduct( a_rcNormal, vTangent );
	vTangent.Normalise();

	TFLOAT fStopImpulse = CalculateCollisionImpulse( a_rcPoint, vTangent, a_pOther );
	TFLOAT fImpulse     = a_fNormalImpulse * a_fFriction;

	if ( fImpulse > fStopImpulse )
		fImpulse = fStopImpulse;

	TVector4 vImpulse;
	vImpulse.Multiply( vTangent, fImpulse );

	return vImpulse;
}

// $Barnyard: FUNCTION 00659bb0
void ARigidBody::SetBoxInertia( TFLOAT a_fDensity, const TVector4& a_rcSize )
{
	TFLOAT fMass = a_rcSize.y * a_rcSize.x * a_fDensity * a_rcSize.z;
	TFLOAT fZZ   = a_rcSize.z * a_rcSize.z;
	TFLOAT fYY   = a_rcSize.y * a_rcSize.y;

	m_vInertia.x = ( fYY + fZZ ) * fMass * 0.083333336f;
	m_vInertia.y = ( a_rcSize.x * a_rcSize.x + fZZ ) * fMass * 0.083333336f;
	m_vInertia.z = ( a_rcSize.x * a_rcSize.x + fYY ) * fMass * 0.083333336f;
	m_vInertia.w = TVector4::VEC_ZERO.w;
	m_fMass      = fMass;

	UpdateInverseInertia();
}

// $Barnyard: FUNCTION 00659c60
void ARigidBody::SetSphereInertia( TFLOAT a_fDensity, TFLOAT a_fRadius )
{
	m_fMass = 3.1415927f * a_fRadius * a_fRadius * a_fRadius * 1.3333334f * a_fDensity * 1000.0f;

	TFLOAT fInertia = m_fMass * a_fRadius * a_fRadius * 0.40000001f;
	m_vInertia.x    = fInertia;
	m_vInertia.y    = fInertia;
	m_vInertia.z    = fInertia;
	m_vInertia.w    = 1.0f;

	UpdateInverseInertia();
}

// $Barnyard: FUNCTION 00659ce0
void ARigidBody::AddContact( ARigidBodyContact* a_pContact )
{
	if ( m_iNumContacts != MAX_NUM_CONTACTS )
	{
		m_apContacts[ m_iNumContacts ] = a_pContact;
		m_iNumContacts++;
	}
}
