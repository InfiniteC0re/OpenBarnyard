#pragma once
#include "ACameraSplinePath.h"

class AUnit;

//-----------------------------------------------------------------------------
// Purpose: Optional spline path used by AGTAStyleCameraHelper (see SetUsePath). While active,
// it takes over the camera in a hardcoded region of the map, following one of two
// splines depending on the target's position.
//
// Owned by AGTAStyleCameraHelper (0x88 bytes, vtable 0x0071cba8 with only the deleting destructor).
// NOTE: the original has no TClass for it, the name is from the RTTI of the Wii build (vtable 8050ae30).
//-----------------------------------------------------------------------------
class ABarnStairCamera
{
public:
	ABarnStairCamera();
	virtual ~ABarnStairCamera();

	// Rebuilds both splines
	void Reset();

	// Moves the camera along the path while a_pUnit is in its region. Returns TTRUE.
	TBOOL Update( TFLOAT a_fDeltaTime, AUnit* a_pUnit );

	// NOTE: inlined in the ctor and in AGTAStyleCameraHelper::SetUsePath
	void SetActive( TBOOL a_bActive )
	{
		if ( m_bActive != a_bActive )
		{
			m_bActive = a_bActive;
			m_bOnPath = TFALSE;

			if ( a_bActive )
			{
				Reset();
				m_fProgress       = -1.0f;
				m_bBlending       = TFALSE;
				m_bReturning      = TFALSE;
				m_fProgressOffset = 0.0f;
				m_pCurrentCurve   = TNULL;
				m_fBlendTime      = 0.0f;
			}
		}
	}

	TBOOL IsActive() const { return m_bActive; }
	TBOOL IsOnPath() const { return m_bOnPath; }

private:
	// Blends the camera from where the path left it back to the GTA style camera
	void UpdateReturn( TFLOAT a_fDeltaTime, AUnit* a_pUnit );

private:
	ACameraSplinePath  m_oCurve1;         // 0x04, used while the target's x >= 15
	ACameraSplinePath  m_oCurve2;         // 0x18, m_oCurve1 mirrored around x = 15
	ACameraSplinePath* m_pCurrentCurve;   // 0x2c
	TBOOL              m_bActive;         // 0x30
	TFLOAT             m_fProgress;       // 0x34
	TBOOL              m_bOnPath;         // 0x38, the camera is being moved by the path
	TFLOAT             m_fProgressOffset; // 0x3c, keeps the camera a bit ahead/behind the target on the path
	Toshi::TVector4    m_vBlendPosition;  // 0x40
	Toshi::TVector4    m_vBlendDirection; // 0x50
	Toshi::TVector4    m_vPosition;       // 0x60
	Toshi::TVector4    m_vDirection;      // 0x70
	TBOOL              m_bBlending;       // 0x80, blending from the camera to the path
	TBOOL              m_bReturning;      // 0x81, blending from the path back to the camera (see UpdateReturn)
	TFLOAT             m_fBlendTime;      // 0x84
};

TSTATICASSERT( sizeof( ABarnStairCamera ) == 0x88 );
