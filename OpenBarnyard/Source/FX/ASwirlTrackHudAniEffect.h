#pragma once
#include "AAniEffect.h"
#include "Units/AUnitProxy.h"
#include "Units/AUnitManager.h"

#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>
#include <Toshi/TGlobalEmitter.h>

class AAnimatableObject;

//-----------------------------------------------------------------------------
// Throws the model of a unit (or an object) from its position to a point of
// the screen (a HUD element), drawn by AGUI2ModelRendererScreen on the way.
// The model is scaled, spun and faded on the way. When it finishes the model
// is given back to the world at its last position.
//
// TClass 0x007b4ea0 (size 0x98), vtable 0x00739674.
//-----------------------------------------------------------------------------
class ASwirlTrackHudAniEffect : public AAniEffect
{
public:
	TDECLARE_CLASS( ASwirlTrackHudAniEffect, AAniEffect );

public:
	ASwirlTrackHudAniEffect();
	~ASwirlTrackHudAniEffect();

	//-----------------------------------------------------------------------------
	// AAniEffect
	//-----------------------------------------------------------------------------
	virtual AUnit* GetUnit() OVERRIDE;                     // +0x10
	virtual TBOOL  Update( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x14

	// Throws the model of a_pUnit to a_rcScreenPos, a_fDepth units in front of the camera, ending with the rotation a_rcRotation
	void Setup( AUnit* a_pUnit, const Toshi::TVector2& a_rcScreenPos, const Toshi::TQuaternion& a_rcRotation, TFLOAT a_fDepth, TFLOAT a_fSpeed, TFLOAT a_fFadeDistance, TFLOAT a_fMinDistance );

	// Same for an object without a unit
	void Setup( AAnimatableObject* a_pObject, const Toshi::TVector2& a_rcScreenPos, const Toshi::TQuaternion& a_rcRotation, TFLOAT a_fDepth, TFLOAT a_fSpeed, TFLOAT a_fFadeDistance, TFLOAT a_fMinDistance );

	void SetUnit( AUnit* a_pUnit );

	// Radians per second around the Z axis
	void SetSpinSpeed( TFLOAT a_fSpinSpeed ) { m_fSpinSpeed = a_fSpinSpeed; }

	void SetSpeed( TFLOAT a_fSpeed ) { m_fSpeed = a_fSpeed; } // folded setter 0054f230

private:
	// Gives the model back to the world
	void ReleaseObject();

	static void OnUnitDestroyedCallback( void* a_pEffect, const AUnitDestroyEvent& a_rcEvent );

private:
	AAnimatableObject*      m_pObject;       // 0x0c
	TFLOAT                  m_fSpinSpeed;    // 0x10
	TFLOAT                  m_fSpeed;        // 0x14
	TFLOAT                  m_fUnk18;        // 0x18
	TFLOAT                  m_fMinDistance;  // 0x1c
	TFLOAT                  m_fFadeDistance; // 0x20, fades the model in this distance (if > 0)
	TFLOAT                  m_fDepth;        // 0x24, distance of the end position from the camera
	TFLOAT                  m_fTime;         // 0x28
	TFLOAT                  m_fStartScale;   // 0x2c
	TFLOAT                  m_fEndScale;     // 0x30
	TFLOAT                  m_fDistance;     // 0x34
	Toshi::TVector2         m_vScreenPos;    // 0x38
	Toshi::TVector4         m_vStartPos;     // 0x40
	Toshi::TVector4         m_vEndPos;       // 0x50
	Toshi::TRef<AUnitProxy> m_oUnit;         // 0x60
	Toshi::TQuaternion      m_qStartRot;     // 0x64
	Toshi::TQuaternion      m_qEndRot;       // 0x74

	Toshi::TGlobalListener<ASwirlTrackHudAniEffect, AUnitDestroyEvent> m_oUnitDestroyListener; // 0x84

	TBOOL m_bStarted; // 0x94, the object (without a unit) has been added to AGUI2ModelRendererScreen
};

TSTATICASSERT( sizeof( ASwirlTrackHudAniEffect ) == 0x98 );
