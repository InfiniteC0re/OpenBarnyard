#pragma once
#include "ACamera.h"
#include "ACameraSplinePath.h"

//-----------------------------------------------------------------------------
// Purpose: Moves a camera along a single spline segment from its current position
// to a destination (used by AConversationCameraHelper and AFollowCameraHelper).
//
// TClass 0x007823b8 (size 0x80, runtime creatable), vtable 0x0071ccac.
//-----------------------------------------------------------------------------
class ACameraHelperSpline
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACameraHelperSpline, Toshi::TObject );

public:
	ACameraHelperSpline();
	~ACameraHelperSpline();

	// Starts moving a_pCamera from its current position to a_rcDestination
	void Setup( ACamera* a_pCamera, const Toshi::TVector4& a_rcDestination, const Toshi::TVector4& a_rcDirection, TFLOAT a_fTangentScale, TFLOAT a_fSpeed );

	// Changes the destination while moving. Within a_fMaxDistance of the current end only the end of the segment
	// changes, otherwise the segment restarts from the current camera position.
	void SetDestination( const Toshi::TVector4& a_rcDestination, const Toshi::TVector4& a_rcEndTangent, const Toshi::TVector4& a_rcDirection, TFLOAT a_fMaxDistance );

	// Moves the camera along the segment. Returns TTRUE while the end isn't reached.
	TBOOL Update( TFLOAT a_fDeltaTime );

private:
	ACameraSplineSegment m_oSegment;        // 0x04
	Toshi::TVector4      m_vStartDirection; // 0x54
	Toshi::TVector4      m_vEndDirection;   // 0x64
	ACamera*             m_pCamera;         // 0x74
	TFLOAT               m_fProgress;       // 0x78
	TFLOAT               m_fSpeed;          // 0x7c
};

TSTATICASSERT( sizeof( ACameraHelperSpline ) == 0x80 );
