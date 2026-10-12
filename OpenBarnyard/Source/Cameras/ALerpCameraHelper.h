#pragma once
#include "ACameraHelper.h"

//-----------------------------------------------------------------------------
// Purpose: Smoothly moves the current camera towards a destination position
// while looking at a smoothly moving point.
//-----------------------------------------------------------------------------
class ALerpCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ALerpCameraHelper, ACameraHelper );

public:
	ALerpCameraHelper();
	~ALerpCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	// RETURNS_FALSE
	virtual TBOOL Unk2( void* ) OVERRIDE { return TFALSE; }
	virtual TBOOL ProcessInputCommand( void* a_pCommand, void* a_pEvent ) OVERRIDE;
	// DOES_NOTHING
	virtual void OnCreate() OVERRIDE {}
	// DOES_NOTHING
	virtual void OnDestroy() OVERRIDE {}
	// DOES_NOTHING
	virtual void OnAttach() OVERRIDE {}

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------

	// Immediately places the camera at a_rcPosition looking at a_rcLookAt
	void SetPosition( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcLookAt );

	// Sets where the camera should smoothly move to and look at
	void SetDestination( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcLookAt );

	// Returns TTRUE if the camera has reached the destination position
	TBOOL HasReachedDestination() const { return m_bReachedDestination; }

	virtual void LerpHelper_Unknown() {} // +0x44, DOES_NOTHING 005acd80

private:
	Toshi::TVector4 m_vDestPosition;       // 0x10
	Toshi::TVector4 m_vPosition;           // 0x20
	Toshi::TVector4 m_vDestLookAt;         // 0x30
	Toshi::TVector4 m_vLookAt;             // 0x40
	TUINT           m_uiUnk;               // 0x50
	TBOOL           m_bReachedDestination; // 0x54
};
