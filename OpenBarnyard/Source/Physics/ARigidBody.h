#pragma once
#include <Math/TOBBox.h>
#include <Math/TQuaternion.h>

class ARigidBody;
class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: Surface properties and collision masks of a rigid body.
// Embedded at ARigidBody+0x16c, size 0x18.
//-----------------------------------------------------------------------------
class ARigidBodyMaterial
{
public:
	ARigidBodyMaterial() = default;
	ARigidBodyMaterial( TFLOAT a_fFriction, TFLOAT a_fRestitution, TUINT a_uiCollideTypes, TUINT a_uiResponseTypes, TUINT a_uiIgnoreGroups, TINT a_iUnk14 );
	ARigidBodyMaterial& operator=( const ARigidBodyMaterial& a_rcOther );

public:
	TFLOAT m_fFriction;       // 0x00
	TFLOAT m_fRestitution;    // 0x04
	TUINT  m_uiCollideTypes;  // 0x08, ACollisionObject types the body collides with (inverted into the ignore mask of the queries)
	TUINT  m_uiResponseTypes; // 0x0C, ACollisionObject types that push the body
	TUINT  m_uiIgnoreGroups;  // 0x10, used as the ignore groups of the swept sphere and as inverted ignore types of the wheel rays
	TINT   m_iUnk14;          // 0x14, ignore groups of the wheel rays
};

TSTATICASSERT( sizeof( ARigidBodyMaterial ) == 0x18 );

//-----------------------------------------------------------------------------
// Purpose: Collision shape of a rigid body (in the local space of the body).
// Embedded at ARigidBody+0x184, size 0xD8.
//-----------------------------------------------------------------------------
class ARigidBodyShape
{
public:
	enum TYPE_ : TINT
	{
		TYPE_NONE   = 0,
		TYPE_SPHERE = 1, // m_aPoints[ 0 ] xyz is the centre, w is the radius
		TYPE_POINTS = 2, // m_iNumPoints spheres in m_aPoints
		TYPE_BOX    = 3, // m_oBox
	};

	static constexpr TINT MAX_NUM_POINTS = 6;

public:
	ARigidBodyShape();
	ARigidBodyShape& operator=( const ARigidBodyShape& a_rcOther );

	// $Barnyard: FUNCTION 006583b0
	void SetType( TINT a_eType )
	{
		m_bBoundsDirty = TTRUE;
		m_eType        = a_eType;
	}

	// $Barnyard: FUNCTION 006583c0
	void SetBox( const Toshi::TOBBox& a_rcBox )
	{
		m_bBoundsDirty = TTRUE;
		m_oBox         = a_rcBox;
	}

	const Toshi::TVector4& GetBoundingCentre();
	TFLOAT                 GetBoundingRadius();

private:
	void UpdateBounds();

public:
	TINT            m_eType;                     // 0x00
	TINT            m_iNumPoints;                // 0x04
	Toshi::TVector4 m_aPoints[ MAX_NUM_POINTS ]; // 0x08
	Toshi::TOBBox   m_oBox;                      // 0x68
	Toshi::TVector4 m_vBoundingCentre;           // 0xC0
	TFLOAT          m_fBoundingRadius;           // 0xD0
	TBOOL           m_bBoundsDirty;              // 0xD4
};

TSTATICASSERT( sizeof( ARigidBodyShape ) == 0xD8 );

//-----------------------------------------------------------------------------
// Purpose: A contact of a rigid body found by the collision step. Allocated
// from the contact pool of AVehicleManager (32 entries, reset every update).
// Size 0x68.
//-----------------------------------------------------------------------------
class ARigidBodyContact
{
public:
	ARigidBodyContact();
	void Reset();

public:
	ARigidBodyMaterial* m_pOtherMaterial;   // 0x00
	ARigidBody*         m_pOtherBody;       // 0x04, TNULL for static contacts
	Toshi::TVector4     m_vPosition;        // 0x08
	Toshi::TVector4     m_vNormal;          // 0x18
	Toshi::TVector4     m_vImpulse;         // 0x28
	Toshi::TVector4     m_vOffset;          // 0x38, contact position relative to the centre of mass
	TFLOAT              m_fImpulse;         // 0x48
	TFLOAT              m_fTime;            // 0x4C
	TINT                m_iSurfaceType;     // 0x50
	Toshi::TVector4     m_vPushOut;         // 0x54
	ACollisionObject*   m_pCollisionObject; // 0x64
};

