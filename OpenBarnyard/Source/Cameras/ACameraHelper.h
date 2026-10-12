#pragma once
#include "ACamera.h"
#include "Units/AUnitProxy.h"

class AUnit;

class ACameraHelper : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACameraHelper, Toshi::TObject );

public:
	ACameraHelper();
	~ACameraHelper();

	virtual void   UpdateCurrentCamera();                                      // +0x0c
	virtual TBOOL  OnUpdate( TFLOAT a_fDeltaTime )     = 0;                    // +0x10
	virtual TBOOL  Unk2( void* )                       = 0;                    // +0x14
	virtual TBOOL  ProcessInputCommand( void*, void* ) = 0;                    // +0x18
	virtual void   OnCreate()                          = 0;                    // +0x1c
	virtual void   OnDestroy()                         = 0;                    // +0x20
	virtual void   OnAttach()                          = 0;                    // +0x24
	virtual void   GetCameraRotation( Toshi::TVector4& a_rOutRotation ) const; // +0x28
	virtual void   ResetPosition( TBOOL a_bSnapCamera );                       // +0x2c, places the camera at its default position around the target
	virtual void   SetTarget( Toshi::TRef<AUnitProxy> a_pTarget );             // +0x30
	virtual void   SetTargetUnit( AUnit* a_pUnit );                            // +0x34
	virtual void   ResetTarget();                                              // +0x38
	virtual void   OnDetach();                                                 // +0x3c
	virtual TFLOAT GetFOV() const;                                             // +0x40

	// Returns TTRUE if the XZ distance between the positions is <= 1.6 and the Y distance is <= 2.5
	// NOTE: located among AGTAStyleCameraHelper's code in the original binary
	static TBOOL IsPositionInFadeRange( const Toshi::TVector4& a_rcCameraPos, const Toshi::TVector4& a_rcPosition );

	// Places the camera at a_rcPosition looking at a_rcTarget, unless it would look along the up axis of the camera
	void LookAt( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcTarget );

	// Makes the camera look along a_rcDirection, unless it would look along the up axis of the camera
	void LookAtDirection( const Toshi::TVector4& a_rcDirection );

	// Makes the camera look at a_rcPoint, unless it would look along the up axis of the camera
	void LookAtPoint( const Toshi::TVector4& a_rcPoint );

	// Moves a_rfOutValue from a_fCurrent towards a_fTarget (a_fSpeedDown while decreasing, a_fSpeedUp while increasing,
	// both scaled by the distance). Returns TTRUE once the target is reached.
	// NOTE: located among ACameraManager's code in the original binary
	static TBOOL MoveTowards( TFLOAT& a_rfOutValue, TFLOAT a_fTarget, TFLOAT a_fCurrent, TFLOAT a_fSpeedDown, TFLOAT a_fSpeedUp, TFLOAT a_fDeltaTime );

	ACamera* GetCamera() { return m_pCamera; }
	void     SetCamera( ACamera* a_pCamera ) { m_pCamera = a_pCamera; }

	AUnitProxy* GetTarget() { return m_pTarget; }

protected:
	Toshi::TRef<AUnitProxy> m_pTarget; // 0x4
	ACamera*                m_pCamera; // 0x8
	TBOOL                   m_bUnk;    // 0xc
};
