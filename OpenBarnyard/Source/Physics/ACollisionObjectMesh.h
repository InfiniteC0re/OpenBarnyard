#pragma once
#include "ACollisionObjectSimple.h"

#include <Math/TSphere.h>
#include <Toshi/TSceneObject.h>

class ACollisionModelSet;

// Collision object using the collision meshes of a model (ACollisionModelSet).
// AVisibleUnit::Create uses it for units whose AUnitCollisionComponentData::m_iCollideFlags is TYPE_MESH.
class ACollisionObjectMesh
    : public ACollisionObjectSimple
{
public:
	TDECLARE_CLASS( ACollisionObjectMesh, ACollisionObjectSimple );

public:
	ACollisionObjectMesh();
	~ACollisionObjectMesh();

	// Creates the collision model of the scene object's model and activates the object.
	TBOOL Create( Toshi::TSceneObject* a_pSceneObject );

	// Recomputes the world sphere from the scene object's transform (after the object was moved) and rehashes it
	void UpdateWorldSphere();

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual TBOOL                  CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit ) OVERRIDE;                                                                        // +0x20
	virtual TBOOL                  CollideSphere( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal ) OVERRIDE; // +0x2c
	virtual const Toshi::TSphere&  GetObjectSphere() OVERRIDE;                                                                                                                   // +0x38
	virtual const Toshi::TVector4& GetObjectPosition() OVERRIDE;                                                                                                                 // +0x3c
	virtual TFLOAT                 GetObjectRadius() OVERRIDE;                                                                                                                   // +0x40
	virtual TFLOAT                 GetHeightScale() OVERRIDE;                                                                                                                    // +0x48
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) OVERRIDE {}                                                              // +0x58, DOES_NOTHING 0054df00

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL CollideRay( const Toshi::TVector4& a_rcRayOrigin, const Toshi::TVector4& a_rcRayDir, TFLOAT a_fMaxDistance, ACollisionHit& a_rHit );                                                                          // +0x6c
	virtual TBOOL SlideSphereAlongModel( TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, void* a_pUnused, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT a_fSpeedScale, Toshi::TVector4& a_rOutPosition ); // +0x70, slides the bounding sphere along the model
	virtual TBOOL SweepSphereAgainstModel( TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration );                  // +0x74, sweeps the bounding sphere against the model
	virtual TBOOL SweepObjectAgainstModel( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal );                            // +0x78, sweeps a sphere of the bounding radius against the model of a_pObject
	virtual TBOOL Unk_0x7c( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, void* a_pUnk4 ) { return TFALSE; }                                                                                                                     // +0x7c, 0061dda0 (shared with ACollisionObjectBox)

	Toshi::TSceneObject* GetSceneObject() const { return m_pSceneObject; }

#ifdef BARNYARD_LATEST
	// Set while AGTAStyleCameraHelper checks the collisions of the camera. The collision models of the
	// objects then ignore the camera collision groups (see AGTAStyleCameraHelper::m_uiCollisionMask).
	// $BarnyardWiiJP: FUNCTION 803c69a8
	static void SetCameraCollision( TBOOL a_bCamera ) { ms_bCameraCollision = a_bCamera; }

	// $BarnyardWiiJP: FUNCTION 803c69b0
	static TBOOL IsCameraCollision() { return ms_bCameraCollision; }

	// Groups the camera ignores in the collision models of the objects (the default AGTAStyleCameraHelper::m_uiCollisionMask)
	static constexpr Toshi::TCollisionCommon::TOSHICGROUPFLAG CAMERA_IGNORE_GROUPS = Toshi::TCollisionCommon::TOSHICGROUPFLAG_WATERWALL | Toshi::TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

private:
	inline static TBOOL ms_bCameraCollision = TFALSE;

public:
#endif

private:
	ACollisionModelSet*  m_pModelSet;        // 0x38
	Toshi::TSceneObject* m_pSceneObject;     // 0x3c
	Toshi::TSphere       m_oModelSphere;     // 0x40, bounding sphere of the model's first LOD
	Toshi::TVector4      m_vecWorldPosition; // 0x50, m_oModelSphere's origin transformed by the scene object
	TFLOAT               m_fWorldRadius;     // 0x60
	TFLOAT               m_fMeshHeightScale; // 0x64
};

TSTATICASSERT( sizeof( ACollisionObjectMesh ) == 0x68 );