TSTATICASSERT( sizeof( ARigidBodyContact ) == 0x68 );

//-----------------------------------------------------------------------------
// Purpose: Result of a collision query of the physics (size 0x44).
//-----------------------------------------------------------------------------
class ARigidBodyCollision
{
public:
	ARigidBodyCollision();

public:
	Toshi::TVector4   m_vPosition;        // 0x00
	Toshi::TVector4   m_vNormal;          // 0x10
	Toshi::TVector4   m_vOffset;          // 0x20, movement of the shape
	TINT              m_iSurfaceType;     // 0x30
	TFLOAT            m_fDepth;           // 0x34
	TFLOAT            m_fTime;            // 0x38
	TFLOAT            m_fImpulse;         // 0x3C
	ACollisionObject* m_pCollisionObject; // 0x40
};

TSTATICASSERT( sizeof( ARigidBodyCollision ) == 0x44 );

//-----------------------------------------------------------------------------
// Purpose: Receives the contacts of a rigid body after the collision step
// (ABikePhysicsEventHandler derives from it, vtable 0x0073ade4).
//-----------------------------------------------------------------------------
class ARigidBodyEventHandler
{
public:
	// $Barnyard: FUNCTION 00653c90
	virtual ~ARigidBodyEventHandler() {} // +0x00

	// NOTE: pure in the original (ABikePhysicsEventHandler has nullsub 005acd80 here)
	virtual void VTable0x04() {}                                       // +0x04
	virtual void OnStaticContact( ARigidBodyContact* a_pContact ) = 0; // +0x08, contact with the world
	virtual void OnBodyContact( ARigidBodyContact* a_pContact )   = 0; // +0x0C, contact with another body
};

//-----------------------------------------------------------------------------
// Purpose: Rigid body of the physics (vehicles, AFallingCow...).
// Embedded at APhysicsVehicle+0x4, size 0x2A4.
//-----------------------------------------------------------------------------
class ARigidBody
{
public:
	static constexpr TINT MAX_NUM_CONTACTS = 16;

public:
	ARigidBody();

	void Reset();

	// Integrates the forces, the velocities and the position
	void Integrate( TFLOAT a_fDeltaTime );

	// Integrates the forces into the velocities only
	void IntegrateVelocity( TFLOAT a_fDeltaTime );

	void SetPosition( const Toshi::TVector4& a_rcPosition, TBOOL a_bResetPrevious );
	void SetOrientation( const Toshi::TQuaternion& a_rcOrientation, TBOOL a_bResetPrevious );
	void SetPreviousPosition( const Toshi::TVector4& a_rcPosition );
	void SetPreviousOrientation( const Toshi::TQuaternion& a_rcOrientation );
	void SetCentreOfMass( const Toshi::TVector4& a_rcCentreOfMass );
	void SetAngularVelocity( const Toshi::TVector4& a_rcAngularVelocity );

	// Mass and inertia of a box of the size a_rcSize
	void SetBoxInertia( TFLOAT a_fDensity, const Toshi::TVector4& a_rcSize );

	// Mass and inertia of a sphere
	void SetSphereInertia( TFLOAT a_fDensity, TFLOAT a_fRadius );

	void UpdateInverseInertia();

	// $Barnyard: FUNCTION 006583d0
	void Disable() { m_bEnabled = TFALSE; }

	// Applies the force a_rcForce at a_rcPoint (world space)
	void ApplyForce( const Toshi::TVector4& a_rcForce, const Toshi::TVector4& a_rcPoint );

	// $Barnyard: FUNCTION 00653ce0
	void AddTorque( const Toshi::TVector4& a_rcTorque )
	{
		m_vTorque.x += a_rcTorque.x;
		m_vTorque.y += a_rcTorque.y;
		m_vTorque.z += a_rcTorque.z;
	}

	// $Barnyard: FUNCTION 0043fb00
	void SetVelocity( const Toshi::TVector4& a_rcVelocity ) { m_vVelocity = a_rcVelocity; }

