#pragma once
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Purpose: Camera of the pool minigame, moves between a list of views
// (ACameraManager::CAMERAHELPER_POOL).
//
// TClass 0x00782548 (size 0xb0), vtable 0x0071d018.
//-----------------------------------------------------------------------------
class APoolCamera
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( APoolCamera, ACameraHelper );

	static constexpr TINT MAX_NUM_VIEWS = 9;

public:
	APoolCamera();
	~APoolCamera();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                       // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                       // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void*, void* ) override { return TFALSE; } // +0x18, RETURNS_FALSE 00556a00
	virtual void  OnCreate() override {}                                          // +0x1c, DOES_NOTHING 005acd80
	virtual void  OnDestroy() override {}                                         // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override {}                                          // +0x24, DOES_NOTHING 005acd80

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void PoolCamera_Unknown() {} // +0x44, DOES_NOTHING 005acd80

	// Adds a view (the vectors are referenced, not copied)
	void AddView( const Toshi::TVector4* a_pPosition, const Toshi::TVector4* a_pLookAt, TINT a_iUserData );

	// Removes every view
	void ClearViews();

	// Makes a_iView the current view and jumps to it
	void SetView( TINT a_iView );

	// Makes a_iView the current view
	void SetCurrentView( TINT a_iView );

	TINT GetCurrentView() const;
	TINT GetCurrentViewUserData() const;

	void SetSpeed( TFLOAT a_fSpeed );

	// Direction the camera looks at
	Toshi::TVector4 GetDirection();

	// NOTE: folded with the other getters of offset 0xac (0041a410) in retail
	TBOOL IsMoving() const { return m_bMoving; }

private:
	const Toshi::TVector4* m_apViewPositions[ MAX_NUM_VIEWS ]; // 0x10
	const Toshi::TVector4* m_apViewLookAts[ MAX_NUM_VIEWS ];   // 0x34
	Toshi::TVector4        m_vPosition;                        // 0x58
	Toshi::TVector4        m_vLookAt;                          // 0x68
	TFLOAT                 m_fPositionSpeed;                   // 0x78
	TFLOAT                 m_fLookAtSpeed;                     // 0x7c
	TINT                   m_aiViewUserData[ MAX_NUM_VIEWS ];  // 0x80
	TINT                   m_iCurrentView;                     // 0xa4
	TINT                   m_iNumViews;                        // 0xa8
	TBOOL                  m_bMoving;                          // 0xac, the camera moved in the last update
};

TSTATICASSERT( sizeof( APoolCamera ) == 0xb0 );
