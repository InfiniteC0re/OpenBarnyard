#pragma once
#include "ACameraHelper.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TRefCounted.h>
#include <Math/TMathInline.h>

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: Camera that keeps several targets (units or points) in view
// (ACameraManager::CAMERAHELPER_MULTICAMERA).
//
// TClass 0x00782510 (size 0xf4), vtable 0x0071cfb0.
//-----------------------------------------------------------------------------
class AMultiCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( AMultiCameraHelper, ACameraHelper );

	static constexpr TINT MAX_NUM_TARGETS = 8;

	struct Target
	{
		Toshi::TVector4 vPosition; // 0x00, used unless bIsUnit
		AUnit*          pUnit;     // 0x10, used if bIsUnit
		TBOOL           bIsUnit;   // 0x14
		TBOOL           bOptional; // 0x15, ignored once the camera can't fit every target
	};

public:
	AMultiCameraHelper();
	~AMultiCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override;                                            // +0x1c
	virtual void  OnDestroy() override {}                                         // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                          // +0x24, DOES_NOTHING 005acd80
	virtual void  OnDetach() override;                                            // +0x3c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void MultiHelper_Unknown() {} // +0x44, DOES_NOTHING 005acd80

	// Removes every target
	void ClearTargets();

	// $Barnyard: FUNCTION 00464b90
	TBOOL RemoveAllTargets()
	{
		ClearTargets();
		return TTRUE;
	}

	// Direction the camera looks in (from the camera to the targets)
	// NOTE: inlined in retail (AGolfMinigameState::SetupHoleCamera)
	void SetViewDirection( const Toshi::TVector4& a_rcDirection ) { m_vViewDirection = a_rcDirection; }

	// Adds a target at a_rcPosition
	TBOOL AddTarget( const Toshi::TVector4& a_rcPosition, TBOOL a_bOptional );

	// Adds the unit of a_pUnit as a target
	TBOOL AddTarget( Toshi::TRef<AUnitProxy> a_pUnit, TBOOL a_bOptional );

	// Removes the first target at a_rcPosition
	void RemoveTarget( const Toshi::TVector4& a_rcPosition );

	// Removes the first target of the unit of a_pUnit
	void RemoveTarget( Toshi::TRef<AUnitProxy> a_pUnit );

	// $Barnyard: FUNCTION 00471550
	// NOTE: inlined in retail
	void SetMinDistance( TFLOAT a_fDistance )
	{
		m_fMinDistance = a_fDistance;
		m_fDistance    = Toshi::TMath::Max( m_fDistance, m_fMinDistance );
	}

	// $Barnyard: FUNCTION 00471590
	// NOTE: inlined in retail
	void SetMaxDistance( TFLOAT a_fDistance )
	{
		m_fMaxDistance = a_fDistance;
		m_fDistance    = Toshi::TMath::Min( m_fDistance, m_fMaxDistance );
	}

private:
	// Removes the target at a_iIndex, the next ones move down
	// NOTE: an instance of T2Vector<Target, MAX_NUM_TARGETS>::Erase in retail
	void RemoveTargetAt( TINT a_iIndex );

	// Moves the camera so every target is in view. With a_bIgnoreOptional, the optional targets are
	// ignored and the distance search starts halfway (used when the targets didn't fit the last frame)
	TBOOL UpdateCamera( TFLOAT a_fDeltaTime, TBOOL a_bIgnoreOptional );

	// Whether every target (but the optional ones with a_bIgnoreOptional) projects into the
	// middle 75% of the screen with the current camera
	TBOOL AreTargetsInView( TBOOL a_bIgnoreOptional );

	// NOTE: inlined in retail
	const Toshi::TVector4& GetTargetPosition( const Target& a_rcTarget ) const;

private:
	TINT            m_iNumTargets;                 // 0x10
	Target          m_aTargets[ MAX_NUM_TARGETS ]; // 0x14
	Toshi::TVector4 m_vViewDirection;              // 0xd4
	TFLOAT          m_fSpeed;                      // 0xe4, of the focus point and the distance
	TFLOAT          m_fMinDistance;                // 0xe8, of the camera from the targets
	TFLOAT          m_fMaxDistance;                // 0xec
	TFLOAT          m_fDistance;                   // 0xf0, current
};

TSTATICASSERT( sizeof( AMultiCameraHelper::Target ) == 0x18 );
TSTATICASSERT( sizeof( AMultiCameraHelper ) == 0xf4 );