	// $Barnyard: FUNCTION 00653cb0
	Toshi::TVector4 GetInertia() const { return m_vInertia; }

	// Velocity of the point a_rcPoint of the body
	Toshi::TVector4 GetPointVelocity( const Toshi::TVector4& a_rcPoint ) const;

	Toshi::TVector4 GetVelocityDirection() const;

	// Adds ( InvInertia * ( r x a_rcDirection ) ) x r to a_rVector, r = a_rcPoint - centre of mass
	void AddAngularResponse( Toshi::TVector4& a_rVector, const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcDirection ) const;

	// Impulse needed so that the velocity of a_rcPoint along a_rcDirection becomes a_rcTargetVelocity in a_fDeltaTime
	TFLOAT CalculateImpulse( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcDirection, TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rcTargetVelocity ) const;

	// Collision impulse against static geometry
	TFLOAT CalculateCollisionImpulse( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcNormal ) const;

	// Collision impulse against a sphere with the mass a_fMass moving with a_rcVelocity
	TFLOAT CalculateCollisionImpulse( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcNormal, TFLOAT a_fMass, const Toshi::TVector4& a_rcVelocity, const Toshi::TVector4& a_rcCentre ) const;

	// Collision impulse against another body
	TFLOAT CalculateCollisionImpulse( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcNormal, const ARigidBody* a_pOther ) const;

	// Friction impulse against static geometry
	Toshi::TVector4 CalculateFrictionImpulse( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcNormal, TFLOAT a_fNormalImpulse, TFLOAT a_fFriction ) const;

	// Friction impulse against another body
	Toshi::TVector4 CalculateFrictionImpulse( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcNormal, const ARigidBody* a_pOther, TFLOAT a_fNormalImpulse, TFLOAT a_fFriction ) const;

	void AddContact( ARigidBodyContact* a_pContact );

	const Toshi::TMatrix44& GetTransform() const { return m_oTransform; }
	const Toshi::TVector4&  GetPosition() const { return m_oTransform.GetTranslation(); }
	const Toshi::TVector4&  GetVelocity() const { return m_vVelocity; }
	TFLOAT                  GetMass() const { return m_fMass; }

public:
	Toshi::TMatrix44        m_oInvInertiaWorld;               // 0x000
	Toshi::TVector4         m_vAngularMomentum;               // 0x040
	Toshi::TVector4         m_vCentreOfMassLocal;             // 0x050
	Toshi::TVector4         m_vCentreOfMass;                  // 0x060, world space
	Toshi::TVector4         m_vAngularVelocity;               // 0x070
	Toshi::TVector4         m_vInertia;                       // 0x080, diagonal of the local inertia tensor
	TFLOAT                  m_fMaxAngularVelocity;            // 0x090
	TFLOAT                  m_fAngularDamping;                // 0x094
	Toshi::TVector4         m_vTorque;                        // 0x098
	TFLOAT                  m_fMass;                          // 0x0A8
	TFLOAT                  m_fMaxVelocity;                   // 0x0AC
	TFLOAT                  m_fContactScale;                  // 0x0B0, scales the averaged angular push out of the contacts
	TFLOAT                  m_fLinearDamping;                 // 0x0B4
	Toshi::TMatrix44        m_oTransform;                     // 0x0B8
	Toshi::TMatrix44        m_oPrevTransform;                 // 0x0F8
	Toshi::TQuaternion      m_qOrientation;                   // 0x138
	Toshi::TVector4         m_vVelocity;                      // 0x148
	Toshi::TVector4         m_vForce;                         // 0x158
	TBOOL                   m_bEnabled;                       // 0x168
	ARigidBodyMaterial      m_oMaterial;                      // 0x16C
	ARigidBodyShape         m_oShape;                         // 0x184
	TINT                    m_iNumContacts;                   // 0x25C
	ARigidBodyContact*      m_apContacts[ MAX_NUM_CONTACTS ]; // 0x260
	ARigidBodyEventHandler* m_pEventHandler;                  // 0x2A0
};

TSTATICASSERT( sizeof( ARigidBody ) == 0x2A4 );
