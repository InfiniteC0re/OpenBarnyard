#pragma once
#include "ACollisionObject.h"

class ACollisionObjectSimple
    : public ACollisionObject
{
public:
	TDECLARE_CLASS( ACollisionObjectSimple, ACollisionObject );

public:
	ACollisionObjectSimple();
	~ACollisionObjectSimple();

	// Sets the sphere up (a_fHeightScale > 0 makes it a vertical capsule) and makes sure it's not hashed.
	TBOOL Create( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fHeightScale, TFLOAT a_fSphereRaise, TUINT a_eTypeFlags, ACollisionModelInstance* a_pModelInstance );

	// Same as Create, but the object is added to ACollisionManager and hashed (Activate, inlined)
	TBOOL CreateActive( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fHeightScale, TFLOAT a_fSphereRaise, TUINT a_eTypeFlags, ACollisionModelInstance* a_pModelInstance );

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual void                   Update( void* ) OVERRIDE {}                                                                                                                                                                                                               // +0x1c, DOES_NOTHING 00628f40
	virtual TBOOL                  CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit ) OVERRIDE;                                                                                                                                                                    // +0x20
	virtual TBOOL                  CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE; // +0x24
	virtual TBOOL                  CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE;                     // +0x28
	virtual TBOOL                  CollideSphere( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal ) OVERRIDE;                                                                                             // +0x2c
	virtual const Toshi::TSphere&  GetObjectSphere() OVERRIDE;                                                                                                                                                                                                               // +0x38
	virtual const Toshi::TVector4& GetObjectPosition() OVERRIDE;                                                                                                                                                                                                             // +0x3c
	virtual TFLOAT                 GetObjectRadius() OVERRIDE;                                                                                                                                                                                                               // +0x40
	virtual void                   GetVelocity( Toshi::TVector4& a_rOutVector ) OVERRIDE;                                                                                                                                                                                    // +0x44
	virtual TFLOAT                 GetHeightScale() OVERRIDE;                                                                                                                                                                                                                // +0x48
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) OVERRIDE;                                                                                                                                                            // +0x58
	virtual void                   SetVelocity( void* ) OVERRIDE {}                                                                                                                                                                                                          // +0x5c, DOES_NOTHING 00628f40

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL SetSphereRaise( TFLOAT a_fSphereRaise ); // +0x60, sets m_fSphereRaise

	// NOTE: MSVC groups the overloads of a virtual at the first declaration of the name and puts the new ones in
	// reverse order. The override of ACollisionObject::UpdateHash is declared here (after SetSphereRaise) and the
	// overloads below in this order so that retail's layout comes out: SetSphereRaise +0x60, UpdateHash( r, pos ) +0x64,
	// UpdateHash( pos ) +0x68.
	virtual TBOOL UpdateHash( TFLOAT a_fRadius ) OVERRIDE; // +0x4c

	// Checks if the position has changed and updates hash if needed
	virtual void UpdateHash( const Toshi::TVector3& a_rcPosition ); // +0x68

	// Checks if the position or the radius has changed and updates hash if needed
	virtual TBOOL UpdateHash( TFLOAT a_fRadius, const Toshi::TVector3& a_rcPosition ); // +0x64

protected:
	// Tests of the sphere (vertical capsule if m_fHeightScale > 0) shared with ACollisionObjectUnit.
	// NOTE: retail passes the sphere data (&m_vPosition), the object and m_pModelInstance explicitly.
	TBOOL CollideRayImpl( const ARayData& a_rcRay, ACollisionHit& a_rHit );
	TBOOL CollideSweptSphereImpl( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight );
	TBOOL CollideSphereImpl( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal );

protected:
	// NOTE: m_vPosition and m_fRadius form the object's TSphere (GetObjectSphere)
	Toshi::TVector3 m_vPosition;    // 0x20
	TFLOAT          m_fRadius;      // 0x2c
	TFLOAT          m_fHeightScale; // 0x30
	TFLOAT          m_fSphereRaise; // 0x34, AUnitCollisionComponentData::m_fSphereRaise for units
};

TSTATICASSERT( sizeof( ACollisionObjectSimple ) == 0x38 );
