#pragma once
#include <Toshi/TObject.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

// Something that other units have to steer around (pooled by
// AKinematicsObstructionManager in the PS2/Wii builds).
// The base implementation describes a static obstruction at the origin.
class AKinematicsObstruction : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AKinematicsObstruction, Toshi::TObject );

	// GetShape, the kinematics test it with masks
	using SHAPE = TINT;
	enum SHAPE_ : SHAPE
	{
		SHAPE_NONE    = 0,
		SHAPE_CIRCLE  = BITFLAG( 0 ), // GetRadius is the radius
		SHAPE_BOX     = BITFLAG( 1 ), // GetHalfSize is the half size
		SHAPE_CAPSULE = BITFLAG( 2 ), // GetHalfSize is the half length and the radius
	};

	// GetObstructionType, the kinematics test it with masks
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_NONE        = 0,
		TYPE_UNIT        = BITFLAG( 0 ), // AKinematicsObstructionUnit
		TYPE_UNK1        = BITFLAG( 1 ),
		TYPE_INTERACTION = BITFLAG( 2 ), // AKinematicsObstructionInteraction
	};

public:
	AKinematicsObstruction();
	~AKinematicsObstruction();

	virtual void                   Unk_0x0c( void* a_pUnk ) {}                              // +0x0c, DOES_NOTHING 00628f40
	virtual const Toshi::TVector4* GetPosition();                                           // +0x10
	virtual Toshi::TVector4        GetDirection();                                          // +0x14
	virtual Toshi::TVector4        GetVelocity();                                           // +0x18
	virtual TFLOAT                 GetSpeed() { return 0.0f; }                              // +0x1c, 0056ce20 (shared)
	virtual TFLOAT                 GetRadius() { return 0.0f; }                             // +0x20, 0056ce20 (shared)
	virtual const Toshi::TVector2* GetHalfSize();                                           // +0x24
	virtual Toshi::TVector4&       PredictPosition( TINT a_iUnk, Toshi::TVector4& a_rOut ); // +0x28
	virtual SHAPE                  GetShape() { return SHAPE_CIRCLE; }                      // +0x2c, 004c4a20 (shared, returns 1)
	virtual TYPE                   GetObstructionType() = 0;                                // +0x30

	// Whether the circle (a_rcCenter is XZ) collides with the obstruction at time a_fTime, a_pOutPenetration (optional) gets how deep.
	TBOOL CollideCircle( TFLOAT a_fTime, const Toshi::TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT* a_pOutPenetration );

	// Converts the point (XZ) to the space of the obstruction (x is the side, y is along its direction)
	void ToLocalXZ( const Toshi::TVector2& a_rcPoint, Toshi::TVector2& a_rOutLocal );

	// The segment (XZ) of a capsule obstruction centered at a_rcCenter and its radius
	void GetCapsuleXZ( const Toshi::TVector2& a_rcCenter, Toshi::TVector2& a_rOutStart, Toshi::TVector2& a_rOutEnd, TFLOAT* a_pOutRadius );
};

TSTATICASSERT( sizeof( AKinematicsObstruction ) == 0x4 );
